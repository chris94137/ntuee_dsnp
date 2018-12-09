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
void
CirGate::dfs(size_t num) const
{
   if(this->getTypeInt() == UNDEF_GATE) return;
   for(size_t i = 0; i < this->_fanin.size(); ++i) _fanin[i]->dfs(num);
   cout << "[" << num << "] " << setw(3) << left << this->getTypeStr() << " " << this->getId();
   if(this->getTypeInt() == AIG_GATE) for(size_t i = 0; i < this->_fanin.size(); ++i) cout << " " << _fanin[i]->getId();
   cout << endl;
}


void
CirGate::reportGate() const
{
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
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