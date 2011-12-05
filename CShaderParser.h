/* 
	CShaderParser.h

	Author:			Tom Naughton
	Description:	<describe the CShaderParser class here>
*/

/* Aftershock 3D rendering engine
 * Copyright (C) 1999 Stephen C. Taylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifndef CShaderParser_H
#define CShaderParser_H

#include "CShader.h"

/* Maps shader keywords to functions */
typedef struct
{
    char *keyword;
    int minargs, maxargs;
    void (* func)(CShader *shader, shaderpass_t *pass,
		  int numargs, char **args);
} shaderkey_t;

class CShaderParser
{

public:

	CShaderParser();
	virtual ~CShaderParser();

	void 	parse(CResourceManager *resources, const char *inFilename, 
				const char *s, UInt32 length);
	CShaderIndexList *index(const char *s, UInt32 length);
	CShader *shaderWithString(CResourceManager *resources, char *s);
	
private:

	void 	parseShader(CShader *theShader);
	void 	shader_readpass(CShader *shader, UInt32 inPass);
	void 	shader_parsetok(CShader *shader, shaderpass_t *pass, shaderkey_t *keys, char *tok);

	void 	shader_skip(void);
	char 	*nexttok(void);
	char 	*nextarg(void);

	char *curpos, *endpos;
};

#endif	// CShaderParser_H
