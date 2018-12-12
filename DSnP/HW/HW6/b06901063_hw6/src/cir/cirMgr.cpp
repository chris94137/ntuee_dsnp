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
//static char buf[1024];
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
CirMgr::CirMgr() { sorted_list.push_back(new CONSTGate(0, 0)); for(int i = 0; i < 5; ++i) header[i] = 0; }
CirMgr::~CirMgr() { delete sorted_list[0]; for(size_t i = 0; i < gate_list.size(); ++i) delete gate_list[i]; lineNo = 0; colNo = 0; }
bool
CirMgr::readCircuit(const string& fileName)
{
   fstream file;
   file.open(fileName, fstream::in);
   if(!file.is_open()) { cerr << "Cannot open design \"" << fileName << "\"!!" << endl; return false;}
   char c;
   vector<string> wholeFile;
   wholeFile.push_back("");
   while(file.get(c))
      if(c == '\n') wholeFile.push_back("");
      else wholeFile.back().push_back(c);
   wholeFile.pop_back();

   stringstream ss;
   string head;
   if(!wholeFile[0].size()) { errMsg = "aag"; return parseError(MISSING_IDENTIFIER); }
   // before "aag"
   size_t pos = wholeFile[0].find_first_not_of(' ');
   // cerr << "pos : " << pos << endl;
   if(pos != 0) return parseError(EXTRA_SPACE);
   pos = wholeFile[0].find_first_not_of(char(9));
   // cerr << "pos : " << pos << endl;
   if(pos != 0) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
   // "aag"
   pos = myStrGetTok(wholeFile[0], head);
   // cerr << "get head (" << head << ") from 0 to " << pos << endl;
   // cerr << "myStrNCmp : " << myStrNCmp(head, "aag", 3) << endl;
   if(myStrNCmp(head, "aag", 3) != 0) { errMsg = head; return parseError(ILLEGAL_IDENTIFIER); }
   else if(myStrNCmp(head, "aag", 3) == 0 && head.size() > 3)
   {
      string str = head.substr(3);
      int num;
      if(myStr2Int(str, num)) { colNo = 3; return parseError(MISSING_SPACE); }
      else { errMsg = head; return parseError(ILLEGAL_IDENTIFIER); }
   }
   colNo = pos;
   // MILOA
   colNo = 3;
   for(int i = 0; i < 5; ++i)
   {
      // cerr << pos << endl;
      size_t newPos = wholeFile[0].find_first_not_of(' ', pos);
      // cerr << "[" << i << "]pos : " << pos << ", newPos : " << newPos << endl;
      if(pos == string::npos) { errMsg = "number of variables"; return parseError(MISSING_NUM);  }
      if(pos + 1 == newPos) pos = newPos;
      else if(newPos == string::npos) { colNo++; errMsg = "number of variables"; return parseError(MISSING_NUM); }
      else if(pos == newPos) return parseError(MISSING_SPACE);
      else { ++colNo; return parseError(EXTRA_SPACE); }
      colNo++;
      newPos = wholeFile[0].find_first_not_of(char(9), pos);
      // cerr << "test char(9) newPos : " << newPos << endl;
      if(pos != newPos) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
      string str, str2;
      size_t pos1;
      pos1 = myStrGetTok(wholeFile[0], str, pos);
      myStrGetTok(wholeFile[0], str2, pos, char(9));
      if(myStr2Int(str, header[i])) pos = pos1;
      else if(myStr2Int(str2, header[i])) { ++colNo; return parseError(MISSING_SPACE); }
      else
      {
         switch(i)
         {
            case 0: errMsg = "number of variables"; break;
            case 1: errMsg = "number of PIs"; break;
            case 2: errMsg = "number of Latches"; break;
            case 3: errMsg = "number of POs"; break;
            case 4: errMsg = "number of AIGs"; break;
         }
         errMsg += "(" + str + ")";
         return parseError(ILLEGAL_NUM);
      }
      //if(!myStr2Int(str, header[i])) { ++colNo; return parseError(MISSING_SPACE); }
      colNo = pos;
   }
   if(pos != string::npos) return parseError(MISSING_NEWLINE);
   if(header[0] < header[1] + header[2] + header[4]) { errMsg = "Number of variables"; errInt = header[0]; return parseError(NUM_TOO_SMALL); }
   if(header[2] != 0) { errMsg = "latches"; return parseError(ILLEGAL_NUM); }
   colNo = 0;
   ++lineNo;
   // cout << "line " << lineNo << " done" << endl;
   sorted_list.resize(header[0] + header[3] + 1, 0);// plus const gate
   int current;
   // cerr << "PI" << endl;
   // PI
   for(int i = 0; i < header[1]; ++i)
   {
      colNo = 0;
      // before PI
      if(wholeFile.size() == lineNo) { errMsg = "PI"; return parseError(MISSING_DEF); }
      size_t PIpos = wholeFile[i + 1].find_first_not_of(' ');
      // cerr << "PIpos : " << PIpos << endl;
      if(PIpos == string::npos) { errMsg = "PI literal ID"; return parseError(MISSING_NUM); }
      if(PIpos != 0) return parseError(EXTRA_SPACE);
      PIpos = wholeFile[i + 1].find_first_not_of(char(9));
      // cerr << "PIpos : " << PIpos << endl;
      if(PIpos != 0) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
      // parse error
      string str;
      PIpos = myStrGetTok(wholeFile[i + 1], str, PIpos);
      if(!myStr2Int(str, current)) { colNo += str.size(); return parseError(MISSING_SPACE); }
      if(PIpos != string::npos) { colNo += str.size(); return parseError(MISSING_NEWLINE); }
      // cerr << "current : " << current << endl;
      // cerr << "line no : " << lineNo << endl;
      if(current == 0 || current == 1) { errInt = current; return parseError(REDEF_CONST); }
      if(current % 2) { errMsg = "PI"; errInt = current; return parseError(CANNOT_INVERTED); }
      if(current / 2 > header[0]) { errInt = current; return parseError(MAX_LIT_ID); }
      // cerr << "check redef" << endl;
      if(getGate(current / 2))
      {
         if(getGate(current / 2)->getTypeInt() != UNDEF_GATE)
         {
            errInt = current;
            errGate = getGate(current / 2);
            return parseError(REDEF_GATE);
         } 
      }
      // cerr << "check done" << endl;
      // for(size_t j = 0; j < defined.size(); ++j)
      //    if(current / 2 == defined[j])
      //    {
      //       errInt = current;
      //       errGate = gate_list[gate_list.size() - defined.size() + j];
      //       return parseError(REDEF_GATE);
      //    }
      ++lineNo;
      // init gate
      vector<size_t> v;
      v.push_back(current);
      value.push_back(v);
      gate_list.push_back(new PIGate(v[0] / 2, lineNo));
      sorted_list[gate_list.back()->getId()] = gate_list.back();
   }
   // LATCH
   for(int i = header[1]; i < header[1] + header[2]; ++i, ++lineNo) {}
   // PO
   // cerr << "PO" << endl;
   for(int i = header[1] + header[2]; i < header[1] + header[2] + header[3]; ++i)
   {
      colNo = 0;
      // before PO
      if(wholeFile.size() == lineNo) { errMsg = "PO"; return parseError(MISSING_DEF); }
      size_t POpos = wholeFile[i + 1].find_first_not_of(' ');
      // cerr << "POpos : " << POpos << endl;
      if(POpos == string::npos) { errMsg = "PO literal ID"; return parseError(MISSING_NUM); }
      if(POpos != 0) return parseError(EXTRA_SPACE);
      POpos = wholeFile[i + 1].find_first_not_of(char(9));
      // cerr << "POpos : " << POpos << endl;
      if(POpos != 0) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
      // parse error
      string str;
      POpos = myStrGetTok(wholeFile[i + 1], str, POpos);
      if(!myStr2Int(str, current)) { ++colNo; return parseError(MISSING_SPACE); }
      if(POpos != string::npos) return parseError(MISSING_NEWLINE);
      // cerr << "current : " << current << endl;
      // cerr << "line no : " << lineNo << endl;
      if(current / 2 > header[0]) { errInt = current; return parseError(MAX_LIT_ID); }
      ++lineNo;
      // init gate
      vector<size_t> v;
      v.push_back(header[0] + i - header[1] + header[2] + 1);
      v.push_back(current);
      value.push_back(v);
      gate_list.push_back(new POGate(header[0] + i - header[1] + header[2] + 1, lineNo));
      sorted_list[gate_list.back()->getId()] = gate_list.back();
   }
   //AIG
   // cerr << "AIG" << endl;
   for(int i = header[1] + header[2] + header[3]; i < header[1] + header[2] + header[3] + header[4]; ++i)
   {
      colNo = 0;
      // before PO
      vector<size_t> currents;
      if(wholeFile.size() == lineNo) { errMsg = "AIG"; return parseError(MISSING_DEF); }
      size_t AIGpos = wholeFile[i + 1].find_first_not_of(' ');
      // cerr << "AIGpos : " << AIGpos << endl;
      if(AIGpos != 0) return parseError(EXTRA_SPACE);
      AIGpos = wholeFile[i + 1].find_first_not_of(char(9));
      // cerr << "AIGpos : " << AIGpos << endl;
      if(AIGpos != 0) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
      // parse error
      string str;
      AIGpos = myStrGetTok(wholeFile[i + 1], str, AIGpos);
      if(AIGpos == string::npos) { ++colNo; return parseError(MISSING_SPACE); }
      myStr2Int(str, current);
      if(current == 0 || current == 1) { errInt = current; return parseError(REDEF_CONST); }
      if(current / 2 > header[0]) { errInt = current; return parseError(MAX_LIT_ID); }
      colNo = AIGpos;
      if(getGate(current / 2))
      {
         if(getGate(current / 2)->getTypeInt() != UNDEF_GATE)
         {
            errInt = current;
            errGate = getGate(current / 2);
            return parseError(REDEF_GATE);
         } 
      }
      currents.push_back(current);
      size_t newPos = wholeFile[i + 1].find_first_not_of(' ', AIGpos);
      // cerr << "[" << i << "]AIGpos : " << AIGpos << ", newPos : " << newPos << endl;
      colNo++;
      if(AIGpos + 1 == newPos) AIGpos = newPos;
      else { return parseError(EXTRA_SPACE); }
      newPos = wholeFile[0].find_first_not_of(char(9), pos);
      // cerr << "test char(9) newPos : " << newPos << endl;
      if(pos != newPos) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
      AIGpos = myStrGetTok(wholeFile[i + 1], str, AIGpos);
      if(AIGpos == string::npos) { ++colNo; return parseError(MISSING_SPACE); }
      myStr2Int(str, current);
      if(current / 2 > header[0]) { errInt = current; return parseError(MAX_LIT_ID); }
      colNo = AIGpos;
      currents.push_back(current); 
      // third num
      newPos = wholeFile[i + 1].find_first_not_of(' ', AIGpos);
      // cerr << "[" << i << "]AIGpos : " << AIGpos << ", newPos : " << newPos << endl;
      colNo++;
      if(AIGpos + 1 == newPos) AIGpos = newPos;
      else { return parseError(EXTRA_SPACE); }
      AIGpos = myStrGetTok(wholeFile[i + 1], str, AIGpos);
      colNo += str.size();
      if(AIGpos != string::npos) return parseError(MISSING_NEWLINE);
      myStr2Int(str, current);
      if(current / 2 > header[0]) { errInt = current; return parseError(MAX_LIT_ID); }
      colNo = AIGpos;
      currents.push_back(current);      
      ++lineNo;

      value.push_back(currents);
      gate_list.push_back(new AIGGate(currents[0] / 2, lineNo));
      sorted_list[gate_list.back()->getId()] = gate_list.back();
   }

   // sorted_list.resize(header[0] + header[3] + 1, 0);// plus const gate

   // for(size_t i = 0; i < gate_list.size(); ++i) { sorted_list[gate_list[i]->getId()] = gate_list[i]; }
   
   // get symbols
   vector<pair<char, int>> defined_sym;
   for(int i = header[1] + header[2] + header[3] + header[4] + 1; i < (int)wholeFile.size(); ++i)
   {
      colNo = 0;
      if(!wholeFile[i].size()) { errMsg = char(NULL); return parseError(ILLEGAL_SYMBOL_TYPE); }
      if(wholeFile[i][0] == 'c') 
      {
         ++colNo;
         if(wholeFile[i].size() > 1) return parseError(MISSING_NEWLINE);
         break;
      }
      // befor Symbol
      size_t Spos = wholeFile[i].find_first_not_of(' ');
      // cerr << "Spos : " << Spos << endl;
      if(Spos != 0) return parseError(EXTRA_SPACE);
      Spos = wholeFile[i].find_first_not_of(char(9));
      // cerr << "Spos : " << Spos << endl;
      if(Spos != 0) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
      // i/o
      if(wholeFile[i][0] != 'i' && wholeFile[i][0] != 'o') { errMsg = wholeFile[i][0]; return parseError(ILLEGAL_SYMBOL_TYPE); }
      ++Spos;
      ++colNo;
      // between i/o and num
      size_t newPos = wholeFile[i].find_first_not_of(' ', Spos);
      if(newPos != Spos) return parseError(EXTRA_SPACE);
      newPos = wholeFile[i].find_first_not_of(char(9), Spos);
      if(newPos != Spos) { errInt = 9; return parseError(ILLEGAL_WSPACE); }
      // get gate id
      string str, str2;
      int gate_num = 0;
      size_t pos1;
      pos1 = myStrGetTok(wholeFile[i], str, Spos);
      myStrGetTok(wholeFile[i], str2, Spos, char(9));
      if(myStr2Int(str, gate_num)) Spos = pos1;
      else if(myStr2Int(str2, gate_num)) { ++colNo; return parseError(MISSING_SPACE); }
      else { errMsg = "symbol index(" + str + ")"; return parseError(ILLEGAL_NUM); }
      if(wholeFile[i][0] == 'i') {if(gate_num > header[1]) { errMsg = "PI index"; errInt = gate_num; return parseError(NUM_TOO_BIG); }}
      else if(wholeFile[i][0] == 'o') if(gate_num > header[3]) { errMsg = "PO index"; errInt = gate_num; return parseError(NUM_TOO_BIG); }
      colNo += str.size();
      // symbol str
      if(Spos == string::npos) { errMsg = "symbolic name"; return parseError(MISSING_IDENTIFIER); }
      ++Spos;
      if(Spos == wholeFile[i].size()) { errMsg = "symbolic name"; return parseError(MISSING_IDENTIFIER); }
      newPos = wholeFile[i].find_first_not_of(' ', Spos);
      Spos = myStrGetTok(wholeFile[i], str, Spos);
      for(size_t j = 0; j < str.size(); ++j) if(!isprint(str[j])) { colNo += j + 1; errInt = int(str[j]); return parseError(ILLEGAL_SYMBOL_NAME); }
      colNo += str.size();
      // cerr << "gate_type: " << wholeFile[i][0] << endl;
      // cerr << "gate_num : " << gate_num << endl;
      // cerr << "sym      : " << str << endl;

      if(wholeFile[i][0] == 'i')
      {
         if(getGate(value[gate_num][0] / 2)->getSymbol().size())
         { errMsg = wholeFile[i][0]; errInt = gate_num; return parseError(REDEF_SYMBOLIC_NAME); }
      }
      else if(wholeFile[i][0] == 'o')
      {
         if(getGate(value[gate_num + header[1] + header[2]][0])->getSymbol().size())
         { errMsg = wholeFile[i][0]; errInt = gate_num; return parseError(REDEF_SYMBOLIC_NAME); }
      }
      // for(size_t j = 0; j < defined_sym.size(); ++j)
      // {
      //    // cerr << "defined_sym[" << j << "].first  : " << defined_sym[j].first << endl;
      //    // cerr << "defined_sym[" << j << "].second : " << defined_sym[j].second << endl;
      //    if(defined_sym[j].first == wholeFile[i][0] && defined_sym[j].second == gate_num) 
      //    { errMsg = wholeFile[i][0]; errInt = gate_num; return parseError(REDEF_SYMBOLIC_NAME); }
      // }
      // defined_sym.push_back(make_pair(wholeFile[i][0], gate_num));
      ++lineNo;

      if(wholeFile[i][0] == 'i') linkSymbol(true, gate_num, str);
      else if(wholeFile[i][0] == 'o') linkSymbol(false, gate_num, str);
   }
   // link gates
   for(int i = 0; i < header[0] + header[3] + 1; ++i)
   {
      if(sorted_list[i] == 0) continue;
      // cerr << "start linking (id : " << i << ") " << sorted_list[i]->getTypeStr() << endl;
      // cerr << "\tline no " << sorted_list[i]->getLineNo() << endl;
      if(sorted_list[i]->getTypeInt() == AIG_GATE) combineAIG(sorted_list[i]->getLineNo() - 2);
      else if(sorted_list[i]->getTypeInt() == PO_GATE) combinePO(sorted_list[i]->getLineNo() - 2);
   }
   return true;
}

void CirMgr::combineAIG(unsigned value_num)
{
   // cerr << "value num : " << value_num << endl;
   // for(size_t i = 0; i < value[value_num].size(); ++i) cerr << "value[" << value_num << "][" << i << "] " << value[value_num][i] << endl;
   CirGate* inGate1 = getGate(value[value_num][1] / 2);
   CirGate* inGate2 = getGate(value[value_num][2] / 2);
   if(!inGate1) { inGate1 = new UNDEFGate(value[value_num][1] / 2); gate_list.push_back(inGate1); sorted_list[value[value_num][1] / 2] = inGate1; }
   if(!inGate2) { inGate2 = new UNDEFGate(value[value_num][2] / 2); gate_list.push_back(inGate2); sorted_list[value[value_num][2] / 2] = inGate2; }
   sorted_list[value[value_num][0] / 2]->setInGate(inGate1, inGate2, value[value_num][1] % 2, value[value_num][2] % 2);
   inGate1->setOutGate(sorted_list[value[value_num][0] / 2], value[value_num][1] % 2);
   inGate2->setOutGate(sorted_list[value[value_num][0] / 2], value[value_num][2] % 2);
}

void CirMgr::combinePO(unsigned value_num)
{
   CirGate* aigGate = getGate(value[value_num][1] / 2);
   // cout << "aigGate : " << aigGate << endl;
   if(!aigGate) { aigGate = new UNDEFGate(value[value_num][1] / 2); gate_list.push_back(aigGate); sorted_list[value[value_num][1] / 2] = aigGate; }
   sorted_list[value[value_num][0]]->setInGate(aigGate, 0,  value[value_num][1] % 2, false);
   aigGate->setOutGate(sorted_list[value[value_num][0]], value[value_num][1] % 2);
}

void CirMgr::linkSymbol(bool in, size_t gate_num, string sym)
{
   CirGate* control_gate;;
   if(in) control_gate = getGate(value[gate_num][0] / 2);
   else control_gate = getGate(value[gate_num + header[1] + header[2]][0]);
   control_gate->setSymbol(sym);
   // cerr << "set symbol " << sym << " to " << control_gate->getTypeStr() << " gate [" << control_gate->getId() << "]" << endl;
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
   CirGate::resetMark();
   size_t num = 0;
   cout << endl;
   for(int i = header[1] + header[2]; i < header[1] + header[2] + header[3]; ++i)
      gate_list[i]->dfs(num);
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(int i = 0; i < header[1]; ++i)
      cout << " " << gate_list[i]->getId(); 
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(int i = header[1] + header[2]; i < header[1] + header[2] + header[3]; ++i)
      cout << " " << gate_list[i]->getId(); 
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   GateList floating;
   GateList unused;
   for(int i = 0; i < header[0] + header[3] + 1; ++i)
   {
      if(sorted_list[i] == 0) continue;
      if(sorted_list[i]->getTypeInt() == AIG_GATE || sorted_list[i]->getTypeInt() == PO_GATE) if(sorted_list[i]->has_floating_fanin()) floating.push_back(sorted_list[i]);
      if(sorted_list[i]->getTypeInt() == AIG_GATE || sorted_list[i]->getTypeInt() == PI_GATE) if(sorted_list[i]->is_unused()) unused.push_back(sorted_list[i]);
   }
   if(floating.size())
   {
      cout << "Gates with floating fanin(s):";
      for(size_t i = 0; i < floating.size(); ++i) cout << " " << floating[i]->getId();
      cout << endl;
   }
   if(unused.size())
   {
      cout << "Gates defined but not used  :";
      for(size_t i = 0; i < unused.size(); ++i) cout << " " << unused[i]->getId();
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   // header
   int cnt = 0;
   for(int i = 0; i < header[0] + header[3] + 1; ++i)
   {
      if(sorted_list[i] == 0) continue;
      if(sorted_list[i]->getTypeInt() == AIG_GATE && !sorted_list[i]->is_unused()) ++cnt;
   }
   outfile << "aag " << header[0] << " " << header[1] << " " << header[2] << " " << header[3] << " " << cnt << endl;
   cout << "aag " << header[0] << " " << header[1] << " " << header[2] << " " << header[3] << " " << cnt << endl;
   // PI and PO
   for(int i = 0; i < header[1] + header[2] + header[3]; ++i)
   {
      outfile << value[i].back();
      outfile << endl;
      cout << value[i].back();
      cout << endl;
   }
   // AIG gate
   IdList idList;
   CirGate::resetMark();
   for(int i = header[1] + header[2]; i < header[1] + header[2] + header[3]; ++i)
      gate_list[i]->dfs(idList);
   for(size_t i = 0; i < idList.size(); ++i)
   {
      for(size_t j = 0; j < value[sorted_list[idList[i]]->getLineNo() - 2].size(); ++j)
      {
         if(j != 0) { outfile << " "; cout << " "; }
         outfile << value[sorted_list[idList[i]]->getLineNo() - 2][j];
         cout << value[sorted_list[idList[i]]->getLineNo() - 2][j];
      }
      outfile << endl;
      cout << endl;
   }
   //symbol
   for(int i = 0; i < header[1]; ++i)
      if(gate_list[i]->getSymbol().size())
         outfile << "i" << i << " " << gate_list[i]->getSymbol() << endl;
   for(int i = header[1] + header[2]; i < header[1] + header[2] + header[3]; ++i)
      if(gate_list[i]->getSymbol().size())
         outfile << "o" << i - header[1] - header[2] << " " << gate_list[i]->getSymbol() << endl;
   outfile << "c\nAAG output by Shih-Hao Huang" << endl; 
}
