#ifndef VCZH_DOCUMENT_CPPDOC_LEXER
#define VCZH_DOCUMENT_CPPDOC_LEXER

#include "Vlpp.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;
using namespace vl::stream;
using namespace vl::filesystem;

/***********************************************************************
Definition
***********************************************************************/

enum class CppTokens
{
	LBRACE, RBRACE,
	LBRACKET, RRACKET,
	LPARENTHESIS, RPARENTHESIS,
	LT, GT, EQ, NOT,
	PERCENT, COLON, SEMICOLON, DOT, QUESTIONMARK, COMMA,
	MUL, ADD, SUB, DIV, XOR, AND, OR, REVERT, SHARP,
	INT, HEX, BIN, FLOAT,
	STRING, CHAR,
	ID,
	SPACE, DOCUMENT, COMMENT1, COMMENT2,
};

extern Ptr<RegexLexer> CreateCppLexer();

/***********************************************************************
Reader
***********************************************************************/

class CppTokenCursor;
class CppTokenReader;

class CppTokenCursor
{
	friend class CppTokenReader;
private:
	CppTokenReader*				reader;
	Ptr<CppTokenCursor>			next;

	CppTokenCursor(CppTokenReader* _reader, RegexToken _token);
public:
	RegexToken					token;

	Ptr<CppTokenCursor>			Next();
};

class CppTokenReader : public Object
{
	friend class CppTokenCursor;
protected:
	Ptr<RegexLexer>				lexer;
	Ptr<RegexTokens>			tokens;
	IEnumerator<RegexToken>*	tokenEnumerator;

	Ptr<CppTokenCursor>			firstToken;

	Ptr<CppTokenCursor>			CreateNextToken();
public:
	CppTokenReader(const WString& input);
	~CppTokenReader();

	Ptr<CppTokenCursor>			GetFirstToken();
};

#endif