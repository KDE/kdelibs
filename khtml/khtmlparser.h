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
#include <qstrlist.h>
#include <qlist.h>
#include <qstring.h>
#include <qbuffer.h>

#include "kurl.h"
#include "khtmlclue.h"
#include "khtmltags.h"
#include "khtmlstyle.h"

//
// External Classes
//
///////////////////


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

// khtmlobj.h
class HTMLAllocator;

//
// Internal Classes
//
///////////////////

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
    			 HTMLSettings *_settings,
    			 QStrList *_formData,
    			 HTMLAllocator *_allocator);
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
    uint parseBody( HTMLClue *_clue, const uint *_end, bool toplevel = FALSE );

protected:
	KHTMLWidget *HTMLWidget;

    enum ListNumType { Numeric = 0, LowAlpha, UpAlpha, LowRoman, UpRoman };
    enum ListType { Unordered, UnorderedPlain, Ordered, Menu, Dir };

    /*
     * This function creates a new flow adds it to '_clue' and sets 'flow'
     */
    void newFlow();
    
    void insertText(HTMLString str, const HTMLFont * fp);

    /*
     * tagID
     *
     * The ID of the tag currently being parsed
     */
    uint tagID;  

    /*
     * Parses the tag set in tagID with options tagOptions
     */
    void parseOneToken();

    /*
     * Approx. each tag has its own function
     */
    void parseTagEnd();			// Generic end-tag function

    void parseTagA();			
    void parseTagAbbr();		
    void parseTagAcronym();		
    void parseTagAddress();
    void parseTagApplet();
    void parseTagArea();
    void parseTagB();
    void parseTagBase();
    void parseTagBaseFont();
    void parseTagBdo();
    void parseTagBig();
    void parseTagBlockQuote();
    void parseTagBody(); 
    void parseTagBr();
    void parseTagButton();
    void parseTagCenter();
    void parseTagCite();
    void parseTagCode();
    void parseTagDD();
    void parseTagDel();
    void parseTagDfn();
    void parseTagDiv();
    void parseTagDL();
    void parseTagDLEnd();
    void parseTagDT();
    void parseTagEM();
    void parseTagEmbed();
    void parseTagFieldset();
    void parseTagFont(); 
    void parseTagForm(); 
    void parseTagFormEnd(); 
    void parseTagFrame(); 
    void parseTagFrameset(); 
    void parseTagHeader(); 		// H1 .. H6
	void parseTagHeaderEnd();   // Handling of end-tag for H1 .. H6
    void parseTagHead(); 		// HEAD
    void parseTagHR(); 
    void parseTagI(); 
    void parseTagIframe(); 
    void parseTagImg(); 
    void parseTagInput(); 
    void parseTagIns(); 
    void parseTagIsindex(); 
    void parseTagKbd(); 
    void parseTagLabel(); 
    void parseTagLegend(); 
    // parseTagListing: see parseTagPre
    void parseTagLi();
    void parseTagLink();
    void parseTagMap();
    void parseTagMeta();
    void parseTagNobr();
    void parseTagNoframes();
    void parseTagNoscript();
    void parseTagObject();
    void parseTagOl();
    void parseTagOptgroup();
    void parseTagOption();
    void parseTagOptionEnd();
    void parseTagP(); 	
    void parseTagPEnd(); 	
    void parseTagParam(); 	
    void parseTagPre(); 	
    void parseTagQ(); 	
    void parseTagSamp(); 	
    void parseTagScript(); 	
	void parseTagSelect();
	void parseTagSelectEnd();
	void parseTagSmall();
	void parseTagSpan();
    void parseTagStrike(); 	
    void parseTagStrong(); 	
    void parseTagStyle(); 	
    void parseTagSub(); 	
    void parseTagSup(); 	
    void parseTagTable(); 	
    void parseTagTextarea(); 	
    void parseTagTextareaEnd(); 	
    void parseTagTitle(); 	
    void parseTagTT(); 	
    void parseTagU(); 	
    void parseTagUL(); 	
    void parseTagVar(); 	
    void parsePlain(); 	

    bool plainText;
	
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
    HTMLString url;

    /*
     * If we are in an <a target=..> ... </a> tag then this points to the
     * target.
     */
    HTMLString target;

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
     * Current style sheet
     */
    CSSStyleSheet *styleSheet; 

    /*
     * Current style
     */
    CSSStyle *currentStyle; 

    /*
     * Set new font based on currentStyle
     */
    void setFont();

    /*
     * Restore font based on info in currentStyle
     */
    void restoreFont();
     
    const HTMLFont *currentFont()  { return currentStyle->font.fp; }

    /*
     * Current fontbase, colors, etc.
     */
    HTMLSettings *settings;

    /*
     * is true if are between <noframes> and </noframes>
     */
    bool inNoframes;

    /*
     * from <BASE TARGET="...">
     */
    HTMLString baseTarget;


    HTMLStackElem *blockStack; 

    void pushBlock( int _id, int _level, 
		    blockFunc _exitFunc = 0, 
		    int _miscData1 = 0);
    					  
    void popBlock( int _id );
 
    void freeBlock( void);
    
    /*
     * Code for closing-tag to restore block
     */
    void blockEnd(HTMLStackElem *stackElem);

    /*
     * Code for closing-tag to close anchor
     */
    void blockEndAnchor(HTMLStackElem *stackElem);

    /*
     * Code for closing-tag to end PRE tag
     */
    void blockEndPre(HTMLStackElem *stackElem);

	 /*
	  * Code for closing-tag to restore alignment
	  * miscData1: int - previous alignment
	  */
    void blockEndAlign(HTMLStackElem *stackElem);

	 /*
	  * Code for remove item from listStack and restore indentation
	  * miscData1: bool - if true insert vspace
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

    /*
     * Data to fill forms with.
     */
    QStrList *formData;

    /* 
     * Memory allocator
     */
    HTMLAllocator *allocator;
};

#endif // HTMLPARSER_H

