/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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


#ifdef GrayScale
#undef GrayScale
#endif

#ifdef Color
#undef Color
#endif

#include "khtmlparser.h"

#include "khtmltags.h"
#include "khtmldata.h"
#include "khtmltoken.h"
#include "khtmlchain.h"
#include "khtmlfont.h"
#include "khtmlobj.h"
#include "khtmltable.h"
#include "khtmlform.h"
#include "khtml.h"
#include "khtmlframe.h"
#include "khtmljscript.h"
#include "khtmlview.h"
#include "khtmlattrs.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <qobject.h>
#include <qimage.h>
#include <qregexp.h>
#include <qkeycode.h>
#include <qprinter.h>
#include <qdrawutil.h>

#include <kurl.h>
#include <kapp.h>

#include <X11/Xlib.h>

// Debug function
void debugM( const char *msg , ...);

#define INDENT_SIZE		30

#define NEW new(allocator)
#define NEWSTRING(x) (allocator->newString(x))

static QChar spaceChar[] = { ' ' };
static HTMLString space( spaceChar, 1 );

//----------------------------------------------------------------------------
// convert number to roman numerals
QString toRoman( int number, bool upper )
{
    QString roman;
    QChar ldigits[] = { 'i', 'v', 'x', 'l', 'c', 'd', 'm' };
    QChar udigits[] = { 'I', 'V', 'X', 'L', 'C', 'D', 'M' };
    QChar *digits = upper ? udigits : ldigits;
    int i, d = 0;

    do
    {   
	int num = number % 10;

	if ( num % 5 < 4 )
	    for ( i = num % 5; i > 0; i-- )
		roman.insert( 0, digits[ d ] );

	if ( num >= 4 && num <= 8)
	    roman.insert( 0, digits[ d+1 ] );

	if ( num == 9 )
	    roman.insert( 0, digits[ d+2 ] );

	if ( num % 5 == 4 )
	    roman.insert( 0, digits[ d ] );

	number /= 10;
	d += 2;
    }
    while ( number );

    return roman;
}

void setNamedColor(QColor &color, const QString name)
{
    bool ok;
    // also recognize "color=ffffff"
    if (name[0] != QChar('#') && name.length() == 6 &&
        name.toInt(&ok, 16) )
    {
        QString col("#");
        col += name;
        color.setNamedColor(col);
    }
    else
    {
        color.setNamedColor(name);
    }   
}

//----------------------------------------------------------------------------

//
// !WARNING!
//
// This is a jump-table index by tagID. If you add new tags to khtmltags.in
// this means that khtmltags.h will change. You must ensure that the order
// of tag-IDs in khtmltags.h corresponds to the order of the tags in the
// table below.
//

tagFunc KHTMLParser::tagJumpTable[ID_MAX+1] =
{
        0,              0,		// (NULL) / (NULL)
&KHTMLParser::parseTagA,	&KHTMLParser::parseTagEnd,	// ID_A
&KHTMLParser::parseTagAbbr,		0,			// ID_ABBR
&KHTMLParser::parseTagAcronym,		0,			// ID_ACRONYM
&KHTMLParser::parseTagAddress,	&KHTMLParser::parseTagEnd,	// ID_ADDRESS
&KHTMLParser::parseTagApplet,		0,			// ID_APPLET
&KHTMLParser::parseTagArea,		0,			// ID_AREA
&KHTMLParser::parseTagB,	&KHTMLParser::parseTagEnd,	// ID_B
&KHTMLParser::parseTagBase,		0,			// ID_BASE
&KHTMLParser::parseTagBaseFont,		0,			// ID_BASEFONT
&KHTMLParser::parseTagBdo,		0,			// ID_BDO
&KHTMLParser::parseTagBig,	&KHTMLParser::parseTagEnd,	// ID_BIG
&KHTMLParser::parseTagBlockQuote,&KHTMLParser::parseTagEnd,	// ID_BLOCKQUOTE
&KHTMLParser::parseTagBody,		0,			// ID_BODY
&KHTMLParser::parseTagBr,		0,			// ID_BR
&KHTMLParser::parseTagButton,		0,			// ID_BUTTON
	0,		0,	// ID_CAPTION ( Tables only )
&KHTMLParser::parseTagCenter,	&KHTMLParser::parseTagEnd,	// ID_CENTER
&KHTMLParser::parseTagCite,	&KHTMLParser::parseTagEnd,	// ID_CITE
&KHTMLParser::parseTagCode,	&KHTMLParser::parseTagEnd,	// ID_CODE
	0,		0,	// ID_COL ( Tables only )
	0,		0,	// ID_COLGROUP ( Tables only )
&KHTMLParser::parseTagDD,		0,			// ID_DD
&KHTMLParser::parseTagDel,		0,			// ID_DEL
&KHTMLParser::parseTagDfn,		0,			// ID_DFN
&KHTMLParser::parseTagUL,	&KHTMLParser::parseTagEnd,	// ID_DIR
&KHTMLParser::parseTagDiv,	&KHTMLParser::parseTagEnd,	// ID_DIV
&KHTMLParser::parseTagDL,	&KHTMLParser::parseTagDLEnd,	// ID_DL
&KHTMLParser::parseTagDT,		0,			// ID_DT
&KHTMLParser::parseTagEM,	&KHTMLParser::parseTagEnd,	// ID_EM
&KHTMLParser::parseTagEmbed,            0,                      // ID_EMBED
&KHTMLParser::parseTagFieldset,		0,			// ID_FIELDSET
&KHTMLParser::parseTagFont,	&KHTMLParser::parseTagEnd, 	// ID_FONT
&KHTMLParser::parseTagForm,	&KHTMLParser::parseTagFormEnd,	// ID_FORM
&KHTMLParser::parseTagFrame,		0,		// ID_FRAME
&KHTMLParser::parseTagFrameset,	&KHTMLParser::parseTagEnd,	// ID_FRAMESET
&KHTMLParser::parseTagHeader,	&KHTMLParser::parseTagHeaderEnd, // ID_H1
&KHTMLParser::parseTagHeader,	&KHTMLParser::parseTagHeaderEnd, // ID_H2
&KHTMLParser::parseTagHeader,	&KHTMLParser::parseTagHeaderEnd, // ID_H3
&KHTMLParser::parseTagHeader,	&KHTMLParser::parseTagHeaderEnd, // ID_H4
&KHTMLParser::parseTagHeader,	&KHTMLParser::parseTagHeaderEnd, // ID_H5
&KHTMLParser::parseTagHeader,	&KHTMLParser::parseTagHeaderEnd, // ID_H6
&KHTMLParser::parseTagHead,		0,			// ID_HEAD
&KHTMLParser::parseTagHR,		0,			// ID_HR
	0,		                0,			// ID_HTML
&KHTMLParser::parseTagI,	&KHTMLParser::parseTagEnd,	// ID_I
&KHTMLParser::parseTagIframe,		0,			// ID_IFRAME
&KHTMLParser::parseTagImg,		0,			// ID_IMG
&KHTMLParser::parseTagInput,		0,			// ID_INPUT
&KHTMLParser::parseTagIns,		0,			// ID_INS
&KHTMLParser::parseTagIsindex,		0,			// ID_ISINDEX
&KHTMLParser::parseTagKbd,	&KHTMLParser::parseTagEnd,	// ID_KBD
&KHTMLParser::parseTagLabel,		0,			// ID_LABEL
&KHTMLParser::parseTagLegend,		0,			// ID_LEGEND
&KHTMLParser::parseTagLi,		0,			// ID_LI
&KHTMLParser::parseTagPre,	&KHTMLParser::parseTagEnd,	// ID_LISTING
&KHTMLParser::parseTagLink,		0,			// ID_LINK
&KHTMLParser::parseTagMap,		0,			// ID_MAP
&KHTMLParser::parseTagUL,	&KHTMLParser::parseTagEnd,	// ID_MENU
&KHTMLParser::parseTagMeta,		0,			// ID_META
&KHTMLParser::parseTagNobr,		0,			// ID_NOBR
&KHTMLParser::parseTagNoframes,		0,			// ID_NOFRAMES
&KHTMLParser::parseTagNoscript,		0,			// ID_NOSCRIPT
&KHTMLParser::parseTagObject,		0,			// ID_OBJECT
&KHTMLParser::parseTagOl,	&KHTMLParser::parseTagEnd,	// ID_OL
&KHTMLParser::parseTagOptgroup,		0,			// ID_OPTGROUP
&KHTMLParser::parseTagOption,	&KHTMLParser::parseTagOptionEnd,// ID_OPTION
&KHTMLParser::parseTagP,	&KHTMLParser::parseTagPEnd,	// ID_P
&KHTMLParser::parseTagParam,		0,			// ID_PARAM
&KHTMLParser::parsePlain,               0,                      // ID_PLAIN
&KHTMLParser::parseTagPre,	&KHTMLParser::parseTagEnd,	// ID_PRE
&KHTMLParser::parseTagQ,	&KHTMLParser::parseTagEnd,	// ID_Q
&KHTMLParser::parseTagStrike,	&KHTMLParser::parseTagEnd,	// ID_S
&KHTMLParser::parseTagSamp,	&KHTMLParser::parseTagEnd,	// ID_SAMP
&KHTMLParser::parseTagScript,		0,			// ID_SCRIPT
&KHTMLParser::parseTagSelect,	&KHTMLParser::parseTagSelectEnd,// ID_SELECT
&KHTMLParser::parseTagSmall,	&KHTMLParser::parseTagEnd,	// ID_SMALL
&KHTMLParser::parseTagSpan,		0,			// ID_SPAN
&KHTMLParser::parseTagStrike,	&KHTMLParser::parseTagEnd,	// ID_STRIKE
&KHTMLParser::parseTagStrong,	&KHTMLParser::parseTagEnd,	// ID_STRONG
&KHTMLParser::parseTagStyle,		0,			// ID_STYLE
&KHTMLParser::parseTagSub,		0,			// ID_SUB
&KHTMLParser::parseTagSup,		0,			// ID_SUP
&KHTMLParser::parseTagTable,		0,			// ID_TABLE
	0,		0,		// ID_TBODY (Tables only)
	0,		0,		// ID_TD (Tables only)
&KHTMLParser::parseTagTextarea,	&KHTMLParser::parseTagEnd,	// ID_TEXTAREA
	0,		0,		// ID_TFOOT (Tables only)
	0,		0,		// ID_TH (Tables only)
	0,		0,		// ID_THEAD (Tables only)
&KHTMLParser::parseTagTitle,	&KHTMLParser::parseTagEnd, 	// ID_TITLE
	0,		0,		// ID_TR (Tables only)
&KHTMLParser::parseTagTT,	&KHTMLParser::parseTagEnd,	// ID_TT
&KHTMLParser::parseTagU,	&KHTMLParser::parseTagEnd,	// ID_U
&KHTMLParser::parseTagUL,	&KHTMLParser::parseTagEnd,	// ID_UL
&KHTMLParser::parseTagVar,	&KHTMLParser::parseTagEnd,	// ID_VAR
};


KHTMLParser::KHTMLParser( KHTMLWidget *_parent,
			  HTMLTokenizer *_ht, 
			  QPainter *_painter,
			  HTMLSettings *_settings,
			  QStrList *_formData,
			  HTMLAllocator *_allocator) 			
{
    printf("parser constructor\n");

    HTMLWidget    = _parent;
    ht            = _ht;
    painter       = _painter;
    settings      = _settings;
    formData      = _formData;
    allocator     = _allocator;
    if (formData)
    {
        (void) formData->first();
    }

    leftBorder    = LEFT_BORDER;
    rightBorder   = RIGHT_BORDER;
    topBorder     = TOP_BORDER;
    bottomBorder  = BOTTOM_BORDER;

    listStack.setAutoDelete( true );
    glossaryStack.setAutoDelete( true );

    divAlign = HTMLClue::Left;

    if (!pFontManager)
    	pFontManager = new HTMLFontManager();

    // Style stuff
    styleSheet = new CSSStyleSheet(_settings);

#ifdef CSS_TEST
    styleSheet->test();
#endif
    
    currentStyle = styleSheet->newStyle(NULL);

    setFont();

    // reset form related stuff
    form = 0;
    formSelect = 0;
    formTextArea = 0;
    inOption = false;
    inTextArea = false;
    inPre = false;
    inTitle = false;
    bodyParsed = false;
    plainText = false;

    baseTarget = HTMLString();
    target = HTMLString();
    url = HTMLString();
    inNoframes = false;
    background = 0;

    flow = 0;
    frameSet = 0;

    listStack.clear();
    glossaryStack.clear();
    
    blockStack = 0;

    parseCount = 0;
    granularity = 1000;

    vspace_inserted = true;

    stringTok = new StringTokenizer;

    // move to the first token
    ht->first();
}

KHTMLParser::~KHTMLParser()
{
    delete styleSheet;

    if ( stringTok )
	delete stringTok;

    freeBlock();
}

// Set font according to currentStyle
void KHTMLParser::setFont(void)
{
    int fontsize = currentStyle->font.size;
    if ( fontsize < 0 )
	fontsize = 0;
    else if ( fontsize >= MAXFONTSIZES )
	fontsize = MAXFONTSIZES - 1;
    
    currentStyle->font.size = fontsize;

    HTMLFont f( currentStyle->font.family, 
                fontsize, 
                currentStyle->font.fixed ? settings->fixedFontSizes : settings->fontSizes,
                currentStyle->font.weight / 10, 
                (currentStyle->font.style != CSSStyleFont::stNormal), 
                settings->charset );
    f.setTextColor( currentStyle->font.color );
    f.setUnderline( currentStyle->font.decoration == CSSStyleFont::decUnderline );
    f.setStrikeOut( currentStyle->font.decoration == CSSStyleFont::decLineThrough );

    const HTMLFont *fp = pFontManager->getFont( f );

    currentStyle->font.fp = fp;
    painter->setFont( *fp );
}

void KHTMLParser::restoreFont(void)
{
    painter->setFont( *(currentStyle->font.fp) );
}

void KHTMLParser::pushBlock(int _id, int _level,
                            blockFunc _exitFunc, 
                            int _miscData1)
{
    HTMLStackElem *Elem = new HTMLStackElem(_id, _level, currentStyle, 
    					    _exitFunc, _miscData1, 
    					    blockStack);

    currentStyle = styleSheet->newStyle(currentStyle);    					    
    blockStack = Elem;
}    					     

void KHTMLParser::popBlock( int _id)
{
    HTMLStackElem *Elem = blockStack;
    int maxLevel = 0;
    
    while( (Elem != 0) && (Elem->id != _id))
    {
    	if (maxLevel < Elem->level)
    	{
    	    maxLevel = Elem->level;
    	}
    	Elem = Elem->next;
    }
    if (Elem == 0)
    {
	return;
    }
    
    if (maxLevel > Elem->level)
    {
    	return;
    }    	   

    Elem = blockStack;

    while (Elem)
    {
    	HTMLStackElem *tmp = Elem;		

    	if (Elem->exitFunc != 0)
    		(this->*(Elem->exitFunc))( Elem );
    		
    	if (Elem->style)
    	{
    	    delete currentStyle;
    	    currentStyle = Elem->style;
    	    restoreFont();
    	}
    		
    	if (Elem->id == _id)
    	{
    	    blockStack = Elem->next;
    	    Elem = 0;
    	}
    	else
    	{
    	    Elem = Elem->next;
    	} 
	delete tmp;
    }    
    
}
                    
void KHTMLParser::freeBlock()
{
    HTMLStackElem *Elem = blockStack; 
    while (Elem != 0)
    {
    	HTMLStackElem *tmp = Elem;
    	Elem = Elem->next;
    	delete tmp;
    }
    blockStack = 0;
}

void KHTMLParser::blockEnd( HTMLStackElem *)
{
    vspace_inserted = insertVSpace( vspace_inserted );
    flow = 0;
}

void KHTMLParser::blockEndAnchor( HTMLStackElem *)
{  
    url = HTMLString();
    target = HTMLString();
}

void KHTMLParser::blockEndPre( HTMLStackElem *)
{
    // We add a hidden space to get the height
    // If there is no flow box, we add one.
    if (!flow)
    	newFlow();
   
    flow->append(NEW HTMLHSpace( currentFont(), painter, true ));

    vspace_inserted = insertVSpace( vspace_inserted );
    flow = 0;
    inPre = false;
}

void KHTMLParser::blockEndAlign( HTMLStackElem *Elem)
{
    divAlign = (HTMLClue::HAlign) Elem->miscData1;
    flow = 0;
}

void KHTMLParser::blockEndList( HTMLStackElem *Elem)
{
    if (Elem->miscData1)
    {
	vspace_inserted = insertVSpace( vspace_inserted );
    }
    if ( !listStack.remove() )
    {
    	fprintf(stderr, "%s : List stack corrupt!\n", __FILE__);
    }
    
    flow = 0;
}

void KHTMLParser::blockEndFrameSet( HTMLStackElem *Elem)
{
    HTMLFrameSet *oldFrameSet = (HTMLFrameSet *) Elem->miscData1;
    
    if (!oldFrameSet)
    {
    	// Toplevel frame set
    	HTMLWidget->showFrameSet( frameSet);
    }
    frameSet = oldFrameSet;
}

void KHTMLParser::blockEndForm( HTMLStackElem *)
{
    vspace_inserted = insertVSpace( vspace_inserted );
    flow = 0;
    form = 0;
}

void KHTMLParser::blockEndTextarea( HTMLStackElem *)
{
    formTextArea->setText( formText );
    inTextArea = false;
    vspace_inserted = false;
    formTextArea = 0;
}

void KHTMLParser::blockEndTitle( HTMLStackElem *)
{
    HTMLWidget->setNewTitle( title.data() );
    inTitle = false;	
}

bool KHTMLParser::insertVSpace( bool _vspace_inserted )
{
    if ( !_vspace_inserted )
    {
	HTMLClueFlow *f = NEW HTMLClueFlow();
	_clue->append( f );
	HTMLVSpace *t = NEW HTMLVSpace( settings->fontSizes[settings->fontBaseSize] );
	f->append( t );
	flow = 0;
    }
    
    return true;
}

void KHTMLParser::newFlow()
{
    if (inPre)
         flow = NEW HTMLClueH();
	    else
         flow = NEW HTMLClueFlow();

    flow->setIndent( currentStyle->text.indent );
    flow->setHAlign( divAlign );
    _clue->append( flow );
}


// Function insertText
// ====
// This function inserts text in the flow. It decides whether to use
// HTMLText or HTMLTextMaster objects for the text.
// 
// HTMLText is used if the text may not be broken.
// HTMLTextMaster is used if the text contains (breaking) spaces.
//
// This function converts non-breaking spaces to normal spaces since
// non-breaking spaces aren't shown correctly for all fonts.
//
// The hard part is to make several objects if the text contains both
// normal spaces and non-breaking spaces.
void KHTMLParser::insertText(HTMLString str, const HTMLFont * fp)
{
    enum { unknown, fixed, variable} textType = unknown; 
    	// Flag, indicating whether text may be broken
    uint i = 0;
    HTMLString remainingStr;
    bool insertSpace = false;
    bool insertNBSP = false;
    bool insertBlock = false;
    
    for(;;)
    {
        if (str[i] == QChar(0xa0))
        {
            // Non-breaking space
            if (textType == variable)
            {
                // We have a non-breaking space in a block of variable text
                // We need to split the text and insert a seperate
                // non-breaking space object
                //str[i] = 0x00; // End of string
                remainingStr = str + (i+1);
		str.setLength(i); // truncate here...
                insertBlock = true; 
                insertNBSP = true;
            }
            else
            {
                // We have a non-breaking space: this makes the block fixed.
                str[i] = 0x20; // Normal space
                textType = fixed;
            }
        }
        else if (str[i] == QChar(0x20))
        {
            // Normal space
            if (textType == fixed)
            {
            	// We have a normal space in a block of fixed text.
            	// We need to split the text and insert a seperate normal
            	// space.
            	//str[i] = 0x00; // End of string
            	remainingStr = str + (i+1);
		str.setLength(i);
            	insertBlock = true;
            	insertSpace = true;
            }
            else
            {
            	// We have a normal space: if this is the first character
            	// we insert a normal space and continue
            	if (i == 0)
            	{
            	    if (str.length() == 1)
            	    {
            	    	++str;
            	    	remainingStr = HTMLString();
            	    }
            	    else
            	    {
            	        remainingStr = str+1;
			str.setLength(0);
            	    }
                    insertBlock = true; // Block is zero-length, no actual insertion
            	    insertSpace = true;
            	}
            	else if (i == str.length())
            	{
            	    // Last character is a space: Insert the block and 
            	    // a normal space
            	    remainingStr = HTMLString();
		    str.setLength(i-1);
            	    insertBlock = true;
            	    insertSpace = true;
            	}
            	else
            	{
            	    textType = variable;
            	}
            }
        } 
        else if (str[i] == QChar(0x00))
        {
            // End of string
            insertBlock = true;
            remainingStr = HTMLString();
        }
        
        if (insertBlock)
        {
            if (str.length())
            {
                if (textType == variable)
                {
		    if ( url.length() || target.length() )
	       		flow->append( NEW HTMLLinkTextMaster( str, 
	       			      fp, painter, url, target) );
	            else
		       	flow->append( NEW HTMLTextMaster( str, fp, painter ) );
                }
                else
                {
	            if ( url.length() || target.length() )
	       	        flow->append( NEW HTMLLinkText( str, 
	       	                      fp, painter, url, target ) );
		    else
		        flow->append( NEW HTMLText( str, fp, painter ) );
		}
            }
            if (insertSpace)
            {
                if ( url.length() || target.length())
                {
		    HTMLLinkText *sp = NEW HTMLLinkText( space, fp, painter,
							 url, target );
		    sp->setSeparator( true );
		    flow->append( sp );
		}
                else
                {
   	            flow->append( NEW HTMLHSpace( fp, painter));
   	        }
            }
            else if (insertNBSP)
            {
                if ( url.length() || target.length())
                {
		    HTMLLinkText *sp = NEW HTMLLinkText( space, fp, painter,
			url, target );
		    sp->setSeparator( false );
		    flow->append( sp );
		}
                else
                {
                    HTMLHSpace *sp = NEW HTMLHSpace( fp, painter);
                    sp->setSeparator(false);
   	            flow->append( sp );
   	        }
            }
            str = remainingStr;
            if (!str.length())
               return; // Finished
	    i = 0;
	    textType = unknown;
	    insertBlock = false;
	    insertSpace = false;
	    insertNBSP = false;
        }
        else
        {
            i++;
        }       
    }
}                                         

uint KHTMLParser::parseBody( HTMLClue *__clue, const uint *_end, bool toplevel )
{
    HTMLString token;
    
    // Before one can write any objects in the body he usually adds a FlowBox
    // to _clue. Then put all texts, images etc. in the FlowBox.
    // If f == 0, you have to create a new FlowBox, otherwise you can
    // use the one stored in f. Setting f to 0 means closing the FlowBox.

    _clue = __clue;

    if ( toplevel)
    {
      if ( !background )
	{
	  background = NEW HTMLBackground( HTMLWidget, settings->bgColor);
	  background->setBorder( leftBorder, rightBorder, topBorder, bottomBorder );
	  _clue->append( background );
	}

      parseCount = granularity;
      // Be sure to set the painter to the current font.
      restoreFont();
    }
    if (plainText)
    {
       parsePlain();
       tagID = 0;
       return (tagID);
    }

    while ( ht->hasMoreTokens() )
    {
	token = ht->nextToken();
#ifdef TOKEN_DEBUG
	printf("parsing token:");
	printf(" %x %d %s\n", token[0].unicode(), token[1].unicode(), token.string().latin1());
#endif	

	// ignore stuff inbetween <noframes> and </noframes> if
	// this has a htmlView
	if(inNoframes)
	{
	    if (token[0] != QChar(TAG_ESCAPE))
	        continue;
            tagID = token[1].unicode();
	    if (tagID == (ID_NOFRAMES + ID_CLOSE_TAG))
		inNoframes = false;
	    
	    continue;
	}

	if ( token[0] == QChar::null )
	    continue;

	if ( token[0] == QChar(' ') && token[1] == QChar::null )
	{
	    // if in* is set this text belongs in a form element
	    if ( inOption || inTextArea )
		formText += " ";
	    else if ( inTitle )
		title += " ";
	    else if ( flow != 0)
	    {
	    	insertText( space, currentFont());
	    }
	}
	else if ( token[0] != QChar(TAG_ESCAPE) )
	{
	    // if in* is set this text belongs in a form element
	    if ( inOption || inTextArea )
	    {
		formText += token;
	    }
	    else if ( inTitle )
	    {
		title += token;
	    }
	    else
	    {
		vspace_inserted = false;

	    	if (!flow)
	            newFlow();
		
		insertText(token, currentFont() ); // Override constness
	    }
	}
	else
	{
	    ++token;
	    tagID = token[0].unicode();
	    ++token;

            for(int j=0; _end[j]; j++)
            {
	      if (_end[j] == tagID)
	      {
	          return (tagID);
              }
            }
	    // The tag used for line break when we are in <pre>...</pre>
	    if ( tagID == ID_NEWLINE )
	    {
		if (!flow)
		    newFlow();

		// Add a hidden space to get the line-height right.
		flow->append(NEW HTMLHSpace( currentFont(), painter, true ));
		vspace_inserted = false;
	
		newFlow(); // Explicitly make a new flow! 
	    }
	    else
	    {
		parseOneToken();
	    }
	}

	if ( toplevel )
	{
	    if ( parseCount <= 0 )
	    {
	        tagID = 0;
                return (tagID);
            }
	}
	parseCount--;
    }

    tagID = 0;
    return (tagID);
}


void KHTMLParser::parsePlain()
{
    HTMLString token;

    if (!plainText)
    { 
       plainText = true;
       vspace_inserted = insertVSpace( vspace_inserted );

       pushBlock(tagID, 2, &KHTMLParser::blockEndPre);

       currentStyle->font.family = settings->fixedFontFace;
       currentStyle->font.fixed = true;
       currentStyle->font.style = CSSStyleFont::stNormal;
       currentStyle->font.weight = CSSStyleFont::Normal;
       setFont();    

       flow = 0;
    } 
    
    while ( ht->hasMoreTokens() )
    {
	token = ht->nextToken();
#ifdef TOKEN_DEBUG
	printf("parsing token:");
	printf(" %x %d %s\n", token[0].unicode(), token[1].unicode(), token.string().latin1());
#endif	

	if ( token[0] == QChar::null )
	    continue;

	if ( token[0] != QChar(TAG_ESCAPE) )
	{
	    _clue->append( NEW HTMLText( token, currentFont(), painter ));
	}
    }
}


void KHTMLParser::parseOneToken()
{
    if (tagID > ID_MAX)
    {
      printf("Unknown tag!! tagID = %d\n", tagID);
      return;
    }
    tagFunc func = tagJumpTable[tagID];
    if (func)
        (this->*(func))();
}


void KHTMLParser::parseTagEnd(void)
{
    popBlock(tagID - ID_CLOSE_TAG);
}

void KHTMLParser::parseTagA(void)
{
    // implemented attributes:
    // href, name, target
    //
    // unimpl:
    // charset, type, hreflang, rel, rev, accesskey, shape, coords,
    // tabindex, onfocus, onblur

    popBlock(ID_A); // Close still open tags.
    
    QString tmpurl;
    target = HTMLString();
    bool visited = false;
    const Attribute *p;

    while ( ( p = ht->nextOption() ) != 0 )
    {
	if ( p->id == ATTR_HREF )
	{
	    if ( p->value()[0] == '#' )
	    {// reference
		KURL u( HTMLWidget->getDocumentURL() );
		u.setRef( p->value() );
	        tmpurl = u.url();
	    }
	    else
	    {
                KURL u( HTMLWidget->getBaseURL(), p->value() );
	        tmpurl = u.url();
	    }		
            visited = HTMLWidget->URLVisited( tmpurl ); 
	}
	else if ( p->id == ATTR_NAME )
	{
	    if ( flow == 0 )
	        _clue->append( NEW HTMLAnchor( p->value() ) );
	    else
	        flow->append( NEW HTMLAnchor( p->value() ) );
	}
	else if ( p->id == ATTR_TARGET )
	{
	    target = p->value();
	}
    }

    if ( !target.length() )
        target = baseTarget;
	
    if ( !tmpurl.isEmpty() )
    {
        vspace_inserted = false;
        pushBlock(tagID, 2, &KHTMLParser::blockEndAnchor);
	if ( visited )
	    currentStyle->font.color = settings->vLinkColor;
	else
	    currentStyle->font.color = settings->linkColor;
	if ( settings->underlineLinks )
	    currentStyle->font.decoration = CSSStyleFont::decUnderline;
	setFont();
        url = NEWSTRING( tmpurl );
        HTMLWidget->addHref( tmpurl ); 
    }
}

void KHTMLParser::parseTagAbbr(void)
{
    // attributes: %attrs
}

void KHTMLParser::parseTagAcronym(void)
{
    // attributes: %attrs
}

void KHTMLParser::parseTagAddress(void)
{
    pushBlock(tagID, 2, &KHTMLParser::blockEnd);
    currentStyle->font.style = CSSStyleFont::stItalic;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();
    flow = 0;
}

void KHTMLParser::parseTagApplet(void)
{
    // java stuff... ignore for the moment
}

void KHTMLParser::parseTagArea(void)
{
    // attributes missing: %attrs, shape, coords, href, nohref,
    // alt, tabindex, accesskey, onfocus, onblur
    HTMLMap *imageMap = HTMLWidget->lastMap();

    if (!imageMap) 
        return;

    HTMLString href;
    HTMLString coords;
    QString atarget = baseTarget;
    HTMLArea::Shape shape = HTMLArea::Rect;

    const Attribute * p;
    
    while ( 0 != (p = ht->nextOption()) )
    {
	if ( p->id == ATTR_SHAPE )
	{
	    if ( ustrcasecmp( p->value(), QString("rect")) == 0 )
	        shape = HTMLArea::Rect;
	    else if ( ustrcasecmp( p->value(), QString("poly")) == 0 )
	        shape = HTMLArea::Poly;
	    else if ( ustrcasecmp( p->value(), QString("circle")) == 0 )
	        shape = HTMLArea::Circle;
	}
	else if ( p->id == ATTR_HREF )
	{
	    if ( p->value()[0] == '#' )
	    {// reference
	        KURL u( HTMLWidget->getDocumentURL() );
	        u.setRef( p->value() );
	        href = NEWSTRING( u.url() );
            }
            else 
	    {
	        KURL u( HTMLWidget->getBaseURL(), p->value() );
	        href = NEWSTRING( u.url() );
	    }
	}
	else if ( p->id == ATTR_TARGET )
	{
	    atarget = p->value();
	}
	else if ( p->id == ATTR_COORDS )
	{
	    coords = p->value();
	}
    }

    if ( coords.length() == 0 )
    	return;
    	
    HTMLArea *area = 0;

    switch ( shape )
    {
        case HTMLArea::Rect:
	    {
		int x1, y1, x2, y2;
		sscanf( coords.string().ascii(), 
			"%d,%d,%d,%d", &x1, &y1, &x2, &y2 );
		QRect rect( x1, y1, x2-x1, y2-y1 );
		area = new HTMLArea( rect, href, atarget );
		debugM( "Area Rect %d, %d, %d, %d\n", x1, y1, x2, y2 );
	    }
	    break;

        case HTMLArea::Circle:
	    {
		int xc, yc, rc;
		sscanf( coords.string().ascii(), "%d,%d,%d", &xc, &yc, &rc );
		area = new HTMLArea( xc, yc, rc, href, atarget );
		debugM( "Area Circle %d, %d, %d\n", xc, yc, rc );
	    }
	    break;

  	case HTMLArea::Poly:
	    {
		debugM( "Area Poly " );
		int count = 0, x = 0, y = 0;
		QPointArray parray;
		HTMLString ptr = coords;
		while ( ptr.length() )
		{
		    x = ptr.toInt();
		    ptr.find( ',' );
		    if ( ptr.length() )
		    {
			++ptr;
			y = ptr.toInt();
			parray.resize( count + 1 );
			parray.setPoint( count, x, y );
			debugM( "%d, %d  ", x, y );
			count++;
			ptr.find( ',' );
			if ( ptr.length() ) ++ptr;
		    }
		}
		debugM( "\n" );
		if ( count > 2 )
			area = new HTMLArea( parray, href, atarget );
	    }
	    break;
    }
    if (href.length())
        HTMLWidget->addHref(href.string());
        
    if ( area )
        imageMap->addArea( area );
}

void KHTMLParser::parseTagB(void)
{
    // attrs: %attrs
    pushBlock(tagID, 1);
    currentStyle->font.weight = CSSStyleFont::Bold;
    setFont();
}

void KHTMLParser::parseTagBase(void)
{
    const Attribute *p;
    while ( (p = ht->nextOption()) != 0 )
    {
        if ( p->id == ATTR_TARGET )
        {
            baseTarget = p->value();
	}
	else if ( p->id == ATTR_HREF )
	{
	    HTMLWidget->setBaseURL( p->value().string() );
	}
    }
}

void KHTMLParser::parseTagBaseFont(void)
{
    // attrs: id size color face
}

void KHTMLParser::parseTagBdo(void)
{
    // I think we'll wait for qt to support right to left...
}

void KHTMLParser::parseTagBig(void)
{
    // attrs %attrs

    pushBlock(tagID, 1);
    currentStyle->font.size += 2;
    setFont();
}

void KHTMLParser::parseTagBlockQuote(void)
{
    // attrs: cite, %attrs

    pushBlock(tagID, 2, &KHTMLParser::blockEnd);
    currentStyle->text.indent += INDENT_SIZE;
    flow = 0; 
}

void KHTMLParser::parseTagBody(void)
{
    // missing attrs: %attrs, onload, onunload, alink

    if ( bodyParsed )
	return;

    bodyParsed = true;

    const Attribute *token;
    while ( (token = ht->nextOption()) != 0 )
    {
	if ( token->id == ATTR_BGCOLOR )
	{
	    QColor bgColor;
	    setNamedColor( bgColor, token->value() );
	    HTMLWidget->setBGColor( bgColor );
	    background->changeColor( bgColor );
	}
	else if ( token->id == ATTR_BACKGROUND )
	{
	  KURL kurl(HTMLWidget->getBaseURL(), token->value().string());

	  background->changeImage(NEWSTRING(kurl.url()));
        }
	else if ( token->id == ATTR_TEXT )
	{
	    setNamedColor( settings->fontBaseColor, token->value() );
	    currentStyle->font.color = settings->fontBaseColor;
	    setFont();
	}
	else if ( token->id == ATTR_LINK )
	{
	    setNamedColor( settings->linkColor, token->value() );
	}
	else if ( token->id == ATTR_VLINK )
	{
	    setNamedColor( settings->vLinkColor, token->value() );
	}
    }
}

void KHTMLParser::parseTagBr(void)
{
    //attrs: %coreattrs

    HTMLVSpace::Clear clear = HTMLVSpace::CNone;

    const Attribute *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_CLEAR )
	{
	    if ( ustrcasecmp( token->value(), "left" ) == 0 )
	        clear = HTMLVSpace::Left;
	    else if ( ustrcasecmp( token->value(), "right" ) == 0 )
	        clear = HTMLVSpace::Right;
	    else if ( ustrcasecmp( token->value(), "all" ) == 0 )
	        clear = HTMLVSpace::All;
        }
    }

    if (!flow)
        newFlow();

    flow->append( NEW HTMLVSpace( currentFont()->pointSize(), clear ));
    vspace_inserted = false;
}

void KHTMLParser::parseTagButton(void)
{
    // forms... HTML4 unimplemented
}

void KHTMLParser::parseTagCenter(void)
{
    pushBlock(tagID, 2, &KHTMLParser::blockEndAlign, divAlign);

    divAlign = HTMLClue::HCenter;
    flow = 0;
}

void KHTMLParser::parseTagCite(void)
{
    pushBlock(tagID, 1 );

    currentStyle->font.style = CSSStyleFont::stItalic;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();
}

void KHTMLParser::parseTagCode(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.family = settings->fixedFontFace;
    currentStyle->font.fixed = true;
    currentStyle->font.style = CSSStyleFont::stNormal;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();
}

void KHTMLParser::parseTagDD(void)
{
    // attrs: %attrs

    if ( !glossaryStack.top() )
        return;

    if ( *glossaryStack.top() != GlossaryDD )
    {
        glossaryStack.push( new GlossaryEntry( GlossaryDD ) );
        currentStyle->text.indent += INDENT_SIZE;
    }
    flow = 0;
}

void KHTMLParser::parseTagDel(void)
{
    // attrs: %attrs, cite, datetime
    // marks deleted text. Could be rendered as struck-through
}

void KHTMLParser::parseTagDfn(void)
{
    // attrs: %attrs
}

void KHTMLParser::parseTagDiv(void)
{
    pushBlock(tagID, 2, &KHTMLParser::blockEndAlign, divAlign);

    const Attribute *token;
    while ( (token = ht->nextOption()) != 0)
    {
        // attrs: %attrs
        if ( token->id == ATTR_ALIGN )
        {
            if ( ustrcasecmp( token->value(), "right" ) == 0 )
	        divAlign = HTMLClue::Right;
	    else if ( ustrcasecmp( token->value(), "center" ) == 0 )
	        divAlign = HTMLClue::HCenter;
	    else if ( ustrcasecmp( token->value(), "left" ) == 0 )
                divAlign = HTMLClue::Left;
	}
    }
    flow = 0;
}

void KHTMLParser::parseTagDL(void)
{
    // attrs: %attrs

    popBlock(ID_A); // Close any <A..> tags
    vspace_inserted = insertVSpace( vspace_inserted );
    if ( glossaryStack.top() )
    {
        currentStyle->text.indent += INDENT_SIZE;
    }
    glossaryStack.push( new GlossaryEntry( GlossaryDL ) );
    flow = 0;
}

void KHTMLParser::parseTagDLEnd(void)
{
    if ( !glossaryStack.top() )
        return;

    if ( *glossaryStack.top() == GlossaryDD )
    {
        glossaryStack.remove();
        currentStyle->text.indent -= INDENT_SIZE;
        if (currentStyle->text.indent < 0)
        {
            printf("Error in indentation! </DL>\n");
            currentStyle->text.indent = 0;
        }
    }
    glossaryStack.remove();
    if ( glossaryStack.top() )
    {
        currentStyle->text.indent -= INDENT_SIZE;
        if (currentStyle->text.indent < 0)
        {
            printf("Error in indentation! </DL>\n");
            currentStyle->text.indent = 0;
        }
    }
    vspace_inserted = insertVSpace( vspace_inserted );
}

void KHTMLParser::parseTagDT(void)
{
    // attrs: %attrs

    if ( !glossaryStack.top() )
        return;

    if ( *glossaryStack.top() == GlossaryDD )
    {
        glossaryStack.pop();
        currentStyle->text.indent -= INDENT_SIZE;
        if (currentStyle->text.indent < 0)
        {
            printf("Error in indentation! </DL>\n");
            currentStyle->text.indent = 0;
        }
    }
    vspace_inserted = false;
    flow = 0;
}

void KHTMLParser::parseTagEM(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.style = CSSStyleFont::stItalic;
    setFont();
}

void KHTMLParser::parseTagFieldset(void)
{
    // Unimplemented HTML 4 
}

void KHTMLParser::parseTagFont(void)
{
    pushBlock(tagID, 1 );
    const Attribute *token;
    while ( 0 != (token = ht->nextOption()) )
    {
	if ( token->id == ATTR_SIZE )
	{
	    int num = token->value().toInt();
	    if ( *token->value().unicode() == '+' || 
		 *token->value().unicode() == '-' )
		currentStyle->font.size = settings->fontBaseSize + num;
	    else
		currentStyle->font.size = num;
	}
	else if ( token->id == ATTR_COLOR )
	{
            setNamedColor( currentStyle->font.color, token->value() );
	}
	else if ( token->id == ATTR_FACE )
	{
	    // try to find a matching font in the font list.
	    StringTokenizer st;
	    const QChar separ[] = { ' ', ',', 0x0 };
            st.tokenize( token->value(), separ );
	    while ( st.hasMoreTokens() )
	    {
                QString fname(st.nextToken());
	        fname = fname.lower();
		QFont tryFont( fname.data() );
		QFontInfo fi( tryFont );
		if ( strcmp( tryFont.family(), fi.family() ) == 0 )
		{
		    // we found a matching font
		    currentStyle->font.family = fname;
		    break;
                }
	    }
        }
    }
    setFont();
}

void KHTMLParser::parseTagForm(void)
{
    QString action = "";
    QString method = "GET";
    QString target = "";

    const Attribute * token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_ACTION )
	{
            KURL u( HTMLWidget->getBaseURL(), token->value() );
            action = u.url();
	}
	else if ( token->id == ATTR_METHOD )
	{
            if ( ustrcasecmp( token->value(), QString("post") ) == 0 )
		method = "POST";
	}
	else if ( token->id == ATTR_TARGET )
	{
	    target = token->value();
	}
    }

    form = new HTMLForm( action, method, target );
    HTMLWidget->addForm( form);

    vspace_inserted = insertVSpace( vspace_inserted );
    // Lars: does not work, if forms extend over several
    // tablecells, and the form gets created in the first one.
    //pushBlock( ID_FORM, 2, &blockEndForm);
}

void KHTMLParser::parseTagFormEnd(void)
{
    if(form)
    {
	vspace_inserted = insertVSpace( vspace_inserted );
	flow = 0;
	form = 0;
    }
    //Lars: see above
    //popBlock( ID_FORM );
}

void KHTMLParser::parseTagFrame(void)
{
    if ( !frameSet)
    	return; // Frames need a frameset

    HTMLString src = HTMLString();
    QString name = 0;
    int marginwidth = leftBorder;
    int marginheight = rightBorder;

    // 0 = no, 1 = yes, 2 = auto
    int scrolling = 2;
    bool noresize = FALSE;

    // -1 = default ( 5 )
    int frameborder = -1;
	      
    const Attribute *token;
    while ( (token = ht->nextOption()) != 0)
    { 
	if ( token->id == ATTR_SRC )
	    src = token->value();

	else if ( token->id == ATTR_NAME )
	    name = token->value().string().copy();

	else if ( token->id == ATTR_MARGINWIDTH )
	    marginwidth = token->value().toInt();

	else if ( token->id == ATTR_MARGINHEIGHT )
	    marginheight = token->value().toInt();

	else if ( token->id == ATTR_FRAMEBORDER )
	{
	    frameborder = token->value().toInt();
	    if ( frameborder < 0 )
	        frameborder = -1;
	}

	else if ( token->id == ATTR_NORESIZE )
	    noresize = TRUE;

	else if ( token->id == ATTR_SCROLLING )
        {
	    if ( ustrcasecmp( token->value(), QString("yes") ) == 0 )
	        scrolling = 1;
	    if ( ustrcasecmp( token->value(), QString("no") ) == 0 )
		scrolling = 0;
	    if ( ustrcasecmp( token->value(), QString("auto") ) == 0 )
		scrolling = -1;
	}
    }	      

    // Create the frame,
    // the parent for the frame is frameSet
    HTMLWidget->addFrame( frameSet, name, scrolling, !noresize,
		    	frameborder, marginwidth, marginheight, src);
}

void KHTMLParser::parseTagEmbed(void)
{
  if ( !frameSet)
    return; // Frames need a frameset

  HTMLString src;
  HTMLString name;
  HTMLString type;
  int marginwidth = leftBorder;
  int marginheight = rightBorder;
  bool noresize = FALSE;
  // -1 = default ( 5 )
  int frameborder = -1;
	
  const Attribute *token;
  while ( ( token = ht->nextOption() ) != 0 )
  {
    if ( token->id == ATTR_SRC )
    {
      src = token->value();
    }
    else if ( token->id == ATTR_NAME )
    {
      name = token->value();
    }
    else if ( token->id == ATTR_TYPE )
    {
      type = token->value();
    }
    else if ( token->id == ATTR_MARGINWIDTH )
    {
      marginwidth = token->value().toInt();
    }
    else if ( token->id == ATTR_MARGINHEIGHT )
    {
      marginheight = token->value().toInt();
    }
    else if ( token->id == ATTR_FRAMEBORDER )
    {
      frameborder = token->value().toInt();
      if ( frameborder < 0 )
	frameborder = -1;
    }
    else if ( token->id == ATTR_NORESIZE )
      noresize = TRUE;
  }	      

  // Create the widget
  KHTMLEmbededWidget *embed = 
      HTMLWidget->getView()->newEmbededWidget( frameSet, name.string().ascii(),
					       src.string().ascii(), 
					       type.string().ascii(),
					       marginwidth,
					       marginheight, 
					       frameborder,
					       !noresize );
  HTMLWidget->addEmbededFrame( frameSet, embed );
}

void KHTMLParser::parseTagFrameset(void)
{
    HTMLString cols = HTMLString();
    HTMLString rows = HTMLString();
    int frameBorder = 1;
    bool bAllowResize = true;

    // We need a view to do frames
    if ( !HTMLWidget->getView())
    	return;

    const Attribute *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_COLS )
	{
	    cols = token->value();
	}
	else if ( token->id == ATTR_ROWS )
	{
	    rows = token->value();
	}
	else if ( token->id == ATTR_FRAMEBORDER )
	{
	    frameBorder = token->value().toInt();
	}
	else if ( token->id == ATTR_NORESIZE )
	{
            bAllowResize = false;
	}
    }


    HTMLFrameSet *oldFrameSet = frameSet;
    if ( !oldFrameSet )
    {
       // This is the toplevel frameset
       frameSet = new HTMLFrameSet( HTMLWidget, cols, rows, 
       				    frameBorder, bAllowResize );
    }
    else
    { 
       // This is a nested frameset
       frameSet = new HTMLFrameSet( oldFrameSet, cols, rows,
       				    frameBorder, bAllowResize );
       oldFrameSet->append(frameSet);
    }
    HTMLWidget->addFrameSet( frameSet);
    pushBlock( tagID, 4, &KHTMLParser::blockEndFrameSet, (int) oldFrameSet );
}

void KHTMLParser::parseTagHeader(void)
{
    pushBlock(ID_H1, 2, &KHTMLParser::blockEnd );

    const Attribute *token;
    vspace_inserted = insertVSpace( vspace_inserted );
    HTMLClue::HAlign align = divAlign;

    while( 0 != (token = ht->nextOption()) )
    {
	if ( token->id == ATTR_ALIGN )
	{
            if ( ustrcasecmp( token->value(), QString("center") ) == 0 )
		align = HTMLClue::HCenter;
            else if ( ustrcasecmp( token->value(), QString("right") ) == 0 )
		align = HTMLClue::Right;
            else if ( ustrcasecmp( token->value(), QString("left") ) == 0 )
		align = HTMLClue::Left;
	}
    }
    // Start a new flow box
    newFlow();
    flow->setHAlign( align );

    switch ( tagID )
    {
        case ID_H1:
        	currentStyle->font.weight = CSSStyleFont::Bold;
        	currentStyle->font.style = CSSStyleFont::stNormal;
		currentStyle->font.size = settings->fontBaseSize+3;
		break;

	case ID_H2:
        	currentStyle->font.weight = CSSStyleFont::Bold;
        	currentStyle->font.style = CSSStyleFont::stNormal;
		currentStyle->font.size = settings->fontBaseSize+2;
		break;

	case ID_H3:
        	currentStyle->font.weight = CSSStyleFont::Normal;
        	currentStyle->font.style = CSSStyleFont::stItalic;
		currentStyle->font.size = settings->fontBaseSize+1;
		break;

	case ID_H4:
        	currentStyle->font.weight = CSSStyleFont::Bold;
        	currentStyle->font.style = CSSStyleFont::stNormal;
		currentStyle->font.size = settings->fontBaseSize;
		break;

	case ID_H5:
        	currentStyle->font.weight = CSSStyleFont::Normal;
        	currentStyle->font.style = CSSStyleFont::stItalic;
		currentStyle->font.size = settings->fontBaseSize;
		break;

	case ID_H6:
        	currentStyle->font.weight = CSSStyleFont::Bold;
        	currentStyle->font.style = CSSStyleFont::stNormal;
		currentStyle->font.size = settings->fontBaseSize-1;
		break;
    }
    setFont();
    // Insert a vertical space and restore the old font at the 
    // closing tag
}

void KHTMLParser::parseTagHeaderEnd(void)
{
    popBlock(ID_H1);
}

void KHTMLParser::parseTagHead(void)
{
    // There is nothing we need to do here at the moment :]
}

void KHTMLParser::parseTagHR(void)
{
    int size = 1;
    int length = UNDEFINED;
    int percent = UNDEFINED;
    HTMLClue::HAlign align = divAlign;
    HTMLClue::HAlign oldAlign = divAlign;
    bool shade = TRUE;

    if ( flow )
        oldAlign = align = flow->getHAlign();

    const Attribute * token;
    
    while ( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_ALIGN )
	{
            if ( ustrcasecmp( token->value(), "left" ) == 0 )
		align = HTMLRule::Left;
            else if ( ustrcasecmp( token->value(), "right" ) == 0 )
		align = HTMLRule::Right;
	    else if ( ustrcasecmp( token->value(), "center" ) == 0 )
		align = HTMLRule::HCenter;
	}
	else if ( token->id == ATTR_SIZE )
	{
	    size = token->value().toInt();
	}
	else if ( token->id == ATTR_WIDTH )
	{
            if ( token->value().percentage(percent) )
            {   
                // Percent set
                length = UNDEFINED;
	    }
	    else
	    {
		// Fixed width
		length = token->value().toInt();
                percent = UNDEFINED; // assume fixed width
	    }
	}
	else if ( token->id == ATTR_NOSHADE )
	{
            shade = FALSE;
	}
    }

    divAlign = align;
    newFlow();

    flow->append( NEW HTMLRule( length, percent, size, shade ) );
printf("HR: length = %d, percent = %d, size = %d, shade = %d\n",
	length, percent, size, shade);

    flow = 0;
    divAlign = oldAlign;

    vspace_inserted = false;
}

void KHTMLParser::parseTagI(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.style = CSSStyleFont::stItalic;
    setFont();
}

void KHTMLParser::parseTagIframe(void)
{
    // inlined frame
}

void KHTMLParser::parseTagImg(void)
{
  printf("************** Parsing an image ****************\n" );
  
    vspace_inserted = FALSE;

    // Parse all arguments but delete '<' and '>' and skip 'cell'
    HTMLString filename;
    HTMLString overlay;
    QString usemap;
    bool    ismap = false;
    int width = UNDEFINED;
    int height = UNDEFINED;
    int percent = UNDEFINED;
    int border = 0;
    HTMLClue::HAlign align = HTMLClue::HNone;
    HTMLClue::VAlign valign = HTMLClue::VNone;

    const Attribute *token;
    while ( (token = ht->nextOption()) != 0)
    {
        if ( token->id == ATTR_SRC )
	    filename = token->value();
	else if ( token->id == ATTR_OVERSRC )
	    // what is this attribute???? It doesn't appear in the dtd!
	    overlay = token->value();
	else if ( token->id == ATTR_WIDTH )
	{
	    if ( ! token->value().percentage(percent) )
            {
	        width = token->value().toInt();
	        percent = UNDEFINED;
	    }
	}
	else if ( token->id == ATTR_HEIGHT )
	    height = token->value().toInt();
	else if ( token->id == ATTR_BORDER )
	    border = token->value().toInt();
	else if (token->id == ATTR_ALIGN )
	{
	    if ( ustrcasecmp( token->value(), "left" ) == 0 )
	        align = HTMLClue::Left;
	    else if ( ustrcasecmp( token->value(), "right" ) == 0 )
	        align = HTMLClue::Right;
	    else if ( ustrcasecmp( token->value(), "top" ) == 0 )
	        valign = HTMLClue::Top;
	    else if ( ustrcasecmp( token->value(), "middle" ) == 0 )
	        valign = HTMLClue::VCenter;
	    else if ( ustrcasecmp( token->value(), "bottom" ) == 0 )
	        valign = HTMLClue::Bottom;
        }
	else if ( token->id == ATTR_USEMAP )
	{
	    if ( *token->value().unicode() == '#' )
	    {
	        // Local map. Format: "#name"
                usemap = token->value();
	    }
	    else
	    {
	        KURL u( HTMLWidget->getBaseURL(), token->value() );
                usemap = u.url();
	    }
	}
	else if ( token->id == ATTR_ISMAP )
	{
	    ismap = true;
	}
    }
    // if we have a file name do it...
    if ( filename != 0 )
    {
        printf("**************** IMAGE Parsing URL=%s\n", filename.string().ascii() );
      
        KURL kurl( HTMLWidget->getBaseURL(), filename );
        // Do we need a new FlowBox ?
        if ( !flow )
            newFlow();

	HTMLImage *image;

	if ( usemap.isEmpty() && !ismap )
	{
	    image =  NEW HTMLImage( HTMLWidget, NEWSTRING(kurl.url()), 
				    url, target,
				    width, height, percent, border );
	}
	else
	{
	    image =  NEW HTMLImageMap( HTMLWidget, NEWSTRING(kurl.url()),
				       url, target,
				       width, height, percent, border );
	    if ( !usemap.isEmpty() )
	    {
	        ((HTMLImageMap *)image)->setMapURL( usemap );
	    }
	    else
	        ((HTMLImageMap *)image)->setMapType( HTMLImageMap::ServerSide );
	}

	// used only by kfm to overlay links, readonly etc.
	if ( overlay.length() )
	    image->setOverlay( overlay );

	image->setBorderColor( currentStyle->box.borderColor );

        if ( valign != HTMLClue::VNone)
	    image->setVAlign( valign );

	if ( align == HTMLClue::HNone )
	{
	    flow->append( image );
#if 0
	    	if ( valign == HTMLClue::VNone)
	    	{
		    flow->append( image );
		}
                else
	    	{
		    HTMLClueH *valigned = NEW HTMLClueH();
		    valigned->setVAlign( valign );
		    valigned->append( image );
		    flow->append( valigned );
	    	}
#endif
        }
	// we need to put the image in a HTMLClueAligned
	else
	{
	    HTMLClueAligned *aligned = NEW HTMLClueAligned( flow );
	    aligned->setHAlign( align );
	    aligned->append( image );
	    flow->append( aligned );
	}
    } 
}

void KHTMLParser::parseTagInput(void)
{
    if ( form == 0 )
	return;

    if ( !flow )
         newFlow();

    enum InputType { CheckBox, Hidden, Radio, Reset, Submit, Text, Image,
	    Button, Password, Undefined };
    HTMLString p;
    InputType type = Text;
    HTMLString name;
    HTMLString value;
    HTMLString imgSrc;
    bool checked = false;
    int size = 20;
    int maxLen = -1;
    QList<JSEventHandler> *handlers = 0;

    const Attribute * token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_TYPE )
	{
	    p = token->value();

	    if ( ustrcasecmp( p, QString("checkbox") ) == 0 )
		type = CheckBox;
	    else if ( ustrcasecmp( p, QString("password") ) == 0 )
		type = Password;
	    else if ( ustrcasecmp( p, QString("hidden") ) == 0 )
		type = Hidden;
	    else if ( ustrcasecmp( p, QString("radio") ) == 0 )
		type = Radio;
	    else if ( ustrcasecmp( p, QString("reset") ) == 0 )
		type = Reset;
	    else if ( ustrcasecmp( p, QString("submit") ) == 0 )
		type = Submit;
	    else if ( ustrcasecmp( p, QString("button") ) == 0 )
		type = Button;      
	    else if ( ustrcasecmp( p, QString("text") ) == 0 )
		type = Text;
	    else if ( ustrcasecmp( p, QString("Image") ) == 0 )
		type = Image;
	}
	else if ( token->id == ATTR_NAME )
	{
	    name = token->value();
	}
	else if ( token->id == ATTR_VALUE )
	{
	    value = token->value();
	}
	else if ( token->id == ATTR_SIZE )
	{
	    size = token->value().toInt();
	}
	else if ( token->id == ATTR_MAXLENGTH )
	{
	    maxLen = token->value().toInt();
	}
	else if ( token->id == ATTR_CHECKED )
	{
	    checked = true;
	}
	else if ( token->id == ATTR_SRC )
	{
	    imgSrc = token->value();
	}
	else if ( token->id == ATTR_ONCLICK )
	{
	    HTMLString code( token->value() );
	    if ( handlers == 0 )
	    {
		handlers = new QList<JSEventHandler>;
		handlers->setAutoDelete( TRUE );
	    }
	    handlers->append( new JSEventHandler( HTMLWidget->getJSEnvironment(), "onClick", code.string().ascii() ) );
	}     
    }

    switch ( type )
    {
	case CheckBox:
	    {
		HTMLCheckBox *cb = NEW HTMLCheckBox( HTMLWidget, name, value,
						     checked, currentFont() );
		cb->setForm( form );
		form->addElement( cb );
		flow->append( cb );
	    }
	    break;

	case Hidden:
	    {
		HTMLHidden *hidden = NEW HTMLHidden( name, value );
		hidden->setForm( form );
		form->addHidden( hidden );
	    }
	    break;

	case Radio:
	    {
		HTMLRadio *radio = NEW HTMLRadio( HTMLWidget, name, value, 
						  checked, currentFont() );
		radio->setForm( form );
		form->addElement( radio );
		flow->append( radio );
		QObject::connect(radio,SIGNAL(radioSelected(QString,QString)),
		    form, SLOT(slotRadioSelected(QString, QString)));
		QObject::connect(form,SIGNAL( radioSelected(QString,QString) ),
		    radio, SLOT(slotRadioSelected(QString, QString)));
	    }
	    break;

	case Reset:
	    {
		HTMLReset *reset = NEW HTMLReset( HTMLWidget, value, 
						  currentFont() );
		reset->setForm( form );
		form->addElement( reset );
		flow->append( reset );
		QObject::connect( reset, SIGNAL( resetForm() ),
			form, SLOT( slotReset() ) );
	    }
	    break;

	case Submit:
	    {
		HTMLSubmit *submit = NEW HTMLSubmit( HTMLWidget, name, value,
						     currentFont() );
		submit->setForm( form );
		form->addElement( submit );
		flow->append( submit );
		QObject::connect( submit, SIGNAL( submitForm() ),
			form, SLOT( slotSubmit() ) );
	    }
	    break;

	case Button:
	    {
		HTMLButton *button = NEW HTMLButton( HTMLWidget,name,value,
						     handlers, currentFont() );
		button->setForm( form );
		form->addElement( button );
		flow->append( button );
		break;
	    }

	case Text:
	case Password:
	    {
		HTMLTextInput *ti = NEW HTMLTextInput( HTMLWidget, name, value, 
						       size, maxLen, 
						       (type == Password),
						       currentFont() );
		ti->setForm( form );
		form->addElement( ti );
		flow->append( ti );
		QObject::connect( ti, SIGNAL( submitForm() ),
			form, SLOT( slotSubmit() ) );
	    }
	    break;

	case Image:
	    if ( !imgSrc.length() )
	    {
		KURL kurl( HTMLWidget->getBaseURL(), imgSrc );
		HTMLImageInput *ii = NEW HTMLImageInput( HTMLWidget, 
							 NEWSTRING(kurl.url()),
							 name );
		ii->setForm( form );
		form->addElement( ii );
		flow->append( ii );
		QObject::connect( ii, SIGNAL( submitForm() ),
			form, SLOT( slotSubmit() ) );
	    }
	    break;

	case Undefined:
	    break;
    }
    vspace_inserted = false;
}

void KHTMLParser::parseTagIns(void)
{
    // inserted text... opposite of <del>
}

void KHTMLParser::parseTagIsindex(void)
{
    // deprecated... is it used at all?
}

void KHTMLParser::parseTagKbd(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.family = settings->fixedFontFace;
    currentStyle->font.fixed = true;
    currentStyle->font.style = CSSStyleFont::stNormal;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();
}

void KHTMLParser::parseTagLabel(void)
{
    // Unimplemented
}

void KHTMLParser::parseTagLegend(void)
{
    // Unimplemented
}

void KHTMLParser::parseTagLi(void)
{
    popBlock(ID_A); // Close any <A..> tags
    QString item;
    ListType listType = Unordered;
    ListNumType listNumType = Numeric;
    int listLevel = 1;
    int itemNumber = 1;
    int indentSize = INDENT_SIZE;
    if ( listStack.count() > 0 )
    {
        listType = listStack.top()->type;
        listNumType = listStack.top()->numType;
        itemNumber = listStack.top()->itemNumber;
        listLevel = listStack.count();
        indentSize = currentStyle->text.indent;
    }
    HTMLClueFlow *f = NEW HTMLClueFlow();
    _clue->append( f );
    HTMLClueH *c = NEW HTMLClueH();
    c->setVAlign( HTMLClue::Top );
    f->append( c );

//@@WABA: This should be handled differently

    // fixed width spacer
    HTMLClueV *vc = NEW HTMLClueV();
    vc->setFixedWidth( indentSize ); // Fixed width clue
    vc->setVAlign( HTMLClue::Top );
    c->append( vc );

    switch ( listType )
    {
    case Unordered:
	flow = NEW HTMLClueFlow();
	flow->setHAlign( HTMLClue::Right );
	vc->append( flow );
	flow->append( NEW HTMLBullet( currentStyle->font.fp->pointSize(),
		listLevel, settings->fontBaseColor ) );
	break;

    case Ordered:
	flow = NEW HTMLClueFlow();
	flow->setHAlign( HTMLClue::Right );
	vc->append( flow );
	switch ( listNumType )
	{
	    case LowRoman:
		item = toRoman( itemNumber, false );
		break;

	    case UpRoman:
		item = toRoman( itemNumber, true );
		break;

	    case LowAlpha:
		item += (QChar) ('a' + itemNumber - 1);
		break;

	    case UpAlpha:
		item += (QChar) ('A' + itemNumber - 1);
		break;

	    default:
		item.sprintf( "%2d", itemNumber );
	}
	item += ". ";
	flow->append(
	    NEW HTMLText( NEWSTRING( item ),
			  currentFont(),
			  painter) 
	    );
	break;

    default:
	break;
    }

    vc = NEW HTMLClueV();
    c->append( vc );
    flow = NEW HTMLClueFlow();
    vc->append( flow );
    if ( listStack.count() > 0 )
	listStack.top()->itemNumber++;
}

void KHTMLParser::parseTagLink(void)
{
    // Not yet implemented
}

void KHTMLParser::parseTagMap(void)
{
    const Attribute *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_NAME )
	{
	    QString mapurl = "#";
	    mapurl += token->value();
	    HTMLWidget->addMap( mapurl );
	}
    }
}

void KHTMLParser::parseTagMeta(void)
{
    HTMLString httpequiv;
    HTMLString name;
    HTMLString content;
    
    const Attribute *token;
    
    while( (token = ht->nextOption()) != 0)
    {
	debugM("token: %s\n",token);
	if ( token->id == ATTR_NAME )
	    name=token->value();
	else if ( token->id == ATTR_HTTP_EQUIV )
 	    httpequiv=token->value();
 	else if ( token->id == ATTR_CONTENT )
	    content=token->value();
    }
    debugM( "Meta: name=%s httpequiv=%s content=%s\n",
	    name.string().latin1(),httpequiv.string().latin1(),
	    content.string().latin1() );
    if (!httpequiv.length())
    {
	if ( ustrcasecmp(httpequiv, "refresh") == 0 )
	{
	    const QChar separ[] = { ' ', '>', ';', 0x0 };
	    stringTok->tokenize( content, separ );
	    HTMLString t = stringTok->nextToken();
            int delay = t.toInt();
	    QString url = HTMLWidget->actualURL.url();
	    while ( stringTok->hasMoreTokens() )
	    {
	        HTMLString token = stringTok->nextToken();
	        debugM("token: %s\n",token.string().latin1());
		QChar urlchars [] = { 'u', 'r', 'l', '=' };
		if ( ustrncasecmp( token.unicode(), urlchars, 4 ) == 0 )
		{
		    token += 4;
		    if ( token[0] == '#' )
		    {// reference
			KURL u( HTMLWidget->actualURL );
			u.setRef( token );
			url = u.url();
		    }
		    else 
		    {
			KURL u( HTMLWidget->baseURL, token );
			url = u.url();
		    }
		}
	    }
	    // set up the redirect...
	    if(!( delay==0 && url == HTMLWidget->actualURL.url()))
		   /*emit*/ HTMLWidget->redirect( delay, url );
	}
    } 
}

void KHTMLParser::parseTagNobr(void)
{
    // Non standardized extension, not supported (yet)
}

void KHTMLParser::parseTagNoframes(void)
{
    if (HTMLWidget->htmlView)
    	inNoframes=true;
}

void KHTMLParser::parseTagNoscript(void)
{
     // Only needed when we support scipts
}

void KHTMLParser::parseTagObject(void)
{
     // HTML4 not impl.
}

void KHTMLParser::parseTagOl(void)
{
    popBlock(ID_A); // Close any <A..> tags
    if ( listStack.isEmpty() )
    {
        vspace_inserted = insertVSpace( vspace_inserted );
	pushBlock( tagID, 2, &KHTMLParser::blockEndList, true);
    }
    else
    {
        pushBlock( tagID, 2, &KHTMLParser::blockEndList, false);
    }

    ListNumType listNumType = Numeric;

    const Attribute *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_TYPE )
	{
	    switch ( *(token->value().unicode()) )
	    {
		case 'i':
			listNumType = LowRoman;
			break;

		case 'I':
			listNumType = UpRoman;
			break;

		case 'a':
			listNumType = LowAlpha;
			break;

		case 'A':
			listNumType = UpAlpha;
			break;
	    }
	}
    }

    listStack.push( new HTMLList( Ordered, listNumType ) );
    currentStyle->text.indent += INDENT_SIZE;
}

void KHTMLParser::parseTagOptgroup(void)
{
    // Grouoing options, HTML4, not impl.
}

void KHTMLParser::parseTagOption(void)
{
    if ( !formSelect )
 	return;

    HTMLString value;
    bool selected = false;

    const Attribute *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_VALUE )
	{
		value = token->value();
	}
	else if ( token->id == ATTR_SELECTED )
	{
		selected = true;
	}
    }

    if ( inOption )
 	formSelect->setText( formText );

    formSelect->addOption( value, selected );

    inOption = true;
    formText = "";
}

void KHTMLParser::parseTagOptionEnd(void)
{
    if ( inOption )
	formSelect->setText( formText );
    inOption = false;
}

void KHTMLParser::parseTagP(void)
{
    popBlock(ID_A); // Close any <A..> tags
    vspace_inserted = insertVSpace( vspace_inserted );
    HTMLClue::HAlign align = divAlign;

    const Attribute *token;
    while ( (token = ht->nextOption()) != 0)
    {    
	if ( token->id == ATTR_ALIGN )
	{
	    if ( ustrcasecmp( token->value(), "center") == 0 )
		align = HTMLClue::HCenter;
	    else if ( ustrcasecmp( token->value(), "right") == 0 )
		align = HTMLClue::Right;
	    else if ( ustrcasecmp( token->value(), "left") == 0 )
		align = HTMLClue::Left;
	}
    }
    if ( flow == 0 && align != divAlign )
        newFlow();

    if ( align != divAlign )
        flow->setHAlign( align );
}

void KHTMLParser::parseTagPEnd(void)
{
    popBlock(ID_A); // Close any <A..> tags
    vspace_inserted = insertVSpace( vspace_inserted );
}

void KHTMLParser::parseTagParam(void)
{
    // HTML4, not impl.
}

void KHTMLParser::parseTagPre(void)
{
    // Used by PRE and LISTING!!
    vspace_inserted = insertVSpace( vspace_inserted );

    pushBlock(tagID, 2, &KHTMLParser::blockEndPre);

    currentStyle->font.family = settings->fixedFontFace;
    currentStyle->font.fixed = true;
    currentStyle->font.style = CSSStyleFont::stNormal;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();    

    flow = 0;
    inPre = true;
}

void KHTMLParser::parseTagQ(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.style = CSSStyleFont::stItalic;
    setFont();
}


void KHTMLParser::parseTagSamp(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.family = settings->fixedFontFace;
    currentStyle->font.fixed = true;
    currentStyle->font.style = CSSStyleFont::stNormal;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();    
}

void KHTMLParser::parseTagScript()
{
    // Javascript, not supported yet
}

void KHTMLParser::parseTagSelect()
{
    if ( !form )
        return;

    HTMLString name;
    int size = 0;
    bool multi = false;

    const Attribute *token;
    while( (token = ht->nextOption()) != 0)
    {
    	if ( token->id == ATTR_NAME )
	{
	    name = token->value();
	}
	else if ( token->id == ATTR_SIZE )
	{
	    size = token->value().toInt();
	}
	else if ( token->id == ATTR_MULTIPLE )
	{
	    multi = true;
	}
    }

    formSelect = NEW HTMLSelect( HTMLWidget, name, size, multi,
				     currentFont() );
    formSelect->setForm( form );
    form->addElement( formSelect );
    if ( !flow )
        newFlow();

    flow->append( formSelect );
}

void KHTMLParser::parseTagSelectEnd()
{
    if ( !form || !formSelect)
    	return;

    if ( inOption )
	formSelect->setText( formText );

    formSelect = 0;
    inOption = false;
    vspace_inserted = false;
}

void KHTMLParser::parseTagSmall()
{
    pushBlock(tagID, 1 );
    currentStyle->font.size = settings->fontBaseSize - 1;
    setFont();
}

void KHTMLParser::parseTagSpan()
{
    // obscure HTML4 tag. not impl.
}

void KHTMLParser::parseTagStrike(void)
{
    // used by S and STRIKE
    pushBlock(tagID, 1 );
    currentStyle->font.decoration = CSSStyleFont::decLineThrough;
    setFont();
}

void KHTMLParser::parseTagStrong()
{
    pushBlock(tagID, 1 );
    currentStyle->font.weight = CSSStyleFont::Bold;
    setFont();
}

void KHTMLParser::parseTagStyle()
{
    // Style sheets, not impl.
}

void KHTMLParser::parseTagSub()
{
    // Subscript, not impl. yet
}

void KHTMLParser::parseTagSup()
{
    // Superscript, not impl. yet
}

void KHTMLParser::parseTagTable(void)
{
    popBlock(ID_A); // Close any <A..> tags
    if ( !vspace_inserted || !flow )
        newFlow();

    // missing tags:
    // <col> <colgroup> </colgroup>
    // <thead> </thead> <tbody> </tbody> <tfoot> </tfoot>
 
    static const uint endthtd[] = { ID_TH + ID_CLOSE_TAG, 
    				    ID_TD + ID_CLOSE_TAG,
    				    ID_TR + ID_CLOSE_TAG,
    				    ID_TH, ID_TD, ID_TR,
    				    ID_TABLE + ID_CLOSE_TAG, 
    				    ID_BODY + ID_CLOSE_TAG, 
    				    0 };
    static const uint endcap[] = { ID_CAPTION + ID_CLOSE_TAG,
    				   ID_TABLE + ID_CLOSE_TAG,
    				   ID_TR, ID_TD, ID_TH,
   				   ID_BODY + ID_CLOSE_TAG, 
    				   0 };    
    static const uint endall[] = { ID_CAPTION + ID_CLOSE_TAG,
    				   ID_TABLE + ID_CLOSE_TAG,
				   ID_TH + ID_CLOSE_TAG, 
    				   ID_TD + ID_CLOSE_TAG,
    				   ID_TR + ID_CLOSE_TAG,
    				   ID_TR, ID_TD, ID_TH,
   				   ID_BODY + ID_CLOSE_TAG, 
    				   0 };    

    HTMLString str;
    bool hasRow = false;
    bool tableTag = true;
    bool noCell = true;
    int padding = 1;
    int spacing = 2;
    int width = 0;
    int percent = UNDEFINED;
    int border = 0;
    HTMLClue::VAlign rowvalign = HTMLClue::VNone;
    HTMLClue::HAlign rowhalign = HTMLClue::HNone;
    HTMLClue::HAlign align = HTMLClue::HNone;
    HTMLClueV *caption = 0;
    HTMLClue::VAlign capAlign = HTMLClue::Bottom;
    HTMLClue::HAlign olddivalign = divAlign;
    HTMLClue *__clue = flow;
    HTMLClue *oldFlow = flow;
    HTMLClue *oldClue = _clue;
    int oldindent = currentStyle->text.indent;
    QColor tableColor;
    QColor rowColor;

    const Attribute *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_CELLPADDING )
	{
	    padding = token->value().toInt();
	}
	else if ( token->id == ATTR_CELLSPACING )
	{
	    spacing = token->value().toInt();
	}
	else if ( token->id == ATTR_BORDER )
	{
	    if ( token->value().length() )
		border = token->value().toInt();
	    else
		border = 1;
	}
	else if ( token->id == ATTR_WIDTH )
	{
	    if ( token->value().percentage(percent) )
            { /* Percent set */ }
	    else if ( ustrchr( token->value().unicode(), '*' ) )
	    { /* ignore */ }
	    else {
		width = token->value().toInt();
		percent = 0; // fixed width
	    }
	}
	else if ( token->id == ATTR_ALIGN )
	{
	    if ( ustrcasecmp( token->value(), "left" ) == 0 )
		align = HTMLClue::Left;
	    else if ( ustrcasecmp( token->value(), "right" ) == 0 )
		align = HTMLClue::Right;
	}
	else if ( token->id == ATTR_BGCOLOR )
	{
	    setNamedColor( tableColor, token->value() );
	    rowColor = tableColor;
	}
    }

    HTMLTable *table = NEW HTMLTable( percent, width, padding, spacing, border );
    //       __clue->append( table ); 
    // CC: Moved at the end since we might decide to discard the table while parsing...

    currentStyle->text.indent = 0;
    
    bool done = false;

    while ( !done && ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( *str.unicode() == QChar(TAG_ESCAPE) )
	{
	    ++str;

	    tableTag = true;

	    tagID = str[0].unicode();
	    for(;;) 
	    {
		if ( tagID == ID_CAPTION )
		{
		    while ( (token = ht->nextOption()) != 0)
		    {
			if ( token->id == ATTR_ALIGN )
			{
			    if ( ustrcasecmp( token->value(), QString("top")) == 0)
				capAlign = HTMLClue::Top;
			}
		    }
		    caption = NEW HTMLClueV();
		    divAlign = HTMLClue::HCenter;
		    flow = 0;
		    _clue = caption;
		    pushBlock(ID_CAPTION, 3 );
		    tagID = parseBody( _clue, endcap );
                    popBlock( ID_CAPTION );
		    table->setCaption( caption, capAlign );
		    flow = 0;

		    if ( tagID == 0 )
		    {  
                        printf("Unexpected end of TABLE!\n");
                        done = true;
                        break;
		    }

		    if ( tagID == (ID_CAPTION + ID_CLOSE_TAG))
		    {
		        // HTML Ok!
		        break; // Get next token from 'ht'
		    }
		    else
		    {
		    	// Bad HTML
		    	// caption ended with </table> <td> <tr> or <th>
		       continue; // parse the returned tag
		    }
		}

		if ( tagID == ID_TR )
		{
		    if ( hasRow )
			table->endRow();
		    table->startRow();
		    hasRow = true;
		    rowvalign = HTMLClue::VNone;
		    rowhalign = HTMLClue::HNone;
		    rowColor = tableColor;

		    while ( (token = ht->nextOption()) != 0)
		    {
			if ( token->id == ATTR_VALIGN )
			{
			    if ( ustrcasecmp( token->value(), "top" ) == 0)
				rowvalign = HTMLClue::Top;
			    else if ( ustrcasecmp( token->value(), "bottom" ) == 0)
				rowvalign = HTMLClue::Bottom;
			    else
				rowvalign = HTMLClue::VCenter;
			}
			else if ( token->id == ATTR_ALIGN )
			{
			    if ( ustrcasecmp( token->value(), "left" ) == 0)
				rowhalign = HTMLClue::Left;
			    else if ( ustrcasecmp( token->value(), "right" ) == 0)
				rowhalign = HTMLClue::Right;
			    else if ( ustrcasecmp( token->value(), "center" ) == 0)
				rowhalign = HTMLClue::HCenter;
			}
			else if ( token->id == ATTR_BGCOLOR )
			{
			    setNamedColor( rowColor, token->value() );
			}
		    }
		    break; // Get next token from 'ht'
		}
		
		if ( tagID == (ID_TABLE + ID_CLOSE_TAG))
		{
		    popBlock(ID_A); // Close any <A..> tags
		    done = true;
		    break;
		}
		if ( tagID == (ID_TR + ID_CLOSE_TAG))
		{
                    if (hasRow)
                       table->endRow();
                    hasRow = false;
		    break;
		}

		// <td, <th, or we get something before the 
		// first <td or <th. Lets put that into one row 
		// of it's own... (Lars)
		bool tableEntry = (tagID == ID_TD) || (tagID == ID_TH);
		if ( tableEntry || noCell ) 
		{
		    bool heading = false;
		    noCell = false;

		    // if ( ustrncasecmp( str, "<th", 3 ) == 0 )
		    if (tagID == ID_TH)
			    heading = true;
		    // <tr> may not be specified for the first row
		    if ( !hasRow )
		    {
			// Bad HTML: No <tr> tag present
			table->startRow();
			hasRow = true;
                        rowvalign = HTMLClue::VNone;
		        rowhalign = HTMLClue::HNone;
		        rowColor = tableColor;
		    }

		    int rowSpan = 1, colSpan = 1;
		    int cellWidth = UNDEFINED;
		    int percent = UNDEFINED;
		    QColor bgcolor = rowColor;
		    HTMLClue::VAlign valign = (rowvalign == HTMLClue::VNone ?
					    HTMLClue::VCenter : rowvalign);

		    if ( heading )
			divAlign = (rowhalign == HTMLClue::HNone ? HTMLClue::HCenter :
			    rowhalign);
		    else
			divAlign = (rowhalign == HTMLClue::HNone ? HTMLClue::Left :
			    rowhalign);

		    if(tableEntry)
		    {
 		      while ( (token = ht->nextOption()) != 0)
		      {
			if ( token->id == ATTR_ROWSPAN )
			{
			    rowSpan = token->value().toInt();
			    if ( rowSpan < 1 )
				rowSpan = 1;
			}
			else if ( token->id == ATTR_COLSPAN )
			{
			    colSpan = token->value().toInt();
			    if ( colSpan < 1 )
				colSpan = 1;
			}
			else if ( token->id == ATTR_VALIGN )
			{
			    if ( ustrcasecmp( token->value(), "top") == 0)
				valign = HTMLClue::Top;
			    else if ( ustrcasecmp( token->value(), "bottom") == 0)
				valign = HTMLClue::Bottom;
			    else
				valign = HTMLClue::VCenter;
			}
			else if ( token->id == ATTR_ALIGN )
			{
			    if ( ustrcasecmp( token->value(), "center" ) == 0)
				divAlign = HTMLClue::HCenter;
			    else if ( ustrcasecmp( token->value(), "right" ) == 0)
				divAlign = HTMLClue::Right;
			    else if ( ustrcasecmp( token->value(), "left" ) == 0)
				divAlign = HTMLClue::Left;
			}
			else if ( token->id == ATTR_WIDTH )
			{
			    if ( token->value().percentage(percent) )
			    { /* percent set */ }
			    else if ( ustrchr( token->value().unicode(), '*' ) )
			    { /* ignore */ }
			    else
			    {
				cellWidth = token->value().toInt();
				percent = 0; // Fixed with
			    }
			}
			else if ( token->id == ATTR_BGCOLOR )
			{
			    setNamedColor( bgcolor, token->value() );
			}
		      } // while (ht->nextOption)
		    } // if(tableEntry)

		    HTMLTableCell *cell = NEW HTMLTableCell( percent, cellWidth,
			rowSpan, colSpan, padding );
		    if ( bgcolor.isValid() )
			cell->setBGColor( bgcolor );
		    cell->setVAlign( valign );
		    table->addCell( cell );
		    flow = 0;
		    _clue = cell;
		    if ( heading )
		    {
			pushBlock( ID_TH, 3 );
		        currentStyle->font.weight = CSSStyleFont::Bold;
			setFont();
		        tagID = parseBody( _clue, endthtd );
                        popBlock( ID_TH );
		    }
		    else if ( !tableEntry )
		    {
			// put all the junk between <table> and the first table
			// tag into one row.
		    	pushBlock( ID_TD, 3 );
			parseOneToken();
			tagID = parseBody( _clue, endall );
			popBlock( ID_TD );
			table->endRow();
			table->startRow();
		    }
		    else
		    {
		    	pushBlock( ID_TD, 3 );
			tagID = parseBody( _clue, endthtd );
			popBlock( ID_TD );
		    }

		    if ( tagID == 0 )
		    { 
                        printf("Unexpected end of TABLE!\n");
                        done = true;
                        break;
		    }

		    if ((tagID == (ID_TD + ID_CLOSE_TAG)) ||
		        (tagID == (ID_TH + ID_CLOSE_TAG)))
		    {
		        // HTML Ok!
		        break; // Get next token from 'ht'
		    }
		    else
		    {
		    	// Bad HTML
		        // td/th tag ended with </table> <th> <td> or <tr> 
		        continue; // parse the returned tag
		    }
		}
		
		// Unknown or unhandled table-tag: ignore
                printf("Unexpected tag inside TABLE: %d\n", tagID); 
		break;
	    }
        }
    }

    if ( hasRow )
        table->endRow();
    table->endTable();
   
    if (table->rows() != 0)
    {
	if ( align != HTMLClue::Left && align != HTMLClue::Right )
	{
	    __clue->append ( table );
	}
	else
	{
	    HTMLClueAligned *aligned = NEW HTMLClueAligned( __clue );
	    aligned->setHAlign( align );
	    aligned->append( table );
	    __clue->append( aligned );
	}
    }
    else
    {
	// CC: last resort -- remove tables that do not contain any cells
	delete table;
    }

    currentStyle->text.indent = oldindent;
    divAlign = olddivalign;
    flow = oldFlow;
    _clue = oldClue;

    flow = 0;
}

void KHTMLParser::parseTagTextarea(void)
{
    if ( !form )
	return;

    HTMLString name;
    int rows = 5, cols = 40;

    const Attribute *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( token->id == ATTR_NAME )
	{
	    name = token->value();
	}
	else if ( token->id == ATTR_ROWS )
	{
	    rows = token->value().toInt();
	}
	else if ( token->id == ATTR_COLS )
	{
	    cols = token->value().toInt();
	}
    }

    formTextArea = NEW HTMLTextArea( HTMLWidget, name, rows, cols,
						 currentFont() );
    formTextArea->setForm( form );
    form->addElement( formTextArea );
    if ( !flow )
        newFlow();

    flow->append( formTextArea );

    formText = "";
    inTextArea = true;
    pushBlock(tagID, 3, &KHTMLParser::blockEndTextarea);
}

void KHTMLParser::parseTagTitle(void)
{
    title = "";
    inTitle = true;
    pushBlock(tagID, 3, &KHTMLParser::blockEndTitle);
}

void KHTMLParser::parseTagTT(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.family = settings->fixedFontFace;
    currentStyle->font.fixed = true;
    currentStyle->font.style = CSSStyleFont::stNormal;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();    
}

void KHTMLParser::parseTagU(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.decoration = CSSStyleFont::decUnderline;
    setFont();
}

void KHTMLParser::parseTagUL(void)
{
    // Used by DIR, MENU and UL!
    popBlock(ID_A); // Close any <A..> tags
    if ( listStack.isEmpty() )
    {
	vspace_inserted = insertVSpace( vspace_inserted );
	pushBlock( tagID, 2, &KHTMLParser::blockEndList, true);
    }
    else
    {
	pushBlock( tagID, 2, &KHTMLParser::blockEndList, false);
    }

    ListType type = Unordered;

    const Attribute *token;
    
    while( (token = ht->nextOption()) != 0)
    {
        if ( token->id == ATTR_PLAIN )
	    type = UnorderedPlain;
    }

    listStack.push( new HTMLList( type ) );
    currentStyle->text.indent += INDENT_SIZE;
    flow = 0;
}

void KHTMLParser::parseTagVar(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.style = CSSStyleFont::stItalic;
    setFont();
}
