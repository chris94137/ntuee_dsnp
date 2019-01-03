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
#include "sat.h"

#include <algorithm>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

bool out_comp(CirGate* & a, CirGate* & b);

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   CirGate() {}
	CirGate(int ti, size_t id, unsigned l) : _typeInt(ti), _fanin1(0), _fanin2(0), _id(id), _line(l), _mark(0), _symbol(0) { }
	CirGate(int ti, size_t id) : _typeInt(ti), _fanin1(0), _fanin2(0), _id(id), _mark(0), _symbol(0) { }
   virtual ~CirGate() { if(_symbol) delete[] _symbol; }

   // Basic access methods
   string getTypeStr() const
	{
		switch(_typeInt)
		{
			case UNDEF_GATE: return "UNDEF";
			case PI_GATE: return "PI";
			case PO_GATE: return "PO";
			case AIG_GATE: return "AIG";
			case CONST_GATE: return "CONST";
			default: return "";
		}
	}
   unsigned getLineNo() const { return _line; }
	unsigned getId() const { return _id; }
	int getTypeInt() const { return _typeInt; }
	string getSymbol() const { if(!_symbol) return ""; return string(_symbol); }
	CirGate* getIn(int num) const { return (!num)? (CirGate*)(size_t(_fanin1) & (~1)) : (CirGate*)(size_t(_fanin2) & (~1)); }
	CirGate* getOut(int num) const { return (CirGate*)(size_t(_fanout[num]) & (~1)); }
   void removeOut(size_t id) { for(size_t i = 0; i < _fanout.size(); ++i) if(getOut(i)->getId() == id) { _fanout.erase(_fanout.begin() + i); return; } }
   size_t getOutSize() const { return _fanout.size(); }
   int findIn(size_t id) const { for(int i = 0; i < 2; ++i) if(getIn(i)->getId() == id) return i; return -1; }
   int findOut(size_t id) const { for(size_t i = 0; i < _fanout.size(); ++i) if(getOut(i)->getId() == id) return i; return -1;}

   // Basic setting methods
	void setInGate(CirGate* gate1, CirGate* gate2, bool inv1, bool inv2)
	{
		if(inv1) _fanin1 = (CirGate*)(size_t(gate1) | 1);
		else _fanin1 = gate1;
		if(inv2) _fanin2 = (CirGate*)(size_t(gate2) | 1);
		else _fanin2 = gate2;
	}
   void setInGate(int num, CirGate* gate, bool inv)
	{
		if(num) _fanin2 = (inv)? (CirGate*)(size_t(gate) | 1) : gate;
		else _fanin1 = (inv)? (CirGate*)(size_t(gate) | 1) : gate;
	}
   void setOutGate(CirGate* gate, bool inv) { if(inv) _fanout.push_back((CirGate*)(size_t(gate) | 1)); else _fanout.push_back(gate); sort_out(); }
	void setSymbol(string sym) { _symbol = new char[sym.size() + 1]; strcpy(_symbol, sym.c_str()); }
	void sort_out() { sort(_fanout.begin(), _fanout.begin() + _fanout.size(), out_comp); }
	void merge(CirGate* gate);

   // dfs functions
   void dfs(size_t& num) const;
	void dfs(bool in, int level, int searched_level = 0) const;
	void dfs(IdList& idList) const;
   void mark() const { _mark = _state; }
	static void resetMark() { ++_state; }

   // Printing functions
   void printGate() {}
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

	// checking functions
	bool fanin_inv(int num) const { return (!num)? size_t(_fanin1) % 2 : size_t(_fanin2) % 2; }
	bool fanout_inv(int num) const { return size_t(_fanout[num]) % 2; }
	bool is_marked() const { return _mark == _state; }
	bool has_floating_fanin() const { for(size_t i = 0; i < 2; ++i) { if(!getIn(i)) continue; if(getIn(i)->getTypeInt() == UNDEF_GATE) return true; } return false; }
	bool is_unused() const { if(!_fanout.size()) return true; for(size_t i = 0; i < _fanout.size(); ++i) if(getOut(i)->getTypeInt() == UNDEF_GATE) return true; return false; }
   bool isAig() const { if(getTypeInt() == AIG_GATE) return true; return false; }

private:
protected:
	int _typeInt;
	GateList _fanout;
	CirGate* _fanin1;
	CirGate* _fanin2;
	unsigned _id;
	unsigned _line;
	mutable int _mark;
	static int _state;
	char* _symbol;
};

class PIGate : public CirGate
{
public:
	PIGate(size_t id, unsigned l) : CirGate(1, id, l) { }
	~PIGate() { }
private:
};

class POGate : public CirGate
{
public:
	POGate(size_t id, unsigned l) : CirGate(2, id, l) { }
	~POGate() {}
private:
};

class AIGGate : public CirGate
{
public:
	AIGGate(size_t id, unsigned l) : CirGate(3, id, l) { }
	~AIGGate() {}
private:
};

class UNDEFGate : public CirGate
{
public:
	UNDEFGate(size_t id) : CirGate(0, id, 0) { }
	~UNDEFGate() {}
private:
};

class CONSTGate : public CirGate
{
public:
	CONSTGate(size_t id, unsigned l) : CirGate(4, id, l) { }
	~CONSTGate() {}
private:
};

#endif // CIR_GATE_H
