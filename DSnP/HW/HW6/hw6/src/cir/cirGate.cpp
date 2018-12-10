/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
int CirGate::_state = 1;
void
CirGate::dfs(size_t& num) const
{
   if(getTypeInt() == UNDEF_GATE) return;
   for(int i = 0; i < 2; ++i) { if(!getIn(i)) continue; if(!getIn(i)->is_marked()) getIn(i)->dfs(num); }
   cout << "[" << num << "] " << setw(3) << left << getTypeStr() << " " << getId();
   if(getTypeInt() == AIG_GATE || getTypeInt() == PO_GATE)
   {
      for(int i = 0; i < 2; ++i)
      {
         if(!getIn(i)) continue; 
         cout << " ";
         if(getIn(i)->getTypeInt() == UNDEF_GATE) cout << "*";
         else
         {
            if(fanin_inv(i)) cout << "!";
            cout << getIn(i)->getId();
         }
      }
   }
   if(getSymbol().size()) cout << " (" << getSymbol() << ")";
   cout << endl;
   mark();
   ++num;
}
void
CirGate::dfs(bool in, int level, int searched_level) const
{
   cout << getTypeStr() << " " << getId();
   if(is_marked() && searched_level != level) { cout << " (*)" << endl; return; }
   else cout << endl;
   mark();
   if(searched_level == level) return;
   
   if(in) 
   {
      for(int i = 0; i < 2; ++i)
      {
         if(!getIn(i)) continue; 
         for(int j = 0; j < searched_level + 1; ++j) cout << "  ";
         if(fanin_inv(i)) cout << "!";
         getIn(i)->dfs(in, level, searched_level + 1);
      }
   }
   else
   {
      for(size_t i = 0; i < _fanout.size(); ++i)
      {
         for(int j = 0; j < searched_level + 1; ++j) cout << "  ";
         if(fanout_inv(i)) cout << "!";
         getOut(i)->dfs(in, level, searched_level + 1);
      }
   }
}

void
CirGate::dfs(IdList& idList) const
{
   if(getTypeInt() == UNDEF_GATE) return;
   for(int i = 0; i < 2; ++i) { if(!getIn(i)) continue; if(!getIn(i)->is_marked()) getIn(i)->dfs(idList); }
   if(getTypeInt() == AIG_GATE) idList.push_back(getId());
   mark();
}

void
CirGate::reportGate() const
{
   cout << "==================================================\n= ";
   stringstream ss;
   ss << getTypeStr() << "(" << getId() << ")";
   if(getSymbol().size()) ss << "\"" << getSymbol() << "\"";
   ss<< ", line " << getLineNo();
   string str = ss.str();
   str.resize(47, ' ');
   str.push_back('=');
   cout << str << endl;
   cout << "==================================================\n";
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   resetMark();
   dfs(true, level);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   resetMark();
   dfs(false, level);
}

/**************************************/
/*    class PIGate member functions   */
/**************************************/



/**************************************/
/*    class POGate member functions   */
/**************************************/



/**************************************/
/*   class AIGGate member functions   */
/**************************************/



/**************************************/
/*  class UNDEFGate member functions  */
/**************************************/



/**************************************/
/*  class CONSTGate member functions  */
/**************************************/