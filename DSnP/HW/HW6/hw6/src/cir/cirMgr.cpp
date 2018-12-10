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
CirMgr::CirMgr() { sorted_list.push_back(new CONSTGate(0, 0)); cerr << "size of CirGate : " << sizeof(CirGate) << endl; cerr << "size of CirMgr : " << sizeof(CirMgr) << endl;}
CirMgr::~CirMgr() { delete sorted_list[0]; for(size_t i = 0; i < gate_list.size(); ++i) delete gate_list[i]; lineNo = 0; colNo = 0; }
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

   // cerr << "whole file : " << endl;
   // for(size_t i = 0; i < wholeFile.size(); ++i) cerr << wholeFile[i] << endl;

   stringstream ss;
   ss << wholeFile[0];
   ++lineNo;
   string head;
   ss >> head;
   if(head == "aag") ss >> M >> I >> L >> O >> A;
   ++lineNo;
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
      v.push_back(M + i - I + L + 1);
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

   sorted_list.resize(M + O + 1, 0);// plus const gate

   for(size_t i = 0; i < gate_list.size(); ++i) { sorted_list[gate_list[i]->getId()] = gate_list[i]; }
   
   // // for debug
   // cerr << "debug start" << endl;
   // for(size_t i = 0; i < value.size(); ++i)
   // {
   //    if(i == 0) cerr << "PI : " << endl;
   //    if(i == I) cerr << "LATCH : " << endl;
   //    if(i == I + L) cerr << "PO : " << endl;
   //    if(i == I + L + O) cerr << "AIG : " << endl;
   //    for(size_t j = 0; j < value[i].size(); ++j) cerr << value[i][j] << " ";
   //    cerr << endl;
   // }
   // for(size_t i = 0; i < sorted_list.size(); ++i)
   // {
   //    cerr << "id[" << i << "] : ";
   //    if(sorted_list[i]) cerr << sorted_list[i]->getTypeStr();
   //    else cerr << 0;
   //    cerr << endl;
   // }
   // cerr << "debug end" << endl;
   // //
   
   // get symbols
   for(size_t i = I + L + O + A + 1; i < wholeFile.size(); ++i)
   {
      size_t space_pos = wholeFile[i].find_first_of(' ', 0);
      ss.str("");
      ss.clear();
      ss << wholeFile[i].substr(1, space_pos - 1);
      size_t gate_num = 0;
      ss >> gate_num;
      string sym = wholeFile[i].substr(space_pos + 1);
      // cerr << "gate_num : " << gate_num << endl;
      // cerr << "sym      : " << sym << endl;
      if(wholeFile[i][0] == 'i') linkSymbol(true, gate_num, sym);
      else if(wholeFile[i][0] == 'o') linkSymbol(false, gate_num, sym);
      else if(wholeFile[i][0] == 'c') break;
      else break;
   }
   // link gates
   for(size_t i = 0; i < M + O + 1; ++i)
   {
      if(sorted_list[i] == 0) continue;
      // cerr << "start linking " << sorted_list[i]->getTypeStr() << endl;
      // cerr << "line no " << sorted_list[i]->getLineNo() << endl;
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
   else control_gate = getGate(value[gate_num + I + L][0]);
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
   for(size_t i = I + L; i < I + L + O; ++i)
      gate_list[i]->dfs(num);
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
   GateList floating;
   GateList unused;
   for(size_t i = 0; i < M + O + 1; ++i)
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
   for(size_t i = 0; i < M + O + 1; ++i)
   {
      if(sorted_list[i] == 0) continue;
      if(sorted_list[i]->getTypeInt() == AIG_GATE && !sorted_list[i]->is_unused()) ++cnt;
   }
   outfile << "aag " << M << " " << I << " " << L << " " << O << " " << cnt << endl;
   cout << "aag " << M << " " << I << " " << L << " " << O << " " << cnt << endl;
   // PI and PO
   for(size_t i = 0; i < I + L + O; ++i)
   {
      outfile << value[i].back();
      outfile << endl;
      cout << value[i].back();
      cout << endl;
   }
   // AIG gate
   IdList idList;
   CirGate::resetMark();
   for(size_t i = I + L; i < I + L + O; ++i)
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
   for(size_t i = 0; i < I; ++i)
      if(gate_list[i]->getSymbol().size())
         outfile << "i" << i << " " << gate_list[i]->getSymbol() << endl;
   for(size_t i = I + L; i < I + L + O; ++i)
      if(gate_list[i]->getSymbol().size())
         outfile << "o" << i - I - L << " " << gate_list[i]->getSymbol() << endl;
   outfile << "c\nAAG output by Shih-Hao Huang" << endl; 
}
