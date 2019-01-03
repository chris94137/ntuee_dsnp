/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
	HashMap<HashKey, size_t> hash(_dfsList.size());
	for(size_t i = 0; i < _dfsList.size(); ++i)
	{
		size_t  mergeGate;
		HashKey k((size_t)sorted_list[_dfsList[i]]->getIn(0), (size_t)sorted_list[_dfsList[i]]->getIn(1), sorted_list[_dfsList[i]]->fanin_inv(0), sorted_list[_dfsList[i]]->fanin_inv(1));
		if(hash.query(k, mergeGate))
		{
			sorted_list[_dfsList[i]]->merge(sorted_list[mergeGate]);
			sorted_list[_dfsList[i]] = 0;
		}
		else hash.insert(k, _dfsList[i]);
	}
	updateDfsList();
}


void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
