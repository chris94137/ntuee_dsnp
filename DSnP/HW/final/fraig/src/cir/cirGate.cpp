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

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

bool out_comp(CirGate* & a, CirGate* & b)
{
	if(a->getId() <= b->getId()) return true;
	else return false;
}

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::merge(CirGate* gate)
{
   for(size_t i = 0; i < _fanout.size(); ++i)
   {
      gate->setOutGate(getOut(i), fanout_inv(i));
      getOut(i)->setInGate(getOut(i)->findIn(getId()), gate, fanout_inv(i));
   }
   _fanout.clear();
   cout << "Strashing: " << gate->getId() << " merging " << getId() << "..." << endl;
}

int CirGate::_state = 1;
void
CirGate::dfs(size_t& num) const
{
   if(getTypeInt() == UNDEF_GATE) return;
   for(int i = 0; i < 2; ++i) { if(!getIn(i)) continue; if(!getIn(i)->is_marked()) getIn(i)->dfs(num); }
   cout << "[" << num << "] " << setw(4) << left << getTypeStr() << getId();
   if(getTypeInt() == AIG_GATE || getTypeInt() == PO_GATE)
   {
      for(int i = 0; i < 2; ++i)
      {
         if(!getIn(i)) continue; 
         cout << " ";
         if(getIn(i)->getTypeInt() == UNDEF_GATE) cout << "*";
         if(fanin_inv(i)) cout << "!";
         cout << getIn(i)->getId();
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
   // if(is_marked() && searched_level != level && getTypeInt() != PI_GATE && getTypeInt() != CONST_GATE) { cout << " (*)" << endl; return; }
   // else cout << endl;
   int count = 0;
   bool no_dfs = false;
   if(in)
      for(int i = 0; i < 2; ++i)
      {
         if(!getIn(i)) continue;
         ++count;
      }
   else count = _fanout.size();
   if(!count) no_dfs = true;
   if((!no_dfs) && is_marked() && getTypeInt() == AIG_GATE && searched_level != level) { cout << " (*)" << endl; return; }
   else cout << endl;

   if(searched_level == level) return;
   mark();
   
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

