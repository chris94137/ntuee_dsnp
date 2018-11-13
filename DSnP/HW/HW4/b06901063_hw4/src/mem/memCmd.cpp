/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   if(!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;
   bool is_array = false;
   int array_size = -1;
   int num_objects = -1;
   for(size_t i = 0; i < tokens.size(); i++)
   {
      if(myStrNCmp("-Array", tokens[i], 2) == 0)
         if(!is_array)
         {
            is_array = true;
            if(i + 1 == tokens.size()) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i]);
            if(!myStr2Int(tokens[i + 1], array_size) || array_size <= 0)
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i + 1]);
            i++;
         }
         else return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
      else if(num_objects == -1)
      {
         if(!myStr2Int(tokens[i], num_objects) || num_objects <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
      }
      else return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
   }
   if(num_objects == -1) return CmdExec::errorOption(CMD_OPT_MISSING, "");
   if(is_array)
   {
      try
      {
         mtest.newArrs(num_objects, array_size);
      }
      catch(std::bad_alloc)
      {
         return CMD_EXEC_ERROR;
      }
      #ifdef MEM_DEBUG
      cout << "new array [" << array_size << "] : " << num_objects << endl;
      #endif // MEM_DEBUG
   }
   else
   {
      mtest.newObjs(num_objects);
      #ifdef MEM_DEBUG
      cout << "new objects : " << num_objects << endl;
      #endif // MEM_DEBUG
   }
   // Use try-catch to catch the bad_alloc exception
   return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   vector<string> tokens;
   if(!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;
   int value = -1;
   int option_ind = -1;
   bool use_rand = false;
   bool is_array = false;
   for(size_t i = 0; i < tokens.size(); i++)
   {
      if(myStrNCmp("-Array", tokens[i], 2) == 0)
         if(!is_array) is_array = true;
         else return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
      else if(value != -1)
      {
         if(myStrNCmp("-Index", tokens[i], 2) == 0 || myStrNCmp("-Random", tokens[i], 2) == 0)
            return CmdExec::errorOption(CMD_OPT_EXTRA, tokens[i]);
      }
      else if(value == -1)
      {
         if(myStrNCmp("-Index", tokens[i], 2) == 0)
         {
            option_ind = i;
            if(i + 1 == tokens.size()) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i]);
            if(!myStr2Int(tokens[i + 1], value) || value < 0)
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i + 1]);
         }
         else if(myStrNCmp("-Random", tokens[i], 2) == 0)
         {
            option_ind = i;
            use_rand = true;
            if(i + 1 == tokens.size()) return CmdExec::errorOption(CMD_OPT_MISSING, tokens[i]);
            if(!myStr2Int(tokens[i + 1], value) || value <= 0)
               return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i + 1]);
         }
         else return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[i]);
      }
   }
   if(value == -1) return CmdExec::errorOption(CMD_OPT_MISSING, "");
   if(!use_rand)
   {
      if(is_array)
      {
         if(value >= (int)mtest.getArrListSize())
         {
            cerr << "Size of array list (" << mtest.getArrListSize() << ") is <= " << value << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[option_ind + 1]);
         }
         mtest.deleteArr(value);
      }
      else
      {
         if(value >= (int)mtest.getObjListSize())
         {
            cerr << "Size of object list (" << mtest.getObjListSize() << ") is <= " << value << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[option_ind + 1]);
         }
         mtest.deleteObj(value);
      }
   }
   else
   {
      if(is_array)
      {
         if(!mtest.getArrListSize())
         {
            cerr << "Size of array list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[option_ind]);
         }
         const int range = mtest.getArrListSize();
         for(int i = 0; i < value; i++)
         {
            int obj_id = rnGen(range);
            mtest.deleteArr(obj_id);
         }
      }
      else
      {
         if(!mtest.getObjListSize())
         {
            cerr << "Size of object list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, tokens[option_ind]);
         }
         const int range = mtest.getObjListSize();
         for(int i = 0; i < value; i++)
         {
            int obj_id = rnGen(range);
            mtest.deleteObj(obj_id);
         }

      }
   }
   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


