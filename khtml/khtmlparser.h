/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- HTML Parser
// $Id$

#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <qpainter.h>
#include <qstrlist.h>
#include <qpixmap.h>
#include <qstack.h>
#include <qfont.h>
#include <qtimer.h>
#include <qlist.h>
#include <qstring.h>
#include <qbuffer.h>

#include "kurl.h"
#include "khtmlclue.h"
#include "khtmltags.h"

//
// External Classes
//
///////////////////

// ../kdecore/kcharset.h
class KCharsetConverter;

// khtml.h
class KHTMLWidget;

// khtmldata.h
class HTMLSettings;

// khtmliter.h
class HTMLIterator;

// khtmltoken.h
class HTMLTokenizer;
class StringTokenizer;

// khtmlclue.h
class HTMLClue;

// khtmlfont.h
class HTMLFont;

// khtmlform.h
class HTMLForm;
class HTMLSelect;
class HTMLTextArea;

// khtmlframe.h
class HTMLFrameSet;

//
// Internal Classes
//
///////////////////

class KHTMLParser;
class HTMLStackElem;

typedef void (KHTMLParser::*blockFunc)(HTMLStackElem *stackElem);

class HTMLStackElem
{
public:
	HTMLStackElem(		int _id, 
	 					int _level, 
	 				   	blockFunc _exitFunc, 
	 				   	int _miscData1,
	 				   	int _miscData2, 
	 				   	HTMLStackElem * _next
	 				  ) 
	 				  :	id(_id), 
	 				   	level(_level),
	 				   	exitFunc(_exitFunc), 
	 				   	miscData1(_miscData1), 
	 				   	miscData2(_miscData2), 
	 				   	next(_next) 
                 { }

    int       id;
    int       level;
   	 
    blockFunc exitFunc;
   
    int       miscData1;
    int       miscData2;

	HTMLStackElem *next;
};

// Default borders between widgets frame and displayed text
#define LEFT_BORDER 10
#define RIGHT_BORDER 20
#define TOP_BORDER 10
#define BOTTOM_BORDER 10

typedef void (KHTMLParser::*tagFunc)();

class KHTMLParser
{
public:
    KHTMLParser( KHTMLWidget *_parent, 
    			 HTMLTokenizer *_ht, 
    			 QPainter *_painter,
    			 HTMLSettings *_settings);
    virtual ~KHTMLParser();

    /*
     * This function is called after &lt;body&gt; usually. You can
     * call it for every rectangular area: For example a tables cell
     * or for a menus <li> tag. ht gives you one token after another.
     * _clue points to a VBox. All HTMLObjects created by this
     * function become direct or indirect children of _clue.
     * The last two parameters define which token signals the end
     * of the section this function should parse, for example &lt;/body&gt;.
     * You can specify two tokens, for example &lt;/li&gt; and &lt;/menu&gt;.
     * You may even set the second one to "" if you dont need it.
     */
    int parseBody( HTMLClue *_clue, const char *_end, bool toplevel = FALSE );

   /**
    * Set document charset.
    *
    * Any <META ...> setting charsets overrides this setting
    *
    * @return TRUE if successfull
    *
    */
    bool setCharset(const char *name);

protected:
	KHTMLWidget *HTMLWidget;

    enum ListNumType { Numeric = 0, LowAlpha, UpAlpha, LowRoman, UpRoman };
    enum ListType { Unordered, UnorderedPlain, Ordered, Menu, Dir };

    // do </a> if necessary
    void closeAnchor()
    {
	if ( url )
	{
	    popColor();
	    popFont();
	}
	url = 0;
	target = 0;
    }

	/*
	 * This function creates a new flow adds it to '_clue' and sets 'flow'
	 */
	void newFlow();

	void insertText(char *str, const HTMLFont * fp);

    /*
     * tagID
     *
     * The ID of the tag currently being parsed
     */
    int tagID;  

    /*
     * Parses the tag set in tagID with options tagOptions
     */
    void parseOneToken();

    /*
     * Approx. each tag has its own function
     */
    void parseTagEnd(void);			// Generic end-tag function

    void parseTagA(void);			
    void parseTagAbbr(void);		
    void parseTagAcronym(void);		
    void parseTagAddress(void);
    void parseTagApplet(void);
    void parseTagArea(void);
    void parseTagB(void);
    void parseTagBase(void);
    void parseTagBaseFont(void);
    void parseTagBdo(void);
    void parseTagBig(void);
    void parseTagBlockQuote(void);
    void parseTagBody(void); 
    void parseTagBr(void);
    void parseTagButton(void);
    void parseTagCell(void);
    void parseTagCenter(void);
    void parseTagCite(void);
    void parseTagCode(void);
    void parseTagDD(void);
    void parseTagDel(void);
    void parseTagDfn(void);
    void parseTagDiv(void);
    void parseTagDL(void);
    void parseTagDLEnd(void);
    void parseTagDT(void);
    void parseTagEM(void);
    void parseTagFieldset(void);
    void parseTagFont(void); 
    void parseTagForm(void); 
    void parseTagFormEnd(void); 
    void parseTagFrame(void); 
    void parseTagFrameset(void); 
    void parseTagHeader(void); 		// H1 .. H6
	void parseTagHeaderEnd(void);   // Handling of end-tag for H1 .. H6
    void parseTagHead(void); 		// HEAD
    void parseTagHR(void); 
    void parseTagI(void); 
    void parseTagIframe(void); 
    void parseTagImg(void); 
    void parseTagInput(void); 
    void parseTagIns(void); 
    void parseTagIsindex(void); 
    void parseTagKbd(void); 
    void parseTagLabel(void); 
    void parseTagLegend(void); 
    // parseTagListing: see parseTagPre
    void parseTagLi(void);
    void parseTagLink(void);
    void parseTagMap(void);
    void parseTagMeta(void);
    void parseTagNobr(void);
    void parseTagNoframes(void);
    void parseTagNoscript(void);
    void parseTagObject(void);
    void parseTagOl(void);
    void parseTagOptgroup(void);
    void parseTagOption(void);
    void parseTagOptionEnd(void);
    void parseTagP(void); 	
    void parseTagPEnd(void); 	
    void parseTagParam(void); 	
    void parseTagPre(void); 	
    void parseTagQ(void); 	
    void parseTagSamp(void); 	
    void parseTagScript(void); 	
	void parseTagSelect(void);
	void parseTagSelectEnd(void);
	void parseTagSmall(void);
	void parseTagSpan(void);
    void parseTagStrike(void); 	
    void parseTagStrong(void); 	
    void parseTagStyle(void); 	
    void parseTagSub(void); 	
    void parseTagSup(void); 	
    void parseTagTable(void); 	
    void parseTagTextarea(void); 	
    void parseTagTextareaEnd(void); 	
    void parseTagTitle(void); 	
    void parseTagTT(void); 	
    void parseTagU(void); 	
    void parseTagUL(void); 	
    void parseTagVar(void); 	
	
    /*
     * This function is used for convenience only. It inserts a vertical space
     * if this has not already been done. For example
     * <h1>Hello</h1><p>How are you ?
     * would insert a VSpace behind </h1> and one in front of <p>. This is one
     * VSpace too much. So we use 'space_inserted' to avoid this. Look at
     * 'parseBody' to see how to use this function.
     * Assign the return value to 'space_inserted'.
     */
    bool insertVSpace( bool _space_inserted );

    /*
     * The <title>...</title>.
     */
    QString title;

    /*
     * If we are in an <a href=..> ... </a> tag then the href
     * is stored in this string.
     */
    char *url;

    /*
     * If we are in an <a target=..> ... </a> tag then this points to the
     * target.
     */
    const char *target;

    /*
     * This painter is created at need, for example to draw
     * a selection or for font metrics stuff.
     */
    QPainter *painter;

    /*
     * This is the current active HTMLClue.
     */
    HTMLClue *_clue;

    /*
     * This object contains the complete text. This text is referenced
     * by HTMLText objects for example. So you may not delete
     * 'ht' until you have delete the complete tree 'clue' is
     * pointing to.
     */
    HTMLTokenizer *ht;
	
    /*
     * This is used generally for processing the contents of < ... >
     * We keep a class global instance of this object to reduce the
     * number of new/deletes performed.  If your function may be
     * called recursively, or somehow from a function using this
     * tokenizer, you should construct your own.
     */
    StringTokenizer *stringTok;

    /*
     * Selects a new font adding _relative_font_size to fontBase
     * to get the new size.
     */
    void selectFont( int _relative_font_size );

    /*
     * Selects a new font using current settings
     */
    void selectFont();

    /*
     * Makes the font specified by parameters the actual font
     */
    void selectFont( const char *_fontfamily, int _size, int _weight, bool _italic );

    /*
     * Pops the top font form the stack and makes the new
     * top font the actual one. If the stack is empty ( should never
     * happen! ) the default font is pushed on the stack.
     */
    void popFont();

    const HTMLFont *currentFont()  { return font_stack.top(); }

    void popColor();

    /*
     * The font stack. The font on top of the stack is the currently
     * used font. Poping a font from the stack deletes the font.
     * So use top() to get the actual font. There must always be at least
     * one font on the stack.
     */
    QStack<HTMLFont> font_stack;

    /*
     * The weight currently selected. This is affected by <b>..</b>
     * for example.
     */
    int weight;

    /*
     * The fonts italic flag. This is affected by <i>..</i>
     * for example.
     */
    bool italic;

    /*
     * The fonts underline flag. This is affected by <u>..</u>
     * for example.
     */
    bool underline;

    /*
     * The fonts underline flag. This is affected by <u>..</u>
     * for example.
     */
    bool strikeOut;

    /*
     * is true if are between <noframes> and </noframes>
     */
    bool inNoframes;

    /*
     * from <BASE TARGET="...">
     */
    const char *baseTarget;

    /*
     * Current text color is at the top of the stack
     */
    QStack<QColor> colorStack;


	 HTMLStackElem *blockStack; 

     void pushBlock( int _id, int _level, 
    					  blockFunc _exitFunc = 0, 
    					  int _miscData1 = 0,
    					  int _miscData2 = 0);
    					  
     void popBlock( int _id );
 
	 void freeBlock( void);
    
	 /*
	  * Code for closing-tag to restore font
	  * miscData1: bool - if true terminate current flow
	  */
    void blockEndFont(HTMLStackElem *stackElem);

	 /*
	  * Code for closing-tag to end PRE tag
	  */
    void blockEndPre(HTMLStackElem *stackElem);

	 /*
	  * Code for closing-tag to restore font and font-color
	  */
    void blockEndColorFont(HTMLStackElem *stackElem);
    
	 /*
	  * Code for closing-tag to restore indentation
	  * miscData1: int - previous indentation
	  */
    void blockEndIndent(HTMLStackElem *stackElem);

	 /*
	  * Code for closing-tag to restore alignment
	  * miscData1: int - previous alignment
	  */
    void blockEndAlign(HTMLStackElem *stackElem);

	 /*
	  * Code for remove item from listStack and restore indentation
	  * miscData1: int - previous indentation
	  * miscData2: bool - if true insert vspace
	  */
    void blockEndList(HTMLStackElem *stackElem);

	 /*
	  * Code for close frameset
	  * miscData1: HTMLFrameSet * - previous frameSet
	  */
    void blockEndFrameSet(HTMLStackElem *stackElem);

	 /*
	  * Code for end of form
	  */
    void blockEndForm(HTMLStackElem *stackElem);

	 /*
	  * Code for end of textarea
	  */
    void blockEndTextarea(HTMLStackElem *stackElem);

	 /*
	  * Code for end of title
	  */
    void blockEndTitle(HTMLStackElem *stackElem);

    /*
     * Have we parsed <body> yet?
     */
    bool bodyParsed;

    /*
     * size of current indenting
     */
    int indent;

    class HTMLList
    {
	public:
	    HTMLList( ListType t, ListNumType nt = Numeric )
		{ type = t; numType = nt; itemNumber = 1; }
	    ListType type;
	    ListNumType numType;
	    int itemNumber;
    };

    /*
     * Stack of lists currently active.
     * The top affects whether a bullet or numbering is used by <li>
     */
    QStack<HTMLList> listStack;

    enum GlossaryEntry { GlossaryDL, GlossaryDD };

    /*
     * Stack of glossary entries currently active
     */
    QStack<GlossaryEntry> glossaryStack;

    /*
     * The current alignment, set by <DIV > or <CENTER>
     */
    HTMLClue::HAlign divAlign;

    /*
     * Number of tokens parsed in the current time-slice
     */
    int parseCount;
    int granularity;

    /*
     * Used to avoid inserting multiple vspaces
     */
    bool vspace_inserted;

    /*
     * The current flow box to add objects to
     */
    HTMLClue *flow;

    /*
     * Array of TAG functions, e.g.
     * <img ...  is processed by tagJumpTable[ID_IMG]
     */
    static tagFunc tagJumpTable[ID_MAX+1];

    /*
     * Current fontbase, colors, etc.
     */
    HTMLSettings *settings;

    // true if the current text is destined for <title>
    bool inTitle;

    // Current form
    HTMLForm *form;

    // Current select form element
    HTMLSelect *formSelect;

    // true if the current text is destined for a <SELECT><OPTION>
    bool inOption;

    // Current textarea form element
    HTMLTextArea *formTextArea;

    // true if the current text is destined for a <TEXTAREA>...</TEXTAREA>
    bool inTextArea;

    // true if the current text is destined for a <PRE>...</PRE>
    bool inPre;

    // the text to be put in a form element
    QString formText;

    /*
     * The toplevel frame set if we have frames otherwise 0L.
     */
    HTMLFrameSet *frameSet;

    /*
     * Stack of framesets used during parsing.
     */
    QList<HTMLFrameSet> framesetStack;

    /*
     * Holds the amount of pixel for the left border. This variable is used
     * to implement the
     * <tt>&lt;frame marginwidth=... &gt;</tt> tag.
     *
     * @see #rightBorder
     * @see #setMarginWidth
     */
    int leftBorder;

    /*
     * Holds the amount of pixel for the right border. This variable is used
     * to implement the
     * <tt>&lt;frame marginwidth=... &gt;</tt> tag.
     *
     * @see #leftBorder
     * @see #setMarginWidth
     */
    int rightBorder;

    /*
     * Holds the amount of pixel for the top border. This variable is used
     * to implement the
     * <tt>&lt;frame marginheight=... &gt;</tt> tag.
     *
     * @see #bottomBorder
     * @see #setMarginHeight
     */
    int topBorder;

    /*
     * Holds the amount of pixel for the bottom border. This variable is used
     * to implement the
     * <tt>&lt;frame marginheight=... &gt;</tt> tag.
     *
     * @see #setMarginHeight
     * @see #topBorder
     */
    int bottomBorder;


    KCharsetConverter *charsetConverter;

};

#endif // HTMLPARSER_H

