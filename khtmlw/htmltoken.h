//-----------------------------------------------------------------------------
//
// KDE HTML Widget
//

#ifndef HTMLTOKEN_H
#define HTMLTOKEN_H

class StringTokenizer;
class HTMLTokenizer;

#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>

#include "jscript.h"

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
    enum Token { Grid, Table, UnorderedList, OrderedList, Menu, Dir, Glossary,
		 FrameSet, Script, Cell };

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
    HTMLTokenizer( KHTMLWidget *_widget = NULL );
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

    // are we in quotes within a html tag
    bool tquote;
    
    // To avoid multiple spaces
    bool space;
    
    // Are we in a <pre> ... </pre> block
    bool pre;
    
    // Are we in a <script> ... </script> block
    bool script;
    
    // Are we in a <-- comment -->
    bool comment;

    // Used to store the code of a srcipting sequence
    char *scriptCode;
    // Size of the script sequenze stored in @ref #scriptCode
    int scriptCodeSize;
    // Maximal size that can be stored in @ref #scriptCode
    int scriptCodeMaxSize;
    
    // Used to store the string "</script>" for comparison
    const char *scriptString;
    // Stores characters if we are scanning for "</script>"
    char scriptBuffer[ 10 ];
    // Counts where we are in the string "</script>"
    int scriptCount;
    
    // Is TRUE if we are in a <script> tag and insideof '...' quotes
    bool squote;
    // Is TRUE if we are in a <script> tag and insideof "..." quotes
    bool dquote;

    KHTMLWidget *widget;
    
    /**
     * This pointer is 0L until used. The @ref KHTMLWidget has an instance of
     * this class for us. We ask for it when we see some JavaScript stuff for
     * the first time.
     */
    JSEnvironment* jsEnvironment;
    
    // These are tokens for which we are awaiting ending tokens
    QList<BlockingToken> blocking;
};

#endif // HTMLTOKEN

