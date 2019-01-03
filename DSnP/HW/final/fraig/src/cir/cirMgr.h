/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr();
   ~CirMgr();

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { if(gid >= sorted_list.size()) return 0; return sorted_list[gid]; }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void updateDfsList();
   void combineAIG(unsigned value_num);
   void combinePO(unsigned value_num);
   void linkSymbol(bool in, size_t gate_num, string sym);

   // Member functions about circuit optimization
   void sweep();
   void removeGate(size_t i);
   void optimize();
   void reduce2fanin(int num, size_t id);
   void reduce2Const(size_t id);

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   ofstream           *_simLog;
   int header[5];
	size_t M, I, L, O, A;
	vector<vector<size_t>> value;
	GateList gate_list, sorted_list;
   IdList _dfsList;
};

#endif // CIR_MGR_H
