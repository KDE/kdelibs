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
// KDE HTML Widget -- Objects
// $Id$

#include <kurl.h>
#include <kapp.h>

#include "khtmlobj.h"

#include "khtmlchain.h"
#include "khtmltoken.h"
#include "khtmlfont.h"
#include "khtml.h"
#include "khtmlcache.h"
#include "khtmltoken.h"

#include <qfile.h>
#include <qimage.h>
#include <qdrawutil.h>
#include <qregexp.h>

// Debug function
void debugM( const char *msg , ...);


HTMLString 
HTMLAllocator::newString( const QString &str)
{
    int len = str.length();
    const QChar *sourceString = str.unicode();
    QChar *resultString = (QChar *) allocate( sizeof(QChar)*len );

    int l = len;
    while (l--)
    {
        resultString[l] = sourceString[l];
    }

    return HTMLString(resultString, len);
}
 

// This will be constructed once and NEVER deleted.
int HTMLObject::objCount = 0;

//-----------------------------------------------------------------------------

HTMLObject::HTMLObject()
{
    flags = 0;
    width = 0;
    ascent = 0;
    descent = 0;
    objCount++;
    nextObj = 0;
    x = 0;
    y = 0;
}

HTMLObject* HTMLObject::checkPoint( int _x, int _y )
{
    if ( _x >= x && _x < x + width )
	if ( _y > y - ascent && _y < y + descent + 1 )
	    return this;

    return 0L;
}

void HTMLObject::select( KHTMLWidget *_htmlw, HTMLChain *_chain, QRect &_rect,
    int _tx, int _ty )
{
    QRect r( x + _tx, y - ascent + _ty, width, ascent + descent );

    bool s = isSelected();
    bool s2;

    if ( _rect.contains( r ) )
	s2 = TRUE;
    else
	s2 = FALSE;

    if ( s != s2 )
	select( _htmlw, _chain, s2, _tx, _ty );
}

void HTMLObject::getSelected( QStringList &_list )
{
    if ( &_list == 0L )
    {
	return;
    }

    QString u = getURL();

    if ( !u.isEmpty() && isSelected() )
    {
	for(QStringList::Iterator it = _list.begin(); it != _list.end(); ++it)
	{
	    if ( *it == u )
		return;
	}
	_list.append( u );
    }
}

void HTMLObject::selectByURL( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QString _url, bool _select, int _tx, int _ty )
{
    QString u = getURL();

    if ( u == 0 )
	return;

    if ( _url == u )
	select( _htmlw, _chain, _select, _tx, _ty );
}

void HTMLObject::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    HTMLString u = getURL();

    if ( !u.length() )
	return;

    KURL ku( u );
    QString filename = ku.filename();

    if ( filename.find( _pattern ) != -1 )
	select( _htmlw, _chain, _select, _tx, _ty );
}

void HTMLObject::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    bool _select, int _tx, int _ty )
{
    HTMLString u = getURL();
    if ( !u.length() || (_select == isSelected()) )
	return;
	
    setSelected( _select );

    _chain->push( this );
    _htmlw->paint(_chain, x + _tx, y - ascent + _ty, width, ascent+descent);
    _chain->pop();
}

bool HTMLObject::selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty )
{
    bool selectIt = false;

    if ( _y1 >= y + descent || _y2 <= y - ascent )
	    selectIt = false;
    // start and end are on this line
    else if ( _y1 >= y - ascent && _y2 <= y + descent )
    {
	if ( _x1 > _x2 )
	{
	    int tmp = _x1;
	    _x1 = _x2;
	    _x2 = tmp;
	}
	if ( _x1 < x + width/2 && _x2 > x + width/2 && _x2 - _x1 > width/2 )
	    selectIt = true;
    }
    // starts on this line and extends past it.
    else if ( _y1 >= y - ascent && _y2 > y + descent )
    {
	if ( _x1 < x + width/2 )
	    selectIt = true;
    }
    // starts before this line and ends on it.
    else if ( _y1 < y - ascent && _y2 <= y + descent )
    {
	if ( _x2 > x + width/2 )
	    selectIt = true;
    }
    // starts before and ends after this line
    else if ( _y1 < y - ascent && _y2 > y + descent )
    {
	selectIt = true;
    }

    if ( selectIt != isSelected() )
    {
	setSelected( selectIt );
	_chain->push( this );
	_htmlw->paint(_chain, x + _tx, y - ascent + _ty, width, ascent+descent);
	_chain->pop();
    }

    return selectIt;
}

int HTMLObject::findPageBreak( int _y )
{
    if ( _y < y + descent )
	return ( y - ascent );

    return -1;
}

bool HTMLObject::getObjectPosition( const HTMLObject *obj, int &xp, int &yp )
{
    if ( obj == this )
    {
	xp += x;
	yp += y;
	return true;
    }

    return false;
}

// HTMLObjects generally do not have children, so propagate is
// uninteresting.
void
HTMLObject::printDebug( bool, int indent, bool printObjects )
{
    // return if printing out the objects is not desired
    if(!printObjects) return;

    const char *str = "    ";
    int i;
    for( i=0; i<indent; i++)
	printf(str);

    printf(objectName());
    printf("\n");
}

//-----------------------------------------------------------------------------

HTMLVSpace::HTMLVSpace( int _vspace, Clear c ) : HTMLObject()
{
    setNewline( true );
    ascent = _vspace;
    descent = 0;
    width = 1;
    cl = c;
}

void HTMLVSpace::getSelectedText( QString &_str )
{
    if ( isSelected() )
	_str += '\n';
}

//-----------------------------------------------------------------------------
HTMLHSpace::HTMLHSpace( const HTMLFont * _font, QPainter *_painter, bool hidden )
	: HTMLObject()
{
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent()+1;
    if ( !hidden)
        width = _painter->fontMetrics().width( ' ' );
    else
        width = 0;
    setSeparator( true );
    setHidden( hidden );
}

void HTMLHSpace::recalcBaseSize( QPainter *_painter )
{
    const QFont &oldFont = _painter->font();
    _painter->setFont( *font );
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent() + 1;
    width = _painter->fontMetrics().width( ' ' );
    _painter->setFont( oldFont );
}

void HTMLHSpace::getSelectedText( QString &_str )
{
    if ( !isHidden() && isSelected() )
    {
        _str += ' ';
    }
}

bool HTMLHSpace::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height < y + descent )
	    return true;
	if ( isPrinted() )
	    return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLHSpace::print( QPainter *_painter, int _tx, int _ty )
{
    if (isHidden())
    	return;
    	
    _painter->setFont( *font );

    if ( isSelected() && _painter->device()->devType() != QInternal::Printer )
    {
	_painter->fillRect( x + _tx, y - ascent + _ty,
		width, ascent + descent, kapp->palette().normal().highlight() );
	_painter->setPen( kapp->palette().normal().highlightedText() );
    }
    else
    {
        _painter->setPen( font->textColor() );
    }
    _painter->drawText( x + _tx, y + _ty, " ", 1);
}

//-----------------------------------------------------------------------------

HTMLText::HTMLText(HTMLString _text, const HTMLFont *_font, QPainter *_painter)
    : HTMLObject()
{
    text = _text;
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent()+1;
    width = _painter->fontMetrics().width( _text );
    selStart = 0;
    selEnd = 0;
}

HTMLText::HTMLText( const HTMLFont *_font, QPainter *_painter ) : HTMLObject()
{
    text = HTMLString();
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent() + 1;
    width = 0;
    setSeparator( true );
    selStart = 0;
    selEnd = 0;
}

HTMLText::~HTMLText()
{
}


bool HTMLText::selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty )
{
    bool selectIt = false;
    int oldSelStart = selStart;
    int oldSelEnd = selEnd;

    if ( _y1 >= y + descent || _y2 <= y - ascent )
	    selectIt = false;
    // start and end are on this line
    else if ( _y1 >= y - ascent && _y2 <= y + descent )
    {
	if ( _x1 > _x2 )
	{
	    int tmp = _x1;
	    _x1 = _x2;
	    _x2 = tmp;
	}
	if ( _x1 < x + width && _x2 > x )
	{
	    selectIt = true;
	    selStart = 0;
	    if ( _x1 > x )
		selStart = getCharIndex( _x1 - x );
	    selEnd = text.length();
	    if ( _x2 < x + width )
		selEnd = getCharIndex( _x2 - x );
	}
    }
    // starts on this line and extends past it.
    else if ( _y1 >= y - ascent && _y2 > y + descent )
    {
	if ( _x1 < x + width )
	{
	    selectIt = true;
	    selStart = 0;
	    if ( _x1 > x )
		selStart = getCharIndex( _x1 - x );
	    selEnd = text.length();
	}
    }
    // starts before this line and ends on it.
    else if ( _y1 < y - ascent && _y2 <= y + descent )
    {
	if ( _x2 > x )
	{
	    selectIt = true;
	    selStart = 0;
	    selEnd = text.length();
	    if ( _x2 < x + width )
		selEnd = getCharIndex( _x2 - x );
	}
    }
    // starts before and ends after this line
    else if ( _y1 < y - ascent && _y2 > y + descent )
    {
	selectIt = true;
	selStart = 0;
	selEnd = text.length();
    }

    if ( selectIt && (selStart == selEnd) )
	selectIt = false;

    if ( (selectIt != isSelected()) || (oldSelStart != selStart) ||
	(oldSelEnd != selEnd) )
    {
	setSelected( selectIt );
	_chain->push( this );
	_htmlw->paint(_chain, x + _tx, y - ascent + _ty, width, ascent+descent);
	_chain->pop();
    }

    return selectIt;
}

bool HTMLText::selectText( const QRegExp &exp )
{
    int len;
    int pos = exp.match( text, 0, &len );

    if ( pos >= 0 )
    {
	selStart = pos;
	selEnd = pos + len;
	setSelected( true );
    }

    return ( pos != -1 );
}

// get the index of the character at _xpos.
//
int HTMLText::getCharIndex( int _xpos )
{
    int charWidth, index = 0, xp = 0, len = text.length();

    QFontMetrics fm( *font );

    while ( index < len )
    {
	charWidth = fm.width( text[ index ] );
	if ( xp + charWidth/2 >= _xpos )
	    break;
	xp += charWidth;
	index++;
    }

    return index;
}

void HTMLText::getSelectedText( QString &_str )
{
    if ( isSelected() )
    {
	if ( isNewline() )
	    _str += '\n';
	else
	{
	    int i = selStart;

	    // skip white space at the start of a line.
	    if ( !_str.isEmpty() && _str[ _str.length() - 1 ] == '\n' )
	    {
		while ( text[ i ] == ' ' )
		    i++;
	    }

	    while ( i < selEnd )
	    {
		_str += text[ i ];
		i++;
	    }
	}
    }
}

void HTMLText::recalcBaseSize( QPainter *_painter )
{
    const QFont &oldFont = _painter->font();
    _painter->setFont( *font );
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent() + 1;
    width = _painter->fontMetrics().width( text );
    _painter->setFont( oldFont );
}

bool HTMLText::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height < y + descent )
	    return true;
	if ( isPrinted() )
	    return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLText::print( QPainter *_painter, int _tx, int _ty )
{
    if ( isNewline() )
	return;

    _painter->setPen( font->textColor() );
    _painter->setFont( *font );

    if ( isSelected() && _painter->device()->devType() != QInternal::Printer )
    {
	_painter->drawText( x + _tx, y + _ty, text, selStart );
	int fillStart = _painter->fontMetrics().width( text, selStart );
	int fillEnd = _painter->fontMetrics().width( text, selEnd );
	_painter->fillRect( x + fillStart + _tx, y - ascent + _ty,
		fillEnd - fillStart, ascent + descent, kapp->palette().normal().highlight() );
	_painter->setPen( kapp->palette().normal().highlightedText() );
	_painter->drawText( x + _tx + fillStart, y + _ty, text + selStart,
		selEnd - selStart );
	_painter->setPen( font->textColor() );
	_painter->drawText( x + _tx + fillEnd, y + _ty, text + selEnd );
    }
    else
    {
	_painter->drawText( x + _tx, y + _ty, text );
    }
}

void
HTMLText::printDebug( bool, int indent, bool printObjects )
{
    // return if printing out the objects is not desired
    if(!printObjects) return;

    QString str = "    ";
    int i;
    for( i=0; i<indent; i++)
	printf(str.ascii());

    str = objectName();
    QString aStr = text;
    str += " (\"";
    if(aStr.length() > 20)
    {
	aStr.truncate(19);
	str += aStr + "...";
    } else
	str += aStr;
    str += "\")\n";
    printf(str.ascii());
}


//-----------------------------------------------------------------------------
HTMLTextMaster::HTMLTextMaster( HTMLString _text, const HTMLFont *_font,
                                QPainter *_painter)
  : HTMLText( _text, _font, _painter)
{
    int runWidth = 0;
    strLen = _text.length();
    QFontMetrics fm(*_font);

    prefWidth = fm.width( text );
    width = 0;
    minWidth = 0;

    int i = 0;
    while ( i < strLen )
    {
         if (_text[i] != ' ')
         {
             runWidth += fm.width( _text[i] );
         }
         else
         {
             if (runWidth > minWidth)
             {
                 minWidth = runWidth;
             }
             runWidth = 0;
         }
         i++;
    }
    if (runWidth > minWidth)
    {
        minWidth = runWidth;
    }
}



HTMLFitType HTMLTextMaster::fitLine( bool , bool , int )
{
    /* split ourselves up :) */
    if ( isNewline() )
    	return HTMLCompleteFit;

    // Remove existing slaves	
    HTMLObject *next_obj = next();
    while (next_obj && next_obj->isSlave())
    {
    	setNext(next_obj->next());
    	delete next_obj;
    	next_obj = next();
    }

    // Turn all text over to our slave.
    HTMLTextSlave *text_slave = new HTMLTextSlave( this, 0, strLen);

    text_slave->setNext(next());
    setNext(text_slave);
    return HTMLCompleteFit;
}

//-----------------------------------------------------------------------------
HTMLTextSlave::HTMLTextSlave( HTMLTextMaster *_owner,
                              short _posStart, short _posLen)
        : owner(_owner),
          posStart(_posStart),
          posLen(_posLen)
{
    QFontMetrics fm (*(_owner->font));
    ascent = _owner->getAscent();
    descent = _owner->getDescent();
    width = fm.width( _owner->text + _posStart, _posLen );
}

HTMLFitType HTMLTextSlave::fitLine( bool startOfLine, bool firstRun,
                             int widthLeft )
{
    int newLen;
    int newWidth;
    HTMLString splitPtr;

    HTMLString text = owner->text;
    // Set font settings in painter for correct width calculation
    QFontMetrics fm( *(owner->font) );

    // Remove existing slaves	
    HTMLObject *next_obj = next();

    if (next_obj && next_obj->isSlave())
    {
	//
	printf("TextSlave: recover layout\n");
	do
	{
    	    setNext(next_obj->next());
            delete next_obj;
    	    next_obj = next();
        }
        while (next_obj && next_obj->isSlave());
    }

    if (startOfLine && (text[posStart] == ' ') && (widthLeft >= 0) )
    {
    	// Skip leading space
    	posStart++;
	posLen--;
    }
    text += posStart;

    int splitPos;
    width = fm.width( text, text.length() );
    if ((width <= widthLeft) || (posLen <= 1) || (widthLeft < 0) )
    {
        // Text fits completely
        if (!next() || next()->isSeparator() || next()->isNewline())
	    return HTMLCompleteFit;
	
	// Text is followed by more text... break it before the last word.
        QString helpStr( (text+1).string() );
	splitPos = helpStr.findRev( QChar (' ') );
	if(splitPos == -1) return HTMLCompleteFit;
	splitPtr = text + 1 + splitPos;
    }
    else
    {
	splitPos = (text+1).string().find( QChar(' ') );
	if( splitPos != -1 )
	{
	    splitPtr = text + splitPos + 1;
	}
	else
	    splitPos = 0;
    }

    if (splitPtr.length())
    {
    	newLen = splitPtr.unicode() - text.unicode();
    	newWidth = fm.width( text, newLen);
    	if (newWidth > widthLeft)
    	{
    	    // Splitting doesn't make it fit
    	    splitPtr = HTMLString();
    	}
    	else
    	{
	    int extraLen;
            int extraWidth;

            for(;;)
            {
                HTMLString splitPtr2 = splitPtr + 1;
		splitPtr2.find( QChar(' ') );
                if (!splitPtr2.length())
                    break;
	    	extraLen = splitPtr2.unicode() - splitPtr.unicode();
                extraWidth = fm.width( splitPtr, extraLen);
                if (extraWidth+newWidth <= widthLeft)
                {
                    // We can break on the next seperator cause it still fits
                    newLen += extraLen;
                    newWidth += extraWidth;
                    splitPtr = splitPtr2;
                }
                else
                {	
                    // Using this seperator would over-do it.
                    break;
                }
            }
    	}	
    }
    else
    {
    	newLen = posLen;
    	newWidth = width;
    }

    if (!splitPtr.length())
    {
    	// No seperator available
    	if (firstRun == false)
    	{
    	    // Text does not fit, wait for next line
    	    return HTMLNoFit;
    	}

	// Text does not fit, too bad.
	// newLen & newWidth are valid
    }

    if (posLen - newLen > 0)
    {
	// Move remaining text to our text-slave
    	HTMLTextSlave *textSlave = new HTMLTextSlave( owner,
    		posStart + newLen, posLen - newLen);

	textSlave->setNext(next());
    	setNext(textSlave);
    }

    posLen = newLen;
    width = newWidth;

    return HTMLPartialFit;
}

bool HTMLTextSlave::selectText( const QRegExp &exp )
{
    short SelStart = owner->selStart;
    short SelEnd = owner->selEnd;
    int len;
    int pos = exp.match( owner->text, posStart, &len );

    if (( pos < posStart ) || ( pos >= posStart+posLen))
    {
    	return false;
    }
    SelStart = pos;
    SelEnd = pos + len;
    if (SelEnd > posStart+posLen)
    {
	SelEnd = posStart+posLen;
    }
    owner->selStart = SelStart;
    owner->selEnd = SelEnd;
    owner->setSelected( true );
    setSelected( true );
    if ((SelStart == posStart) && (SelEnd == posStart+posLen))
    {
	setAllSelected(true);
    }
    return true;
}

// get the index of the character at _xpos.
//
int HTMLTextSlave::getCharIndex( int _xpos )
{
    int charWidth, index = 0, xp = 0;
    HTMLString text = (owner->text) + posStart;

    QFontMetrics fm( *(owner->font) );

    while ( index < posLen )
    {
	charWidth = fm.width( text[ index ] );
	if ( xp + charWidth/2 >= _xpos )
	    break;
	xp += charWidth;
	index++;
    }

    return index;
}


bool HTMLTextSlave::selectText( KHTMLWidget *_htmlw,
                                        HTMLChain *_chain,
                                        int _x1, int _y1,
                                        int _x2, int _y2,
                                        int _tx, int _ty )
{
    // AAAAA
    // A I B
    // BBBBB
    typedef enum { mstA, mstAandI, mstI,
    		   mstAandIandB, mstIandB, mstB
    		 } metaSelectionType;

    metaSelectionType metaSelection;

    short oldSelStart = owner->selStart;
    short oldSelEnd = owner->selEnd;
    short newSelStart = oldSelStart;
    short newSelEnd = oldSelEnd;

    if ( _y1 >= y + descent)
    {
	// Only lines behind us are selected
	metaSelection = mstB;
    }
    else if ( _y2 <= y - ascent )
    {
    	// Only lines before us are selected
	metaSelection = mstA;
    }
    // start and end are on this line
    else if ( _y1 >= y - ascent && _y2 <= y + descent )
    {
	if ( _x1 > _x2 )
	{
	    int tmp = _x1;
	    _x1 = _x2;
	    _x2 = tmp;
	}
	if ( _x1 >= x + width)
	{
	    // Only text behind us is selected
	    metaSelection = mstB;
	}
	else if ( _x2 <= x)
	{
	    // Only text in front of us is selected
	    metaSelection = mstA;
	}
	else
	{
	    // We are (partly) selected
	    if ( _x1 > x )
	    {
	        // There is no text in front of us selected
	    	newSelStart = posStart+getCharIndex( _x1 - x );
		if ( _x2 < x + width )
		{
		    newSelEnd = posStart + getCharIndex( _x2 - x );
		    // There is no text in front or after us selected
		    metaSelection = mstI;
		}
		else
		{
		    // There is also text behind us selected
		    metaSelection = mstIandB;
		}
	    }
	    else
	    {
	        // There is text in front of us selected
		if ( _x2 < x + width )
		{
		    newSelEnd = posStart + getCharIndex( _x2 - x );
		    // There is also text in front of us selected
		    metaSelection = mstAandI;
		}
		else
		{
		    // There is also text in front and behind us selected
		    metaSelection = mstAandIandB;
		}
	    }
	}
    }
    // starts on this line and extends past it.
    else if ( _y1 >= y - ascent && _y2 > y + descent )
    {
	if ( _x1 < x + width )
	{
	    // We are selected
	    if ( _x1 > x )
	    {
		newSelStart = posStart + getCharIndex( _x1 - x );
		// There is no text in front of us selected
		metaSelection = mstIandB;
	    }
	    else
	    {
	    	// There is text in front of us selected as well
	    	metaSelection = mstAandIandB;
	    }
	}
	else
	{
	    // There is only text behind us selected
	    metaSelection = mstB;
	}
    }
    // starts before this line and ends on it.
    else if ( _y1 < y - ascent && _y2 <= y + descent )
    {
	if ( _x2 > x )
	{
	    // We are selected
	    if ( _x2 < x + width )
	    {
		newSelEnd = posStart+getCharIndex( _x2 - x );
		// There is no text behind us selected
		metaSelection = mstAandI;
	    }
	    else
	    {
	    	// There is text behind us selected as well
	    	metaSelection = mstAandIandB;
	    }
	}
	else
	{
	    // There is only text in front of us selected;
	    metaSelection = mstA;
	}
    }
    // starts before and ends after this line
    else if ( _y1 < y - ascent && _y2 > y + descent )
    {
	metaSelection = mstAandIandB;
    }

    switch (metaSelection)
    {
    case mstA:	
    	    if (newSelEnd > posStart)
    	        newSelEnd = posStart;
    	    if (newSelStart > posStart)
    	        newSelStart = posStart;
	break;
		
    case mstB:
	    if (newSelEnd < posStart+posLen)
 		newSelEnd = posStart+posLen;
	    if (newSelStart < posStart+posLen)
    	        newSelStart = posStart+posLen;
	break;    	

    case mstAandI:
    	if (newSelStart > posStart)
    	    newSelStart = posStart; 	
	break;

    case mstI:
	break;

    case mstIandB:
    	if (newSelEnd < posStart+posLen)
    	    newSelEnd = posStart+posLen; 	
	break;

    case mstAandIandB:
    	if (newSelStart > posStart)
    	    newSelStart = posStart; 	
    	if (newSelEnd < posStart+posLen)
    	    newSelEnd = posStart+posLen; 	
	break;

    default:
    	// Error	
    	newSelStart = 0;
    	newSelEnd = 0;
	break;
    }

    bool selectIt;
    bool selectItAll;

    if (newSelStart == newSelEnd)
    {
    	selectIt = false;
    	selectItAll = false;
	newSelStart = 0;
	newSelEnd = 0;
    }
    else
    {
    	selectIt = (newSelStart < (posStart+posLen)) &&
    		   (newSelEnd   > posStart);
	selectItAll = (newSelStart <= posStart) &&
		      (newSelEnd >= posStart+posLen);
    }

    if ( (selectIt != isSelected()) || (selectItAll != isAllSelected()) ||
        (oldSelStart != newSelStart) || (oldSelEnd != newSelEnd) )
    {
	owner->selStart = newSelStart;
	owner->selEnd = newSelEnd;
	owner->setSelected( newSelStart != newSelEnd );
	setSelected( selectIt );
	setAllSelected( selectItAll );
	_chain->push( this );
	_htmlw->paint(_chain, x + _tx, y - ascent + _ty, width, ascent+descent);
	_chain->pop();
    }

    return selectIt;
}

bool HTMLTextSlave::print( QPainter *_painter,
                                   int , int _y,
                                   int , int _height,
                                   int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height < y + descent )
	    return true;
	if ( isPrinted() )
	    return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLTextSlave::print( QPainter *_painter, int _tx, int _ty )
{
    HTMLString text;
    const HTMLFont *font;

    text = owner->text + posStart;
    font = owner->font;

    _painter->setPen( font->textColor() );
    _painter->setFont( *font );

    if ( owner->isSelected() && _painter->device()->devType() != QInternal::Printer )
    {
    	if (isSelected())
    	{
    	    if (isAllSelected())
    	    {
	    	int fillStart = 0;
	        int fillEnd = _painter->fontMetrics().width( text, posLen );
	        _painter->fillRect( x + fillStart + _tx, y - ascent + _ty,
		    fillEnd - fillStart, ascent + descent, kapp->palette().normal().highlight() );
	        _painter->setPen( kapp->palette().normal().highlightedText() );
	        _painter->drawText( x + _tx + fillStart, y + _ty, text,
	            posLen );
	        return;
    	    }
    	    else
    	    {
	    	short selStart = owner->selStart - posStart;
    		short selEnd = owner->selEnd - posStart;
    	
		// (partly) selected
		if (selEnd > posLen)
	    	    selEnd = posLen;
	    	else if (selEnd < 0)
	    	    selEnd = 0;

	    	if (selStart < 0)
	    	    selStart = 0;
	    	else if (selStart > posLen)
	    	    selStart = posLen;
	    	
		_painter->setPen( font->textColor() );
	    	_painter->drawText( x + _tx, y + _ty, text, selStart );
	    	int fillStart = _painter->fontMetrics().width( text, selStart );
	        int fillEnd = _painter->fontMetrics().width( text, selEnd );
	        _painter->fillRect( x + fillStart + _tx, y - ascent + _ty,
		    fillEnd - fillStart, ascent + descent, kapp->palette().normal().highlight() );
	        _painter->setPen( kapp->palette().normal().highlightedText() );
	        _painter->drawText( x + _tx + fillStart, y + _ty, text + selStart,
	            selEnd - selStart );
	        _painter->setPen( font->textColor() );
	        _painter->drawText( x + _tx + fillEnd, y + _ty, text + selEnd, posLen - selEnd );
	        return;
	    }
	}
    }

    _painter->setPen( font->textColor() );
    _painter->drawText( x + _tx, y + _ty, text, posLen );
}

//-----------------------------------------------------------------------------

HTMLRule::HTMLRule( int _length, int _percent, int _size, bool _shade )
	: HTMLObject()
{
    // Width is set in setMaxWidth()
    if ( _size < 2 )
	_size = 2;
    ascent = 6 + _size;
    descent = 6;
    length = _length;
    percent = _percent;
    shade = _shade;
    if ((length == UNDEFINED) && (percent == UNDEFINED))
    {
        percent = 100; // Default: full width
    }
}

int HTMLRule::calcMinWidth()
{
    if ( percent == UNDEFINED )
    {
        // Fixed width, lenght is minimum width
	return length;
    }
    else
    {
        // Percent width, we scale into whatever size is required
        return 1;
    }
}

void HTMLRule::setMaxWidth( int _max_width )
{
    if ( percent == UNDEFINED )
    {
        // Fixed width, our width is length, if allowed by _max_width
        if (length < _max_width)
            width = length;
        else
            width = _max_width;

    }
    else
    {
         // Percent width, we scale according to _max_width.
	 width = (percent * _max_width) / 100;
	 if (width < 0)
	     width = 1;
	 if (width > _max_width)
	     width = _max_width;
    }
}

bool HTMLRule::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height <= y + descent )
		return true;
	if ( isPrinted() )
		return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLRule::print( QPainter *_painter, int _tx, int _ty )
{
    QColorGroup colorGrp( Qt::black, Qt::black, Qt::lightGray, Qt::black, Qt::gray, Qt::black, Qt::black );

    int size = ascent - 6;
    int xp = x + _tx;
    int yp = y + _ty;
    int lw = size/2;

    if ( shade )
	qDrawShadePanel( _painter, xp, yp-size, width, size,
		colorGrp, false, lw, 0 );
    else
	_painter->fillRect( xp, yp-size, width, size, Qt::black );
}

//-----------------------------------------------------------------------------

HTMLBullet::HTMLBullet( int _height, int _level, const QColor &col )
	: HTMLObject(), color( col )
{
    ascent = _height;
    descent = 0;
    width = 14;
    level = _level;
}

bool HTMLBullet::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

	if ( toPrinter )
	{
		if ( _y + _height <= y + descent )
			return true;
		if ( isPrinted() )
			return false;
		setPrinted( true );
	}

    print( _painter, _tx, _ty );

	return false;
}

void HTMLBullet::print( QPainter *_painter, int _tx, int _ty )
{
	int yp = y + _ty - 9;
	int xp = x + _tx + 2;

	_painter->setPen( QPen( color ) );

	switch ( level )
	{
		case 1:
			_painter->setBrush( QBrush( color ) );
			_painter->drawEllipse( xp, yp, 7, 7 );
			break;

		case 2:
			_painter->setBrush( QBrush() );
			_painter->drawEllipse( xp, yp, 7, 7 );
			break;

		case 3:
			_painter->setBrush( QBrush( color ) );
			_painter->drawRect( xp, yp, 7, 7 );
			break;

		default:
			_painter->setBrush( QBrush() );
			_painter->drawRect( xp, yp, 7, 7 );
	}
}

//-----------------------------------------------------------------------------

HTMLImage::HTMLImage( KHTMLWidget *widget, HTMLString _filename,
	HTMLString _url, HTMLString _target, int _width, int _height,
	int _percent, int bdr )
    : HTMLObject()
{
    // Width is set in setMaxWidth()
    lastWidth = UNDEFINED;
    lastHeight = UNDEFINED;
    pixmap = 0;
    overlay = 0;
    bComplete = true;

    valign = Bottom;

    htmlWidget = widget;

    url = _url;
    target = _target;

    imageURL = _filename;

    predefinedWidth = _width;
    if (predefinedWidth == UNDEFINED)
        predefinedWidth = 0;

    predefinedHeight = _height;
    if (predefinedHeight == UNDEFINED)
        predefinedHeight = 0;

    border = bdr;
    percent = _percent;

    if (_height == UNDEFINED)
        ascent = 32 + border;
    else
        ascent = _height + border;

    descent = border;

    absX = -1;
    absY = -1;

    // A HTMLJSImage ?
    if ( !_filename.length() )
    {
      // Do not load an image yet
      imageURL = 0;
      bComplete = false;
      return;
    }

    printf("********* IMAGE %s ******\n",_filename.string().latin1() );

    htmlWidget->requestImage( this, imageURL );
}

void HTMLImage::changeImage( HTMLString _url )
{
  htmlWidget->imageCache()->free( imageURL, this);
  imageURL = _url;
  pixmap = 0;

  bComplete = false;
  htmlWidget->requestImage( this, imageURL );
}

void HTMLImage::setPixmap( QPixmap *p )
{
    printf("KHTMLImage::setPixmap()\n");
    pixmap = p;

    if (width == 0)
        return; // setMaxSize has not yet been called. Do nothing.
    printf("KHTMLImage::setPixmap() 1\n");
    if (  (
           (predefinedWidth) ||
           (percent != UNDEFINED) ||
           (pixmap->width() == lastWidth)
          )
          &&
    	  (
    	   (predefinedHeight) ||
    	   (pixmap->height() == lastHeight)
          )
       )
    {
	// Image dimension are predefined or have not changed:
	// draw ourselves.
	htmlWidget->paintSingleObject( this );
	printf("KHTMLImage::setPixmap() painted\n");    }
    else
    {
        // Image dimensions have been changed, recalculate layout
	htmlWidget->calcSize();
	htmlWidget->calcAbsolutePos();
	htmlWidget->scheduleUpdate( true );
	printf("KHTMLImage::setPixmap() paint scheduled\n");
    }
}

void HTMLImage::pixmapChanged(QPixmap *p)
{
    if( p )
	setPixmap( p );
}

void HTMLImage::setOverlay( HTMLString _ol )
{
    // overlays must be cached
    overlay = KHTMLCache::image( _ol );
}

int HTMLImage::calcMinWidth()
{
    if ( percent == UNDEFINED )
    {
        if (predefinedWidth)
            return predefinedWidth; // Image with predefined.
        else if (!pixmap || pixmap->isNull())
            return 32+2*border; // Image width not yet known.
        else
            return pixmap->width()+2*border;
    }
    // Percent width, we scale according to _max_width.
    return 1;
}

int HTMLImage::calcPreferredWidth()
{
    int pref_width;
    if ( percent == UNDEFINED )
    {
        if (predefinedWidth)
            pref_width = predefinedWidth; // Image with predefined.
        else if (!pixmap || pixmap->isNull())
            pref_width = 32+2*border; // Image width not yet known.
        else
            pref_width = pixmap->width()+2*border;
    }
    else
    {
        // Preferred is the image width, if we know it.
        if (!pixmap || pixmap->isNull())
            pref_width = 32+2*border; // Image width not yet known.
        else
            pref_width = pixmap->width()+2*border;
    }
    return pref_width;
}

void HTMLImage::setMaxWidth( int _max_width )
{
    if (predefinedHeight)
        ascent = predefinedHeight + border;
    else if (!pixmap || pixmap->isNull())
        ascent = 32+border;
    else
        ascent = pixmap->height() + border;

    if ( percent == UNDEFINED )
    {
        if (predefinedWidth)
            width = predefinedWidth; // Image with predefined.
        else if (!pixmap || pixmap->isNull())
            width = 32+2*border; // Image width not yet known.
        else
            width = pixmap->width()+2*border;

        if (width > _max_width)
            width = _max_width;
    }
    else
    {
         // Percent width, we scale according to _max_width.
	 width = (percent*_max_width) / 100;
	 if (width > _max_width)
	     width = _max_width;

         // If the height has not been defined, it scales as well
         if (!predefinedHeight && pixmap && !pixmap->isNull())
         {
             ascent = pixmap->height() * width / pixmap->width();
         }
    }
    if (pixmap && !pixmap->isNull())
    {
        lastWidth = pixmap->width();
        lastHeight = pixmap->height();
    }
    else
    {
        lastWidth = UNDEFINED;
        lastHeight = UNDEFINED;
    }
}

bool HTMLImage::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height <= y + border )
	    return true;
	if ( isPrinted() )
	    return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLImage::print( QPainter *_painter, int _tx, int _ty )
{
    const QPixmap *pixptr = pixmap;
    QRect rect( 0, 0, width - border*2, ascent - border );

    if ( pixmap )
    {
	if ( predefinedWidth )
	    rect.setWidth( pixmap->width() );

	if ( predefinedHeight )
	    rect.setHeight( pixmap->height() );
    }

    if ( pixptr == 0 || pixptr->isNull() )
    {
	if ( !predefinedWidth || !predefinedHeight )
	{
	    QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, Qt::darkGray, Qt::gray,
		    Qt::black, Qt::white );
	    qDrawShadePanel( _painter, x + _tx, y - ascent + _ty, width, ascent,
		    colorGrp, true, 1 );
	}
    }
    else
    {
	QPixmap pm = *pixptr;

	if ( isSelected() )
	{
	    QPainter p( &pm );
//	    p.setRasterOp( NotEraseROP );
//	    p.fillRect( 0, 0, pm.width(), pm.height(), blue );
	    QBrush b( kapp->palette().normal().highlight(), Qt::Dense4Pattern );
	    p.fillRect( 0, 0, pm.width(), pm.height(), b );
	}

	if ( (width - border*2 != pixptr->width() ||
	    ascent - border != pixptr->height() ) &&
	    pixptr->width() != 0 && pixptr->height() != 0 )
	{
	    QWMatrix matrix;
	    matrix.scale( (float)(width-border*2)/pixptr->width(),
		    (float)(ascent-border)/pixptr->height() );
	    QPixmap tp = pm.xForm( matrix );
	    rect.setRight( rect.width() * (width-border*2)/pixptr->width() );
	    rect.setBottom( rect.height() * (ascent-border)/pixptr->height());
	    _painter->drawPixmap( QPoint( x + _tx + border,
		    y - ascent + _ty + border ), tp, rect );
	}
	else
	    _painter->drawPixmap( QPoint( x + _tx + border,
		    y - ascent + _ty + border), pm, rect );
	
	if ( overlay )
	    _painter->drawPixmap( QPoint( x + _tx + border,
		    y - ascent + _ty + border ), *overlay, rect );
    }

    if ( border )
    {
	QPen pen( borderColor );
	_painter->setPen( pen );
	QBrush brush;
	_painter->setBrush( brush );	// null brush
	for ( int i = 1; i <= border; i++ )
	    _painter->drawRect( x+_tx+border-i, y - ascent + border + _ty - i,
		(width - border*2) + i*2, ascent - border + i*2 );
    }
}

void HTMLImage::calcAbsolutePos( int _x, int _y )
{
    absX = _x + x;
    absY = _y + y - ascent;
}

int HTMLImage::getAbsX()
{
    return absX;
}

int HTMLImage::getAbsY()
{
    return absY;
}

HTMLImage::~HTMLImage()
{
   htmlWidget->imageCache()->free( imageURL, this );
}

void
HTMLImage::printDebug( bool, int indent, bool printObjects )
{
    // return if printing out the objects is not desired
    if(!printObjects) return;

    QString str = "    ";
    int i;
    for( i=0; i<indent; i++)
	printf(str.ascii());

    QString aStr = imageURL;
    if(aStr.length() > 20)
    {
	aStr.truncate(19);
	str = aStr + "...";
    } else
	str = aStr;

    printf("%s (\"%s \"/%dx%d)\n", objectName(), str.data(),
		width, ascent);
}

//----------------------------------------------------------------------------

HTMLArea::HTMLArea( const QPointArray &_points, HTMLString _url,
	HTMLString _target )
{
	region = QRegion( _points );
	url = _url;
	target = _target;
}

HTMLArea::HTMLArea( const QRect &_rect, HTMLString _url, HTMLString _target )
{
	region = QRegion( _rect );
	url = _url;
	target = _target;
}

HTMLArea::HTMLArea( int _x, int _y, int _r, HTMLString _url,
	HTMLString _target )
{
	QRect r( _x - _r, _y - _r, _r * 2, _y * 2 );

	region = QRegion( r, QRegion::Ellipse );
	url = _url;
	target = _target;
}

//----------------------------------------------------------------------------

HTMLMap::HTMLMap( KHTMLWidget *w, HTMLString _url )
{
    areas.setAutoDelete( true );
    mapurl = _url;
    htmlWidget = w;
    
    KURL u(mapurl);
    if ( !u.isLocalFile() )
	htmlWidget->requestFile( this, mapurl );
}

HTMLMap::~HTMLMap()
{
    KURL u(mapurl);
    if ( !u.isLocalFile() )
	htmlWidget->cancelRequestFile( this );
}

bool HTMLMap::fileLoaded( QString, QBuffer& _buffer, bool )
{
  if ( !_buffer.open( IO_ReadOnly ) )
  {
    warning("Could not open buffer for reading a map\n" );
    return false;
  }

  bool res = fileLoaded( _buffer );

  _buffer.close();

  return res;
}

// The external map has been downloaded
void HTMLMap::fileLoaded( QString, QString _filename )
{
  QFile file( _filename );
  if ( !file.open( IO_ReadOnly ) )
  {
    warning("Could not open %s for reading a map\n", _filename.ascii() );
    return;
  }

  (void)fileLoaded( file );

  file.close();
}

bool HTMLMap::fileLoaded( QIODevice& file )
{
    QString buffer;
    QString href;
    QString coords;
    HTMLArea::Shape shape = HTMLArea::Rect;
    char ch;

    while ( !file.atEnd() )
    {
      // read in a line
      buffer[0] = '\0';
      do
      {
	ch = file.getch();
	if ( ch != '\n' && ch != -1 );
	buffer += ch;
      }
      while ( ch != '\n' && ch != -1 );

      // comment?
      if ( buffer[0] == '#' )
	continue;

      StringTokenizer st;
      QChar separ [] = { ' ', 0x0 };
      st.tokenize( HTMLString((QChar *)buffer.unicode(), 
			      buffer.length()), separ );

      // get shape
      HTMLString p = st.nextToken();

      if ( ustrcasecmp( p, "rect" ) == 0 )
	shape = HTMLArea::Rect;
      else if ( ustrcasecmp( p, "poly" ) == 0 )
	shape = HTMLArea::Poly;
      else if ( ustrcasecmp( p, "circle" ) == 0 )
	shape = HTMLArea::Circle;

      // get url
      p = st.nextToken();
	
      if ( p[0] == '#' )
      {// reference
	KURL u( htmlWidget->getDocumentURL() );
	u.setRef( p + 1 );
	href = u.url();
      }
      else if ( ustrchr( p.unicode(), ':' ) )
      {// full URL
	href =  p;
      }
      else
      {// relative URL
	KURL u2( htmlWidget->getBaseURL(), p );
	href = u2.url();
      }

      // read coords and create object
      HTMLArea *area = 0;

      switch ( shape )
      {
      case HTMLArea::Rect:
	{
	  p = st.nextToken();
	  int x1, y1, x2, y2;
	  sscanf( p.string().latin1(), "%d,%d,%d,%d", &x1, &y1, &x2, &y2 );
	  QRect rect( x1, y1, x2-x1, y2-y1 );
	  area = new HTMLArea( rect, href, 0 );
	  printf( "Area Rect %d, %d, %d, %d\n", x1, y1, x2, y2 );
	}
      break;

      case HTMLArea::Circle:
	{
	  p = st.nextToken();
	  int xc, yc, rc;
	  sscanf( p.string().latin1(), "%d,%d,%d", &xc, &yc, &rc );
	  area = new HTMLArea( xc, yc, rc, href, 0 );
	  printf( "Area Circle %d, %d, %d\n", xc, yc, rc );
	}
      break;

      case HTMLArea::Poly:
	{
	  printf( "Area Poly " );
	  int count = 0, x, y;
	  QPointArray parray;
	  while ( st.hasMoreTokens() )
	    {
	      p = st.nextToken();
	      sscanf( p.string().latin1(), "%d,%d", &x, &y );
	      parray.resize( count + 1 );
	      parray.setPoint( count, x, y );
	      printf( "%d, %d  ", x, y );
	      count++;
	    }
	  printf( "\n" );
	  if ( count > 2 )
	    area = new HTMLArea( parray, href, 0 );
	}
      break;
      }

      if ( area )
	addArea( area );
    }

    // No update needed.
    return false;
}

const HTMLArea *HTMLMap::containsPoint( int _x, int _y )
{
    const HTMLArea *area;

    for ( area = areas.first(); area != 0; area = areas.next() )
    {
	if ( area->contains( QPoint( _x, _y ) ) )
	    return area;
    }

    return 0;
}

//----------------------------------------------------------------------------

HTMLImageMap::HTMLImageMap( KHTMLWidget *widget, HTMLString _filename,
	                    HTMLString _url, HTMLString _target,
	                    int _width, int _height, int _percent, int bdr )
    : HTMLImage( widget, _filename,
                 _url, _target,
                 _width, _height, _percent, bdr )
{
    type = ClientSide;
    serverurl = _url;
}

HTMLObject* HTMLImageMap::checkPoint( int _x, int _y )
{
    if ( _x >= x && _x < x + width )
    {
	if ( _y > y - ascent && _y < y + descent + 1 )
	{
	    if ( type == ClientSide )
	    {
		HTMLMap *map = htmlWidget->getMap( mapurl );
		if ( map )
		{
		    const HTMLArea *area = map->containsPoint( _x - x,
			_y - ( y -ascent ) );
		    if ( area )
		    {
			dynamicURL = area->getURL();
			dynamicTarget = area->getTarget();
			return this;
		    }
		    else
		    {
			dynamicURL = QString::null;
			dynamicTarget = QString::null;
		    }
		}
	    }
	    else
	    {
		QString coords;
		coords.sprintf( "?%d,%d", _x - x, _y - ( y -ascent ) );
		dynamicURL = serverurl;
		dynamicTarget = url;
		dynamicTarget += coords;
		return this;
	    }
	}
    }

    return 0;
}

//-----------------------------------------------------------------------------

HTMLAnchor* HTMLAnchor::findAnchor( QString _name, int &_x, int &_y )
{
    if ( name == _name )
    {
    	_x += x;
    	_y += y;
	return this;
    }

    return 0L;
}

//-----------------------------------------------------------------------------

