/****************************************************************************
  FileName     [ dbJson.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Json member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <cmath>
#include <string>
#include <algorithm>
#include "dbJson.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream&
operator << (ostream& os, const DBJsonElem& j)
{
   os << "\"" << j._key << "\" : " << j._value;
   return os;
}

istream& operator >> (istream& is, DBJson& j)
{
   // TODO: to read in data from Json file and store them in a DB 
   // - You can assume the input file is with correct JSON file format
   // - NO NEED to handle error file format
   assert(j._obj.empty());
	string trashstr;
	string readkey;
	int readvalue;
	while(1)
	{
		is >> trashstr; // first:{ , else:,
		if(trashstr == "}") break;
		is >> readkey; 	// key
		if(readkey == "}") break;
		is >> trashstr; // :
		is >> readvalue;// value
		readkey = readkey.substr(1, readkey.size() - 2);
		j.add(DBJsonElem(readkey, readvalue));
	}
	j._isReadIn = true;
   return is;
}

ostream& operator << (ostream& os, const DBJson& j)
{
   // TODO
	os << "{\n";
	for(unsigned i = 0; i < j._obj.size(); i++)
	{
		os << "  " << j._obj[i];
		if(i != j._obj.size() - 1) cout << ",";
		cout << endl;
	}
	os << "}\n";
   return os;
}

/**********************************************/
/*   Member Functions for class DBJsonElem    */
/**********************************************/
/*****************************************/
/*   Member Functions for class DBJson   */
/*****************************************/
void
DBJson::reset()
{
   // TODO
	_obj.clear();
	if(_isReadIn) _isReadIn = false;
}

// return false if key is repeated
bool
DBJson::add(const DBJsonElem& elm)
{
   // TODO
   for(unsigned i = 0; i < _obj.size(); i++)
		if(elm.key() == _obj[i].key())
			return false;
   _obj.push_back(elm);
   return true;
}

// return NAN if DBJson is empty
float
DBJson::ave() const
{
   // TODO
	if(_obj.size() == 0) return NAN;
	float average = float(sum()) / float(_obj.size());
	return average;
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::max(size_t& idx) const
{
   // TODO
	if(_obj.empty())
	{
		idx = _obj.size();
		return INT_MIN;
	}
   int maxN = INT_MIN;
	for(unsigned i = 0; i < _obj.size(); i++)
		if(_obj[i].value() > maxN)
		{
			maxN = _obj[i].value();
			idx = i;
		}
   return  maxN;
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::min(size_t& idx) const
{
   // TODO
	if(_obj.empty())
	{
		idx = _obj.size();
		return INT_MAX;
	}
	int minN = INT_MAX;
	for(unsigned i = 0; i < _obj.size(); i++)
		if(_obj[i].value() < minN)
		{
			minN = _obj[i].value();
			idx = i;
		}
   return  minN;
}

void
DBJson::sort(const DBSortKey& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

void
DBJson::sort(const DBSortValue& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

// return 0 if empty
int
DBJson::sum() const
{
   // TODO
   int s = 0;
	for(unsigned i = 0; i < _obj.size(); i++)
		s += _obj[i].value();
   return s;
}
