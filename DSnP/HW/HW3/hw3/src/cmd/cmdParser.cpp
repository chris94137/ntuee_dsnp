/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"
#include <dirent.h>
#include <algorithm>

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   _dofile = new ifstream(dof.c_str());
   if(_dofileStack.size() >= 1024)
   {
	   cerr << "Error: dofile stack overflow (" << _dofileStack.size() << ")" << endl;
	   closeDofile();
	   return false;
   }
   _dofileStack.push(_dofile);
   if(!_dofile->is_open()) 
   {
		closeDofile();
      return false;
	}
   return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   // TODO...
	_dofile->close();
	_dofileStack.pop();
   delete _dofile;
	if(_dofileStack.size())
		_dofile = _dofileStack.top();
	else
		_dofile = 0;
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }
   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(optCmd);
   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
	_tabPressCount = 0;
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);
   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
   // TODO...
	for(auto iter = _cmdMap.begin() ; iter != _cmdMap.end(); iter++)
		iter->second->help();
	cout << endl;
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
   assert(_tempCmdStored == false);
   assert(!_history.empty());
   string str = _history.back();

   // TODO...
	string firstCmd;
	size_t nextPos = myStrGetTok(str, firstCmd);
	CmdExec* correctCmd = getCmd(firstCmd);
	if(correctCmd == 0)
	{
		cerr << "Illegal command!! (" << firstCmd << ")" << endl;
		return 0;
	}
	if(nextPos != string::npos)
		option = str.substr(nextPos, str.size() - nextPos);
	else option = "";
   assert(str[0] != 0 && str[0] != ' ');
   return correctCmd;
}

// Remove this function for TODO...
//
// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    Considering the following cases in which prefix is empty:
//    --- 6.1.1 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and they do not have a common prefix,
//    cmd> help $sdfgh
//    [After] print all the file names
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.1.2 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and all of them have a common prefix,
//    cmd> help $orld
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help mydb-$orld
//    --- 6.1.3 ---
//    [Before] if prefix is empty, and only one file in the current directory
//    cmd> help $ydb
//    [After] print out the single file name followed by a ' '
//    cmd> help mydb $
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

void
CmdParser::listCmd(const string& str)
{
	//cerr << endl << "tab count : " << _tabPressCount << endl;
   // TODO...
	bool isAllSpace = true;
   for(int i = 0; i < _readBufEnd - _readBuf; i++)
		if(_readBuf[i] != ' ')
		{
			isAllSpace = false;
			break;
		}
	int count = 0;
	if(isAllSpace)
	{
		cout << endl;
		for(auto iter = _cmdMap.begin() ; iter != _cmdMap.end(); iter++)
		{
			cout << setw(12) << left << iter->first + iter->second->getOptCmd();
			count++;
			if(count == 5)
			{
				cout << endl;
				count = 0;
			}
		}
		reprintCmd();
		_tabPressCount = 0;
		return;
	}

	string WordsBeforeCursor = "";
	for(int i = 0; i < _readBufPtr - _readBuf; i++)
		WordsBeforeCursor += _readBuf[i];
	string firstWord = WordsBeforeCursor.substr(0, WordsBeforeCursor.find_first_of(' '));
	if(WordsBeforeCursor.size() > 0)
	{
		vector<CmdMap::iterator> correctIter;
		for(auto iter = _cmdMap.begin(); iter != _cmdMap.end(); iter++)
		{
			string wholeCmd = iter->first + iter->second->getOptCmd();
			if(myStrNCmp(WordsBeforeCursor, wholeCmd.substr(0, WordsBeforeCursor.size()), WordsBeforeCursor.size()) == 0)
				correctIter.push_back(iter);
		}
		if(correctIter.size() == 1)
		{
			string remainStr = correctIter[0]->first;
			string optStr = correctIter[0]->second->getOptCmd();
			if(WordsBeforeCursor.size() < remainStr.size() + optStr.size())
			{
				if(WordsBeforeCursor.size() < correctIter[0]->first.size())
				{
					remainStr = remainStr.substr(WordsBeforeCursor.size(), correctIter[0]->first.size() - WordsBeforeCursor.size());
					for(unsigned i = 0; i < remainStr.size(); i++) insertChar(remainStr[i]);
					for(unsigned i = 0; i < optStr.size(); i++) insertChar(optStr[i]);
				}
				else
				{
					optStr = optStr.substr(WordsBeforeCursor.size() - remainStr.size(), optStr.size());
					for(unsigned i = 0; i < optStr.size(); i++) insertChar(optStr[i]);
				}
			}
			insertChar(' ');
			_tabPressCount = 0;
			return;
		}
		else if(correctIter.size() > 1)
		{
			count = 0;
			for(unsigned i = 0; i < correctIter.size(); i++)
			{
				if(count % 5 == 0) cout << endl;
				cout << setw(12) << left << correctIter[i]->first + correctIter[i]->second->getOptCmd();
				count++;
			}
			reprintCmd();
			_tabPressCount = 0;
			return;
		}
	}
	string comparingCmd = "";
	CmdExec* correctCmd;
	bool is_cmd = false;
	WordsBeforeCursor.erase(0, WordsBeforeCursor.find_last_of(" ") + 1);
	correctCmd = getCmd(firstWord);
	if(correctCmd) is_cmd = true;
	if(is_cmd)
	{
		if(_tabPressCount == 1)
		{
		cout << endl;
		correctCmd->usage(cout);
		reprintCmd();
		return;
		}
		else
		{
			string dir = ".";
			vector<string> files;
			DIR *dp;
			struct dirent *dirp;
			if((dp = opendir(dir.c_str())) != NULL)
				while((dirp = readdir(dp)) != NULL)
					files.push_back(string(dirp->d_name));
			closedir(dp);
			if(files.size())
			{
				sort(files.begin(), files.begin() + files.size());
				files.erase(files.begin());
				files.erase(files.begin());
				if(WordsBeforeCursor == "")
				{
					string sameStr;
					for(unsigned i = 0; i < files[0].size(); i++)
					{
						bool same = true;
						for(unsigned j = 1; j < files.size(); j++)
							if(files[0][i] != files[j][i])
							{
								same = false;
								break;
							}
						if(same) sameStr += files[0][i];
						else break;
					}
					if(sameStr.size())
					{
						for(unsigned i = 0; i < sameStr.size(); i++)
							insertChar(sameStr[i]);
						return;
					}
					for(unsigned i = 0; i < files.size(); i++)
					{
						if(i % 5 == 0) cout << endl;
						cout << setw(16) << left << files[i];
					}
				}
				else
				{
					int prefixSize = WordsBeforeCursor.size();
					vector<string> matchFiles;
					for(unsigned i = 0; i < files.size(); i++)
					{
						string compareStr;
						for(int j = 0; j < prefixSize; j++)
							compareStr += files[i][j];
						if(compareStr == WordsBeforeCursor)
							matchFiles.push_back(files[i]);
					}
					if(matchFiles.size() == 0)
					{
						mybeep();
						return;
					}
					else if(matchFiles.size() == 1)
					{
						string remainStr = matchFiles[0].substr(WordsBeforeCursor.size(), matchFiles[0].size() - WordsBeforeCursor.size());
						for(unsigned i = 0; i < remainStr.size(); i++) insertChar(remainStr[i]);
						insertChar(' ');
						return;
					}
					else
					{
						string sameStr;
						for(unsigned i = 0; i < matchFiles[0].size(); i++)
						{
							bool same = true;
							for(unsigned j = 1; j < matchFiles.size(); j++)
								if(matchFiles[0][i] != matchFiles[j][i])
								{
									same = false;
									break;
								}
							if(same) sameStr += matchFiles[0][i];
							else break;
						}
						if(sameStr.size() > WordsBeforeCursor.size())
						{
							sameStr = sameStr.substr(WordsBeforeCursor.size(), sameStr.size() - WordsBeforeCursor.size());
							for(unsigned i = 0; i < sameStr.size(); i++)
								insertChar(sameStr[i]);
							return;
						}
						for(unsigned i = 0; i < matchFiles.size(); i++)
						{
							if(i % 5 == 0) cout << endl;
							cout << setw(16) << left << matchFiles[i];
						}
					}
				}
			}
			else mybeep();
			reprintCmd();
			return;
		}
	}
	mybeep();
}


// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   //CmdExec* e = 0;
   // TODO...
	for(unsigned i = 0; i < cmd.size(); i++)
		cmd[i] = toupper(cmd[i]);
	string comparingCmd = "";
	CmdMap::iterator iter;
	for(unsigned i = 0; i < cmd.size(); i++)
	{
		comparingCmd += cmd[i];
		iter = _cmdMap.find(comparingCmd);
		if(iter != _cmdMap.end()) break;
		if(i == cmd.size() - 1)
			return 0;
	}
	if(iter == _cmdMap.end()) return 0;
	cmd = cmd.substr(comparingCmd.size(), cmd.size() - comparingCmd.size());
	string optCmd = iter->second->getOptCmd();
	for(unsigned i = 0; i < cmd.size(); i++)
		cmd[i] = tolower(cmd[i]);
	for(unsigned i = 0; i < cmd.size(); i++)
		if(cmd[i] != optCmd[i])
			return 0;
	return iter->second;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// return false if option contains an token
bool
CmdExec::lexNoOption(const string& option) const
{
   string err;
   myStrGetTok(option, err);
   if (err.size()) {
      errorOption(CMD_OPT_EXTRA, err);
      return false;
   }
   return true;
}

// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);//substr first argv, n is the pos of sec argv
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

