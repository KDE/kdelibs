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

enum ID { ID_ADDRESS, ID_BIG, ID_BLOCKQUOTE, ID_B, ID_CELL, ID_CITE, 
    ID_CODE, ID_EM, ID_FONT, ID_FORM, ID_FRAMESET, ID_HEADER, ID_I, ID_KBD,
    ID_PRE, ID_SAMP, ID_SMALL, ID_STRIKE, ID_STRONG, ID_S, ID_TEXTAREA, 
    ID_LISTING, ID_TITLE, ID_TT, ID_U, ID_CAPTION, ID_TH, ID_TD, ID_TABLE, 
    ID_DIR, ID_MENU, ID_OL, ID_UL, ID_VAR };


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

//----------------------------------------------------------------------------

// Array of mark parser functions, e.g:
// <img ...  is processed by KHTMLParser::parseI()
// </ul>     is processed by KHTMLParser::parseU()
//
parseFunc KHTMLParser::parseFuncArray[26] = {
	&KHTMLParser::parseA,
	&KHTMLParser::parseB,
	&KHTMLParser::parseC,
	&KHTMLParser::parseD,
	&KHTMLParser::parseE,
	&KHTMLParser::parseF,
	&KHTMLParser::parseG,
	&KHTMLParser::parseH,
	&KHTMLParser::parseI,
	&KHTMLParser::parseJ,
	&KHTMLParser::parseK,
	&KHTMLParser::parseL,
	&KHTMLParser::parseM,
	&KHTMLParser::parseN,
	&KHTMLParser::parseO,
	&KHTMLParser::parseP,
	&KHTMLParser::parseQ,
	&KHTMLParser::parseR,
	&KHTMLParser::parseS,
	&KHTMLParser::parseT,
	&KHTMLParser::parseU,
	&KHTMLParser::parseV,
	&KHTMLParser::parseW,
	&KHTMLParser::parseX,
	&KHTMLParser::parseY,
	&KHTMLParser::parseZ
};


KHTMLParser::KHTMLParser( KHTMLWidget *_parent,
			  HTMLTokenizer *_ht, 
			  QPainter *_painter,
			  HTMLSettings *_settings) 			
{
    HTMLWidget    = _parent;
    ht            = _ht;
    painter       = _painter;
    settings      = _settings;

    leftBorder    = LEFT_BORDER;
    rightBorder   = RIGHT_BORDER;
    topBorder     = TOP_BORDER;
    bottomBorder  = BOTTOM_BORDER;

    listStack.setAutoDelete( true );
    glossaryStack.setAutoDelete( true );
    colorStack.setAutoDelete( true );

    // Initialize the font stack with the default font.
    italic = false;
    underline = false;
    strikeOut = false;
    weight = QFont::Normal;
    inNoframes = false;

    colorStack.clear();
    colorStack.push( new QColor( settings->fontBaseColor ) );

    if (!pFontManager)
    	pFontManager = new HTMLFontManager();

    charsetConverter = 0;
    
    HTMLFont f( settings->fontBaseFace, settings->fontBaseSize, settings->fontSizes );
    f.setCharset(settings->charset);
    f.setTextColor( settings->fontBaseColor );

    const HTMLFont *fp = pFontManager->getFont( f );
    font_stack.push( fp );
    
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

    flow = 0;
    frameSet = 0;

    listStack.clear();
    glossaryStack.clear();
    
    blockStack = 0;

    parseCount = 0;
    granularity = 500;

    indent = 0;
    vspace_inserted = true;
    divAlign = HTMLClue::Left;

    stringTok = new StringTokenizer;

    // move to the first token
    ht->first();
}

KHTMLParser::~KHTMLParser()
{
    if ( stringTok )
	delete stringTok;

    font_stack.clear();

    freeBlock();
}

// changes a current font
// needed for headings, and whenever the font family change is necessery
void KHTMLParser::selectFont( const char *_fontfamily, int _fontsize, int _weight, bool _italic )
{
    if ( _fontsize < 0 )
	_fontsize = 0;
    else if ( _fontsize >= MAXFONTSIZES )
	_fontsize = MAXFONTSIZES - 1;

    HTMLFont f( _fontfamily, _fontsize, settings->fontSizes, 
                _weight, _italic, settings->charset );
    f.setTextColor( *(colorStack.top()) );
    const HTMLFont *fp = pFontManager->getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLParser::selectFont( int _relative_font_size )
{
    int fontsize = settings->fontBaseSize + _relative_font_size;

    if ( !currentFont() )
    {
	fontsize = settings->fontBaseSize;
	debug( "aarrrgh - no font" );
    }

    if ( fontsize < 0 )
	fontsize = 0;
    else if ( fontsize >= MAXFONTSIZES )
	fontsize = MAXFONTSIZES - 1;

    HTMLFont f( font_stack.top()->family(), fontsize, settings->fontSizes,
        weight, italic, font_stack.top()->charset() );

    f.setUnderline( underline );
    f.setStrikeOut( strikeOut );
    f.setTextColor( *(colorStack.top()) );

    const HTMLFont *fp = pFontManager->getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLParser::selectFont()
{
    int fontsize;
	
    if ( currentFont() )
	fontsize = currentFont()->size();
    else
    {
	fontsize = settings->fontBaseSize;
	debug( "aarrrgh - no font" );
	assert(0);
    }

    HTMLFont f( font_stack.top()->family(), fontsize, settings->fontSizes,
                weight, italic,  font_stack.top()->charset() );

    f.setUnderline( underline );
    f.setStrikeOut( strikeOut );
    f.setTextColor( *(colorStack.top()) );

    const HTMLFont *fp = pFontManager->getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLParser::popFont()
{
    font_stack.pop();
    if ( font_stack.isEmpty() )
    {
	HTMLFont f( settings->fontBaseFace, settings->fontBaseSize, 
		    settings->fontSizes );
	f.setCharset(settings->charset);
	const HTMLFont *fp = pFontManager->getFont( f );
	font_stack.push( fp );
    }

    // we keep the current font color
    font_stack.top()->setTextColor( *(colorStack.top()) );

    painter->setFont( *(font_stack.top()) );
    weight = font_stack.top()->weight();
    italic = font_stack.top()->italic();
    underline = font_stack.top()->underline();
    strikeOut = font_stack.top()->strikeOut();
}

void KHTMLParser::popColor()
{
    colorStack.remove();

    if ( colorStack.isEmpty() )
	colorStack.push( new QColor( settings->fontBaseColor ) );
}

void KHTMLParser::pushBlock(int _id, int _level,
                            blockFunc _exitFunc, 
                            int _miscData1, int _miscData2)
{
    HTMLStackElem *Elem = new HTMLStackElem(_id, _level, _exitFunc, 
    					    _miscData1, _miscData2, 
    					    blockStack);
    blockStack = Elem;
}    					     

void KHTMLParser::popBlock( int _id, HTMLClue *_clue)
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
    		(this->*(Elem->exitFunc))( _clue, Elem );
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

void KHTMLParser::blockEndFont( HTMLClue *_clue, HTMLStackElem *Elem)
{
    popFont();
    if (Elem->miscData1)
    {
	vspace_inserted = insertVSpace( _clue, vspace_inserted );
	flow = 0;
    }
}

void KHTMLParser::blockEndPre( HTMLClue *_clue, HTMLStackElem *Elem)
{
    // We add a hidden space to get the height
    // If there is no flow box, we add one.
    if (!flow)
    	newFlow( _clue );
   
    flow->append(new HTMLHSpace( currentFont(), painter, true ));

    popFont();
    vspace_inserted = insertVSpace( _clue, vspace_inserted );
    flow = 0;
    inPre = false;
}

void KHTMLParser::blockEndColorFont( HTMLClue *_clue, HTMLStackElem *Elem)
{
    popColor();
    popFont();
}

void KHTMLParser::blockEndIndent( HTMLClue *_clue, HTMLStackElem *Elem)
{
    indent = Elem->miscData1;
    flow = 0;
}

void KHTMLParser::blockEndList( HTMLClue *_clue, HTMLStackElem *Elem)
{
    if (Elem->miscData2)
    {
	vspace_inserted = insertVSpace( _clue, vspace_inserted );
    }
    if ( !listStack.remove() )
    {
    	fprintf(stderr, "%s : List stack corrupt!\n", __FILE__);
    }
    
    indent = Elem->miscData1;
    flow = 0;
}

void KHTMLParser::blockEndFrameSet( HTMLClue *_clue, HTMLStackElem *Elem)
{
    HTMLFrameSet *oldFrameSet = (HTMLFrameSet *) Elem->miscData1;
    
    if (!oldFrameSet)
    {
    	// Toplevel frame set
    	HTMLWidget->showFrameSet( frameSet);
    }
    frameSet = oldFrameSet;
}

void KHTMLParser::blockEndForm( HTMLClue *_clue, HTMLStackElem *Elem)
{
    printf("in endForm\n");
    vspace_inserted = insertVSpace( _clue, vspace_inserted );
    flow = 0;
    form = 0;
}

void KHTMLParser::blockEndTitle( HTMLClue *_clue, HTMLStackElem *Elem)
{
    HTMLWidget->setNewTitle( title.data() );
    inTitle = false;	
}

bool KHTMLParser::insertVSpace( HTMLClue *_clue, bool _vspace_inserted )
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

void KHTMLParser::newFlow(HTMLClue * _clue)
{
    if (inPre)
         flow = new HTMLClueH();
	    else
         flow = new HTMLClueFlow();

    flow->setIndent( indent );
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

const char* KHTMLParser::parseBody( HTMLClue *_clue, const char *_end[], bool toplevel )
{
    const char *str;
    
    // Before one can write any objects in the body he usually adds a FlowBox
    // to _clue. Then put all texts, images etc. in the FlowBox.
    // If f == 0, you have to create a new FlowBox, otherwise you can
    // use the one stored in f. Setting f to 0 means closing the FlowBox.

    if ( toplevel)
    {
	parseCount = granularity;
	// Be sure to set the painter to the current font.
        painter->setFont( *font_stack.top() );
    }

    while ( ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// ignore stuff inbetween <noframes> and </noframes> if
	// this has a htmlView
	if(inNoframes)
	{
	    str++;
	    if(strncmp( str, "</noframes", 10 ) == 0)
		inNoframes = false;
	    else
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
	            newFlow(_clue);
		
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

	    int i = 0;

	    while ( _end[i] != 0 )
	    {
		if ( strncasecmp( str, _end[i], strlen( _end[i] ) ) == 0 )
		{
		    return str;
		}
		i++;
	    }
	    
	    // The tag used for line break when we are in <pre>...</pre>
	    if ( *str == '\n' )
	    {
		if (!flow)
		    newFlow(_clue);

		// Add a hidden space to get the line-height right.
		flow->append(new HTMLHSpace( currentFont(), painter, true ));
		vspace_inserted = false;
	
		newFlow(_clue); // Explicitly make a new flow! 
	    }
	    else if (*str == '&')
	    {
		int l;
		const char *str1;

		if (!flow)
		    newFlow(_clue);

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
		parseOneToken( _clue, str );
	    }
	}

	if ( toplevel )
	{
	    if ( parseCount <= 0 )
              return 0;
	}
	parseCount--;
    }

    return 0;
}

const char *KHTMLParser::parseOneToken( HTMLClue *_clue, const char *str )
{
    if ( *str == '<' )
    {
	int indx;

	str++;

	if ( *str == '/' )
	    indx = *(str+1) - 'a';
	else
	    indx = *str - 'a';
	
	if ( indx >= 0 && indx < 26 )
	    (this->*(parseFuncArray[indx]))( _clue, str );
	
    }

    return 0;
}

/* the following attributes are standard sets used quite often for specific
   tags. There parsing can be done in functions, and return a struct defining 
   the atributes.

<!ENTITY % coreattrs
"id          ID             #IMPLIED  -- document-wide unique id --
class       CDATA          #IMPLIED  -- space separated list of classes --
style       %StyleSheet;   #IMPLIED  -- associated style info --
title       %Text;         #IMPLIED  -- advisory title/amplification --"
>

struct coreattrs
{
    QString id;
    QString class;
    StyleSheet style;
    QString title;
};

<!ENTITY % i18n
"lang        %LanguageCode; #IMPLIED  -- language code --
dir         (ltr|rtl)      #IMPLIED  -- direction for weak/neutral text --"
>

struct i18n
{
    Language lang;
    Direction dir;
};

<!ENTITY % events
"onclick     %Script;       #IMPLIED  -- a pointer button was clicked --
ondblclick  %Script;       #IMPLIED  -- a pointer button was double clicked--
onmousedown %Script;       #IMPLIED  -- a pointer button was pressed down --
onmouseup   %Script;       #IMPLIED  -- a pointer button was released --
onmouseover %Script;       #IMPLIED  -- a pointer was moved onto --
onmousemove %Script;       #IMPLIED  -- a pointer was moved within --
onmouseout  %Script;       #IMPLIED  -- a pointer was moved away --
onkeypress  %Script;       #IMPLIED  -- a key was pressed and released --
onkeydown   %Script;       #IMPLIED  -- a key was pressed down --
onkeyup     %Script;       #IMPLIED  -- a key was released --"
>

struct events
{
    JScript onclick;
    JScript ondblclick;
    JScript onmousedown;
    JScript onmouseup;
    JScript onmouseover;
    JScript onmousemove;
    JScript onmouseout;
    JScript onkeypress;
    JScript onkeydown;
    JScript onkeyup;
};

<!ENTITY % attrs "%coreattrs; %i18n; %events;">

*/

// <a>              </a>
// <abbr>           </abbr>      unimpl. HTML4
// <acronym>        </acronym>   unimpl. HTML4
// <address>        </address>
// <applet>         </applet>    unimpl. We'll need java for that one...
// <area            
void KHTMLParser::parseA( HTMLClue *_clue, const char *str )
{
    if ( strncmp( str, "area", 4 ) == 0 )
    {
	// attributes missing: %attrs, shape, coords, href, nohref,
	// alt, tabindex, accesskey, onfocus, onblur
	HTMLMap *imageMap = HTMLWidget->lastMap();

	if (!imageMap) 
	    return;

	stringTok->tokenize( str + 5, " >" );

	char * href = "";
	QString coords;
	const char * atarget = baseTarget;
	HTMLArea::Shape shape = HTMLArea::Rect;

	while ( stringTok->hasMoreTokens() )
	{
	    const char* p = stringTok->nextToken();

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

	if ( !coords.isEmpty() )
	{
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

	    if ( area )
	        imageMap->addArea( area );
	}
    }
    else if ( strncmp( str, "address", 7) == 0 )
    {
//	vspace_inserted = insertVSpace( _clue, vspace_inserted );
	flow = 0;
	italic = TRUE;
	weight = QFont::Normal;
	selectFont();
	pushBlock(ID_ADDRESS, 2, &KHTMLParser::blockEndFont, true);
    }
    else if ( strncmp( str, "/address", 8) == 0 )
    {
	popBlock( ID_ADDRESS, _clue);
    }
    else if ( strncmp( str, "abbr", 4) == 0 )
    {
	// attributes: %attrs
    }
    else if ( strncmp( str, "/abbr", 5) == 0 )
    {
    }
    else if ( strncmp( str, "acronym", 7) == 0 )
    {
	// attributes: %attrs
    }
    else if ( strncmp( str, "/acronym", 8) == 0 )
    {
    }
    else if ( strncmp( str, "applet", 6) == 0 )
    {
	// java stuff... ignore for the moment
    }
    else if ( strncmp( str, "/applet", 7) == 0 )
    {
    }
    else if ( strncmp( str, "a ", 2 ) == 0 )
    {
	// implemented attributes:
	// href, name, target
	//
	// unimpl:
	// charset, type, hreflang, rel, rev, accesskey, shape, coords,
	// tabindex, onfocus, onblur

	closeAnchor();
	QString tmpurl;
	target = 0;
	bool visited = false;
	const char *p;

	stringTok->tokenize( str + 2, " >" );

	while ( ( p = stringTok->nextToken() ) != 0 )
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
	    if ( visited )
		colorStack.push( new QColor( settings->vLinkColor ) );
	    else
		colorStack.push( new QColor( settings->linkColor ) );
	    if ( settings->underlineLinks )
		underline = true;
	    selectFont();
	    url = ht->newString( tmpurl.data(), tmpurl.length() );
	}
    }
    else if ( strncmp( str, "/a", 2 ) == 0 )
    {
	closeAnchor();
    }
}

// <b>              </b>
// <base                            complete
// <basefont                        unimpl.
// <bdo>            </bdo>          unimpl. HTML4 bidirectional writing
// <big>            </big>
// <blockquote>     </blockquote>
// <body
// <br
// <button>         </button>       unimpl. HTML4
void KHTMLParser::parseB( HTMLClue *_clue, const char *str )
{
    if ( strncmp( str, "br", 2 ) == 0 )
    {
	//attrs: %coreattrs

	HTMLVSpace::Clear clear = HTMLVSpace::CNone;

	stringTok->tokenize( str + 3, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
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
	    newFlow(_clue);

	HTMLObject *last = flow->lastChild(); 
	if (!last || last->isNewline())
	{
		// Start of line, add vertical space based on current font.
		flow->append( new HTMLVSpace( 
				currentFont()->pointSize(),
				clear ));
	}
	else
	{
		// Terminate current line
		flow->append( new HTMLVSpace(0, clear));
	}

	vspace_inserted = false;
    }
    else if ( strncmp( str, "basefont", 8 ) == 0 )
    {
	// attrs: id size color face
    }
    else if ( strncmp(str, "base", 4 ) == 0 )
    {
	stringTok->tokenize( str + 5, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
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
    else if ( strncmp(str, "big", 3 ) == 0 )
    {
	// attrs %attrs

	selectFont( +2 );
	pushBlock(ID_BIG, 1, &KHTMLParser::blockEndFont);
    }
    else if ( strncmp(str, "/big", 4 ) == 0 )
    {
	popBlock( ID_BIG, _clue);
    }
    else if ( strncmp(str, "blockquote", 10 ) == 0 )
    {
	// attrs: cite, %attrs

	pushBlock(ID_BLOCKQUOTE, 2, &KHTMLParser::blockEndIndent, indent);
	indent += INDENT_SIZE;
	flow = 0; 
    }
    else if ( strncmp(str, "/blockquote", 11 ) == 0 )
    {
	popBlock( ID_BLOCKQUOTE, _clue);
    }
    else if ( strncmp( str, "button", 6 ) == 0 )
    {
	// forms...
    }
    else if ( strncmp( str, "/button", 7 ) == 0 )
    {
    }
    else if ( strncmp( str, "bdo", 3 ) == 0 )
    {
	// I think we'll wait for qt to support right to left...
    }
    else if ( strncmp( str, "/bdo", 4 ) == 0 )
    {
    }
    else if ( strncmp( str, "body", 4 ) == 0 )
    {
	// missing attrs: %attrs, onload, onunload, alink

	if ( bodyParsed )
	    return;

	bodyParsed = true;

	stringTok->tokenize( str + 5, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
	    if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
	    {
                QColor bgColor;
		if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
		{
		    QString col = "#";
		    col += token+8;
		    bgColor.setNamedColor( col );
		}
		else
		{
		    bgColor.setNamedColor( token+8 );
		}
		HTMLWidget->setBGColor( bgColor );
	    }
	    else if ( strncasecmp( token, "background=", 11 ) == 0 )
	    {
		HTMLWidget->setBGImage( token + 11 );
	    }
	    else if ( strncasecmp( token, "text=", 5 ) == 0 )
	    {
		settings->fontBaseColor.setNamedColor( token+5 );
		*(colorStack.top()) = settings->fontBaseColor;
		font_stack.top()->setTextColor( settings->fontBaseColor );
	    }
	    else if ( strncasecmp( token, "link=", 5 ) == 0 )
	    {
		settings->linkColor.setNamedColor( token+5 );
	    }
	    else if ( strncasecmp( token, "vlink=", 6 ) == 0 )
	    {
		settings->vLinkColor.setNamedColor( token+6 );
	    }
	}
    }
    else if ( strncmp(str, "b", 1 ) == 0 )
    {
	if ( str[1] == '>' || str[1] == ' ' )
	{
	    // attrs: %attrs
	    weight = QFont::Bold;
	    selectFont();
	    pushBlock(ID_B, 1, &KHTMLParser::blockEndFont);
	}
    }
    else if ( strncmp(str, "/b", 2 ) == 0 )
    {
	popBlock( ID_B, _clue);
    }
}

// <center>         </center>      
// <cite>           </cite>        
// <code>           </code>        
// <cell>           </cell>         (not in HTML4...)
// <comment>        </comment>      unimplemented (??? doesn't exist
//                                  according to HTML4 specs...)
void KHTMLParser::parseC( HTMLClue *_clue, const char *str )
{
	if (strncmp( str, "center", 6 ) == 0)
	{
		divAlign = HTMLClue::HCenter;
		flow = 0;
	}
	else if (strncmp( str, "/center", 7 ) == 0)
	{
		divAlign = HTMLClue::Left;
		flow = 0;
	}
	else if (strncmp( str, "cell", 4 ) == 0)
	{
	    if (!flow)
	        newFlow(_clue);

	    parseCell( flow, str );

	    // Add a hidden space
	    flow->append( new HTMLHSpace( currentFont(), painter, true ) );
	}
	else if (strncmp( str, "cite", 4 ) == 0)
	{
		italic = TRUE;
		weight = QFont::Normal;
		selectFont();
		pushBlock(ID_CITE, 1, &KHTMLParser::blockEndFont);
	}
	else if (strncmp( str, "/cite", 5) == 0)
	{
		popBlock( ID_CITE, _clue);
	}
	else if (strncmp(str, "code", 4 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
		pushBlock(ID_CODE, 1, &KHTMLParser::blockEndFont);
	}
	else if (strncmp(str, "/code", 5 ) == 0 )
	{
		popBlock( ID_CODE, _clue);
	}
}

// <dd>             </dd>           
// <del>            </del>          unimpl. HTML4
// <dfn>            </dfm>          unimpl. HTML4
// <dir             </dir>          partial
// <div             </div>
// <dl>             </dl>
// <dt>             </dt>
void KHTMLParser::parseD( HTMLClue *_clue, const char *str )
{
    if ( strncmp( str, "dir", 3 ) == 0 )
    {
	closeAnchor();
	pushBlock(ID_DIR, 2, &KHTMLParser::blockEndList, indent, false);
	listStack.push( new HTMLList( Dir ) );
	indent += INDENT_SIZE;
    }
    else if ( strncmp( str, "/dir", 4 ) == 0 )
    {
	popBlock( ID_DIR, _clue);
    }
    else if ( strncmp( str, "div", 3 ) == 0 )
    {
	stringTok->tokenize( str + 4, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    // attrs: %attrs
	    const char* token = stringTok->nextToken();
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
    else if ( strncmp( str, "/div", 4 ) == 0 )
    {
	divAlign = HTMLClue::Left;
	flow = 0;
    }
    else if ( strncmp( str, "dl", 2 ) == 0 )
    {
	// attrs: %attrs

	vspace_inserted = insertVSpace( _clue, vspace_inserted );
	closeAnchor();
	if ( glossaryStack.top() )
	{
	    indent += INDENT_SIZE;
	}
	glossaryStack.push( new GlossaryEntry( GlossaryDL ) );
	flow = 0;
    }
    else if ( strncmp( str, "/dl", 3 ) == 0 )
    {
	if ( !glossaryStack.top() )
	    return;

	if ( *glossaryStack.top() == GlossaryDD )
	{
	    glossaryStack.remove();
	    indent -= INDENT_SIZE;
		 if (indent < 0)
		 	indent = 0;
	}
	glossaryStack.remove();
	if ( glossaryStack.top() )
	{
	    indent -= INDENT_SIZE;
		 if (indent < 0)
		 	indent = 0;
	}
	vspace_inserted = insertVSpace( _clue, vspace_inserted );
    }
    else if (strncmp( str, "dt", 2 ) == 0)
    {
	// attrs: %attrs

	if ( !glossaryStack.top() )
	    return;

	if ( *glossaryStack.top() == GlossaryDD )
	{
	    glossaryStack.pop();
	    indent -= INDENT_SIZE;
		 if (indent < 0)
		 	indent = 0;
	}
	vspace_inserted = false;
	flow = 0;
    }
    else if (strncmp( str, "dd", 2 ) == 0)
    {
	// attrs: %attrs

	if ( !glossaryStack.top() )
	    return;

	if ( *glossaryStack.top() != GlossaryDD )
	{
	    glossaryStack.push( new GlossaryEntry( GlossaryDD ) );
	    indent += INDENT_SIZE;
	}
	flow = 0;
    }
    else if (strncmp( str, "dfn", 3 ) == 0)
    {
	// attrs: %attrs
    }
    else if (strncmp( str, "/dfn", 4 ) == 0)
    {
    }
    else if (strncmp( str, "del", 3 ) == 0)
    {
	// attrs: %attrs, cite, datetime
	// marks deleted text. Could be rendered as struck-through
    }
    else if (strncmp( str, "/del", 4 ) == 0)
    {
    }
}

// <em>             </em>
void KHTMLParser::parseE( HTMLClue * _clue, const char *str )
{
	if ( strncmp( str, "em", 2 ) == 0 )
	{
		italic = TRUE;
		selectFont();
		pushBlock(ID_EM, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp( str, "/em", 3 ) == 0 )
	{
		popBlock( ID_EM, _clue);
	}
}

// <fieldset>       </fieldset>     unimpl. HTML4
// <font>           </font>
// <form>           </form>         partial
// <frame           <frame>
// <frameset        </frameset>
void KHTMLParser::parseF( HTMLClue * _clue, const char *str )
{
	if ( strncmp( str, "font", 4 ) == 0 )
	{
	    stringTok->tokenize( str + 5, " >" );
	    int newSize = currentFont()->size() - settings->fontBaseSize;
	    QString newFace;
	    QColor *color = new QColor( *(colorStack.top()) );
	    while ( stringTok->hasMoreTokens() )
	    {
		    const char* token = stringTok->nextToken();
		    if ( strncasecmp( token, "size=", 5 ) == 0 )
		    {
			    int num = atoi( token + 5 );
			    if ( *(token + 5) == '+' || *(token + 5) == '-' )
				newSize = num;
			    else
				newSize = num - 3;
		    }
		    else if ( strncasecmp( token, "color=", 6 ) == 0 )
		    {
			if ( *(token+6) != '#' && strlen( token+6 ) == 6 )
			{
			    QString col = "#";
			    col += token+6;
			    color->setNamedColor( col );
			}
			else
			    color->setNamedColor( token+6 );
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
				newFace = fname;
				break;
			    }
			}
		    }
	    }
	    colorStack.push( color );
	    if ( !newFace.isEmpty() )
		selectFont( newFace, newSize + settings->fontBaseSize,
		    currentFont()->weight(), currentFont()->italic() );
	    else
		selectFont( newSize );
	    pushBlock(ID_FONT, 1, &KHTMLParser::blockEndColorFont);
	}
	else if ( strncmp( str, "/font", 5 ) == 0 )
	{
	    popBlock( ID_FONT, _clue);
	}
	else if ( strncmp( str, "frameset", 8 ) == 0 )
        {
	    // We need a view to do frames
	    if ( !HTMLWidget->getView())
	    	return;

	    HTMLFrameSet *oldFrameSet = frameSet;
	    if ( !oldFrameSet )
	    {
	       // This is the toplevel frameset
	       frameSet = new HTMLFrameSet( HTMLWidget, str );
	    }
	    else
	    { 
	       // This is a nested frameset
	       frameSet = new HTMLFrameSet( oldFrameSet, str );
	       oldFrameSet->append(frameSet);
	    }
	    HTMLWidget->addFrameSet( frameSet);
	    pushBlock( ID_FRAMESET, 4, &KHTMLParser::blockEndFrameSet, (int) oldFrameSet );
	}
	else if ( strncmp( str, "/frameset", 9 ) == 0 )
        {
            popBlock( ID_FRAMESET, _clue);

	}	
	else if ( strncmp( str, "frame", 5 ) == 0 )
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
	      
	    stringTok->tokenize( str + 6, " >" );
	    while ( stringTok->hasMoreTokens() )
	    {
	        const char* token = stringTok->nextToken();
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
	else if ( strncmp( str, "form", 4 ) == 0 )
	{
	    printf("Creating form\n");
	    QString action = "";
	    QString method = "GET";
	    QString target = "";

	    stringTok->tokenize( str + 5, " >" );
            while ( stringTok->hasMoreTokens() )
	    {
		const char* token = stringTok->nextToken();
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

            vspace_inserted = insertVSpace( _clue, vspace_inserted );
	    // Lars: does not work, if forms extend over several
	    // tablecells, and the form gets created in the first one.
	    //pushBlock( ID_FORM, 2, &blockEndForm);
	}
	else if ( strncmp( str, "/form", 5 ) == 0 )
	{
	    if(form)
	    {
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = 0;
		form = 0;
	    }
	    //Lars: see above
	    //popBlock( ID_FORM, _clue);
	}
	else if ( strncmp( str, "fieldset", 8 ) == 0 )
	{
	}
	else if ( strncmp( str, "/fieldset", 9 ) == 0 )
	{
	}
	
}

// 
void KHTMLParser::parseG( HTMLClue *, const char * )
{
}

// <h[1-6]>         </h[1-6]>
// <head>           </head>     can perhaps safely ignore this tag...
// <hr
void KHTMLParser::parseH( HTMLClue *_clue, const char *str )
{
        if ( *(str)=='h' &&
	    ( *(str+1)=='1' || *(str+1)=='2' || *(str+1)=='3' ||
     	      *(str+1)=='4' || *(str+1)=='5' || *(str+1)=='6' ) )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		HTMLClue::HAlign align = divAlign;

		stringTok->tokenize( str + 3, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
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
		newFlow(_clue);

		flow->setHAlign( align );

		switch ( str[1] )
		{
			case '1':
				weight = QFont::Bold;
				selectFont( +3 );
				break;

			case '2':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( +2 );
				break;

			case '3':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( +1 );
				break;

			case '4':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( +0 );
				break;

			case '5':
				weight = QFont::Normal;
				italic = TRUE;
				selectFont( +0 );
				break;

			case '6':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( -1 );
				break;
		}
		// Insert a vertical space and restore the old font at the 
		// closing tag
		pushBlock(ID_HEADER, 2, &KHTMLParser::blockEndFont, true );
	}
	else if ( *str=='/' && *(str+1)=='h' &&
	    ( *(str+2)=='1' || *(str+2)=='2' || *(str+2)=='3' ||
 	      *(str+2)=='4' || *(str+2)=='5' || *(str+2)=='6' ))
	{
		// Close tag
		popBlock( ID_HEADER, _clue);
	}
	else if ( strncmp(str, "hr", 2 ) == 0 )
	{
		int size = 1;
		int length = UNDEFINED;
		int percent = UNDEFINED;
		HTMLClue::HAlign align = divAlign;
		HTMLClue::HAlign oldAlign = divAlign;
		bool shade = TRUE;

		if ( flow )
		    oldAlign = align = flow->getHAlign();

		stringTok->tokenize( str + 3, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
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
					percent = atoi( token+6 );
				else
				{
					length = atoi( token+6 );
					percent = 0; // fixed width
				}
			}
			else if ( strncasecmp( token, "noshade", 6 ) == 0 )
			{
				shade = FALSE;
			}
		}

		newFlow(_clue);

		flow->append( new HTMLRule( length, percent, size, shade ) );

		flow = 0;

		vspace_inserted = false;
	}
}

// <i>              </i>
// <iframe>         </iframe>       unimpl. HTML4
// <img                             partial
// <input                           partial
// <ins>            </ins>          unimpl. HTML4
// <isindex                         unimpl. HTML4
void KHTMLParser::parseI( HTMLClue *_clue, const char *str )
{
    if (strncmp( str, "img", 3 ) == 0)
    {
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

	stringTok->tokenize( str + 4, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
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
	    KURL kurl( HTMLWidget->getBaseURL(), filename );
	    // Do we need a new FlowBox ?
	    if ( !flow )
	    	newFlow(_clue);

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

	    image->setBorderColor( *(colorStack.top()) );

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
    else if (strncmp( str, "input", 5 ) == 0)
    {
	if ( form == 0 )
		return;

	if ( !flow )
	    newFlow(_clue);

	parseInput( str + 6 );
	vspace_inserted = false;
    }
    else if ( strncmp(str, "iframe", 6 ) == 0 )
    {
	// inlined frame
    }
    else if ( strncmp(str, "/iframe", 7 ) == 0 )
    {
    }
    else if ( strncmp(str, "ins", 3 ) == 0 )
    {
	// inserted text... opposite of <del>
    }
    else if ( strncmp(str, "/ins", 4 ) == 0 )
    {
    }
    else if ( strncmp(str, "isindex", 7 ) == 0 )
    {
	// deprecated... is it used at all?
    }
    else if ( strncmp(str, "/isindex", 8 ) == 0 )
    {
    }
    else if ( strncmp(str, "i", 1 ) == 0 )
    {
	if ( str[1] == '>' || str[1] == ' ' )
	{
	    italic = TRUE;
	    selectFont();
	    pushBlock(ID_I, 1, &KHTMLParser::blockEndFont);
	}
    }
    else if ( strncmp( str, "/i", 2 ) == 0 )
    {
	popBlock( ID_I, _clue);
    }
}

void KHTMLParser::parseJ( HTMLClue *, const char * )
{
}

// <kbd>            </kbd>
void KHTMLParser::parseK( HTMLClue * _clue, const char *str )
{
	if ( strncmp(str, "kbd", 3 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
		pushBlock(ID_KBD, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp(str, "/kbd", 4 ) == 0 )
	{
		popBlock( ID_KBD, _clue);
	}
}

// <label>          </label>        unimpl.
// <legend>         </legend>       unimpl.
// <listing>        </listing>      
// <li>
// <link                            unimpl. HTML4
void KHTMLParser::parseL( HTMLClue *_clue, const char *str )
{
    if (strncmp( str, "listing", 7 ) == 0)
    {
	vspace_inserted = insertVSpace( _clue, vspace_inserted );
	selectFont( settings->fixedFontFace, settings->fontBaseSize,
        QFont::Normal, FALSE );
	flow = 0;
	inPre = true;
	pushBlock(ID_LISTING, 2, &KHTMLParser::blockEndPre);
    }	
    else if (strncmp( str, "/listing", 8 ) == 0)
    {
	popBlock(ID_LISTING, _clue);
    }
    else if (strncmp( str, "link", 4 ) == 0)
    {
    }
    else if (strncmp( str, "li", 2 ) == 0)
    {
	closeAnchor();
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
	    indentSize = indent;
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
		flow->append( new HTMLBullet( font_stack.top()->pointSize(),
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
    else if (strncmp( str, "label", 5 ) == 0)
    {
	// form field label text
    }
    else if (strncmp( str, "/label", 6 ) == 0)
    {
    }
    else if (strncmp( str, "legend", 6 ) == 0)
    {
	// fieldset legend
    }
    else if (strncmp( str, "/legend", 7 ) == 0)
    {
    }
}

// <map             </map>
// <menu>           </menu>         partial
// <meta> 			    partial - only charset and refresh
void KHTMLParser::parseM( HTMLClue *_clue, const char *str )
{
	if (strncmp( str, "menu", 4 ) == 0)
	{
		closeAnchor();
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		pushBlock( ID_MENU, 2, &KHTMLParser::blockEndList, indent, false);
		listStack.push( new HTMLList( Menu ) );
		indent += INDENT_SIZE;
	}
	else if (strncmp( str, "/menu", 5 ) == 0)
	{
		popBlock( ID_MENU, _clue);
	}
	else if ( strncmp( str, "map", 3 ) == 0 )
	{
		stringTok->tokenize( str + 4, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "name=", 5 ) == 0)
			{
				QString mapurl = "#";
				mapurl += token+5;
				HTMLWidget->addMap( ht->newString( mapurl.data() ) );
			}
		}
	}
	else if ( strncmp( str, "meta", 4 ) == 0 )
	{
                QString httpequiv;
                QString name;
		QString content;
		debugM("Parsing <META>: %s\n",str);
		stringTok->tokenize( str + 5, " >" );
		while ( stringTok->hasMoreTokens() )
	   	{
			const char* token = stringTok->nextToken();
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
}

// <noframes>        </noframes>
// <noscript>        </noscript>     HTML4
// <nobr>            </nobr>         netscape or IE extension
void KHTMLParser::parseN( HTMLClue *, const char *str )
{
    // only ignore the stuff in noframes, if we have a htmlview
    if( HTMLWidget->htmlView && strncmp( str, "noframes", 8 ) == 0)
	inNoframes = true;	
    else if( strncmp( str, "noscript", 8 ) == 0 )
    {
    }
    else if( strncmp( str, "/noscript", 9 ) == 0 )
    {
    }
    else if( strncmp( str, "nobr", 4 ) == 0 )
    {
	// do not break lines
    }
    else if( strncmp( str, "/nobr", 5 ) == 0 )
    {
    }
}

// <object>         </object>       unimpl. HTML4        
// <ol>             </ol>           partial
// <optgroup>       </optgroup>     unimpl. HTML4
// <option
void KHTMLParser::parseO( HTMLClue *_clue, const char *str )
{
    if ( strncmp( str, "ol", 2 ) == 0 )
    {
	closeAnchor();
	if ( listStack.isEmpty() )
	{
	    vspace_inserted = insertVSpace( _clue, vspace_inserted );
	    pushBlock( ID_OL, 2, &KHTMLParser::blockEndList, indent, true);
	}
	else
	{
	    pushBlock( ID_OL, 2, &KHTMLParser::blockEndList, indent, false);
	}

	ListNumType listNumType = Numeric;

	stringTok->tokenize( str + 3, " >" );
	while ( stringTok->hasMoreTokens() )
	{
		const char* token = stringTok->nextToken();
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
	indent += INDENT_SIZE;
    }
    else if ( strncmp( str, "/ol", 3 ) == 0 )
    {
	popBlock( ID_OL, _clue);
    }
    else if ( strncmp( str, "option", 6 ) == 0 )
    {
	if ( !formSelect )
		return;

	QString value = "";
	bool selected = false;

	stringTok->tokenize( str + 7, " >" );
	while ( stringTok->hasMoreTokens() )
	{
		const char* token = stringTok->nextToken();
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
    else if ( strncmp( str, "/option", 7 ) == 0 )
    {
	if ( inOption )
		formSelect->setText( formText );
	inOption = false;
    }
    else if ( strncmp( str, "optgroup", 8 ) == 0 )
    {
	// grouping options
    }
    else if ( strncmp( str, "/optgroup", 9 ) == 0 )
    {
    }
    else if ( strncmp( str, "object", 6 ) == 0 )
    {
	// external objects, java, etc...
    }
    else if ( strncmp( str, "/object", 7 ) == 0 )
    {
    }
}

// <p
// <param                      unimpl. HTML4
// <pre             </pre>
void KHTMLParser::parseP( HTMLClue *_clue, const char *str )
{
	if ( strncmp( str, "pre", 3 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
		flow = 0;
		inPre = true;
		pushBlock(ID_PRE, 2, &KHTMLParser::blockEndPre);
	}	
	else if ( strncmp( str, "/pre", 4 ) == 0 )
	{
		popBlock( ID_PRE, _clue);
	}
	else if ( *str == 'p' && ( *(str+1) == ' ' || *(str+1) == '>' ) )
	{
		closeAnchor();
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		HTMLClue::HAlign align = divAlign;

		stringTok->tokenize( str + 2, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
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
		    newFlow(_clue);

		if ( align != divAlign )
		    flow->setHAlign( align );
	}
	else if ( *str == '/' && *(str+1) == 'p' &&
	    ( *(str+2) == ' ' || *(str+2) == '>' ) )
	{
	    closeAnchor();
	    vspace_inserted = insertVSpace( _clue, vspace_inserted );
	}
	else if ( strncmp( str, "param", 5 ) == 0 )
	{
	    // named property value
	}
}

// <q>            </q>            unimpl. HTML4
void KHTMLParser::parseQ( HTMLClue *, const char *str )
{
    if ( *str == 'q' && ( *(str+1) == ' ' || *(str+1) == '>' ) )
    {
	// inline quotation
    }
    else if ( *str == '/' && *(str+1) == 'q' &&
	      ( *(str+2) == ' ' || *(str+2) == '>' ) )
    {
    }
}

void KHTMLParser::parseR( HTMLClue *, const char * )
{
}

// <s>              </s>
// <samp>           </samp>
// <script>         </script>       unimpl. HTML4
// <select          </select>       partial
// <small>          </small>
// <span>           </span>         unimpl. HTML4
// <strike>         </strike>
// <strong>         </strong>
// <style>          </style>        unimpl. HTML4
// <sub>            </sub>          unimplemented
// <sup>            </sup>          unimplemented
void KHTMLParser::parseS( HTMLClue *_clue, const char *str )
{
	if ( strncmp(str, "samp", 4 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
		pushBlock(ID_SAMP, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp(str, "/samp", 5 ) == 0)
	{
		popBlock( ID_SAMP, _clue);
	}
	else if ( strncmp(str, "select", 6 ) == 0)
	{
		if ( !form )
		{
		    printf("form missing\n");
			return;
		}
		QString name = "";
		int size = 0;
		bool multi = false;

		stringTok->tokenize( str + 7, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
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
		    newFlow(_clue);

		flow->append( formSelect );
	}
	else if ( strncmp(str, "/select", 7 ) == 0 )
	{
		if ( inOption )
			formSelect->setText( formText );

		formSelect = 0;
		inOption = false;
		vspace_inserted = false;
	}
	else if ( strncmp(str, "small", 5 ) == 0 )
	{
		selectFont( -1 );
		pushBlock(ID_SMALL, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp(str, "/small", 6 ) == 0 )
	{
		popBlock( ID_SMALL, _clue);
	}
	else if ( strncmp(str, "strong", 6 ) == 0 )
	{
		weight = QFont::Bold;
		selectFont();
		pushBlock(ID_STRONG, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp(str, "/strong", 7 ) == 0 )
	{
		popBlock( ID_STRONG, _clue);
	}
	else if ( strncmp(str, "script", 6 ) == 0 )
	{
	    // javascript...
	}
	else if ( strncmp(str, "/script", 7 ) == 0 )
	{
	}
	else if ( strncmp(str, "span", 4 ) == 0 )
	{
	    // inline <div>...
	}
	else if ( strncmp(str, "/span", 5 ) == 0 )
	{
	}
	else if ( strncmp(str, "style", 5 ) == 0 )
	{
	    // style sheets...
	}
	else if ( strncmp(str, "/style", 6 ) == 0 )
	{
	}
	else if ( strncmp(str, "sub", 3 ) == 0 )
	{
	    // subscript
	}
	else if ( strncmp(str, "/sub", 4 ) == 0 )
	{
	}
	else if ( strncmp(str, "sup", 3 ) == 0 )
	{
	    // superscript
	}
	else if ( strncmp(str, "/sup", 4 ) == 0 )
	{
	}
	else if ( strncmp( str, "strike", 6 ) == 0 )
	{
	    strikeOut = TRUE;
	    selectFont();
	    pushBlock(ID_STRIKE, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp(str, "s", 1 ) == 0 )
	{
	    if ( str[1] == '>' || str[1] == ' ' )
	    {
		strikeOut = TRUE;
		selectFont();
		pushBlock(ID_S, 1, &KHTMLParser::blockEndFont);
	    }
	}
	else if ( strncmp(str, "/s", 2 ) == 0 )
	{
	    if ( str[2] == '>' || str[2] == ' ')
	    {
	    	popBlock( ID_S, _clue);
	    }
	    else if ( strncmp( str+2, "trike", 5 ) == 0 )
	    {
		popBlock( ID_STRIKE, _clue);
	    }
	}
}

// <table           </table>        HTML4 stuff missing
// <textarea        </textarea>
// <title>          </title>
// <tt>             </tt>
void KHTMLParser::parseT( HTMLClue *_clue, const char *str )
{
	if ( strncmp( str, "table", 5 ) == 0 )
	{
		closeAnchor();
		if ( !vspace_inserted || !flow )
		    newFlow(_clue);

		parseTable( flow, str + 6 );

		flow = 0;
	}
	else if ( strncmp( str, "title", 5 ) == 0 )
	{
		title = "";
		inTitle = true;
		pushBlock(ID_TITLE, 3, &KHTMLParser::blockEndTitle);
	}
	else if ( strncmp( str, "/title", 6 ) == 0 )
	{
		popBlock(ID_TITLE, _clue);
	}
	else if ( strncmp( str, "textarea", 8 ) == 0 )
	{
		if ( !form )
			return;

		QString name = "";
		int rows = 5, cols = 40;

		stringTok->tokenize( str + 9, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
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
		    newFlow(_clue);

		flow->append( formTextArea );

		formText = "";
		inTextArea = true;
		pushBlock(ID_TEXTAREA, 3);
	}
	else if ( strncmp( str, "/textarea", 9 ) == 0 )
	{
		popBlock(ID_TEXTAREA, _clue);
		if ( inTextArea )
		{
		    formTextArea->setText( formText );
		    inTextArea = false;
		    vspace_inserted = false;
		    formTextArea = 0;
		}
	}
	else if ( strncmp( str, "tt", 2 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
		pushBlock(ID_TT, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp( str, "/tt", 3 ) == 0 )
	{
		popBlock( ID_TT, _clue);
	}
}
 
// <u>              </u>
// <ul              </ul>
void KHTMLParser::parseU( HTMLClue *_clue, const char *str )
{
    if ( strncmp( str, "ul", 2 ) == 0 )
    {
	    closeAnchor();
	    if ( listStack.isEmpty() )
	    {
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		pushBlock( ID_UL, 2, &KHTMLParser::blockEndList, indent, true);
	    }
	    else
	    {
		pushBlock( ID_UL, 2, &KHTMLParser::blockEndList, indent, false);
	    }

	    ListType type = Unordered;

	    stringTok->tokenize( str + 3, " >" );
	    while ( stringTok->hasMoreTokens() )
	    {
		    const char* token = stringTok->nextToken();
		    if ( strncasecmp( token, "plain", 5 ) == 0 )
			    type = UnorderedPlain;
	    }

	    listStack.push( new HTMLList( type ) );
	    indent += INDENT_SIZE;
	    flow = 0;
    }
    else if ( strncmp( str, "/ul", 3 ) == 0 )
    {
	popBlock( ID_UL, _clue);
    }
    else if ( strncmp(str, "u", 1 ) == 0 )
    {
	if ( str[1] == '>' || str[1] == ' ' )
	{
	    underline = TRUE;
	    selectFont();
	    pushBlock(ID_U, 1, &KHTMLParser::blockEndFont);
	}
    }
    else if ( strncmp( str, "/u", 2 ) == 0 )
    {
	    popBlock( ID_U, _clue);
    }
}

// <var>            </var>
void KHTMLParser::parseV( HTMLClue * _clue, const char *str )
{
	if ( strncmp(str, "var", 3 ) == 0 )
	{
		italic = TRUE;
		selectFont();
	   	pushBlock(ID_VAR, 1, &KHTMLParser::blockEndFont);
	}
	else if ( strncmp( str, "/var", 4 ) == 0)
	{
		popBlock( ID_VAR, _clue);
	}
}

void KHTMLParser::parseW( HTMLClue *, const char *)
{
}

void KHTMLParser::parseX( HTMLClue *, const char * )
{
}

void KHTMLParser::parseY( HTMLClue *, const char * )
{
}

void KHTMLParser::parseZ( HTMLClue *, const char * )
{
}

const char* KHTMLParser::parseCell( HTMLClue *_clue, const char *str )
{
    static const char *end[] = { "</cell>", 0 }; 
    HTMLClue::HAlign olddivalign = divAlign;
    HTMLClue *oldFlow = flow;
    int oldindent = indent;

    HTMLClue::HAlign gridHAlign = HTMLClue::HCenter;// global align of all cells
    int cell_width = 90;

    stringTok->tokenize( str + 5, " >" );
    while ( stringTok->hasMoreTokens() )
    {
	const char* token = stringTok->nextToken();
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
    _clue->append( vc );
    vc->setFixedWidth( cell_width ); // fixed width
    vc->setVAlign( valign );
    vc->setHAlign( halign );

    flow = 0;
    indent = 0;
    divAlign = HTMLClue::Left;
                 
    pushBlock( ID_CELL, 3 );
    str = parseBody( vc, end );
    popBlock( ID_CELL, vc );

// @@WABA What does this do?
//    vc = new HTMLClueV( 0, 0 ); // fixed width
//    _clue->append( vc );

    indent = oldindent;
    divAlign = olddivalign;
    flow = oldFlow;

    return str;
}

const char* KHTMLParser::parseTable( HTMLClue *_clue, const char *attr )
{
    // missing tags:
    // <col> <colgroup> </colgroup>
    // <thead> </thead> <tbody> </tbody> <tfoot> </tfoot>
 
    static const char *endthtd[] = { "</th", "</td", "</tr", "<th", "<td", "<tr", "</table", 0 };
    static const char *endcap[] = { "</caption>", "</table>", "<tr", "<td", "<th", 0 };    
    static const char *endall[] = { "</caption>", "</table>", "<tr", "<td", "<th"," </th", "</td", "</tr", 0 };    
    const char* str = 0;
    bool firstRow = true;
    bool tableTag = true;
    bool noCell = true;
    int padding = 1;
    int spacing = 2;
    int width = 0;
    int percent = UNDEFINED;
    int border = 0;
    char has_cell = 0;
    HTMLClue::VAlign rowvalign = HTMLClue::VNone;
    HTMLClue::HAlign rowhalign = HTMLClue::HNone;
    HTMLClue::HAlign align = HTMLClue::HNone;
    HTMLClueV *caption = 0;
    HTMLTableCell *tmpCell = 0;
    HTMLClue::VAlign capAlign = HTMLClue::Bottom;
    HTMLClue::HAlign olddivalign = divAlign;
    HTMLClue *oldFlow = flow;
    int oldindent = indent;
    QColor tableColor;
    QColor rowColor;

    stringTok->tokenize( attr, " >" );
    while ( stringTok->hasMoreTokens() )
    {
	const char* token = stringTok->nextToken();
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
	    if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
	    {
		QString col = "#";
		col += token+8;
		tableColor.setNamedColor( col );
	    }
	    else
		tableColor.setNamedColor( token+8 );
	    rowColor = tableColor;
	}
    }

    HTMLTable *table = new HTMLTable( percent, width, padding, spacing, border );
    //       _clue->append( table ); 
    // CC: Moved at the end since we might decide to discard the table while parsing...

    indent = 0;
    
    bool done = false;

    while ( !done && ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( str[0] == TAG_ESCAPE )
	{
	    str++;

	    tableTag = true;

	    for(;;) 
	    {
		if ( strncmp( str, "<caption", 8 ) == 0 )
		{
		    stringTok->tokenize( str + 9, " >" );
		    while ( stringTok->hasMoreTokens() )
		    {
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0)
			{
			    if ( strncasecmp( token+6, "top", 3 ) == 0)
				capAlign = HTMLClue::Top;
			}
		    }
		    caption = new HTMLClueV();
		    divAlign = HTMLClue::HCenter;
		    flow = 0;
		    pushBlock(ID_CAPTION, 3 );
		    str = parseBody( caption, endcap );
                    popBlock( ID_CAPTION, caption );
		    table->setCaption( caption, capAlign );
		    flow = 0;

		    if ( str == 0 )
		    { 
			// CC: Close table description in case of a malformed table
			// before returning!
			if ( !firstRow )
			    table->endRow();
			table->endTable(); 
			delete table;
			divAlign = olddivalign;
			flow = oldFlow;
			delete tmpCell;
			return 0;
		    }

		    if (strncmp( str, "</caption", 9) == 0 )
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

		if ( strncmp( str, "<tr", 3 ) == 0 )
		{
		    if ( !firstRow )
			table->endRow();
		    table->startRow();
		    firstRow = FALSE;
		    rowvalign = HTMLClue::VNone;
		    rowhalign = HTMLClue::HNone;
		    rowColor = tableColor;

		    stringTok->tokenize( str + 4, " >" );
		    while ( stringTok->hasMoreTokens() )
		    {
			const char* token = stringTok->nextToken();
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
			    if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
			    {
				QString col = "#";
				col += token+8;
				rowColor.setNamedColor( col );
			    }
			    else
				rowColor.setNamedColor( token+8 );
			}
		    }
		    break; // Get next token from 'ht'
		}
		
		if ( strncmp( str, "</table", 7 ) == 0 )
		{
		    closeAnchor();
		    done = true;
		    break;
		}

		// <td, <th, or we get something before the 
		// first <td or <th. Lets put that into one row 
		// of it's own... (Lars)
		bool tableEntry = *str=='<' && *(str+1)=='t' && 
		    (*(str+2)=='d' || *(str+2)=='h');
		if ( tableEntry || noCell ) 
		//		else if ( strncmp( str, "<td", 3 ) == 0 ||
		//			strncmp( str, "<th", 3 ) == 0 )
		{
		    printf("making cell %d %d\n", tableEntry, noCell);
		    bool heading = false;
		    noCell = false;

		    // if ( strncasecmp( str, "<th", 3 ) == 0 )
		    if (*(str+2)=='h')
			    heading = true;
		    // <tr> may not be specified for the first row
		    if ( firstRow )
		    {
			// Bad HTML: No <tr> tag present
			table->startRow();
			firstRow = FALSE;
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
		    stringTok->tokenize( str + 4, " >" );
		    while ( stringTok->hasMoreTokens() )
		    {
			const char* token = stringTok->nextToken();
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
				percent = atoi( token + 6 );
			    else if ( strchr( token+6, '*' ) )
			    { /* ignore */ }
			    else
			    {
				cellWidth = atoi( token + 6 );
				percent = 0; // Fixed with
			    }
			}
			else if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
			{
			    if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
			    {
				QString col = "#";
				col += token+8;
				bgcolor.setNamedColor( col );
			    }
			    else
				bgcolor.setNamedColor( token+8 );
			}
		    } // while (hasMoreTokens)
		    } // if(tableEntry)

		    HTMLTableCell *cell = new HTMLTableCell( percent, cellWidth,
			rowSpan, colSpan, padding );
		    if ( bgcolor.isValid() )
			cell->setBGColor( bgcolor );
		    cell->setVAlign( valign );
		    table->addCell( cell );
		    has_cell = 1;
		    flow = 0;
		    if ( heading )
		    {
			weight = QFont::Bold;
			selectFont();
			pushBlock( ID_TH, 3, &KHTMLParser::blockEndFont);
		        str = parseBody( cell, endthtd );
                        popBlock( ID_TH, cell );
		    }
		    else if ( !tableEntry )
		    {
			// put all the junk between <table> and the first table
			// tag into one row.
		    	pushBlock( ID_TD, 3 );
			parseOneToken( cell, str );
			str = parseBody( cell, endall );
			popBlock( ID_TD, cell );
			table->endRow();
			table->startRow();
		    }
		    else
		    {
		    	pushBlock( ID_TD, 3 );
			str = parseBody( cell, endthtd );
			popBlock( ID_TD, cell );
		    }

		    if ( str == 0 )
		    { 
			// CC: Close table description in case of a malformed table
			// before returning!
			if ( !firstRow )
			    table->endRow();
			table->endTable(); 
			delete table;
			divAlign = olddivalign;
			flow = oldFlow;
			delete tmpCell;
			return 0;
		    }

		    if ((strncmp( str, "</td", 4) == 0 ) ||
		        (strncmp( str, "</th", 4) == 0))
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
		break;
#if 0
		else
		{
		  // catch-all for broken tables
      		  if ( *str != '<' || *(str+1) != '/' || *(str+2) != 't' ||
                      ( *(str+3)!='d' && *(str+3)!='h' && *(str+3)!='r' ) )
/*
		    if ( strncmp( str, "</td", 4 ) &&
			    strncmp( str, "</th", 4 ) &&
			    strncmp( str, "</tr", 4 ) )
*/
		    {
			flow = 0;
			if ( !tmpCell )
			{
			    // Variable width cell
			    tmpCell = new HTMLTableCell( UNDEFINED, UNDEFINED, 1, 1, padding );
			    if ( tableColor.isValid() )
				tmpCell->setBGColor( tableColor );
			}
    			parseOneToken( tmpCell, str );
			str = parseBody( tmpCell, endall );
			closeAnchor();
		    }
		    else
			tableTag = false;
		}
#endif
	    }
	}
    }

    // Did we catch any illegal HTML
    if ( tmpCell )
    {
	// if no cells have been added then this must be a table with
	// one cell and no <tr, <td, etc.  I HATE people who abuse HTML
	// like this.
	if ( !has_cell )
	{
	    if ( firstRow )
	    {
		table->startRow();
		firstRow = FALSE;
	    }
	    table->addCell( tmpCell );
	    has_cell = 1;
	}
	else
	    delete tmpCell;
    }

    if (has_cell)
    {
	// CC: the ending "</table>" might be missing, so 
	// we close the table here... ;-) 
	if ( !firstRow )
	    table->endRow();
	table->endTable();
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
    }
    else
    {
	// CC: last ressort -- remove tables that do not contain any cells
	delete table;
    }

    indent = oldindent;
    divAlign = olddivalign;
    flow = oldFlow;

    return str;
}

const char *KHTMLParser::parseInput( const char *attr )
{
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

    stringTok->tokenize( attr, " >" );
    while ( stringTok->hasMoreTokens() )
    {
	const char* token = stringTok->nextToken();
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

    return 0;
}


bool KHTMLParser::setCharset(const char *name){

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
          HTMLFont f( *font_stack.top());
	  debugM("Original font: face: %s qtCharset: %i\n"
	                              ,QFont(f).family(),(int)QFont(f).charSet());
	  f.setCharset(charset);
	  debugM("Changed font: face: %s qtCharset: %i\n"
	                              ,QFont(f).family(),(int)QFont(f).charSet());
          const HTMLFont *fp = pFontManager->getFont( f );
	  debugM("Got font: %p\n",fp);
	  debugM("Got font: face: %s qtCharset: %i\n",QFont(*fp).family(),(int)QFont(*fp).charSet());
          font_stack.push( fp );
	  debugM("painter: %p\n",painter);
	  debugM("Font stack top: %p\n",font_stack.top());
          if (painter) painter->setFont( *font_stack.top() );
	}
	return TRUE;
}

