/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
   CirGate() {}
	CirGate(string t, int ti, size_t id, unsigned l) : _type(t), _typeInt(ti), _id(id), _line(l){ }
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return _type; }
   unsigned getLineNo() const { return _line; }
	size_t getId() const { return _id; }
	int getTypeInt() const { return _typeInt; }

   // Basic setting methods
   void setInGate(CirGate* gate) { _fanin.push_back(gate); }
   void setOutGate(CirGate* gate) { _fanout.push_back(gate); }
   void setInInv(bool inv) { _faninInv.push_back(inv); }
   void setOutInv(bool inv) { _fanoutInv.push_back(inv); }

   // dfs functions
   void dfs(size_t num = 0) const;

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

private:
protected:
	const string _type;
	int _typeInt;
	GateList _fanin, _fanout;
	vector<bool> _faninInv, _fanoutInv;
	size_t _faninSize, _fanoutSize;
	size_t _id;
	unsigned _line;
	bool value;
};

class PIGate : public CirGate
{
public:
	PIGate() { }
	PIGate(size_t id, unsigned l) : CirGate("PI", 1, id, l) { }
	~PIGate() { }
	void printGate() const { }
private:
};

class POGate : public CirGate
{
public:
	POGate() { }
	POGate(size_t id, unsigned l) : CirGate("PO", 2, id, l) { }
	~POGate() {}
	void printGate() const { }
private:
};

class AIGGate : public CirGate
{
public:
	AIGGate() { }
	AIGGate(size_t id, unsigned l) : CirGate("AIG", 3, id, l) { }
	~AIGGate() {}
	void printGate() const { }
private:
};

class UNDEFGate : public CirGate
{
public:
	UNDEFGate() { }
	UNDEFGate(size_t id, unsigned l) : CirGate("UNDEF", 0, id, l) { }
	~UNDEFGate() {}
	void printGate() const { }
private:
};

class CONSTGate : public CirGate
{
public:
	CONSTGate() { }
	CONSTGate(size_t id, unsigned l) : CirGate("CONST", 4, id, l) { }
	~CONSTGate() {}
	void printGate() const { }
private:
};

#endif // CIR_GATE_H
