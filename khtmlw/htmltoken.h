//-----------------------------------------------------------------------------
//
// KDE HTML Widget
//

#ifndef HTMLTOKEN_H
#define HTMLTOKEN_H

#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>

// Every tag as deliverd by HTMLTokenizer starts with TAG_ESCAPE. This way
// you can devide between tags and words.
#define TAG_ESCAPE 13

// The count of spaces used for each tab.
#define TAB_SIZE 8

//-----------------------------------------------------------------------------

class StringTokenizer
{
public:
    StringTokenizer( const QString &, const char * );
    ~StringTokenizer();

    const char* nextToken();
    bool hasMoreTokens();

protected:
    char *pos;
    char *end;
    char *buffer;
};

//-----------------------------------------------------------------------------

class BlockingToken
{
public:
	enum Token { Grid, Table, UnorderedList, OrderedList, Menu, Dir, Glossary };

	BlockingToken( Token t, int p )
		{	ttype = t; pos = p; }

	int getPosition()
		{	return pos; }
	const char *token();

protected:
	Token ttype;
	int pos;
};

//-----------------------------------------------------------------------------

class HTMLTokenizer
{
public:
    HTMLTokenizer();
    ~HTMLTokenizer();

	void begin();
	void write( const char * );
	void end();

    const char* nextToken();
    bool hasMoreTokens();

	int getPosition()
		{	return tokenList.at(); }
	void setPosition( int p )
		{	if ( p >= 0 ) tokenList.at( p ); }

	const char *first()
		{	return tokenList.first(); }
	const char *last()
		{	return tokenList.last(); }

protected:
	QStrList tokenList;
	char *buffer;
	char *dest;

	// the size of buffer
	int size;

	// are we in a html tag
	bool tag;

	// To avoid multiple spaces
	bool space;

	// Are we in a <pre> ... </pre> block
	bool pre;

	// Are we in a <-- comment -->
	bool comment;

	// These are tokens which we are waiting for ending tokens for
	QList<BlockingToken> blocking;
};

#endif // HTMLTOKEN

