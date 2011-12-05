/* 
	CTokenizer.cpp

	Author:			Tom Naughton
	Description:	<describe the CTokenizer class here>
*/

#include <stdio.h>
#include "CTokenizer.h"



CTokenizer::CTokenizer(string &inString, string &inDelimiters) 
: _str(inString), _delimiters(inDelimiters)
{
	_foundcomment = false;
	_pos = 0;
}


CTokenizer::~CTokenizer()
{
}

Boolean CTokenizer::charInSet(char c, string & inSet)
{
	int length = inSet.length();
	for(int i = 0; i < length; i++) {
		if (inSet[i] == c)
			return true;
	}
	return false;
}

int CTokenizer::countTokens()
{
	int savepos = _pos;
	int count = 0;
	string token;
	do {
		token = nextToken();
		if (token.length())
			count++;
	} while (token.length());
	_pos = savepos;
	_foundcomment = 0;
	return count;
}

int CTokenizer::integerValueForToken(string & token) 
{
	int value = 0;
	int sign = 1;
	
	for(int i = 0; i < token.length(); ++i) {
		char c = token[i];
		if (c == '-' && i == 0) {
			sign = -1;
		} else if (c >= '0' && c <='9') {
			value *= 10;
			value += c - '0';
		} else {
			dprintf("noninteger token!\n");
			return 0;
		}
	}
	
	//dprintf("integerValueForToken %s %d\n", token.c_str(), value * sign);
	return value * sign;
}

int CTokenizer::nextInt() 
{
	string token = nextToken();
	return integerValueForToken(token);
}

string CTokenizer::nextToken()
{
	// skip _delimiters
	while(_pos < _str.length() && charInSet(_str[_pos], _delimiters))
		_pos++;
	int	start = _pos;
	int	length = 0;
	
	if (!_foundcomment && _str.length() > _pos+1 && _str[_pos] == '/' && _str[_pos+1] == '/') {
		
		// rest of the line is a comment
		_foundcomment = true;
		start = _pos = _str.length();
		
	} else if(_str[_pos] == '"') {
	
		// scan "string"
		start++;
		_pos++;
		while(_pos < _str.length() && _str[_pos] != '"')
			_pos++;
			
		length = (_pos-start);
			
		if (_pos == _str.length() && _str[_pos] != '"')
			length++; // no closing quote
		else
			_pos++; // skip quote
			
	} else {
	
		// scan up to delimeter
		while(_pos < _str.length() && !charInSet(_str[_pos], _delimiters))
			_pos++;
		length = _pos-start; 
	}
		
	//dprintf("token: '%s'\n", _str.substr(start,length).c_str());
	return _str.substr(start,length);
}


