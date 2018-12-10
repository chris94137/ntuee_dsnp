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
	CirGate(int ti, size_t id, unsigned l) : _typeInt(ti), _fanin1(0), _fanin2(0), _id(id), _line(l), _mark(0) { }
	CirGate(int ti, size_t id) : _typeInt(ti), _fanin1(0), _fanin2(0), _id(id), _mark(0) { }
   virtual ~CirGate() {}

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
	string getSymbol() const { return _symbol; }
	CirGate* getIn(int num) const { return (!num)? (CirGate*)(size_t(_fanin1) & (~1)) : (CirGate*)(size_t(_fanin2) & (~1)); }
	CirGate* getOut(int num) const { return (CirGate*)(size_t(_fanout[num]) & (~1)); }

   // Basic setting methods
	void setInGate(CirGate* gate1, CirGate* gate2, bool inv1, bool inv2)
	{
		if(inv1) _fanin1 = (CirGate*)(size_t(gate1) | 1);
		else _fanin1 = gate1;
		if(inv2) _fanin2 = (CirGate*)(size_t(gate2) | 1);
		else _fanin2 = gate2;
	}
   //void setInGate(CirGate* gate, bool inv) { _fanin.push_back(gate); _faninInv.push_back(inv); }
   void setOutGate(CirGate* gate, bool inv) { if(inv) _fanout.push_back((CirGate*)(size_t(gate) | 1)); else _fanout.push_back(gate);}
	void setSymbol(string sym) { _symbol = sym; }

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
	bool is_unused() const { if(!_fanout.size()) return true; for(size_t i = 0; i < _fanout.size(); ++i) if(_fanout[i]->getTypeInt() == UNDEF_GATE) return true; return false; }

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
	string _symbol;
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
	UNDEFGate(size_t id) : CirGate(0, id) { }
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
