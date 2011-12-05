/* 
	CTokenizer.h

	Author:			Tom Naughton
	Description:	<describe the CTokenizer class here>
*/

#ifndef CTokenizer_H
#define CTokenizer_H

#include <iostream.h>
#include <string>

using std::string;

class CTokenizer
{

public:

	CTokenizer(string &inString, string &inDelimiters);
	virtual ~CTokenizer();
	Boolean		charInSet(char c, string &set);
	string		nextToken();
	int			nextInt();
	int 		countTokens();
	int 		integerValueForToken(string & token);

private:

	string 		&_str;
	string 		&_delimiters;
	int 		_pos;
	Boolean 	_foundcomment;
};

#endif	// CTokenizer_H
