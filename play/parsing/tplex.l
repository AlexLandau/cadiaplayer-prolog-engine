%option c++
%option noyywrap
%option caseless

delim		[ \t]
ws		{delim}+
eol		[\n\r]
eof		[<<EOF>>]
letter		[A-Za-z]
digit		[0-9]
digits		{digit}+
comment		[;](.)*({eol}|{eof})
id   		({letter}|{digit}|[_]|[-]|[+])*
var		[\?]{id}

%{
#include <iostream>
#include <fstream>
#include "token.h"
using namespace cadiaplayer::play::parsing;
OpType _type = op_NONE;
bool _predicate = false;
Token _currentToken;
int _lineno = 0;
FlexLexer* _lexer = NULL;
std::filebuf* _f = NULL;
std::istream* _is = NULL;
%}

%%


{ws}		{	_type=op_NONE;		_predicate = false;			/* ignore return */	}
{eol}		{	_type=op_NONE;		_predicate = false;	++_lineno; 	/* ignore return */	}

"("		{	_type=op_NONE;		_predicate = true;	return(tc_LPAREN);	}
")"		{	_type=op_NONE;		_predicate = false;	return(tc_RPAREN);	}

"role"		{ 	_type=op_ROLE;		_predicate = false;	return(tc_RELATION);	}
"true"		{ 	_type=op_TRUE;		_predicate = false;	return(tc_RELATION);	}
"init"		{ 	_type=op_INIT;		_predicate = false;	return(tc_RELATION);	}
"next"		{ 	_type=op_NEXT;		_predicate = false;	return(tc_RELATION);	}
"legal"		{ 	_type=op_LEGAL;		_predicate = false;	return(tc_RELATION);	}
"does"		{ 	_type=op_DOES;		_predicate = false;	return(tc_RELATION);	}
"goal"		{ 	_type=op_GOAL;		_predicate = false;	return(tc_RELATION);	}
"terminal"	{ 	_type=op_TERMINAL;	_predicate = false;	return(tc_RELATION);	}

"<="		{	_type=op_IMPLICATION;	_predicate = false;	return(tc_OP);		}
"or"		{	_type=op_OR;		_predicate = false;	return(tc_OP);		}
"not"		{	_type=op_NOT;		_predicate = false;	return(tc_OP);		}
"distinct"	{	_type=op_DISTINCT;	_predicate = false;	return(tc_OP);		}

{digits}	{	_type=op_INT;	
			int ret = 0; 
			if(_predicate) ret = tc_PREDICATE; else ret = tc_CONSTANT; 
						_predicate = false;	return(ret);		}
{id}		{	_type=op_ATOM;	
			int ret = 0; 
			if(_predicate) ret = tc_PREDICATE; else ret = tc_CONSTANT; 
						_predicate = false;	return(ret);		}
{var}		{	_type=op_NONE;		_predicate = false;	return(tc_VARIABLE);	}

{comment}	{	_type=op_NONE;		_predicate = false;	++_lineno; 	return(tc_COMMENT); 	}
<<EOF>>		{	_type=op_NONE;		_predicate = false;	return(tc_EOF);		}


%%

int info()
{
	printf("KIF for GDL lexical analyzer\n");
	printf("by Hilmar Finnsson");
	return 0;
}

FlexLexer* allocateLexer( std::istream* arg_yyin, std::ostream* arg_yyout )
{
	_lexer = new yyFlexLexer(arg_yyin, arg_yyout);
	_lineno = 0;
	return _lexer;
}

FlexLexer* allocateLexerToFile(const char* filename)
{
	_f = new std::filebuf();
	_f->open(filename, std::ios::in);
	_is = new std::istream(_f);
	return allocateLexer(_is, &std::cout);
}

void freeLexer(void)
{
	if(_lexer != NULL)
		delete _lexer;
	if(_f != NULL)
		delete _f;
	if(_is != NULL)
		delete _is;
}

void freeStringLexer(void)
{
	if(_lexer != NULL)
		delete _lexer;
}

Token* nextToken()
{
	if(_lexer == NULL)
		return NULL;
	int lex = _lexer->yylex();
	_currentToken.setTokenCode((TokenCode)lex);
	_currentToken.setOpType(_type);
	_currentToken.setLexeme(_lexer->YYText());
	return &_currentToken;
}

int getLineNo()
{
	if(_lexer == NULL)
		return 0;
	return _lineno;
}
