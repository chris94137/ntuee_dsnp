/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "sstream"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
CirGate* CirMgr::getGate(unsigned gid) const
{
   for(size_t i = 0; i < gate_list.size(); ++i)
      if(gate_list[i]->getId() == gid) return gate_list[i];
   return 0;
}
bool
CirMgr::readCircuit(const string& fileName)
{
   fstream file;
   file.open(fileName, fstream::in);
   if(!file.is_open()) return false;
   char c;
   vector<string> wholeFile;
   wholeFile.push_back("");
   while(file.get(c))
      if(c == '\n') wholeFile.push_back("");
      else wholeFile.back().push_back(c);
   wholeFile.pop_back();

   cerr << "whole file : " << endl;
   for(size_t i = 0; i < wholeFile.size(); ++i) cerr << wholeFile[i] << endl;

   stringstream ss;
   ss << wholeFile[0];
   ++lineNo;
   string head;
   ss >> head;
   if(head == "aag") ss >> M >> I >> L >> O >> A;
   size_t current_value;
   // PI
   for(size_t i = 0; i < I; ++i, ++lineNo)
   {
      ss.str("");
      ss.clear();
      ss << wholeFile[i + 1];
      ss >> current_value;
      vector<size_t> v;
      v.push_back(current_value);
      value.push_back(v);
      gate_list.push_back(new PIGate(v[0] / 2, lineNo));
   }
   // LATCH
   for(size_t i = I; i < I + L; ++i, ++lineNo) {}
   // PO
   for(size_t i = I + L; i < I + L + O; ++i, ++lineNo)
   {
      ss.str("");
      ss.clear();
      ss << wholeFile[i + 1];
      ss >> current_value;
      vector<size_t> v;
      v.push_back(current_value);
      value.push_back(v);
      gate_list.push_back(new POGate(M + i - I + L + 1, lineNo));
   }
   //AIG
   for(size_t i = I + L + O; i < I + L + O + A; ++i, ++lineNo)
   {
      ss.str("");
      ss.clear();
      ss << wholeFile[i + 1];
      vector<size_t> v;
      for(int j = 0; j < 3; ++j)
      {
         ss >> current_value;
         v.push_back(current_value);
      }
      value.push_back(v);
      gate_list.push_back(new AIGGate(v[0] / 2, lineNo));
   }
   // CONST
   gate_list.push_back(new CONSTGate(0, 0));
   for(size_t i = 0; i < value.size(); ++i)
   {
      if(i == 0) cerr << "PI : " << endl;
      if(i == I) cerr << "LATCH : " << endl;
      if(i == I + L) cerr << "PO : " << endl;
      if(i == I + L + O) cerr << "AIG : " << endl;
      for(size_t j = 0; j < value[i].size(); ++j) cerr << value[i][j] << " ";
      cerr << endl;
   }
   for(size_t i = I + L + O; i < value.size(); ++i)
   {
      CirGate* aigGate = getGate(value[i][0] / 2);
      CirGate* inGate1 = getGate(value[i][1] / 2);
      CirGate* inGate2 = getGate(value[i][2] / 2);
      cerr << "aigGate : " << aigGate << endl;
      cerr << "inGate1 : " << inGate1 << endl;
      cerr << "inGate2 : " << inGate2 << endl;
      combine(value[i][1] % 2, value[i][0] % 2, inGate1, aigGate);
      combine(value[i][2] % 2, value[i][0] % 2, inGate2, aigGate);
      GateList outGate;
      for(size_t j = I + L; j < I + L + O; ++j)
      {
         if(value[j][0] == value[i][0])
         {
            outGate.push_back(getGate(M + j - I + L + 1));
            break;
         }
      }
      for(size_t j = 0; j < outGate.size(); ++j)
         cerr << "outGate : " << outGate[j] << endl;
      if(outGate.size()) for(size_t j = 0; j < outGate.size(); ++j) combine(value[i][0] % 2, false, aigGate, outGate[j]);
      else
      {
         aigGate->setOutGate(0);
         aigGate->setOutInv(false);
      }
   }
   return true;
}

void CirMgr::combine(bool inv_prev, bool inv_next, CirGate* prev, CirGate* next)
{
   next->setInGate(prev);
   prev->setOutGate(next);
   next->setInInv(inv_prev);
   prev->setOutInv(inv_next);
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   size_t PIcount = 0, POcount = 0, AIGcount = 0;
   for(size_t i = 0; i < gate_list.size(); i++)
   {
      switch(gate_list[i]->getTypeInt())
      {
         case 1: ++PIcount; break;
         case 2: ++POcount; break;
         case 3: ++AIGcount; break;
      }
   }
   cout << "\nCircuit Statistics\n==================\n";
   cout << setw(7) << left << "  PI" << setw(9) << right << PIcount << endl;
   cout << setw(7) << left << "  PO" << setw(9) << right << POcount << endl; 
   cout << setw(7) << left << "  AIG" << setw(9) << right << AIGcount << endl;
   cout << "------------------\n";
   cout << setw(7) << left << "  Total" << setw(9) << right << PIcount + POcount + AIGcount << endl;
}

void
CirMgr::printNetlist() const
{
   
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(size_t i = 0; i < I; ++i)
      cout << " " << gate_list[i]->getId(); 
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(size_t i = I + L; i < I + L + O; ++i)
      cout << " " << gate_list[i]->getId(); 
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   for(size_t i = I + L + O; i < I + L + O + A; ++i)
      if(gate_list[i]->size())
}

void
CirMgr::writeAag(ostream& outfile) const
{
}
