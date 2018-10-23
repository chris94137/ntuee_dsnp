/****************************************************************************
  FileName     [ p2Json.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define member functions of class Json and JsonElem ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2018-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include "p2Json.h"

using namespace std;

// Implement member functions of class Row and Table here
bool
Json::read(const string& JsonFile)
{
	fstream file;
	file.open(JsonFile, fstream::in);
	if(!file.is_open()) return false;
	else
	{
		string trashstr;
		string readkey;
		int readvalue;

		while(1)
		{
			file >> trashstr; // first:{ , else:,
			if(trashstr == "}") break;
			file >> readkey;  // key
			if(readkey == "}") break;
			file >> trashstr; // :
			file >> readvalue;// value
			readkey = readkey.substr(1, readkey.size() - 2);
			
			_obj.push_back(JsonElem(readkey, readvalue));
		}
	}
	file.close();
	return true;
}

ostream&
operator << (ostream& os, const JsonElem& j)
{
   return (os << "\"" << j._key << "\" : " << j._value);
}

bool
Json::elemExist()
{
	if(_obj.size() == 0)
	{
		cerr << "Error: No element found!!\n";
		return false;
	}
	return true;
}

void
Json::print()
{
	cout << "{\n";
	for(unsigned int i = 0; i < _obj.size(); i++)
	{
		cout << "  " << _obj[i];
		if(i != _obj.size() - 1) cout << ",";
		cout << endl;
	}
	cout << "}\n";
}

int
Json::getSum()
{
	int sum = 0;
	for(unsigned int i = 0; i < _obj.size(); i++)
		sum += _obj[i].getValue();
	return sum;
}

void
Json::showSum()
{
	if(elemExist())
		cout << "The summation of the values is: " << getSum() << ".\n";
}

void
Json::showAve()
{
	if(elemExist())
	{
		double average = double(getSum())/double(_obj.size());
		cout << "The average of the values is: " << fixed <<  setprecision(1) << average << ".\n";
	}	
}

void
Json::showMax()
{
	if(elemExist())
	{
		string maxKey = _obj[0].getKey();
		int maxValue = _obj[0].getValue();
		for(unsigned int i = 0; i < _obj.size(); i++)
			if(maxValue < _obj[i].getValue())
			{
				maxKey = _obj[i].getKey();
				maxValue = _obj[i].getValue();
			}
		cout << "The maximum element is: { \"" << maxKey << "\" : " << maxValue << " }.\n";
	}
}

void
Json::showMin()
{
	if(elemExist())
	{
		string minKey = _obj[0].getKey();
		int minValue = _obj[0].getValue();
		for(unsigned int i = 0; i < _obj.size(); i++)
			if(minValue > _obj[i].getValue())
			{
				minKey = _obj[i].getKey();
				minValue = _obj[i].getValue();
			}
		cout << "The minimum element is: { \"" << minKey << "\" : " << minValue << " }.\n";
	}
}

void
Json::add(const string& newKey, const int& newValue)
{
	_obj.push_back(JsonElem(newKey, newValue));
}
