/****************************************************************************
  FileName     [ p2Main.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define main() function ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2016-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include <sstream>
#include "p2Json.h"

using namespace std;

int main()
{
   Json json;

   // Read in the csv file. Do NOT change this part of code.
   string jsonFile;
   cout << "Please enter the file name: ";
   cin >> jsonFile;
   if (json.read(jsonFile))
      cout << "File \"" << jsonFile << "\" was read in successfully." << endl;
   else {
      cerr << "Failed to read in file \"" << jsonFile << "\"!" << endl;
      exit(-1); // jsonFile does not exist.
   }
   // TODO read and execute commands
   while (true)
   {
   	string input, command;
   	getline(cin, input);
   	stringstream str(input);
   	str >> command;
   	if(command == "PRINT"){json.print();}
   	else if(command == "SUM"){json.showSum();}
   	else if(command == "AVE"){json.showAve();}
   	else if(command == "MAX"){json.showMax();}
   	else if(command == "MIN"){json.showMin();}
   	else if(command == "ADD")
   	{
   		string newKey;
   		int newValue;
   		str >> newKey;
   		str >> newValue;
   		json.add(newKey, newValue);
   	}
   	else if(command == "EXIT")
   		exit(0);
   	cout << "Enter command: ";
   }
}
