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
#include <kcharsets.h>

#include <X11/Xlib.h>

// Debug function
void debugM( const char *msg , ...);

#define INDENT_SIZE		30

//----------------------------------------------------------------------------
// convert number to roman numerals
QString toRoman( int number, bool upper )
{
    QString roman;
    char ldigits[] = { 'i', 'v', 'x', 'l', 'c', 'd', 'm' };
    char udigits[] = { 'I', 'V', 'X', 'L', 'C', 'D', 'M' };
    char *digits = upper ? udigits : ldigits;
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

void setNamedColor(QColor &color, const char *name)
{
    char *endP;
    if ((*name != '#') && (strlen(name) == 6) &&
        (strtoul(name, &endP, 16), (endP == name+6)) )
    {
        QString col("#");
        col += name;
        color.setNamedColor(col.data());
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
&KHTMLParser::parseTagCaption,		0,	                // ID_CAPTION
&KHTMLParser::parseTagCell,		0,			// ID_CELL
&KHTMLParser::parseTagCenter,	&KHTMLParser::parseTagEnd,	// ID_CENTER
&KHTMLParser::parseTagCite,	&KHTMLParser::parseTagEnd,	// ID_CITE
&KHTMLParser::parseTagCode,	&KHTMLParser::parseTagEnd,	// ID_CODE
&KHTMLParser::parseTagCol,		0,	                // ID_COL
&KHTMLParser::parseTagColgroup,		0,	                // ID_COLGROUP
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
&KHTMLParser::parseTagI,	&KHTMLParser::parseTagEnd,	// ID_I
&KHTMLParser::parseTagIframe,		0,			// ID_IFRAME
&KHTMLParser::parseTagImg,		0,			// ID_IMG
&KHTMLParser::parseTagInput,		0,			// ID_INPUT
&KHTMLParser::parseTagIns,		0,			// ID_INS
&KHTMLParser::parseTagIsindex,		0,			// ID_ISINDEX
&KHTMLParser::parseTagKbd,	&KHTMLParser::parseTagEnd,	// ID_KBD
&KHTMLParser::parseTagLabel,		0,			// ID_LABEL
&KHTMLParser::parseTagLegend,		0,			// ID_LEGEND
&KHTMLParser::parseTagPre,	&KHTMLParser::parseTagEnd,	// ID_LISTING
&KHTMLParser::parseTagLi,		0,			// ID_LI
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
&KHTMLParser::parseTagTable,	&KHTMLParser::parseTagTableEnd,	// ID_TABLE
	0,		0,		// ID_TBODY
&KHTMLParser::parseTagTD,	&KHTMLParser::parseTagEnd,	// ID_TD
&KHTMLParser::parseTagTextarea,	&KHTMLParser::parseTagEnd,	// ID_TEXTAREA
	0,		0,		// ID_TFOOT (Tables only)
&KHTMLParser::parseTagTD,	&KHTMLParser::parseTagEnd,	// ID_TH
	0,		0,		// ID_THEAD (Tables only)
&KHTMLParser::parseTagTitle,	&KHTMLParser::parseTagEnd, 	// ID_TITLE
&KHTMLParser::parseTagTR,	&KHTMLParser::parseTagEnd,     	// ID_TR
&KHTMLParser::parseTagTT,	&KHTMLParser::parseTagEnd,	// ID_TT
&KHTMLParser::parseTagU,	&KHTMLParser::parseTagEnd,	// ID_U
&KHTMLParser::parseTagUL,	&KHTMLParser::parseTagEnd,	// ID_UL
&KHTMLParser::parseTagVar,	&KHTMLParser::parseTagEnd	// ID_VAR
};


KHTMLParser::KHTMLParser( KHTMLWidget *_parent,
			  HTMLTokenizer *_ht, 
			  QPainter *_painter,
			  HTMLSettings *_settings,
			  QStrList *_formData) 			
{
    HTMLWidget    = _parent;
    ht            = _ht;
    painter       = _painter;
    settings      = _settings;
    formData      = _formData;
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

    charsetConverter = 0;
    
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

    baseTarget = 0;
    target = 0;
    url = 0;
    title = 0;
    formText = 0;
    inNoframes = false;

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
                fontsize, settings->fontSizes, 
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

void KHTMLParser::blockEnd( HTMLStackElem *Elem)
{
    vspace_inserted = insertVSpace( vspace_inserted );
    flow = 0;
}

void KHTMLParser::blockEndAnchor( HTMLStackElem *Elem)
{  
    url = 0;
    target = 0;
}

void KHTMLParser::blockEndPre( HTMLStackElem *Elem)
{
    // We add a hidden space to get the height
    // If there is no flow box, we add one.
    if (!flow)
    	newFlow();
   
    flow->append(new HTMLHSpace( currentFont(), painter, true ));

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

void KHTMLParser::blockEndForm( HTMLStackElem *Elem)
{
    vspace_inserted = insertVSpace( vspace_inserted );
    flow = 0;
    form = 0;
}

void KHTMLParser::blockEndTextarea( HTMLStackElem *Elem)
{
    formTextArea->setText( formText );
    inTextArea = false;
    vspace_inserted = false;
    formTextArea = 0;
}

void KHTMLParser::blockEndTitle( HTMLStackElem *Elem)
{
    HTMLWidget->setNewTitle( title.data() );
    inTitle = false;	
}

bool KHTMLParser::insertVSpace( bool _vspace_inserted )
{
    if ( !_vspace_inserted )
    {
	HTMLClueFlow *f = new HTMLClueFlow();
	_clue->append( f );
	HTMLVSpace *t = new HTMLVSpace( settings->fontSizes[settings->fontBaseSize] );
	f->append( t );
	flow = 0;
    }
    
    return true;
}

void KHTMLParser::newFlow()
{
    if (inPre)
         flow = new HTMLClueH();
	    else
         flow = new HTMLClueFlow();

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
void KHTMLParser::insertText(char *str, const HTMLFont * fp)
{
    enum { unknown, fixed, variable} textType = unknown; 
    	// Flag, indicating whether text may be broken
    int i = 0;
    char *remainingStr = 0;
    bool insertSpace = false;
    bool insertNBSP = false;
    bool insertBlock = false;
    
    for(;;)
    {
        if (((unsigned char *)str)[i] == 0xa0)
        {
            // Non-breaking space
            if (textType == variable)
            {
                // We have a non-breaking space in a block of variable text
                // We need to split the text and insert a seperate
                // non-breaking space object
                str[i] = 0x00; // End of string
                remainingStr = &(str[i+1]);
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
        else if (str[i] == 0x20)
        {
            // Normal space
            if (textType == fixed)
            {
            	// We have a normal space in a block of fixed text.
            	// We need to split the text and insert a seperate normal
            	// space.
            	str[i] = 0x00; // End of string
            	remainingStr = &(str[i+1]);
            	insertBlock = true;
            	insertSpace = true;
            }
            else
            {
            	// We have a normal space: if this is the first character
            	// we insert a normal space and continue
            	if (i == 0)
            	{
            	    if (str[i+1] == 0x00)
            	    {
            	    	str++;
            	    	remainingStr = 0;
            	    }
            	    else
            	    {
            	        str[i] = 0x00; // End of string
            	        remainingStr = str+1;
            	    }
                    insertBlock = true; // Block is zero-length, no actual insertion
            	    insertSpace = true;
            	}
            	else if (str[i+1] == 0x00)
            	{
            	    // Last character is a space: Insert the block and 
            	    // a normal space
            	    str[i] = 0x00; // End of string
            	    remainingStr = 0;
            	    insertBlock = true;
            	    insertSpace = true;
            	}
            	else
            	{
            	    textType = variable;
            	}
            }
        } 
        else if (str[i] == 0x00)
        {
            // End of string
            insertBlock = true;
            remainingStr = 0;
        }
        
        if (insertBlock)
        {
            if (*str)
            {
                if (textType == variable)
                {
		    if ( url || target )
	       		flow->append( new HTMLLinkTextMaster( str, 
	       			      fp, painter, url, target) );
	            else
		       	flow->append( new HTMLTextMaster( str, fp, painter ) );
                }
                else
                {
	            if ( url || target )
	       	        flow->append( new HTMLLinkText( str, 
	       	                      fp, painter, url, target ) );
		    else
		        flow->append( new HTMLText( str, fp, painter ) );
		}
            }
            if (insertSpace)
            {
                if ( url || target)
                {
		    HTMLLinkText *sp = new HTMLLinkText( " ", fp, painter,
			url, target );
		    sp->setSeparator( true );
		    flow->append( sp );
		}
                else
                {
   	            flow->append( new HTMLHSpace( fp, painter));
   	        }
            }
            else if (insertNBSP)
            {
                if ( url || target)
                {
		    HTMLLinkText *sp = new HTMLLinkText( " ", fp, painter,
			url, target );
		    sp->setSeparator( false );
		    flow->append( sp );
		}
                else
                {
                    HTMLHSpace *sp = new HTMLHSpace( fp, painter);
                    sp->setSeparator(false);
   	            flow->append( sp );
   	        }
            }
            str = remainingStr;
            if ((str == 0) || (*str == 0x00))
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

int KHTMLParser::parseBody( HTMLClue *__clue, const char _end[], bool toplevel )
{
    const char *str;
    
    // Before one can write any objects in the body he usually adds a FlowBox
    // to _clue. Then put all texts, images etc. in the FlowBox.
    // If f == 0, you have to create a new FlowBox, otherwise you can
    // use the one stored in f. Setting f to 0 means closing the FlowBox.

    _clue = __clue;

    if ( toplevel)
    {
	parseCount = granularity;
	// Be sure to set the painter to the current font.
        restoreFont();
    }

    while ( ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// ignore stuff inbetween <noframes> and </noframes> if
	// this has a htmlView
	if(inNoframes)
	{
	    if (*str++ != TAG_ESCAPE)
	        continue;
            tagID = *((unsigned char *) str);
	    if (tagID == (ID_NOFRAMES + ID_CLOSE_TAG))
		inNoframes = false;
	    
	    continue;
	}

	if ( *str == '\0' )
	    continue;

	if ( *str == ' ' && *(str+1) == '\0' )
	{
	    // if in* is set this text belongs in a form element
	    if ( inOption || inTextArea )
		formText += " ";
	    else if ( inTitle )
		title += " ";
	    else if ( flow != 0)
	    {
	    	insertText( " ", currentFont());
	    }
	}
	else if ( *str != TAG_ESCAPE )
	{
	    // if in* is set this text belongs in a form element
	    if ( inOption || inTextArea )
	    {
		formText += str;
	    }
	    else if ( inTitle )
	    {
		title += str;
	    }
	    else
	    {
		vspace_inserted = false;

	    	if (!flow)
	            newFlow();
		
    	        if (charsetConverter){
		   debugM("Using charset converter...");
		   QList<KCharsetConversionResult> rl=
		           charsetConverter->multipleConvert(str);
		   debugM("OK\n");
		   KCharsetConversionResult *r;
		   for(r=rl.first();r;r=rl.next()){ 
		        debugM("Getting result string...");
		      	char *str1= ht->newString(*r);
		        debugM("Got: %s",str1);
			debugM("Getting current font...");
		        HTMLFont f=*currentFont();
			debugM("OK\n");
			debugM("Setting charset to %s...",(const char *)r->charset());
			f.setCharset(r->charset());
			debugM("OK\n");
			debugM("Getting preloaded font...");
	                const HTMLFont *fp = pFontManager->getFont( f );
			debugM("OK\n");
		       
			debugM("Adding string to flow...");

			insertText(str1 , fp);
			
			debugM("OK\n");
		  }		
		}
		else{
		  insertText((char *)str, currentFont() ); // Override constness
		}      
	    }
	}
	else
	{
	    str++;

	    tagID = *((unsigned char *) str);

	    if (index(_end, *str))
	    {
           	str++;
	        return (tagID);
	    }
	    str++;
#if 0
	    int i = 0;

	    while ( _end[i] != 0 )
	    {
		if ( strncasecmp( str, _end[i], strlen( _end[i] ) ) == 0 )
		{
		    return str;
		}
		i++;
	    }
#endif	    
	    // The tag used for line break when we are in <pre>...</pre>
	    if ( tagID == ID_NEWLINE )
	    {
		if (!flow)
		    newFlow();

		// Add a hidden space to get the line-height right.
		flow->append(new HTMLHSpace( currentFont(), painter, true ));
		vspace_inserted = false;
	
		newFlow(); // Explicitly make a new flow! 
	    }
	    else if (tagID == ID_ENTITY)
	    {
		int l;
		const char *str1;

		if (!flow)
		    newFlow();

	    	const HTMLFont *fp = currentFont();
		// Handling entities
		char *buffer=new char[strlen(str)+2]; // buffer will never
		                                      // have to be longer

		KCharsets *charsets=
			KApplication::getKApplication()->getCharsets();

		const KCharsetConversionResult &r=charsets->convertTag(str,l);
		
		str1=r;					 
		
		if (str1 && l){
		    HTMLFont f=*fp;
		    if (r.charset().ok()){
		        f.setCharset(r.charset());
	                fp = pFontManager->getFont( f );
		    } 
		    strcpy(buffer,str1);
		    strcat(buffer,str+l);

		    insertText(ht->newString(buffer), fp); // Override constness
		}
		else
		{
		    insertText((char *)str, currentFont());
		}
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

void KHTMLParser::parseOneToken()
{
    if ((tagID < 0) || (tagID > ID_MAX))
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
    target = 0;
    bool visited = false;
    const char *p;

    while ( ( p = ht->nextOption() ) != 0 )
    {
	if ( strncasecmp( p, "href=", 5 ) == 0 )
	{
	    p += 5;
	    if ( *p == '#' )
	    {// reference
		KURL u( HTMLWidget->getDocumentURL() );
		u.setReference( p + 1 );
	        tmpurl = u.url();
	    }
	    else
	    {
                KURL u( HTMLWidget->getBaseURL(), p );
	        tmpurl = u.url();
	    }		
            visited = HTMLWidget->URLVisited( tmpurl );
	}
	else if ( strncasecmp( p, "name=", 5 ) == 0 )
	{
	    if ( flow == 0 )
	        _clue->append( new HTMLAnchor( p+5 ) );
	    else
	        flow->append( new HTMLAnchor( p+5 ) );
	}
	else if ( strncasecmp( p, "target=", 7 ) == 0 )
	{
	    target = p+7;
	}
    }

    if ( !target )
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
        url = ht->newString( tmpurl.data(), tmpurl.length() );
        HTMLWidget->addHref(tmpurl.data());
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

    char * href = "";
    QString coords;
    const char * atarget = baseTarget;
    HTMLArea::Shape shape = HTMLArea::Rect;

    const char* p;
    
    while ( 0 != (p = ht->nextOption()) )
    {
	if ( strncasecmp( p, "shape=", 6 ) == 0 )
	{
	    if ( strncasecmp( p+6, "rect", 4 ) == 0 )
	        shape = HTMLArea::Rect;
	    else if ( strncasecmp( p+6, "poly", 4 ) == 0 )
	        shape = HTMLArea::Poly;
	    else if ( strncasecmp( p+6, "circle", 6 ) == 0 )
	        shape = HTMLArea::Circle;
	}
	else if ( strncasecmp( p, "href=", 5 ) == 0 )
	{
	    p += 5;
	    if ( *p == '#' )
	    {// reference
	        KURL u( HTMLWidget->getDocumentURL() );
	        u.setReference( p + 1 );
	        href = ht->newString(u.url().data());
            }
            else 
	    {
	        KURL u( HTMLWidget->getBaseURL(), p );
	        href = ht->newString(u.url().data());
	    }
	}
	else if ( strncasecmp( p, "target=", 7 ) == 0 )
	{
	    atarget = p+7;
	}
	else if ( strncasecmp( p, "coords=", 7 ) == 0 )
	{
	    coords = p+7;
	}
    }

    if ( coords.isEmpty() )
    	return;
    	
    HTMLArea *area = 0;

    switch ( shape )
    {
        case HTMLArea::Rect:
	    {
		int x1, y1, x2, y2;
		sscanf( coords, "%d,%d,%d,%d", &x1, &y1, &x2, &y2 );
		QRect rect( x1, y1, x2-x1, y2-y1 );
		area = new HTMLArea( rect, href, atarget );
		debugM( "Area Rect %d, %d, %d, %d\n", x1, y1, x2, y2 );
	    }
	    break;

        case HTMLArea::Circle:
	    {
		int xc, yc, rc;
		sscanf( coords, "%d,%d,%d", &xc, &yc, &rc );
		area = new HTMLArea( xc, yc, rc, href, atarget );
		debugM( "Area Circle %d, %d, %d\n", xc, yc, rc );
	    }
	    break;

  	case HTMLArea::Poly:
	    {
		debugM( "Area Poly " );
		int count = 0, x, y;
		QPointArray parray;
		const char *ptr = coords;
		while ( ptr )
		{
		    x = atoi( ptr );
		    ptr = strchr( ptr, ',' );
		    if ( ptr )
		    {
			y = atoi( ++ptr );
			parray.resize( count + 1 );
			parray.setPoint( count, x, y );
			debugM( "%d, %d  ", x, y );
			count++;
			ptr = strchr( ptr, ',' );
			if ( ptr ) ptr++;
		    }
		}
		debugM( "\n" );
		if ( count > 2 )
			area = new HTMLArea( parray, href, atarget );
	    }
	    break;
    }
    if (strlen(href))
        HTMLWidget->addHref(href);
        
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
    const char *token;
    while ( (token = ht->nextOption()) != 0 )
    {
        if ( strncasecmp( token, "target=", 7 ) == 0 )
        {
            baseTarget = token+7;
	}
	else if ( strncasecmp( token, "href=", 5 ) == 0 )
	{
	    HTMLWidget->setBaseURL( token + 5);
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

    char *token;
    while ( 0 != (token = ht->nextOption()) )
    {
	if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
	{
	    QColor bgColor;
	    setNamedColor( bgColor, token+8 );
	    HTMLWidget->setBGColor( bgColor );
	}
	else if ( strncasecmp( token, "background=", 11 ) == 0 )
	{
	    HTMLWidget->setBGImage( token + 11 );
        }
	else if ( strncasecmp( token, "text=", 5 ) == 0 )
	{
	    setNamedColor( settings->fontBaseColor, token+5 );
	    currentStyle->font.color = settings->fontBaseColor;
	    setFont();
	}
	else if ( strncasecmp( token, "link=", 5 ) == 0 )
	{
	    setNamedColor( settings->linkColor, token+5 );
	}
	else if ( strncasecmp( token, "vlink=", 6 ) == 0 )
	{
	    setNamedColor( settings->vLinkColor, token+6 );
	}
    }
}

void KHTMLParser::parseTagBr(void)
{
    //attrs: %coreattrs

    HTMLVSpace::Clear clear = HTMLVSpace::CNone;

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "clear=", 6 ) == 0 )
	{
	    if ( strcasecmp( token+6, "left" ) == 0 )
	        clear = HTMLVSpace::Left;
	    else if ( strcasecmp( token+6, "right" ) == 0 )
	        clear = HTMLVSpace::Right;
	    else if ( strcasecmp( token+6, "all" ) == 0 )
	        clear = HTMLVSpace::All;
        }
    }

    if (!flow)
        newFlow();

    flow->append( new HTMLVSpace( currentFont()->pointSize(), clear ));
    vspace_inserted = false;
}

void KHTMLParser::parseTagButton(void)
{
    // forms... HTML4 unimplemented
}

void KHTMLParser::parseTagCaption(void)
{
    HTMLTable *table = tableStack.current();
    if(!table) return;

    HTMLClueV *caption;
    HTMLClue::VAlign capAlign = HTMLClue::Bottom;

    static const char endcap[] = { ID_CAPTION + ID_CLOSE_TAG,
    				   ID_TABLE + ID_CLOSE_TAG,
    				   ID_TR, ID_TD, ID_TH,
   				   ID_BODY + ID_CLOSE_TAG, 
    				   0 };    

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strncasecmp( token+6, "top", 3 ) == 0)
		capAlign = HTMLClue::Top;
	}
    }
    caption = new HTMLClueV();
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
	// CC: Close table description in case of a malformed table
	// before returning!
	table->endRow();
	table->endTable(); 
	popBlock( ID_TABLE );
    }
}

void KHTMLParser::parseTagCell(void)
{
    if (!flow)
        newFlow();

    static const char end[] = { ID_CELL + ID_CLOSE_TAG, 0 }; 
    HTMLClue::HAlign olddivalign = divAlign;
    HTMLClue *oldFlow = flow;
    HTMLClue *__clue = flow;
    HTMLClue *oldClue = _clue;
    int oldindent = currentStyle->text.indent;

    HTMLClue::HAlign gridHAlign = HTMLClue::HCenter;// global align of all cells
    int cell_width = 90;

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
	    cell_width = atoi( token + 6 );
	}
	else if ( strncasecmp( token, "align=", 6 ) == 0 )
	{
	    if ( strcasecmp( token + 6, "left" ) == 0 )
		gridHAlign = HTMLClue::Left;
	    else if ( strcasecmp( token + 6, "right" ) == 0 )
		gridHAlign = HTMLClue::Right;
	}
    }
    
    HTMLClue::VAlign valign = HTMLClue::Top;
    HTMLClue::HAlign halign = gridHAlign;
    
    HTMLClueV *vc = new HTMLCell( url, target );     
    __clue->append( vc );
    vc->setFixedWidth( cell_width ); // fixed width
    vc->setVAlign( valign );
    vc->setHAlign( halign );

    flow = 0;
    currentStyle->text.indent = 0;
    divAlign = HTMLClue::Left;
    _clue = vc;             
    
    pushBlock( ID_CELL, 3 );
    (void) parseBody( _clue, end );
    popBlock( ID_CELL );

// @@WABA What does this do?
//    vc = new HTMLClueV( 0, 0 ); // fixed width
//    _clue->append( vc );

    currentStyle->text.indent = oldindent;
    divAlign = olddivalign;
    flow = oldFlow;
    _clue = oldClue;

    // Add a hidden space
    flow->append( new HTMLHSpace( currentFont(), painter, true ) );
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
    currentStyle->font.style = CSSStyleFont::stNormal;
    currentStyle->font.weight = CSSStyleFont::Normal;
    setFont();
}

void KHTMLParser::parseTagCol(void)
{
    parseTagCol( UNDEFINED, HTMLTable::Variable,
		 HTMLClue::VNone, HTMLClue::HNone);
}

void KHTMLParser::parseTagCol(int defWidth, HTMLTable::ColType colType,
			      HTMLClue::VAlign valign, HTMLClue::HAlign halign)
{
    HTMLTable *table = tableStack.current();
    if(!table) return;

    int span = 1;

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "span=", 5 ) == 0)
	{
	    span = atoi( token+5 );
	    if ( span < 1 )
		span = 1;
	}
	else if ( strncasecmp( token, "valign=", 7 ) == 0)
	{
	    if ( strncasecmp( token+7, "top", 3 ) == 0)
		valign = HTMLClue::Top;
	    else if ( strncasecmp( token+7, "bottom", 6 ) == 0)
		valign = HTMLClue::Bottom;
	    else
		valign = HTMLClue::VCenter;
	}
	else if ( strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strcasecmp( token+6, "center" ) == 0)
		halign = HTMLClue::HCenter;
	    else if ( strcasecmp( token+6, "right" ) == 0)
		halign = HTMLClue::Right;
	    else if ( strcasecmp( token+6, "left" ) == 0)
		halign = HTMLClue::Left;
	}
	else if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
	    if ( strchr( token + 6, '%' ) )
		colType = HTMLTable::Percent;
	    else if ( strchr( token+6, '*' ) )
		colType = HTMLTable::Relative;
	    else
		colType = HTMLTable::Fixed; // Fixed with
	    defWidth = atoi( token + 6 );
	}
    }

    table->addColumns(span, defWidth, colType, halign, valign);
}

void KHTMLParser::parseTagColgroup(void)
{
    HTMLTable *table = tableStack.current();
    if(!table) return;

    HTMLTable::ColType colType = HTMLTable::Variable;
    HTMLClue::VAlign valign = HTMLClue::VNone;
    HTMLClue::HAlign halign = HTMLClue::HNone;
    int defWidth = UNDEFINED;
    int span = 1;
    int width = UNDEFINED;

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "span=", 5 ) == 0)
	{
	    span = atoi( token+5 );
	    if ( span < 1 )
		span = 1;
	}
	else if ( strncasecmp( token, "valign=", 7 ) == 0)
	{
	    if ( strncasecmp( token+7, "top", 3 ) == 0)
		valign = HTMLClue::Top;
	    else if ( strncasecmp( token+7, "bottom", 6 ) == 0)
		valign = HTMLClue::Bottom;
	    else
		valign = HTMLClue::VCenter;
	}
	else if ( strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strcasecmp( token+6, "center" ) == 0)
		halign = HTMLClue::HCenter;
	    else if ( strcasecmp( token+6, "right" ) == 0)
		halign = HTMLClue::Right;
	    else if ( strcasecmp( token+6, "left" ) == 0)
		halign = HTMLClue::Left;
	}
	else if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
	    if ( strchr( token + 6, '%' ) )
		colType = HTMLTable::Percent;
	    else if ( strchr( token+6, '*' ) )
		colType = HTMLTable::Relative;
	    else
		colType = HTMLTable::Fixed; // Fixed with
	    defWidth = atoi( token + 6 );
	}
    }

    bool found_col = false;
    bool found_end = false;

    // parse until we find anything else than <col> or </colgroup>
    const char *str;
    while ( ht->hasMoreTokens() )
    {
	str = ht->nextToken();
    
	if( *str != TAG_ESCAPE ) break; //FIXME: what if some text follows???
	str++;
	
	if( *str == ID_COL )
	    parseTagCol( width, colType, valign, halign);
	else if( *str == (ID_COLGROUP + ID_CLOSE_TAG) )
	{
	    found_end = true;
	    break;
	}
	else
	    break;
    }

    if(!found_col)
	table->addColumns(span, defWidth, colType, halign, valign);
    if(!found_end)
    {
	// the current token is not </colgroup>
	tagID = *((unsigned char *) str);
	parseOneToken();
    }
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

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
        // attrs: %attrs
        if ( strncasecmp( token, "align=", 6 ) == 0 )
        {
            if ( strcasecmp( token + 6, "right" ) == 0 )
	        divAlign = HTMLClue::Right;
	    else if ( strcasecmp( token + 6, "center" ) == 0 )
	        divAlign = HTMLClue::HCenter;
	    else if ( strcasecmp( token + 6, "left" ) == 0 )
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
    char *token;
    while ( 0 != (token = ht->nextOption()) )
    {
	if ( strncasecmp( token, "size=", 5 ) == 0 )
	{
	    int num = atoi( token + 5 );
	    if ( *(token + 5) == '+' || *(token + 5) == '-' )
		currentStyle->font.size = settings->fontBaseSize + num;
	    else
		currentStyle->font.size = num;
	}
	else if ( strncasecmp( token, "color=", 6 ) == 0 )
	{
            setNamedColor( currentStyle->font.color, token+6 );
	}
	else if ( strncasecmp( token, "face=", 5 ) == 0 )
	{
	    // try to find a matching font in the font list.
	    StringTokenizer st;
            st.tokenize( token+5, " ," );
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

    const char* token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "action=", 7 ) == 0 )
	{
            KURL u( HTMLWidget->getBaseURL(), token + 7 );
            action = u.url();
	}
	else if ( strncasecmp( token, "method=", 7 ) == 0 )
	{
            if ( strncasecmp( token + 7, "post", 4 ) == 0 )
		method = "POST";
	}
	else if ( strncasecmp( token, "target=", 7 ) == 0 )
	{
	    target = token+7;
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

    const char *src = 0;
    const char *name = 0;
    int marginwidth = leftBorder;
    int marginheight = rightBorder;

    // 0 = no, 1 = yes, 2 = auto
    int scrolling = 2;
    bool noresize = FALSE;

    // -1 = default ( 5 )
    int frameborder = -1;
	      
    const char *token;
    while ( (token = ht->nextOption()) != 0)
    { 
	if ( strncasecmp( token, "SRC=", 4 ) == 0 )
	    src = token + 4;

	else if ( strncasecmp( token, "NAME=", 5 ) == 0 )
	    name = token + 5;

	else if ( strncasecmp( token, "MARGINWIDTH=", 12 ) == 0 )
	    marginwidth = atoi( token + 12 );

	else if ( strncasecmp( token, "MARGINHEIGHT=", 13 ) == 0 )
	    marginheight = atoi( token + 13 );

	else if ( strncasecmp( token, "FRAMEBORDER=", 12 ) == 0 )
	{
	    frameborder = atoi( token + 12 );
	    if ( frameborder < 0 )
	        frameborder = -1;
	}

	else if ( strncasecmp( token, "NORESIZE", 8 ) == 0 )
	    noresize = TRUE;

	else if ( strncasecmp( token, "SCROLLING=", 10 ) == 0 )
        {
	    if ( strncasecmp( token + 10, "yes", 3 ) == 0 )
	        scrolling = 1;
	    if ( strncasecmp( token + 10, "no", 2 ) == 0 )
		scrolling = 0;
	    if ( strncasecmp( token + 10, "auto", 4 ) == 0 )
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

  QString src;
  QString name;
  QString type;
  int marginwidth = leftBorder;
  int marginheight = rightBorder;
  bool noresize = FALSE;
  // -1 = default ( 5 )
  int frameborder = -1;
	
  const char *token;
  while ( ( token = ht->nextOption() ) != 0 )
  {
    if ( strncasecmp( token, "SRC=", 4 ) == 0 )
    {
      src = token + 4;
    }
    else if ( strncasecmp( token, "NAME=", 5 ) == 0 )
    {
      name = token + 5;
    }
    else if ( strncasecmp( token, "TYPE=", 4 ) == 0 )
    {
      type = token + 4;
    }
    else if ( strncasecmp( token, "MARGINWIDTH=", 12 ) == 0 )
    {
      marginwidth = atoi( token + 12 );
    }
    else if ( strncasecmp( token, "MARGINHEIGHT=", 13 ) == 0 )
    {
      marginheight = atoi( token + 13 );
    }
    else if ( strncasecmp( token, "FRAMEBORDER=", 12 ) == 0 )
    {
      frameborder = atoi( token + 12 );
      if ( frameborder < 0 )
	frameborder = -1;
    }
    else if ( strncasecmp( token, "NORESIZE", 8 ) == 0 )
      noresize = TRUE;
  }	      

  // Create the widget
  KHTMLEmbededWidget *embed = HTMLWidget->getView()->newEmbededWidget( frameSet, name,
								       src, type,
								       marginwidth,
								       marginheight, 
								       frameborder,
								       !noresize );
  HTMLWidget->addEmbededFrame( frameSet, embed );
}

void KHTMLParser::parseTagFrameset(void)
{
    const char *cols = 0;
    const char *rows = 0;
    int frameBorder = 1;
    bool bAllowResize = true;

    // We need a view to do frames
    if ( !HTMLWidget->getView())
    	return;

    const char *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "cols=", 5 ) == 0 )
	{
	    cols = token + 5;
	}
	else if ( strncasecmp( token, "rows=", 5 ) == 0 )
	{
	    rows = token + 5;
	}
	else if ( strncasecmp( token, "frameborder=", 12 ) == 0 )
	{
	    frameBorder = atoi( token + 12 );
	}
	else if ( strncasecmp( token, "noresize", 8 ) == 0 )
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

    char *token;
    vspace_inserted = insertVSpace( vspace_inserted );
    HTMLClue::HAlign align = divAlign;

    while( 0 != (token = ht->nextOption()) )
    {
	if ( strncasecmp( token, "align=", 6 ) == 0 )
	{
            if ( strcasecmp( token + 6, "center" ) == 0 )
		align = HTMLClue::HCenter;
            else if ( strcasecmp( token + 6, "right" ) == 0 )
		align = HTMLClue::Right;
            else if ( strcasecmp( token + 6, "left" ) == 0 )
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

    const char* token;
    
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "align=", 6 ) == 0 )
	{
            if ( strcasecmp( token + 6, "left" ) == 0 )
		align = HTMLRule::Left;
            else if ( strcasecmp( token + 6, "right" ) == 0 )
		align = HTMLRule::Right;
	    else if ( strcasecmp( token + 6, "center" ) == 0 )
		align = HTMLRule::HCenter;
	}
	else if ( strncasecmp( token, "size=", 5 ) == 0 )
	{
	    size = atoi( token+5 );
	}
	else if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
            if ( strchr( token+6, '%' ) )
            {
                length = UNDEFINED;
		percent = atoi( token+6 );
	    }
	    else
	    {
		length = atoi( token+6 );
		percent = UNDEFINED; // fixed width
	    }
	}
	else if ( strncasecmp( token, "noshade", 6 ) == 0 )
	{
            shade = FALSE;
	}
    }

    divAlign = align;
    newFlow();

    flow->append( new HTMLRule( length, percent, size, shade ) );
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
    const char* filename = 0;
    const char *overlay = 0;
    QString fullfilename;
    QString usemap;
    bool    ismap = false;
    int width = UNDEFINED;
    int height = UNDEFINED;
    int percent = UNDEFINED;
    int border = url == 0 ? 0 : 2;
    HTMLClue::HAlign align = HTMLClue::HNone;
    HTMLClue::VAlign valign = HTMLClue::VNone;

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
        if (strncasecmp( token, "src=", 4 ) == 0)
	    filename = token + 4;
	else if (strncasecmp( token, "oversrc=", 8 ) == 0)
	    overlay = token + 8;
	else if (strncasecmp( token, "width=", 6 ) == 0)
	{
	    if ( strchr( token + 6, '%' ) )
		percent = atoi( token + 6 );
	    else {
	        width = atoi( token + 6 );
	        percent = UNDEFINED;
	    }
	}
	else if (strncasecmp( token, "height=", 7 ) == 0)
	    height = atoi( token + 7 );
	else if (strncasecmp( token, "border=", 7 ) == 0)
	    border = atoi( token + 7 );
	else if (strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strcasecmp( token + 6, "left" ) == 0 )
	        align = HTMLClue::Left;
	    else if ( strcasecmp( token + 6, "right" ) == 0 )
	        align = HTMLClue::Right;
	    else if ( strcasecmp( token + 6, "top" ) == 0 )
	        valign = HTMLClue::Top;
	    else if ( strcasecmp( token + 6, "middle" ) == 0 )
	        valign = HTMLClue::VCenter;
	    else if ( strcasecmp( token + 6, "bottom" ) == 0 )
	        valign = HTMLClue::Bottom;
        }
	else if ( strncasecmp( token, "usemap=", 7 ) == 0 )
	{
	    if ( *(token + 7 ) == '#' )
	    {
	        // Local map. Format: "#name"
                usemap = token + 7;
	    }
	    else
	    {
	        KURL u( HTMLWidget->getBaseURL(), token + 7 );
                usemap = u.url();
	    }
	}
	else if ( strncasecmp( token, "ismap", 5 ) == 0 )
	{
	    ismap = true;
	}
    }
    // if we have a file name do it...
    if ( filename != 0 )
    {
        printf("**************** IMAGE Parsing URL=%s\n", filename );
      
        KURL kurl( HTMLWidget->getBaseURL(), filename );
        // Do we need a new FlowBox ?
        if ( !flow )
            newFlow();

	HTMLImage *image;

	if ( usemap.isEmpty() && !ismap )
	{
	    image =  new HTMLImage( HTMLWidget, kurl.url(), url, target,
			                width, height, percent, border );
	}
	else
	{
	    image =  new HTMLImageMap( HTMLWidget, kurl.url(), url, target,
			                   width, height, percent, border );
	    if ( !usemap.isEmpty() )
	    {
	        ((HTMLImageMap *)image)->setMapURL( ht->newString( usemap.data() ) );
	    }
	    else
	        ((HTMLImageMap *)image)->setMapType( HTMLImageMap::ServerSide );
	}

	// used only by kfm to overlay links, readonly etc.
	if ( overlay )
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
		    HTMLClueH *valigned = new HTMLClueH();
		    valigned->setVAlign( valign );
		    valigned->append( image );
		    flow->append( valigned );
	    	}
#endif
        }
	// we need to put the image in a HTMLClueAligned
	else
	{
	    HTMLClueAligned *aligned = new HTMLClueAligned( flow );
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
    const char *p;
    InputType type = Text;
    QString name = "";
    QString value = "";
    QString imgSrc;
    bool checked = false;
    int size = 20;
    int maxLen = -1;
    QList<JSEventHandler> *handlers = 0;

    const char* token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "type=", 5 ) == 0 )
	{
	    p = token + 5;
	    if ( *p == '"' ) p++;
	    if ( strncasecmp( p, "checkbox", 8 ) == 0 )
		type = CheckBox;
	    else if ( strncasecmp( p, "password", 8 ) == 0 )
		type = Password;
	    else if ( strncasecmp( p, "hidden", 6 ) == 0 )
		type = Hidden;
	    else if ( strncasecmp( p, "radio", 5 ) == 0 )
		type = Radio;
	    else if ( strncasecmp( p, "reset", 5 ) == 0 )
		type = Reset;
	    else if ( strncasecmp( p, "submit", 5 ) == 0 )
		type = Submit;
	    else if ( strncasecmp( p, "button", 6 ) == 0 )
		type = Button;      
	    else if ( strncasecmp( p, "text", 5 ) == 0 )
		type = Text;
	    else if ( strncasecmp( p, "Image", 5 ) == 0 )
		type = Image;
	}
	else if ( strncasecmp( token, "name=", 5 ) == 0 )
	{
	    p = token + 5;
	    if ( *p == '"' ) p++;
	    name = p;
	    if ( name[ name.length() - 1 ] == '"' )
		name.truncate( name.length() - 1 );
	}
	else if ( strncasecmp( token, "value=", 6 ) == 0 )
	{
	    p = token + 6;
	    if ( *p == '"' ) p++;
	    value = p;
	    if ( value[ value.length() - 1 ] == '"' )
		value.truncate( value.length() - 1 );
	}
	else if ( strncasecmp( token, "size=", 5 ) == 0 )
	{
	    size = atoi( token + 5 );
	}
	else if ( strncasecmp( token, "maxlength=", 10 ) == 0 )
	{
	    maxLen = atoi( token + 10 );
	}
	else if ( strncasecmp( token, "checked", 7 ) == 0 )
	{
	    checked = true;
	}
	else if ( strncasecmp( token, "src=", 4 ) == 0 )
	{
	    imgSrc = token + 4;
	}
	else if ( strncasecmp( token, "onClick=", 8 ) == 0 )
	{
	    QString code;
	    p = token + 8;
	    if ( *p == '"' ) p++;
	    code = p;
	    if ( code[ code.length() - 1 ] == '"' )
		code.truncate( value.length() - 1 );
	    if ( handlers == 0 )
	    {
		handlers = new QList<JSEventHandler>;
		handlers->setAutoDelete( TRUE );
	    }
	    handlers->append( new JSEventHandler( HTMLWidget->getJSEnvironment(), "onClick", code ) );
	}     
    }

    switch ( type )
    {
	case CheckBox:
	    {
		HTMLCheckBox *cb = new HTMLCheckBox( HTMLWidget,name,value,
						     checked, currentFont() );
		cb->setForm( form );
		form->addElement( cb );
		flow->append( cb );
	    }
	    break;

	case Hidden:
	    {
		HTMLHidden *hidden = new HTMLHidden( name, value );
		hidden->setForm( form );
		form->addHidden( hidden );
	    }
	    break;

	case Radio:
	    {
		HTMLRadio *radio = new HTMLRadio( HTMLWidget, name, value, 
						  checked, currentFont() );
		radio->setForm( form );
		form->addElement( radio );
		flow->append( radio );
		QObject::connect(radio,SIGNAL(radioSelected(const char *,const char *)),
		    form, SLOT(slotRadioSelected(const char *, const char *)));
		QObject::connect(form,SIGNAL( radioSelected(const char*,const char *) ),
		    radio, SLOT(slotRadioSelected(const char *, const char *)));
	    }
	    break;

	case Reset:
	    {
		HTMLReset *reset = new HTMLReset( HTMLWidget, value, 
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
		HTMLSubmit *submit = new HTMLSubmit( HTMLWidget, name, value,
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
		HTMLButton *button = new HTMLButton( HTMLWidget,name,value,
						     handlers, currentFont() );
		button->setForm( form );
		form->addElement( button );
		flow->append( button );
		break;
	    }

	case Text:
	case Password:
	    {
		HTMLTextInput *ti = new HTMLTextInput( HTMLWidget, name, value, 
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
	    if ( !imgSrc.isEmpty() )
	    {
		KURL kurl( HTMLWidget->getBaseURL(), imgSrc );
		HTMLImageInput *ii = new HTMLImageInput( HTMLWidget, kurl.url(),
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
    HTMLClueFlow *f = new HTMLClueFlow();
    _clue->append( f );
    HTMLClueH *c = new HTMLClueH();
    c->setVAlign( HTMLClue::Top );
    f->append( c );

//@@WABA: This should be handled differently

    // fixed width spacer
    HTMLClueV *vc = new HTMLClueV();
    vc->setFixedWidth( indentSize ); // Fixed width clue
    vc->setVAlign( HTMLClue::Top );
    c->append( vc );

    switch ( listType )
    {
    case Unordered:
	flow = new HTMLClueFlow();
	flow->setHAlign( HTMLClue::Right );
	vc->append( flow );
	flow->append( new HTMLBullet( currentStyle->font.fp->pointSize(),
		listLevel, settings->fontBaseColor ) );
	break;

    case Ordered:
	flow = new HTMLClueFlow();
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
		item += (char) ('a' + itemNumber - 1);
		break;

	    case UpAlpha:
		item += (char) ('A' + itemNumber - 1);
		break;

	    default:
		item.sprintf( "%2d", itemNumber );
	}
	item += ". ";
	flow->append(
		new HTMLText( ht->newString( item.data(), 
					     item.length() ),
			      currentFont(),
			      painter) 
		);
	break;

    default:
	break;
    }

    vc = new HTMLClueV();
    c->append( vc );
    flow = new HTMLClueFlow();
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
    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "name=", 5 ) == 0)
	{
	    QString mapurl = "#";
	    mapurl += token+5;
	    HTMLWidget->addMap( ht->newString( mapurl.data() ) );
	}
    }
}

void KHTMLParser::parseTagMeta(void)
{
    QString httpequiv;
    QString name;
    QString content;
    
    const char *token;
    
    while( (token = ht->nextOption()) != 0)
    {
	debugM("token: %s\n",token);
	if ( strncasecmp( token, "name=", 5 ) == 0)
	    name=token+5;
	else if ( strncasecmp( token, "http-equiv=", 11 ) == 0)
 	    httpequiv=token+11;
 	else if ( strncasecmp( token, "content=", 8 ) == 0)
	    content=token+8;
    }
    debugM( "Meta: name=%s httpequiv=%s content=%s\n",
                          (const char *)name,(const char *)httpequiv,(const char *)content );
    if (!httpequiv.isEmpty())
    {
        if(strcasecmp(httpequiv.data(),"content-type") == 0)
        {
	    stringTok->tokenize( content, " >;" );
	    while ( stringTok->hasMoreTokens() )
	    {
		const char* token = stringTok->nextToken();
		debugM("token: %s\n",token);
		if ( strncasecmp( token, "charset=", 8 ) == 0)
		    if( !HTMLWidget->overrideCharset )
			setCharset(token+8);
            }                         
	}
	if ( strcasecmp(httpequiv.data(), "refresh") == 0 )
	{
	    stringTok->tokenize( content, " >;" );
	    QString t = stringTok->nextToken();
	    bool ok;
            int delay = t.toInt( &ok );
	    QString url = HTMLWidget->actualURL.url();
	    if ( !ok ) delay = 0;
	    while ( stringTok->hasMoreTokens() )
	    {
	        const char* token = stringTok->nextToken();
	        debugM("token: %s\n",token);
		if ( strncasecmp( token, "url=", 4 ) == 0 )
		{
		     token += 4;
		     if ( *token == '#' )
		     {// reference
		         KURL u( HTMLWidget->actualURL );
		         u.setReference( token + 1 );
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

    const char *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "type=", 5 ) == 0 )
	{
	    switch ( *(token+5) )
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

    QString value = "";
    bool selected = false;

    const char *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "value=", 6 ) == 0 )
	{
		const char *p = token + 6;
		value = p;
	}
	else if ( strncasecmp( token, "selected", 8 ) == 0 )
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

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {    
	if ( strncasecmp( token, "align=", 6 ) == 0 )
	{
	    if ( strcasecmp( token + 6, "center") == 0 )
		align = HTMLClue::HCenter;
	    else if ( strcasecmp( token + 6, "right") == 0 )
		align = HTMLClue::Right;
	    else if ( strcasecmp( token + 6, "left") == 0 )
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

    QString name = "";
    int size = 0;
    bool multi = false;

    const char *token;
    while( (token = ht->nextOption()) != 0)
    {
    	if ( strncasecmp( token, "name=", 5 ) == 0 )
	{
	    const char *p = token + 5;
	    name = p;
	}
	else if ( strncasecmp( token, "size=", 5 ) == 0 )
	{
	    size = atoi( token + 5 );
	}
	else if ( strncasecmp( token, "multiple", 8 ) == 0 )
	{
	    multi = true;
	}
    }

    formSelect = new HTMLSelect( HTMLWidget, name, size, multi,
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

    int padding = 1;
    int spacing = 2;
    int width = 0;
    int percent = UNDEFINED;
    int border = 0;
    HTMLClue::HAlign align = HTMLClue::HNone;
    QColor tableColor;
    QColor rowColor;

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "cellpadding=", 12 ) == 0 )
	{
	    padding = atoi( token + 12 );
	}
	else if ( strncasecmp( token, "cellspacing=", 12 ) == 0 )
	{
	    spacing = atoi( token + 12 );
	}
	else if ( strncasecmp( token, "border", 6 ) == 0 )
	{
	    if ( *(token + 6) == '=' )
		border = atoi( token + 7 );
	    else
		border = 1;
	}
	else if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
	    if ( strchr( token+6, '%' ) )
		percent = atoi( token + 6 );
	    else if ( strchr( token+6, '*' ) )
	    { /* ignore */ }
	    else {
		width = atoi( token + 6 );
		percent = 0; // fixed width
	    }
	}
	else if (strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strcasecmp( token + 6, "left" ) == 0 )
		align = HTMLClue::Left;
	    else if ( strcasecmp( token + 6, "right" ) == 0 )
		align = HTMLClue::Right;
	}
	else if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
	{
	    setNamedColor( tableColor, token+8 );
	    rowColor = tableColor;
	}
    }

    HTMLTable *table = new HTMLTable( percent, width, padding, spacing, border );
    table->setColor( tableColor );
    table->setOldDivAlign( divAlign );
    table->setOldIndent( currentStyle->text.indent );
    table->setOldClue( _clue );
    table->setOldFlow( flow );

    if ( align != HTMLClue::Left && align != HTMLClue::Right )
    {
	_clue->append ( table );
    }
    else
    {
	HTMLClueAligned *aligned = new HTMLClueAligned( _clue );
	aligned->setHAlign( align );
	aligned->append( table );
	_clue->append( aligned );
    }

    _clue = table;
    flow = 0;
    currentStyle->text.indent = 0;
    pushBlock( ID_TABLE, 4 );
    tableStack.push( table );
}

void KHTMLParser::parseTagTableEnd(void)
{
    popBlock( ID_TABLE );
    HTMLTable *table = tableStack.pop();
    table->endTable();
    currentStyle->text.indent = table->oldIndent();
    divAlign = table->oldDivAlign();
    flow = table->oldFlow();
    _clue = table->oldClue();

    flow = 0;
}

void KHTMLParser::parseTagTD(void)
{
    // parses <TD> and <TH>
    HTMLTable *table = tableStack.current();
    if(!table) return;

    // close open <td> or <th> tags
    popBlock(ID_TD);
    popBlock(ID_TH);

    // tell table the last cell has ended
    table->endCell();

    bool heading = false;

    if (tagID == ID_TH)
	heading = true;
	
    int rowSpan = 1, colSpan = 1;
    int cellWidth = UNDEFINED;
    HTMLTable::ColType colType = HTMLTable::Variable;
    QColor bgcolor = table->rowColor();
    HTMLClue::VAlign valign = (table->rowVAlign() == HTMLClue::VNone ?
			       HTMLClue::Top : table->rowVAlign() );

    if ( heading )
	divAlign = (table->rowHAlign() == HTMLClue::HNone ? HTMLClue::HCenter :
		    table->rowHAlign() );
    else
	divAlign = (table->rowHAlign() == HTMLClue::HNone ? HTMLClue::Left :
		    table->rowHAlign() );

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "rowspan=", 8 ) == 0)
	{
	    rowSpan = atoi( token+8 );
	    if ( rowSpan < 1 )
		rowSpan = 1;
	}
	else if ( strncasecmp( token, "colspan=", 8 ) == 0)
	{
	    colSpan = atoi( token+8 );
	    if ( colSpan < 1 )
		colSpan = 1;
	}
	else if ( strncasecmp( token, "valign=", 7 ) == 0)
	{
	    if ( strncasecmp( token+7, "top", 3 ) == 0)
		valign = HTMLClue::Top;
	    else if ( strncasecmp( token+7, "bottom", 6 ) == 0)
		valign = HTMLClue::Bottom;
	    else
		valign = HTMLClue::VCenter;
	}
	else if ( strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strcasecmp( token+6, "center" ) == 0)
		divAlign = HTMLClue::HCenter;
	    else if ( strcasecmp( token+6, "right" ) == 0)
		divAlign = HTMLClue::Right;
	    else if ( strcasecmp( token+6, "left" ) == 0)
		divAlign = HTMLClue::Left;
	}
	else if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
	    if ( strchr( token + 6, '%' ) )
		colType = HTMLTable::Percent;
	    else if ( strchr( token+6, '*' ) )
		colType = HTMLTable::Relative;
	    else
		colType = HTMLTable::Fixed; // Fixed with
	    cellWidth = atoi( token + 6 );
	}
	else if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
	{
	    setNamedColor( bgcolor, token+8 );
	}
    } // while (ht->nextOption)

    
    HTMLTableCell *cell;
    cell = table->append( cellWidth, colType, rowSpan, colSpan, 
			  bgcolor, valign );
    flow = 0;
    _clue = cell;
    if ( heading )
    {
	pushBlock( ID_TH, 3 );
	currentStyle->font.weight = CSSStyleFont::Bold;
	setFont();
    }
    else
	pushBlock( ID_TD, 3 );

    vspace_inserted = true;
}	

    
void KHTMLParser::parseTagTextarea(void)
{
    if ( !form )
	return;

    QString name = "";
    int rows = 5, cols = 40;

    const char *token;
    while( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "name=", 5 ) == 0 )
	{
	    const char *p = token + 5;
	    if ( *p == '"' ) p++;
	    name = p;
	    if ( name[ name.length() - 1 ] == '"' )
		name.truncate( name.length() - 1 );
	}
	else if ( strncasecmp( token, "rows=", 5 ) == 0 )
	{
	    rows = atoi( token + 5 );
	}
	else if ( strncasecmp( token, "cols=", 5 ) == 0 )
	{
	    cols = atoi( token + 5 );
	}
    }

    formTextArea = new HTMLTextArea( HTMLWidget, name, rows, cols,
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


void KHTMLParser::parseTagTR(void)
{
    HTMLTable *table = tableStack.current();
    if(!table) return;
    
    // close open <tr> tag
    popBlock(ID_TR);

    table->endRow();
    table->startRow();
    HTMLClue::VAlign rowvalign = HTMLClue::VNone;
    HTMLClue::HAlign rowhalign = HTMLClue::HNone;
    QColor rowColor = table->color();

    const char *token;
    while ( (token = ht->nextOption()) != 0)
    {
	if ( strncasecmp( token, "valign=", 7 ) == 0)
	{
	    if ( strncasecmp( token+7, "top", 3 ) == 0)
		rowvalign = HTMLClue::Top;
	    else if ( strncasecmp( token+7, "bottom", 6 ) == 0)
		rowvalign = HTMLClue::Bottom;
	    else
		rowvalign = HTMLClue::VCenter;
	}
	else if ( strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strcasecmp( token+6, "left" ) == 0)
		rowhalign = HTMLClue::Left;
	    else if ( strcasecmp( token+6, "right" ) == 0)
		rowhalign = HTMLClue::Right;
	    else if ( strcasecmp( token+6, "center" ) == 0)
		rowhalign = HTMLClue::HCenter;
	}
	else if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
	{
	    setNamedColor( rowColor, token+8 );
	}
    }

    table->setRowColor( rowColor );
    table->setRowVAlign( rowvalign );
    table->setRowHAlign( rowhalign );

    pushBlock( ID_TR, 3 );

    vspace_inserted = true;
}

void KHTMLParser::parseTagTT(void)
{
    pushBlock(tagID, 1 );
    currentStyle->font.family = settings->fixedFontFace;
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

    const char *token;
    
    while( (token = ht->nextOption()) != 0)
    {
        if ( strncasecmp( token, "plain", 5 ) == 0 )
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


bool KHTMLParser::setCharset(const char *name)
{
	KCharsets *charsets=kapp->getCharsets();
	KCharset charset;
        if (!name || !name[0])
          charset=charsets->defaultCharset();
	else
	  charset=KCharset(name);
	if (!charset.isDisplayable()){
		if (charsetConverter) delete charsetConverter;
	        charsetConverter=0;
		if (!charset.isAvailable()){
			warning("Charset %s not available",name);
			return FALSE;
		}
		debugM("Initializing conversion from %s\n",charset.name());
		charsetConverter=new KCharsetConverter(charset
				,KCharsetConverter::AMP_SEQUENCES);
		if (!charsetConverter->ok()){
			warning("Couldn't initialize converter from %s",
			          name);
			delete charsetConverter;
			charsetConverter=0;
			return FALSE;
		}
	        charset=charsetConverter->outputCharset();
	}
	else{
	  if (charsetConverter) delete charsetConverter;
	  charsetConverter=0;
	}  
 	debugM("Setting charset to: %s\n",charset.name());
	settings->charset=charset;
        if (currentFont()!=0){	
          setFont();
	}
	return TRUE;
}

