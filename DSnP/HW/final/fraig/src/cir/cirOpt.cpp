/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed

void
CirMgr::sweep()
{
   IdList dfsList;
   for(size_t i = 0; i < _dfsList.size(); ++i) dfsList.push_back(_dfsList[i]);
   sort(dfsList.begin(), dfsList.begin() + dfsList.size());
   for(size_t i = 0, j = 0; i < sorted_list.size(); ++i)
   {
      if(sorted_list[i] == 0) continue;
      if(j != dfsList.size()) if(i == dfsList[j]) { ++j; continue; }
      removeGate(i);
   }

}

void
CirMgr::removeGate(size_t id)
{
   int type = sorted_list[id]->getTypeInt();
   if(type == AIG_GATE || type == UNDEF_GATE)
   {
      if(type == AIG_GATE)
      {
         CirGate* inGate1 = sorted_list[id]->getIn(0);
         CirGate* inGate2 = sorted_list[id]->getIn(1);
         inGate1->removeOut(id);
         inGate2->removeOut(id);
         cout << "Sweeping: AIG(" << id << ") removed..." << endl;
      }
      else cout << "Sweeping: UNDEF(" << id << ") removed..." << endl;
      sorted_list[id] = 0;
   }
}
/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...

void
CirMgr::optimize()
{
   // cerr << "dfs list : ";
   // for(size_t i = 0; i < _dfsList.size(); ++i) cerr << _dfsList[i] << " ";
   // cerr << endl;
   for(size_t i = 0; i < _dfsList.size(); ++i)
   {
      // cerr << "id : " << _dfsList[i] << endl;
      // cerr << sorted_list[_dfsList[i]]->getIn(0)->getId() << " (inv : " << sorted_list[_dfsList[i]]->fanin_inv(0) << ")" << endl;
      // cerr << sorted_list[_dfsList[i]]->getIn(1)->getId() << " (inv : " << sorted_list[_dfsList[i]]->fanin_inv(1) << ")" << endl;
      // cerr << "==========================================================================================================" << endl;
      if(sorted_list[_dfsList[i]]->getTypeInt() != AIG_GATE) continue;
      if(sorted_list[_dfsList[i]]->getIn(0) == sorted_list[_dfsList[i]]->getIn(1))
      {
         // cerr << "find same input (id : " << _dfsList[i] << ")" << endl;
         if(sorted_list[_dfsList[i]]->fanin_inv(0) == sorted_list[_dfsList[i]]->fanin_inv(1)) reduce2fanin(0, _dfsList[i]);
         else reduce2Const(_dfsList[i]);
         continue;
      }
      if(sorted_list[_dfsList[i]]->getIn(0)->getTypeInt() == CONST_GATE)
      {
         // cerr << "find const input1 (id : " << _dfsList[i] << ")" << endl;
         if(sorted_list[_dfsList[i]]->fanin_inv(0)) reduce2fanin(1, _dfsList[i]);
         else reduce2Const(_dfsList[i]);
      }
      else if(sorted_list[_dfsList[i]]->getIn(1)->getTypeInt() == CONST_GATE)
      {
         // cerr << "find const input2 (id : " << _dfsList[i] << ")" << endl;
         if(sorted_list[_dfsList[i]]->fanin_inv(1)) reduce2fanin(0, _dfsList[i]);
         else reduce2Const(_dfsList[i]);
      }
   }

   updateDfsList();
}

void
CirMgr::reduce2fanin(int num, size_t id)
{
   CirGate* InGate = (sorted_list[id]->getIn(0)->getId() != 0)? sorted_list[id]->getIn(0) : sorted_list[id]->getIn(1);
   bool inv = sorted_list[id]->fanin_inv(num);
   GateList outList;
   vector<bool> out_inv;
   for(size_t i = 0; i < sorted_list[id]->getOutSize(); ++i)
   {
      outList.push_back(sorted_list[id]->getOut(i));
      out_inv.push_back(sorted_list[id]->fanout_inv(i));
   }

   InGate->removeOut(id);
   for(size_t i = 0; i < outList.size(); ++i)
   {
      // cerr << "id " << outList[i]->getId() << " inv : " << out_inv[i] << endl;
      outList[i]->setInGate(outList[i]->findIn(id), InGate, (out_inv[i] == inv)? false : true) ;
      InGate->setOutGate(outList[i], (out_inv[i] == inv)? false : true);
   }
   sorted_list[id] = 0;
   // cerr << "change id " << id << " to id " << InGate->getId() << endl;
   // cerr << "==================================================" << endl;
   cout << "Simplifying: " << InGate->getId() << " merging ";
   if(inv) cout << "!";
   cout << id << "..." << endl;
}

void
CirMgr::reduce2Const(size_t id)
{
   CirGate* InGate1 = sorted_list[id]->getIn(0);
   CirGate* InGate2 = sorted_list[id]->getIn(1);
   // cerr << "inGate1 : " << sorted_list[id]->getIn(0)-> getId() << " (inv : " << sorted_list[id]->fanin_inv(0) << ")" << endl;
   // cerr << "inGate2 : " << sorted_list[id]->getIn(1)-> getId() << " (inv : " << sorted_list[id]->fanin_inv(1) << ")" << endl;
   GateList outList;
   vector<bool> out_inv;
   for(size_t i = 0; i < sorted_list[id]->getOutSize(); ++i)
   {
      outList.push_back(sorted_list[id]->getOut(i));
      out_inv.push_back(sorted_list[id]->fanout_inv(i));
   }
   // cerr << "id " << id << " out list : " << endl;
   // for(size_t i = 0; i < outList.size(); ++i) cerr << "\t" << outList[i]->getId() << endl;
   InGate1->removeOut(id);
   InGate2->removeOut(id);
   for(size_t i = 0; i < outList.size(); ++i)
   {
      // cerr << "origin inGate of gate id " << outList[i]->getId() << endl;
      // for(size_t j = 0; j < 2; ++j) cerr << outList[i]->getIn(j)->getId() << endl;
      // cerr << endl;
      // cerr << "id " << outList[i]->getId() << " set inGate " << outList[i]->findIn(id) << " to const " << inv << endl;
      outList[i]->setInGate(outList[i]->findIn(id), sorted_list[0], out_inv[i]);
      sorted_list[0]->setOutGate(outList[i], out_inv[i]);
      // cerr << "after set inGate of gate id " << outList[i]->getId() << endl;
      // for(size_t j = 0; j < 2; ++j) cerr << outList[i]->getIn(j)->getId() << " (inv : " << outList[i]->getIn(j)->fanin_inv(j) << " )" << endl;
      // cerr << endl;
   }
   sorted_list[id] = 0;
   // cerr << "change id " << id << " to const " << inv << endl;
   // cerr << "==================================================" << endl;
   cout << "Simplifying: 0 merging ";
   cout << id << "..." << endl;
}

