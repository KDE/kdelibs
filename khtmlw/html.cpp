//-----------------------------------------------------------------------------
//
// KDE HTML Widget
//

#include <kurl.h>

#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <qimage.h>
#include <qregexp.h>
#include <qkeycode.h>

#ifdef HAVE_LIBGIF
#include "gif.h"
#endif

#ifdef HAVE_LIBJPEG
#include "jpeg.h"
#endif

HTMLFontManager fontManager;

// Array of mark parser functions, e.g:
// <img ...  is processed by KHTMLWidget::parseI()
// </ul>     is processed by KHTMLWidget::parseU()
//
parseFunc KHTMLWidget::parseFuncArray[26] = {
	KHTMLWidget::parseA,
	KHTMLWidget::parseB,
	KHTMLWidget::parseC,
	KHTMLWidget::parseD,
	KHTMLWidget::parseE,
	KHTMLWidget::parseF,
	KHTMLWidget::parseG,
	KHTMLWidget::parseH,
	KHTMLWidget::parseI,
	KHTMLWidget::parseJ,
	KHTMLWidget::parseK,
	KHTMLWidget::parseL,
	KHTMLWidget::parseM,
	KHTMLWidget::parseN,
	KHTMLWidget::parseO,
	KHTMLWidget::parseP,
	KHTMLWidget::parseQ,
	KHTMLWidget::parseR,
	KHTMLWidget::parseS,
	KHTMLWidget::parseT,
	KHTMLWidget::parseU,
	KHTMLWidget::parseV,
	KHTMLWidget::parseW,
	KHTMLWidget::parseX,
	KHTMLWidget::parseY,
	KHTMLWidget::parseZ
};


KHTMLWidget::KHTMLWidget( QWidget *parent, const char *name, const char *_pix_path )
    : KDNDWidget( parent, name ), tempStrings( TRUE )
{
    setBackgroundColor( lightGray );
    x_offset = 0;
    y_offset = 0;
    url[ 0 ] = 0;
    title = "";    
    clue = 0L;
    italic = false;
    weight = QFont::Normal;
    ht = 0L;
    pressed = FALSE;
    pressedURL = "";
    actualURL= "";
    baseURL = "";
    text = 0;
    painter = NULL;
    parsing = FALSE;
    defaultFontBase = 3;
    overURL = "";
	granularity = 500;
    linkCursor = arrowCursor;
    waitingImageList.setAutoDelete( FALSE );
    bParseAfterLastImage = FALSE;
    bPaintAfterParsing = FALSE;
	formList.setAutoDelete( TRUE );

	standardFont = "times";
	fixedFont = "courier";
    
    QString f = _pix_path;
    f += "/khtmlw_dnd.xpm";
    dndDefaultPixmap.load( f.data() );
    
    registerFormats();

    connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
    
    setMouseTracking( TRUE );    
}

void KHTMLWidget::requestImage( HTMLImage *_image, const char *_url )
{
    waitingImageList.append( _image );
    emit imageRequest( _url );
}

void KHTMLWidget::requestBackgroundImage( const char *_url )
{
    bgPixmapURL = _url;
    bgPixmapURL.detach();
    emit imageRequest( _url );
}

void KHTMLWidget::slotImageLoaded( const char *_url, const char *_filename )
{
    QList<HTMLImage> del;
    del.setAutoDelete( FALSE );

    HTMLImage *p;    
    for ( p = waitingImageList.first(); p != 0L; p = waitingImageList.next() )
    {
	if ( strcmp( _url, p->getImageURL() ) == 0 )
	{
	    del.append( p );
	    p->imageLoaded( _filename );
	}
    }

    for ( p = del.first(); p != 0L; p = del.next() )
	waitingImageList.removeRef( p );

    if ( waitingImageList.count() == 0 && bParseAfterLastImage )
    {
	clue->reset();
	parse();
    }
    // Are we waiting for the background image ?
    else if ( !bgPixmapURL.isNull() )
    {
	// Did the background image arrive ?
	if ( strcmp( bgPixmapURL, _url ) == 0 )
	{
	    bgPixmap.load( _filename );					
	    bgPixmapURL = 0L;
	    if ( parsing )
		bPaintAfterParsing = TRUE;
	    else
	    {
		QPaintEvent pe( QRect( 0, 0, width(), height() ) );
		paintEvent( &pe );
		bPaintAfterParsing = FALSE;
	    }
	}
    }
}
	    
void KHTMLWidget::parseAfterLastImage()
{
    bParseAfterLastImage = TRUE;
}

void KHTMLWidget::slotFormSubmitted( const char *_method, const char *_url )
{
	emit formSubmitted( _method, _url );
}

void KHTMLWidget::mousePressEvent( QMouseEvent *_mouse )
{
    if ( clue == 0L )
	return;
    
    pressed = TRUE;
	    
    HTMLObject *obj;

    obj = clue->checkPoint( _mouse->pos().x() + x_offset - LEFT_BORDER, _mouse->pos().y() + y_offset );
    
    if ( obj != 0L)
	if ( obj->getURL() != 0 )
	    if (obj->getURL()[0] != 0)
	    {
		if ( _mouse->button() == RightButton )
		{
		    emit popupMenu( obj->getURL(), mapToGlobal( _mouse->pos() ) );
		    return;
		}
	
		// Save data. Perhaps the user wants to start a drag.
		press_x = _mouse->pos().x();
		press_y = _mouse->pos().y();    
		pressedURL = obj->getURL();
		pressedURL.detach();
		return;
	    }
    
    pressedURL = "";

    if ( _mouse->button() == RightButton )
	emit popupMenu( 0L, mapToGlobal( _mouse->pos() ) );
}

void KHTMLWidget::mouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if ( clue == 0L )
	return;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( _mouse->pos().x() + x_offset - LEFT_BORDER, _mouse->pos().y() + y_offset );
    
    if ( obj != 0L)
	if ( obj->getURL() != 0 )
	    if (obj->getURL()[0] != 0)
		emit doubleClick( obj->getURL(), _mouse->button() );
}

void KHTMLWidget::dndMouseMoveEvent( QMouseEvent * _mouse )
{
    if ( !pressed )
    {
    HTMLObject *obj = clue->checkPoint( _mouse->pos().x()+x_offset-LEFT_BORDER,
		 _mouse->pos().y() + y_offset );
	if ( obj != NULL )
	{
		if ( obj->getURL() && obj->getURL()[0] != 0 )
		{
			if ( overURL != obj->getURL() )
			{
				setCursor( linkCursor );
				overURL = obj->getURL();
				emit onURL( overURL );
			}
		}
		else if ( overURL != "" )
		{
			setCursor( arrowCursor );
			emit onURL( NULL );
			overURL = "";
		}
	}
	else if ( overURL != "" )
	{
		setCursor( arrowCursor );
		emit onURL( NULL );
		overURL = "";
	}
	return;
    }
    if ( pressedURL.isNull() )
	return;
    if ( pressedURL.data()[0] == 0 )
	return;
    
    int x = _mouse->pos().x();
    int y = _mouse->pos().y();

    if ( abs( x - press_x ) > Dnd_X_Precision || abs( y - press_y ) > Dnd_Y_Precision )
    {
	QPoint p = mapToGlobal( _mouse->pos() );

	int dx = - dndDefaultPixmap.width() / 2;
	int dy = - dndDefaultPixmap.height() / 2;

	startDrag( new KDNDIcon( dndDefaultPixmap, p.x() + dx, p.y() + dy ), pressedURL.data(), pressedURL.length(), DndURL, dx, dy );
    }
}

void KHTMLWidget::dndMouseReleaseEvent( QMouseEvent * _mouse )
{
    // Used to prevent dndMouseMoveEvent from initiating a drag before
    // the mouse is pressed again.
    pressed = false;

    if ( clue == 0L )
	return;
    if ( pressedURL.isNull() )
	return;
    if ( pressedURL[0] == 0 )
	return;
    
    // if ( pressedURL.data()[0] == '#' )
    //	gotoAnchor( pressedURL.data() + 1 );
    // else
	emit URLSelected( pressedURL.data(), _mouse->button() );
}

void KHTMLWidget::dragEndEvent()
{
    // Used to prevent dndMouseMoveEvent from initiating a new drag before
    // the mouse is pressed again.
    pressed = false;
}

/*
 * Checks out wether there is a URL under the point p and returns a pointer
 * to this URL or 0L if there is none.
 */
const char* KHTMLWidget::getURL( QPoint &p )
{
    if ( clue == 0L )
	return 0L;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( p.x() + x_offset, p.y() + y_offset );
    
    if ( obj != 0L)
	if (obj->getURL()[0] != 0)
	    return obj->getURL();
    
    return 0L;
}

void KHTMLWidget::paintEvent( QPaintEvent* _pe )
{
    bool newPainter = FALSE;

    if ( clue == 0L )
	return;

    if ( painter == NULL )
    {
	painter = new QPainter;
	painter->begin( this );
	newPainter = TRUE;
    }

    // painter->translate( x_offset, -y_offset );    
    int tx = -x_offset + LEFT_BORDER;
    int ty = -y_offset;
    
    if ( !bgPixmap.isNull() )
	drawBackground( x_offset, y_offset, _pe->rect().x(), _pe->rect().y(),
			_pe->rect().width(), _pe->rect().height() );
    clue->print( painter, _pe->rect().x()-x_offset, _pe->rect().y()+y_offset,
		 _pe->rect().width(), _pe->rect().height(), tx, ty );
    
    if ( newPainter )
    {
	painter->end();
	delete painter;
	painter = NULL;
    }
}

void KHTMLWidget::resizeEvent( QResizeEvent* _re )
{
	emit resized( _re->size() );
}

void KHTMLWidget::keyPressEvent( QKeyEvent *_ke )
{
	switch ( _ke->key() )
	{
		case Key_Down:
			{
				if ( docHeight() < height() ) break;
				int newY = y_offset + 20;
				if ( newY > docHeight() - height() )
					newY = docHeight() - height();
				slotScrollVert( newY );
				emit scrollVert( newY );
			}
			break;

		case Key_Next:
			{
				if ( docHeight() < height() ) break;
				int newY = y_offset + height() - 20;
				if ( newY > docHeight() - height() )
					newY = docHeight() - height();
				slotScrollVert( newY );
				emit scrollVert( newY );
			}
			break;

		case Key_Up:
			{
				if ( docHeight() < height() ) break;
				int newY = y_offset - 20;
				if ( newY < 0 )
					newY = 0;
				slotScrollVert( newY );
				emit scrollVert( newY );
			}
			break;

		case Key_Prior:
			{
				if ( docHeight() < height() ) break;
				int newY = y_offset - height() + 20;
				if ( newY < 0 )
					newY = 0;
				slotScrollVert( newY );
				emit scrollVert( newY );
			}
			break;

		case Key_Right:
			{
				if ( docWidth() < width() ) break;
				int newX = x_offset + 20;
				if ( newX > docWidth() - width() )
					newX = docWidth() - width();
				slotScrollHorz( newX );
				emit scrollHorz( newX );
			}
			break;

		case Key_Left:
			{
				if ( docWidth() < width() ) break;
				int newX = x_offset - 20;
				if ( newX < 0 )
					newX = 0;
				slotScrollHorz( newX );
				emit scrollHorz( newX );
			}
			break;

		default:
			KDNDWidget::keyPressEvent( _ke );
	}
}

void KHTMLWidget::paintSingleObject( HTMLObject *_obj )
{
    bool newPainter = FALSE;

    if ( parsing )
	return;
    
    if ( clue == 0L )
	return;

    if ( painter == NULL )
    {
	painter = new QPainter;
	painter->begin( this );
	newPainter = TRUE;
    }

    int tx = x_offset + LEFT_BORDER;
    int ty = -y_offset;
    
    clue->print( painter, x_offset, y_offset,
		 width(), height(), tx, ty, _obj );
    
    if ( newPainter )
    {
	painter->end();
	delete painter;
	painter = NULL;
    }
}

void KHTMLWidget::getSelected( QStrList &_list )
{
    if ( clue == 0L )
	return;
    
    clue->getSelected( _list );
}

void KHTMLWidget::begin( const char *_url, int _x_offset, int _y_offset )
{
    x_offset = _x_offset;
    y_offset = _y_offset;

    emit scrollHorz( x_offset );
    emit scrollVert( y_offset );
    
    bgPixmapURL = 0;
    
    stopParser();
    
    if ( text != NULL )
	delete [] text;

    if ( _url != 0L )
    {
      actualURL = _url;
      KURL u( actualURL );
      QString p = u.path();
      if ( p.length() == 0 )
      {
	u.setReference( "" );
	QString base( u.url() );
	base += "/";
	baseURL = base;
      }
      else if ( p.right(1) == "/" )
      {
	u.setReference( "" );
	baseURL = u.url();
      }
      else
      {
	u.setReference( "" );
	QString base( u.url() );
	int pos = base.findRev( '/' );
	if ( pos > 0 )
		base.truncate( pos );
	baseURL = base;
      }
    }

    text = new char[ 30000 ];
    text_pos = 0;
    text_size = 30000;
}

void KHTMLWidget::write( QString &_str)
{
    if ( _str.isNull() )
	return;
    
    // text += _str;
    char *p = _str.data();
    	while ( *p != 0 )
    {
	text[ text_pos++ ] = *p++;
	if ( text_pos >= text_size )
	{
	    char *p2 = new char[ text_size * 2 ];
	    memcpy( p2, text, text_size );
	    delete [] text;
	    text = p2;
	    text_size *= 2;
	}
    }
}

void KHTMLWidget::write( const char *_str)
{
    if ( _str == 0L )
	return;
    
    // text += _str;
    const char *p = _str;
    while ( *p != 0 )
    {
	text[ text_pos++ ] = *p++;
	if ( text_pos >= text_size )
	{
	    char *p2 = new char[ text_size * 2 ];
	    memcpy( p2, text, text_size );
	    delete [] text;
	    text = p2;
	    text_size *= 2;
	}
    }
}

void KHTMLWidget::end()
{
    text[ text_pos ] = 0;
    printf("TEXT SIZE=%i of %i, CLUE = %x\n",text_pos,text_size,(int)clue);
    // printf("\n\n%s\n",text);
}

// changes a current font
// needed for headings, and whenever the font family change is necessery
void KHTMLWidget::selectFont( const char *_fontfamily, int _fontsize, int _weight, bool _italic )
{
	if ( _fontsize < 0 )
		_fontsize = 0;
	else if ( _fontsize >= MAXFONTSIZES )
		_fontsize = MAXFONTSIZES - 1;

	HTMLFont f( _fontfamily, _fontsize, _weight, _italic );
	f.setTextColor( textColor );
	f.setLinkColor( linkColor );
	f.setVLinkColor( vLinkColor );
	const HTMLFont *fp = fontManager.getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLWidget::selectFont( int _relative_font_size )
{
	int fontsize;
	
	if ( currentFont() )
		fontsize = currentFont()->size() + _relative_font_size;
	else
		fontsize = fontBase + _relative_font_size;

	if ( fontsize < 0 )
		fontsize = 0;
	else if ( fontsize >= MAXFONTSIZES )
		fontsize = MAXFONTSIZES - 1;

	HTMLFont f( font_stack.top()->family(), fontsize, weight,
		italic );

	f.setUnderline( underline );
	f.setStrikeOut( strikeOut );
	f.setTextColor( textColor );
	f.setLinkColor( linkColor );
	f.setVLinkColor( vLinkColor );

	const HTMLFont *fp = fontManager.getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLWidget::popFont()
{
    font_stack.pop();
    if ( font_stack.isEmpty() )
	{
		HTMLFont f( standardFont, fontBase );
		const HTMLFont *fp = fontManager.getFont( f );
		font_stack.push( fp );
	}
    painter->setFont( *(font_stack.top()) );
    weight = font_stack.top()->weight();
    italic = font_stack.top()->italic();
    underline = font_stack.top()->underline();
    strikeOut = font_stack.top()->strikeOut();
	textColor = font_stack.top()->textColor();
	linkColor = font_stack.top()->linkColor();
	vLinkColor = font_stack.top()->vLinkColor();
}

void KHTMLWidget::parse()
{
    bParseAfterLastImage = FALSE;
    bPaintAfterParsing = FALSE;

    // Cancel all remaining image requests
    HTMLImage *p;
    for ( p = waitingImageList.first(); p != 0L; p = waitingImageList.next() )
	emit cancelImageRequest( p->getImageURL() );
    waitingImageList.clear();

    stopParser();
    
    if ( painter )
    {
	painter->end();
	delete painter;
    }
    painter = new QPainter();
    painter->begin( this );

    if ( ht != 0L )
	delete ht;

    QString txt = text;
    ht = new HTMLTokenizer( txt );

    tempStrings.clear();

    // Initialize the font stack with the default font.
    italic = false;
    underline = false;
    strikeOut = false;
    weight = QFont::Normal;
    textColor = black;
    linkColor = blue;
    vLinkColor = magenta;
    fontBase = defaultFontBase;
    
    font_stack.clear();
	HTMLFont f( standardFont, fontBase );
	f.setTextColor( textColor );
	f.setLinkColor( linkColor );
	f.setVLinkColor( vLinkColor );
	const HTMLFont *fp = fontManager.getFont( f );
    font_stack.push( fp );

	formList.clear();
	form = NULL;
	formSelect = NULL;
	inOption = false;
	inTextArea = false;

	parsing = TRUE;
	listLevel = 0;
	vspace_inserted = FALSE;
	divAlign = HTMLClue::Left;

	// clear page
	if ( bgPixmap.isNull() )
		painter->eraseRect( 0, 0, width(), height() );
	else
		drawBackground( x_offset, y_offset, 0, 0, width(), height() );

	if (clue)
		delete clue;
	clue = new HTMLClueV( 0, 0, width() - LEFT_BORDER - RIGHT_BORDER );
	clue->setVAlign( HTMLClue::Top );
	clue->setHAlign( HTMLClue::Left );

	flow = 0;

	// this will call slotTimeout repeatedly which in turn calls parseBody
	timer.start( 0 );
}

void KHTMLWidget::stopParser()
{
    if ( !parsing )
	return;
    
    timer.stop();
    
    parsing = FALSE;
    
    if ( bPaintAfterParsing )
    {
	QPaintEvent pe( QRect( 0, 0, width(), height() ) );
	paintEvent( &pe );
	bPaintAfterParsing = FALSE;
    }
}

void KHTMLWidget::slotTimeout()
{
    static const char *end[] = { "</body>", NULL }; 

	if ( !painter )
		return;

	const QFont &oldFont = painter->font();

	painter->setFont( *font_stack.top() );

	parseCount = granularity;
	parseBody( clue, end, TRUE ); 

	int lastHeight = docHeight();

	clue->calcSize();
	clue->setPos( 0, clue->getAscent() );
	if ( formList.count() > 0 )
		clue->calcAbsolutePos( 0, 0 );

	painter->setFont( oldFont );

	// if ( lastHeight - y_offset < height() )

	// If the visible rectangle was not filled before the parsing and
	// if we have something to display in the visible area now then repaint.
	if ( lastHeight - y_offset < height() && docHeight() - y_offset > 0 )
	{
		positionFormElements();
	    repaint( FALSE );
	}

	emit documentChanged();

	// Parsing is over ?
	if ( !parsing )
	{
	    // Is y_offset too big ?
	    if ( docHeight() - y_offset < height() )
	    {
		y_offset = docHeight() - height();
		if ( y_offset < 0 )
		    y_offset = 0;
	    }
	    // Adjust the scrollbar
	    emit scrollVert( y_offset );

	    // Is x_offset too big ?
	    if ( docWidth() - x_offset < width() )
	    {
		x_offset = docWidth() - width();
		if ( x_offset < 0 )
		    x_offset = 0;
	    }	    
	    // Adjust the scrollbar
	    emit scrollHorz( x_offset );
	    
	    painter->end();
	    delete painter;
	    painter = NULL;

		emit documentDone();
	}
}

bool KHTMLWidget::insertVSpace( HTMLClueV *_clue, bool _vspace_inserted )
{
    if ( !_vspace_inserted )
    {
	HTMLClueFlow *f = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	_clue->append( f );
	HTMLVSpace *t = new HTMLVSpace( HTMLFont::pointSize( fontBase ) );
	f->append( t );
    }
    
    return TRUE;
}

const char* KHTMLWidget::parseBody( HTMLClueV *_clue, const char *_end[], bool toplevel )
{
    const char *str;
    int i;
    
    // Before one can write any objects in the body he usually adds a FlowBox
    // to _clue. Then put all texts, images etc. in the FlowBox.
    // If f == 0L, you have to create a new FlowBox, otherwise you can
    // use the one stored in f. Setting f to 0L means closing the FlowBox.

    while ( ht->hasMoreTokens() && parsing )
    {
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( *str == TAG_ESCAPE )
	{
	    str++;

	    i = 0;
	    while ( _end[i] != NULL )
	    {
		if ( strcasecmp( str, _end[i] ) == 0 )
		{
		    if ( toplevel )
			stopParser();
		    return str;
		}
		i++;
	    }
	    
	    // The tag used for line break when we are in <pre>...</pre>
	    if ( *str == '\n' )
	    {
		// tack a space on the end to ensure the previous line is not 0 high
		if ( flow )
			flow->append( new HTMLText( currentFont(), painter ) );
		flow = new HTMLClueH( 0, 0, _clue->getMaxWidth() );
		_clue->append( flow );
	    }
		else if ( *str == '<' )
		{
			int indx;

			if ( str[1] == '/' )
				indx = toupper( str[2] ) - 'A';
			else
				indx = toupper( str[1] ) - 'A';

			if ( indx >= 0 && indx < 26 )
				parseFuncArray[indx]( _clue, str );
		}
	}
	else if ( strcmp( str, " " ) == 0 )
	{
		// if in* is set this text belongs in a form element
		if ( inOption || inTextArea )
			formText += " ";
		else if ( flow != 0)
		flow->append( new HTMLText( currentFont(), painter ) );
	}
	else if ( *str != 0 )
	{
		// if in* is set this text belongs in a form element
		if ( inOption || inTextArea )
		{
			formText += str;
		}
		else
		{
	    	vspace_inserted = FALSE;

			if ( flow == NULL )
			{
			flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
			flow->setHAlign( divAlign );
			_clue->append( flow );
			}
			flow->append( new HTMLText( str, currentFont(), painter, url ) );
		}
	}

	if ( toplevel )
	{
		if ( parseCount <= 0 )
			return 0L;
	}
	parseCount--;
    }

	if ( !ht->hasMoreTokens() && toplevel )
		stopParser();

    return 0L;
}

// <a               </a>            partial
// <address>        </address>
void KHTMLWidget::parseA( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp( str, "<a ", 3 ) == 0 )
	{
		vspace_inserted = FALSE;
		url[0] = '\0';
		
		const char *p = str + 3;

		while ( *p != '>' )
		{
			if ( strncasecmp( p, "href=", 5 ) == 0 )
			{
				p += 5;

				char buffer[ 1024 ];
				int j = 0;
				int i = 0;
				bool quoted = FALSE;
				while ( (p[i] != '\0' && p[i] != ' ' && p[i] != '>') || quoted )
				{
					if ( p[i] == '\"' )
					quoted = !quoted;
					else if ( j < 1024 )
					buffer[ j++ ] = p[ i ];
					i++;
				}
				buffer[j] = 0;

				if ( buffer[0] == '#' )
				{// reference
				    KURL u( actualURL );
				    u.setReference( buffer + 1 );
				    strcpy( url, u.url() );
				}
				else if ( strchr( buffer, ':' ) )
				{// full URL
					strcpy( url, buffer );
				}
				else
				{// relative URL
				    KURL u( baseURL );
				    KURL u2( baseURL, buffer );
				    strcpy( url, u2.url() );
				}
				
				p += i;
				if ( *p == ' ' )
				p++;
			}
			if ( strncasecmp( p, "name=", 5 ) == 0 )
			{
				p += 5;

				char buffer[ 1024 ];
				int i = 0;
				int j = 0;
				bool quoted = FALSE;
				while ( ( p[i] != ' ' && p[i] != '>' ) || quoted )
				{
					if ( p[i] == '\"' )
					quoted = !quoted;
					else if ( j < 1024 )
					buffer[ j++ ] = p[ i ];
					i++;
				}
				buffer[j] = 0;

				if ( flow == 0 )
					_clue->append( new HTMLAnchor( buffer ) );
				else
					flow->append( new HTMLAnchor( buffer ) );

				p += i;
				if ( *p == ' ' )
					p++;
			}
			else
			{
				char *p2 = strchr( p, ' ' );
				if ( p2 == 0L )
				    p2 = strchr( p, '>');
				else
				    p2++;
				p = p2;
			}
		}
	}
	else if ( strncasecmp( str, "</a>", 4 ) == 0 )
	{
		vspace_inserted = FALSE;
		url[ 0 ] = 0;
	}
	else if ( strncasecmp( str, "<address>", 9) == 0 )
	{
//		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		flow->setHAlign( divAlign );
		_clue->append( flow );
		italic = TRUE;
		weight = QFont::Normal;
		selectFont( 0 );
	}
	else if ( strncasecmp( str, "</address>", 10) == 0 )
	{
		popFont();
	}
}

// <b>              </b>
// <base>                           unimplemented
// <basefont                        unimplemented
// <big>            </big>
// <blockquote>     </blockquote>
// <body
// <br
void KHTMLWidget::parseB( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp(str, "<b>", 3 ) == 0 )
	{
		weight = QFont::Bold;
		selectFont( 0 );
	}
	else if ( strncasecmp(str, "</b>", 4 ) == 0 )
	{
		popFont();
	}
	else if ( strncasecmp( str, "<basefont", 9 ) == 0 )
	{
	}
	else if ( strncasecmp(str, "<big>", 5 ) == 0 )
	{
		selectFont( +2 );
	}
	else if ( strncasecmp(str, "</big>", 6 ) == 0 )
	{
		popFont();
	}
	else if ( strncasecmp(str, "<blockquote>", 12 ) == 0 )
	{
                static const char *end[] = { "</blockquote>", NULL };  
		HTMLClueH *c = new HTMLClueH( 0, 0, _clue->getMaxWidth() );
		c->setVAlign( HTMLClue::Top );
		_clue->append( c );

		HTMLClueV *vc = new HTMLClueV( 0, 0, 30, 0 );	// fixed width spacer
		c->append( vc );

		vc = new HTMLClueV( 0, 0, c->getMaxWidth()-60 );
		c->append( vc );
		flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth() );
		flow->setHAlign( divAlign );
		vc->append( flow );
		str = parseBody( vc, end, NULL );

		vc = new HTMLClueV( 0, 0, 30, 0 );	// fixed width spacer
		c->append( vc );

		flow = 0;
	}
	else if ( strncasecmp( str, "<body", 5 ) == 0 )
	{
		bool bgColorSet = FALSE;
		bool bgPixmapSet = FALSE;
		bool fontColorSet = FALSE;
		QString s = str + 6;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "bgcolor=", 8 ) == 0 && !bgPixmapSet )
			{
				QColor color;
				if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
				{
					QString col = "#";
					col += token+8;
					color.setNamedColor( col );
				}
				else
					color.setNamedColor( token+8 );
				setBackgroundColor( color );
				bgColorSet = TRUE;
			}
			else if ( strncasecmp( token, "background=", 11 ) == 0 )
			{
				const char* filename = token + 11;
				if ( filename[0] == '/' )
				{
					bgPixmap.load( filename );
					if ( !bgPixmap.isNull() )
						bgPixmapSet = TRUE;
				}
				else
				{
					KURL kurl( baseURL, filename );
					if ( strcmp( kurl.protocol(), "file" ) == 0 )
					    bgPixmap.load( kurl.path() );
					else
					    requestBackgroundImage( kurl.url() );
					
					if ( !bgPixmap.isNull() )
						bgPixmapSet = TRUE;
				}
			}
			else if ( strncasecmp( token, "text=", 5 ) == 0 )
			{
				textColor.setNamedColor( token+5 );
				fontColorSet = TRUE;
			}
			else if ( strncasecmp( token, "link=", 5 ) == 0 )
			{
				linkColor.setNamedColor( token+5 );
				fontColorSet = TRUE;
			}
			else if ( strncasecmp( token, "vlink=", 6 ) == 0 )
			{
				vLinkColor.setNamedColor( token+6 );
				fontColorSet = TRUE;
			}
		}

		if ( !bgColorSet && !bgPixmapSet )
			setBackgroundColor( lightGray );
		if ( !bgPixmapSet )
			bgPixmap.resize( -1, -1 );
		if ( fontColorSet )
			selectFont( 0 );
	}
	else if ( strncasecmp( str, "<br", 3 ) == 0 )
	{
		vspace_inserted = FALSE;

		if ( flow != 0 )
			flow = 0;
		else
		{
			HTMLText *t = new HTMLText( currentFont(), painter );
			t->setNewline( true );
			_clue->append( t );
		}
	}
}

// <center>         </center>
// <cite>           </cite>
// <code>           </code>
// <comment>        </comment>      unimplemented
void KHTMLWidget::parseC( HTMLClueV *, const char *str )
{
	if (strncasecmp( str, "<center>", 8 ) == 0)
	{
		divAlign = HTMLClue::HCenter;
		flow = NULL;
	}
	else if (strncasecmp( str, "</center>", 9 ) == 0)
	{
		divAlign = HTMLClue::Left;
		flow = NULL;
	}
	else if (strncasecmp( str, "<cite>", 6 ) == 0)
	{
		italic = TRUE;
		weight = QFont::Normal;
		selectFont( 0 );
	}
	else if (strncasecmp( str, "</cite>", 7) == 0)
	{
		popFont();
	}
	else if (strncasecmp(str, "<code>", 6 ) == 0 )
	{
		selectFont( fixedFont, fontBase, QFont::Normal, FALSE );
	}
	else if (strncasecmp(str, "</code>", 7 ) == 0 )
	{
		popFont();
	}
}

// <dir             </dir>          partial
// <div             </div>
// <dl>             </dl>           partial
void KHTMLWidget::parseD( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp( str, "<dir", 4 ) == 0 )
	{
		parseList( _clue, _clue->getMaxWidth(), Dir );
	}
	else if ( strncasecmp( str, "<div", 4 ) == 0 )
	{
		QString s = str + 5;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0 )
			{
				if ( strcasecmp( token + 6, "right" ) == 0 )
					divAlign = HTMLClue::Right;
				else if ( strcasecmp( token + 6, "center" ) == 0 )
					divAlign = HTMLClue::HCenter;
				else
					divAlign = HTMLClue::Left;
			}
		}

		flow = NULL;
	}
	else if ( strncasecmp( str, "</div>", 6 ) == 0 )
	{
		divAlign = HTMLClue::Left;
		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		_clue->append( flow );
		flow->setHAlign( divAlign );
	}
	else if ( strncasecmp( str, "<dl>", 4 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		parseGlossary( _clue, _clue->getMaxWidth() );
	}
}

// <em>             </em>
void KHTMLWidget::parseE( HTMLClueV *, const char *str )
{
	if ( strncasecmp( str, "<em>", 4 ) == 0 )
	{
		italic = TRUE;
		selectFont( 0 );
	}
	else if ( strncasecmp( str, "</em>", 5 ) == 0 )
	{
		popFont();
	}
}

// <font>           </font>         partial
// <form>           </form>         partial
void KHTMLWidget::parseF( HTMLClueV *, const char *str )
{
	if ( strncasecmp( str, "<font", 5 ) == 0 )
	{
		QString s = str + 5;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "size=", 5 ) == 0 )
			{
				int num = atoi( token + 5 );
				if ( *(token + 5) == '+' || *(token + 5) == '-' )
					selectFont( num );
				else
					selectFont( num - currentFont()->size() - 1 );
			}
			else if ( strncasecmp( token, "color=", 6 ) == 0 )
			{
				textColor.setNamedColor( token+6 );
				selectFont( 0 );
			}
		}
	}
	else if ( strncasecmp( str, "</font>", 7 ) == 0 )
	{
		popFont();
	}
	else if ( strncasecmp( str, "<form", 5 ) == 0 )
	{
		QString action = "";
		QString method = "GET";

		QString s = str + 5;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "action=", 7 ) == 0 )
			{
				action = token + 7;
			}
			else if ( strncasecmp( token, "method=", 7 ) == 0 )
			{
				if ( strncasecmp( token + 7, "post", 4 ) == 0 )
					method = "POST";
			}
		}

		form = new HTMLForm( action, method );
		formList.append( form );
		connect( form, SIGNAL( submitted( const char *, const char *) ),
				SLOT( slotFormSubmitted( const char *, const char * ) ) );
	}
	else if ( strncasecmp( str, "</form>", 7 ) == 0 )
	{
		form = NULL;
	}
}

// <grid                            extension
void KHTMLWidget::parseG( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp( str, "<grid", 5 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = 0L;
		str = parseGrid( _clue, _clue->getMaxWidth(), str + 6 );
	}
}

// <h[1-6]>         </h[1-6]>
// <hr
void KHTMLWidget::parseH( HTMLClueV *_clue, const char *str )
{
	static QRegExp re("<h[1-6]", FALSE );

	if ( re.match( str ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		HTMLClue::HAlign align = divAlign;

		QString s = str + 3;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0 )
			{
				if ( strcasecmp( token + 6, "center" ) == 0 )
					align = HTMLClue::HCenter;
				else if ( strcasecmp( token + 6, "right" ) == 0 )
					align = HTMLClue::Right;
			}
		}
		// Start a new flow box
		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		flow->setHAlign( align );
		_clue->append( flow );

		switch ( str[2] )
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
	}
	else if ( strncasecmp(str, "</h1>", 5 ) == 0 ||
		strncasecmp(str, "</h2>", 5 ) == 0 ||
		strncasecmp(str, "</h3>", 5 ) == 0 ||
		strncasecmp(str, "</h4>", 5 ) == 0 ||
		strncasecmp(str, "</h5>", 5 ) == 0 ||
		strncasecmp(str, "</h6>", 5 ) == 0 )
	{
		// Insert a vertical space if this did not happen already.
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		// Close the FlowBox of the headline. This way we get a line break.
		flow = 0L;
		// Restore the old font
		popFont();
	}
	else if ( strncasecmp(str, "<hr", 3 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		flow->setHAlign( divAlign );
		_clue->append( flow );

		int size = 1;
		int length = _clue->getMaxWidth();
		int percent = 100;
		HTMLRule::HAlign align = divAlign;
		bool shade = TRUE;

		QString s = str + 4;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0 )
			{
				if ( strncasecmp( token + 6, "left", 4 ) == 0 )
					align = HTMLRule::Left;
				else if ( strncasecmp( token + 6, "right", 5 ) == 0 )
					align = HTMLRule::Right;
				else
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
					percent = 0;
				}
			}
			else if ( strncasecmp( token, "noshade", 6 ) == 0 )
			{
				shade = FALSE;
			}
		}
		flow->append( new HTMLRule( _clue->getMaxWidth(), length, percent,
			size, align, shade ));
		flow = 0;
	}
}

// <i>              </i>
// <img                             partial
// <input                           partial
void KHTMLWidget::parseI( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp(str, "<i>", 3 ) == 0 )
	{
		italic = TRUE;
		selectFont( 0 );
	}
	else if ( strncasecmp( str, "</i>", 4 ) == 0 )
	{
		popFont();
	}
	else if (strncasecmp( str, "<img", 4 ) == 0)
	{
		vspace_inserted = FALSE;

		// Parse all arguments but delete '<' and '>' and skip 'cell'
		const char* filename = 0L;
		QString s = str + 5;
		QString fullfilename;
		int width = -1;
		int height = -1;
		int percent = 0;
		HTMLClue::HAlign align = HTMLClue::HNone;

		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if (strncasecmp( token, "src=", 4 ) == 0)
			filename = token + 4;
			else if (strncasecmp( token, "width=", 6 ) == 0)
			{
				if ( strchr( token + 6, '%' ) )
					percent = atoi( token + 6 );
				else
					width = atoi( token + 6 );
			}
			else if (strncasecmp( token, "height=", 7 ) == 0)
				height = atoi( token + 7 );
			else if (strncasecmp( token, "align=", 6 ) == 0)
			{
				if ( strcasecmp( token + 6, "left" ) == 0 )
					align = HTMLClue::Left;
				else if ( strcasecmp( token + 6, "right" ) == 0 )
					align = HTMLClue::Right;
			}
		}
		// if we have a file name do it...
		if ( filename != 0L )
		{
			KURL kurl( baseURL, filename );
			// Do we need a new FlowBox ?
			if ( flow == 0)
			{
				flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
				flow->setHAlign( divAlign );
				_clue->append( flow );
			}

			if ( align != HTMLClue::Left && align != HTMLClue::Right )
			{
				flow->append( new HTMLImage( this, kurl.url(), url,
								 _clue->getMaxWidth(), width, height, percent ) );
			}
			// we need to put the image in a HTMLClueAligned
			else
			{
				HTMLClueAligned *aligned = new HTMLClueAligned (flow, 0, 0, _clue->getMaxWidth() );
				aligned->setHAlign( align );
				aligned->append( new HTMLImage( this, kurl.url(), url, _clue->getMaxWidth(), width, height, percent ) );
				flow->append( aligned );
			}
		} 
	} 
	else if (strncasecmp( str, "<input", 6 ) == 0)
	{
		if ( form == NULL )
			return;
		if ( flow == NULL )
		{
			flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
			flow->setHAlign( divAlign );
			_clue->append( flow );
		}
		parseInput( str + 7 );
	}
}

void KHTMLWidget::parseJ( HTMLClueV *, const char * )
{
}

// <kbd>            </kbd>
void KHTMLWidget::parseK( HTMLClueV *, const char *str )
{
	if ( strncasecmp(str, "<kbd>", 5 ) == 0 )
	{
		selectFont( fixedFont, fontBase, QFont::Normal, FALSE );
	}
	else if ( strncasecmp(str, "</kbd>", 6 ) == 0 )
	{
		popFont();
	}
}

// <listing>        </listing>      unimplemented
void KHTMLWidget::parseL( HTMLClueV *, const char * )
{
}

// <menu>           </menu>         partial
void KHTMLWidget::parseM( HTMLClueV *_clue, const char *str )
{
	if (strncasecmp( str, "<menu>", 6 ) == 0)
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		HTMLClueV *v = new HTMLClueV( 30, 0, _clue->getMaxWidth() - 30 );
		_clue->append( v );
		parseList( _clue, _clue->getMaxWidth(), Menu );
		// Force a vertical space here.
		vspace_inserted = FALSE;
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = 0;
	}
}

void KHTMLWidget::parseN( HTMLClueV *, const char * )
{
}

// <ol>             </ol>           partial
// <option
void KHTMLWidget::parseO( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp( str, "<ol", 3 ) == 0 )
	{
		parseList( _clue, _clue->getMaxWidth(), Ordered );
	}
	else if ( strncasecmp( str, "<option", 7 ) == 0 )
	{
		if ( !formSelect )
			return;

		QString value = "";
		bool selected = false;

		QString s = str + 7;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "value=", 6 ) == 0 )
			{
				const char *p = token + 5;
				if ( *p == '"' ) p++;
				value = p;
				if ( value[ value.length() - 1 ] == '"' )
					value.truncate( value.length() - 1 );
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
	else if ( strncasecmp( str, "</option>", 9 ) == 0 )
	{
		inOption = false;
	}
}

// <p
// <pre             </pre>          partial
void KHTMLWidget::parseP( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp( str, "<pre", 4 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = new HTMLClueH( 0, 0, _clue->getMaxWidth() );
		_clue->append( flow );
		selectFont( fixedFont, fontBase, QFont::Normal, FALSE );
	}	
	else if ( strncasecmp( str, "</pre>", 6 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = 0;
		popFont();
	}
	else if ( strncasecmp( str, "<p", 2 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		HTMLClue::HAlign align = divAlign;

		QString s = str + 3;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0 )
			{
				if ( strcasecmp( token + 6, "center" ) == 0 )
					align = HTMLClue::HCenter;
				else if ( strcasecmp( token + 6, "right" ) == 0 )
					align = HTMLClue::Right;
			}
		}
		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		flow->setHAlign( align );
		_clue->append( flow );
	}
}

void KHTMLWidget::parseQ( HTMLClueV *, const char * )
{
}

void KHTMLWidget::parseR( HTMLClueV *, const char * )
{
}

// <s>              </s>
// <samp>           </samp>
// <select          </select>       partial
// <small>          </small>
// <strike>         </strike>
// <strong>         </strong>
// <sub>            </sub>          unimplemented
// <sup>            </sup>          unimplemented
void KHTMLWidget::parseS( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp(str, "<s>", 3 ) == 0 ||
		strncasecmp( str, "<strike>", 8 ) == 0 )
	{
		strikeOut = TRUE;
		selectFont( 0 );
	}
	else if ( strncasecmp(str, "</s>", 4 ) == 0 ||
		strncasecmp( str, "</strike>", 9 ) == 0 )
	{
		popFont();
	}
	else if ( strncasecmp(str, "<samp>", 6 ) == 0 )
	{
		selectFont( fixedFont, fontBase, QFont::Normal, FALSE );
	}
	else if ( strncasecmp(str, "</samp>", 7 ) == 0)
	{
		popFont();
	}
	else if ( strncasecmp(str, "<select", 7 ) == 0)
	{
		if ( !form )
			return;

		QString name = "";
		int size = 0;
		bool multi = false;

		QString s = str + 8;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "name=", 5 ) == 0 )
			{
				const char *p = token + 5;
				if ( *p == '"' ) p++;
				name = p;
				if ( name[ name.length() - 1 ] == '"' )
					name.truncate( name.length() - 1 );
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

		formSelect = new HTMLSelect( this, name, size, multi );
		form->addElement( formSelect );
		if ( flow == NULL )
		{
			flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
			flow->setHAlign( divAlign );
			_clue->append( flow );
		}

		flow->append( formSelect );
	}
	else if ( strncasecmp(str, "</select>", 9 ) == 0 )
	{
		if ( inOption )
			formSelect->setText( formText );

		formSelect = NULL;
		inOption = false;
	}
	else if ( strncasecmp(str, "<small>", 7 ) == 0 )
	{
		selectFont( -1 );
	}
	else if ( strncasecmp(str, "</small>", 8 ) == 0 )
	{
		popFont();
	}
	else if ( strncasecmp(str, "<strong>", 8 ) == 0 )
	{
		weight = QFont::Bold;
		selectFont( 0 );
	}
	else if ( strncasecmp(str, "</strong>", 9 ) == 0 )
	{
		popFont();
	}
}

// <table           </table>        partial
// <textarea        </textarea>
// <title>          </title>
// <tt>             </tt>
void KHTMLWidget::parseT( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp( str, "<table", 6 ) == 0 )
	{
		parseTable( _clue, _clue->getMaxWidth(), str + 7 );
	}
	else if ( strncasecmp( str, "<title>", 7 ) == 0 )
	{
		QString t = "";
		bool bend = FALSE;

		do
		{
			if ( !ht->hasMoreTokens() )
				bend = TRUE;
			else
			{
				str = ht->nextToken();
				if ( str[0] == TAG_ESCAPE &&
						strncasecmp( str + 1, "</title>", 8 ) == 0 )
					bend = TRUE;
				else
					t += str;
			}
		}
		while ( !bend );

		emit setTitle( t.data() );
	}
	else if ( strncasecmp( str, "<textarea", 9 ) == 0 )
	{
		if ( !form )
			return;

		QString name = "";
		int rows = 5, cols = 40;

		QString s = str + 8;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
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

		formTextArea = new HTMLTextArea( this, name, rows, cols );
		form->addElement( formTextArea );
		if ( flow == NULL )
		{
			flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
			flow->setHAlign( divAlign );
			_clue->append( flow );
		}

		flow->append( formTextArea );

		formText = "";
		inTextArea = true;
	}
	else if ( strncasecmp( str, "</textarea>", 11 ) == 0 )
	{
		if ( inTextArea )
			formTextArea->setText( formText );

		formTextArea = NULL;
		inTextArea = false;
	}
	else if ( strncasecmp( str, "<tt>", 4 ) == 0 )
	{
		selectFont( fixedFont, fontBase, QFont::Normal, FALSE );
	}
	else if ( strncasecmp( str, "</tt>", 5 ) == 0 )
	{
		popFont();
	}
}
 
// <u>              </u>
// <ul              </ul>           partial
void KHTMLWidget::parseU( HTMLClueV *_clue, const char *str )
{
	if ( strncasecmp(str, "<u>", 3 ) == 0 )
	{
		underline = TRUE;
		selectFont( 0 );
	}
	else if ( strncasecmp( str, "</u>", 4 ) == 0 )
	{
		popFont();
	}
	else if ( strncasecmp( str, "<ul", 3 ) == 0 )
	{
		ListType type = Unordered;

		QString s = str + 4;
		StringTokenizer st( s, " >" );
		while ( st.hasMoreTokens() )
		{
			const char* token = st.nextToken();
			if ( strncasecmp( token, "plain", 5 ) == 0 )
				type = UnorderedPlain;
		}

		// if we have a file name do it...
		parseList( _clue, _clue->getMaxWidth(), type );
	}
}

// <var>            </var>
void KHTMLWidget::parseV( HTMLClueV *, const char *str )
{
	if ( strncasecmp(str, "<var>", 5 ) == 0 )
	{
		italic = TRUE;
		selectFont( 0 );
	}
	else if ( strncasecmp( str, "</var>", 6 ) == 0)
	{
		popFont();
	}
}

void KHTMLWidget::parseW( HTMLClueV *, const char *)
{
}

void KHTMLWidget::parseX( HTMLClueV *, const char * )
{
}

void KHTMLWidget::parseY( HTMLClueV *, const char * )
{
}

void KHTMLWidget::parseZ( HTMLClueV *, const char * )
{
}

const char* KHTMLWidget::parseGrid( HTMLClue *_clue, int _max_width,
	const char *attr )
{
    const char* str;
    static const char *end[] = { "</cell>", NULL }; 
	HTMLClue::HAlign gridHAlign = HTMLClue::HCenter;// global align of all cells
	int cell_width = 100;

	QString s = attr;
	StringTokenizer st( s, " >" );
	while ( st.hasMoreTokens() )
	{
		const char* token = st.nextToken();
		if ( strncasecmp( token, "width=", 6 ) == 0 )
		{
			cell_width = atoi( token + 6 );
		}
		else if ( strncasecmp( token, "align=", 6 ) == 0 )
		{
			if ( strncasecmp( token + 6, "left", 4 ) == 0 )
				gridHAlign = HTMLClue::Left;
			else if ( strncasecmp( token + 6, "right", 5 ) == 0 )
				gridHAlign = HTMLClue::Right;
		}
	}

    HTMLClueFlow *c = new HTMLClueFlow( 0, 0, _max_width );
    _clue->append( c );

    while ( ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( str[0] == TAG_ESCAPE )
	{
	    str++;
	    
	    if (strncasecmp( str, "</grid>", 7 ) == 0)
	    {
		return str;
	    }
	    else if (strncasecmp( str, "<cell>", 6 ) == 0)
	    {
		HTMLClue::VAlign valign = HTMLClue::Top;
		HTMLClue::HAlign halign = gridHAlign;
		
		// Parse all arguments but delete '<' and '>' and skip 'cell'
		/*   QString s = str;
		     QString tmp = s.mid( 1, s.length() - 2 );
		     StringTokenizer st( tmp, " " );
		     st.nextToken();
		     while ( st.hasMoreTokens() )
		     {
		     QString token = st.nextToken();
		     if (token.find( "halign=left" ) == 0)
		     halign = HTMLClue::Left;
		     else if (token.find( "halign=center" ) == 0)
		     halign = HTMLClue::HCenter;
		     else if (token.find( "halign=right" ) == 0)
		     halign = HTMLClue::Right;
		     else if (token.find( "valign=top" ) == 0)
		     valign = HTMLClue::Top;
		     else if (token.find( "valign=center" ) == 0)
		     valign = HTMLClue::VCenter;
		     else if (token.find( "valign=bottom" ) == 0)
		     valign = HTMLClue::Bottom;
		     } */
		
		HTMLClueV *vc = new HTMLClueV( 0, 0, cell_width, 0 ); // fixed width
		c->append( vc );
		vc->setVAlign( valign );
		vc->setHAlign( halign );
		flow = NULL;
		str = parseBody( vc, end );

		vc = new HTMLClueV( 0, 0, 10, 0 ); // fixed width
		c->append( vc );
	    }
	}
    }

	flow = NULL;

    return 0L;
}

// list parser.
// <dir> really should be placed across the page like <grid>
//
const char* KHTMLWidget::parseList(HTMLClueV *_clue,int _max_width,ListType t)
{
    const char* str = NULL;
	static const char *endul[] = { "<li>", "</ul>", NULL };
	static const char *endol[] = { "<li>", "</ol>", NULL };
	static const char *endmenu[] = { "<li>", "</menu>", NULL };
	static const char *enddir[] = { "<li>", "</dir>", NULL };
	const char **end;     
	int   itemNum = 1;
	QString item;

	listLevel++;

	switch ( t )
	{
		case Unordered:
		case UnorderedPlain:
			end = endul;
			break;

		case Ordered:
			end = endol;
			break;

		case Menu:
			end = endmenu;
			break;

		case Dir:
			end = enddir;
			break;
	}

	HTMLClueV *container = new HTMLClueV( 0, 0, _max_width );
	container->setHAlign( divAlign );
	_clue->append( container );

    while ( ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( str[0] == TAG_ESCAPE )
	{
	    str++;
	    
	    while ( str && strncasecmp( str, "<li>", 4 ) == 0)
	    {
		HTMLClueH *c = new HTMLClueH( 0, 0, _max_width );
		c->setVAlign( HTMLClue::Top );
		container->append( c );
		HTMLClueV *vc = new HTMLClueV( 0, 0, 30, 0 ); // fixed width spacer
		vc->setHAlign( HTMLClue::Right );
		c->append( vc );
		switch ( t )
		{
			case Unordered:
				flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth(), 0 );
				flow->setVAlign( HTMLClue::Top );
				flow->setHAlign( HTMLClue::Right );
				vc->append( flow );
				flow->append( new HTMLBullet( font_stack.top()->pointSize(),
					listLevel, textColor ) );
				break;

			case Ordered:
				flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth(), 0 );
				flow->setVAlign( HTMLClue::Top );
				vc->append( flow );
				item.sprintf( "%2d. ", itemNum );
				tempStrings.append( item );
				flow->append(new HTMLText(tempStrings.getLast(),currentFont(),
					painter,""));
				break;

			default:
				break;
		}
		vc = new HTMLClueV( 0, 0, _max_width - 30 );
		c->append( vc );
		flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth() );
		vc->append( flow );
		str = parseBody( vc, end );
		itemNum++;
		}

	    if ( !str || strncasecmp( str, end[1], strlen( end[1]) ) == 0)
			break;
	}
    }

    flow = NULL;
    
    listLevel--;

    return str;
}

// glossary parser.
//
const char* KHTMLWidget::parseGlossary( HTMLClueV *_clue, int _max_width )
{
    const char* str = NULL;
    HTMLClueV *vc;
    static const char *enddt[] = { "<dt>", "<dd>", "<dl>","</dl>", NULL };
    static const char *enddd[] = { "<dt>", "</dl>", NULL };
 
    while ( ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( str[0] == TAG_ESCAPE )
	{
	    str++;
	    
		while ( str && ( strncasecmp( str, "<dt>", 4 ) == 0 ||
			strncasecmp( str, "<dd>", 4 ) == 0 ||
			strncasecmp( str, "<dl>", 4 ) == 0 ) )
		{
			if ( strncasecmp( str, "<dt>", 4 ) == 0 )
			{
				HTMLClueFlow *c = new HTMLClueFlow( 0, 0, _max_width );
				c->setVAlign( HTMLClue::Top );
				_clue->append( c );
				vc = new HTMLClueV( 0, 0, _max_width );
				c->append( vc );
				flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth() );
				vc->append( flow );
				str = parseBody( vc, enddt );
			}
			else if ( strncasecmp( str, "<dd>", 4 ) == 0 )
			{
				HTMLClueH *c = new HTMLClueH( 0, 0, _max_width );
				_clue->append( c );
				vc = new HTMLClueV( 0, 0, 30, 0 ); // fixed width spacer
				c->append( vc );
				vc = new HTMLClueV( 0, 0, _max_width-30 );
				c->append( vc );
				flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth() );
				vc->append( flow );
				str = parseBody( vc, enddd );
			}
			else if ( strncasecmp( str, "<dl>", 4 ) == 0 )
			{
			    HTMLClueH *c = new HTMLClueH( 0, 0, _max_width );
			    _clue->append( c );
			    vc = new HTMLClueV( 0, 0, 30, 0 ); // fixed width spacer
			    c->append( vc );
			    vc = new HTMLClueV( 0, 0, _max_width-30 );
			    c->append( vc );
			    str = parseGlossary( vc, _max_width-30 ); 
			}
		}

		if ( !str || strncasecmp( str, "</dl>", 5 ) == 0 )
			break;
	}
    }

    flow = NULL;
    
    return str;
}

const char* KHTMLWidget::parseTable( HTMLClueV *_clue, int _max_width,
	const char *attr )
{
	static const char *endth[] = { "</th>", NULL };
	static const char *endtd[] = { "</td>", NULL };    
	const char* str = NULL;
	bool firstRow = TRUE;
	int padding = 1;
	int spacing = 2;
	int width = 0;
	int percent = 0;
	int border = 0;
	HTMLClue::VAlign rowvalign = HTMLClue::VNone;
	HTMLClue::HAlign rowhalign = HTMLClue::HNone;

	QString s = attr;
	StringTokenizer st( s, " >" );
	while ( st.hasMoreTokens() )
	{
		const char* token = st.nextToken();
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
			else
				width = atoi( token + 6 );
		}
	}

	HTMLTable *table = new HTMLTable( 0, 0, _max_width, width, percent,
		 padding, spacing, border );
	_clue->append( table );

    while ( ht->hasMoreTokens() )
	{
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( str[0] == TAG_ESCAPE )
	{
		str++;

		if ( strncasecmp( str, "<tr", 3 ) == 0 )
		{
			if ( !firstRow )
				table->endRow();
			table->startRow();
			firstRow = FALSE;
			rowvalign = HTMLClue::VNone;
			rowhalign = HTMLClue::HNone;

			QString s = str + 4;
			StringTokenizer st( s, " >" );
			while ( st.hasMoreTokens() )
			{
				const char* token = st.nextToken();
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
					if ( strncasecmp( token+6, "left", 4 ) == 0)
						rowhalign = HTMLClue::Left;
					else if ( strncasecmp( token+6, "right", 5 ) == 0)
						rowhalign = HTMLClue::Right;
					else
						rowhalign = HTMLClue::HCenter;
				}
			}
		}
		else if ( strncasecmp( str, "<th", 3 ) == 0 )
		{
			// <tr> may not be specified for the first row
			if ( firstRow )
			{
				table->startRow();
				firstRow = FALSE;
			}

			int rowSpan = 1, colSpan = 1;
			int width = 0;
			int percent = 0;
			QColor bgcolor;
			HTMLClue::VAlign valign = (rowvalign == HTMLClue::VNone ?
							HTMLClue::VCenter : rowvalign);
			HTMLClue::HAlign oldhalign = divAlign;
			divAlign = (rowhalign == HTMLClue::HNone ? HTMLClue::HCenter :
					rowhalign);

			QString s = str + 4;
			StringTokenizer st( s, " >" );
			while ( st.hasMoreTokens() )
			{
				const char* token = st.nextToken();
				if ( strncasecmp( token, "rowspan=", 8 ) == 0)
					rowSpan = atoi( token+8 );
				else if ( strncasecmp( token, "colspan=", 8 ) == 0)
					colSpan = atoi( token+8 );
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
					if ( strncasecmp( token+6, "left", 4 ) == 0)
						divAlign = HTMLClue::Left;
					else if ( strncasecmp( token+6, "right", 5 ) == 0)
						divAlign = HTMLClue::Right;
					else
						divAlign = HTMLClue::HCenter;
				}
				else if ( strncasecmp( token, "width=", 6 ) == 0 )
				{
					if ( strchr( token + 6, '%' ) )
						percent = atoi( token + 6 );
					else
					{
						width = atoi( token + 6 );
						percent = 0;
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
			}

			HTMLTableCell *cell = new HTMLTableCell(0, 0, _clue->getMaxWidth(),
				width, percent, rowSpan, colSpan, padding);
			if ( bgcolor.isValid() )
				cell->setBGColor( bgcolor );
			cell->setVAlign( valign );
			table->addCell( cell );
			flow = 0;
			weight = QFont::Bold;
			selectFont( 0 );
			str = parseBody( cell, endth );
			popFont();
			divAlign = oldhalign;
		}
		else if ( strncasecmp( str, "<td", 3 ) == 0 )
		{
			// <tr> may not be specified for the first row
			if ( firstRow )
			{
				table->startRow();
				firstRow = FALSE;
			}

			int rowSpan = 1, colSpan = 1;
			int width = 0;
			int percent = 0;
			QColor bgcolor;
			HTMLClue::VAlign valign = (rowvalign == HTMLClue::VNone ?
							HTMLClue::VCenter : rowvalign);
			HTMLClue::HAlign oldhalign = divAlign;
			divAlign = (rowhalign == HTMLClue::HNone ? divAlign : rowhalign);

			QString s = str + 4;
			StringTokenizer st( s, " >" );
			while ( st.hasMoreTokens() )
			{
				const char* token = st.nextToken();
				if ( strncasecmp( token, "rowspan=", 8 ) == 0)
					rowSpan = atoi( token+8 );
				else if ( strncasecmp( token, "colspan=", 8 ) == 0)
					colSpan = atoi( token+8 );
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
					if ( strncasecmp( token+6, "center", 6 ) == 0)
						divAlign = HTMLClue::HCenter;
					else if ( strncasecmp( token+6, "right", 5 ) == 0)
						divAlign = HTMLClue::Right;
					else
						divAlign = HTMLClue::Left;
				}
				else if ( strncasecmp( token, "width=", 6 ) == 0 )
				{
					if ( strchr( token + 6, '%' ) )
						percent = atoi( token + 6 );
					else
					{
						width = atoi( token + 6 );
						percent = 0;
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
			}

			HTMLTableCell *cell = new HTMLTableCell(0, 0, _clue->getMaxWidth(),
				width, percent, rowSpan, colSpan, padding );
			if ( bgcolor.isValid() )
				cell->setBGColor( bgcolor );
			cell->setVAlign( valign );
			table->addCell( cell );
			flow = 0;
			str = parseBody( cell, endtd );
			divAlign = oldhalign;
		}
		else if ( strncasecmp( str, "</table>", 8 ) == 0 )
		{
			if ( !firstRow )
				table->endRow();
			table->endTable();
			break;
		}
	}
    }

	flow = NULL;

	return str;
}

const char *KHTMLWidget::parseInput( const char *attr )
{
	enum Type { CheckBox, Hidden, Radio, Reset, Submit, Text, Image };
	const char *p;
	HTMLInput *element = NULL;
	Type type = Text;
	QString name = "";
	QString value = "";
	bool checked = false;
	int size = 20;

	QString s = attr;
	StringTokenizer st( s, " >" );
	while ( st.hasMoreTokens() )
	{
		const char* token = st.nextToken();
		if ( strncasecmp( token, "type=", 5 ) == 0 )
		{
			p = token + 5;
			if ( *p == '"' ) p++;
			if ( strncasecmp( p, "checkbox", 8 ) == 0 )
				type = CheckBox;
			else if ( strncasecmp( p, "hidden", 6 ) == 0 )
				type = Hidden;
			else if ( strncasecmp( p, "radio", 5 ) == 0 )
				type = Radio;
			else if ( strncasecmp( p, "reset", 5 ) == 0 )
				type = Reset;
			else if ( strncasecmp( p, "submit", 5 ) == 0 )
				type = Submit;
			else if ( strncasecmp( p, "text", 5 ) == 0 )
				type = Text;
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
		else if ( strncasecmp( token, "checked", 7 ) == 0 )
		{
			checked = true;
		}
	}

	switch ( type )
	{
		case CheckBox:
			element = new HTMLCheckBox( this, name, value, checked );
			break;

		case Hidden:
			element = new HTMLHidden( name, value );
			break;

		case Radio:
			element = new HTMLRadio( this, name, value, checked );
			connect( element, SIGNAL(radioSelected(const char *, const char *)),
				form, SLOT(slotRadioSelected(const char *, const char *)));
			connect( form, SIGNAL( radioSelected(const char*, const char *) ),
				element, SLOT(slotRadioSelected(const char *, const char *)));
			break;

		case Reset:
			element = new HTMLReset( this, value );
			connect( element, SIGNAL( resetForm() ),
				form, SLOT( slotReset() ) );
			break;

		case Submit:
			element = new HTMLSubmit( this, value );
			connect( element, SIGNAL( submitForm() ),
				form, SLOT( slotSubmit() ) );
			break;

		case Text:
			element = new HTMLTextInput( this, name, value, size );
			connect( element, SIGNAL( submitForm() ),
				form, SLOT( slotSubmit() ) );
			break;

		case Image:
			break;
	}

	if ( element )
	{
		form->addElement( element );
		flow->append( element );
	}

	return NULL;
}

void KHTMLWidget::slotScrollVert( int _val )
{
	int newPainter = FALSE;

    if ( clue == 0L )
	return;
    
    bitBlt( this, 0, ( y_offset - _val ), this );

	if ( painter == NULL )
	{
		painter = new QPainter;
		painter->begin( this );
		newPainter = TRUE;
	}

    if ( _val > y_offset)
    {
	if ( bgPixmap.isNull() )
	{
		painter->eraseRect( 0, height() - ( _val - y_offset ),
			width(), ( _val - y_offset ) );
	}
	else
	{
		drawBackground( x_offset, _val, 0, height() - ( _val - y_offset ) ,
			width(), ( _val - y_offset ) );
	}
	int tx = -x_offset + LEFT_BORDER;
	int ty = -_val;
	
	clue->print( painter, 0 + x_offset, height() - ( _val-y_offset ) + _val,
		 width(), _val - y_offset, tx, ty );
    }
    else
    {
	if ( bgPixmap.isNull() )
		painter->eraseRect( 0, 0, width(), ( y_offset - _val) );
	else
		drawBackground( x_offset, _val, 0, 0, width(), ( y_offset - _val) );
	int tx = -x_offset + LEFT_BORDER;
	int ty = -_val;

	clue->print( painter, 0 + x_offset, 0 + _val, width(), ( y_offset - _val), tx, ty );
    }

    y_offset = _val;

	if ( formList.count() > 0 )
		positionFormElements();
    
	if ( newPainter )
	{
    	painter->end();
		delete painter;
		painter = NULL;
	}
}

void KHTMLWidget::slotScrollHorz( int _val )
{
	int newPainter = FALSE;

    if ( clue == 0L )
	return;
    
    bitBlt( this, x_offset - _val, 0, this );

	if ( painter == NULL )
	{
		painter = new QPainter;
		painter->begin( this );
		newPainter = TRUE;
	}

	if ( _val > x_offset)
	{
		if ( bgPixmap.isNull() )
		{
			painter->eraseRect( width() - (_val-x_offset), 0,
				width(), height() );
		}
		else
		{
			drawBackground( _val, y_offset,  width() - (_val-x_offset), 0,
				width(), height() );
		}
		clue->print( painter, width() - (_val-x_offset) + _val, y_offset,
			_val-x_offset, height(), -_val+LEFT_BORDER, -y_offset );
    }
	else
	{
		if ( bgPixmap.isNull() )
			painter->eraseRect( 0, 0, x_offset - _val, height() );
		else
			drawBackground( _val, y_offset, 0, 0, x_offset - _val, height() );
		clue->print( painter, _val, y_offset,
			x_offset-_val, height(), -_val+LEFT_BORDER, -y_offset );
	}

    x_offset = _val;

	if ( formList.count() > 0 )
		positionFormElements();

	if ( newPainter )
	{
    	painter->end();
		delete painter;
		painter = NULL;
	}
}

void KHTMLWidget::positionFormElements()
{
	HTMLForm *f;

	for ( f = formList.first(); f != NULL; f = formList.next() )
	{
		f->position( x_offset - LEFT_BORDER, y_offset, width(), height() );
	}
}

void KHTMLWidget::drawBackground( int _xval, int _yval, int _x, int _y,
	int _w, int _h )
{
	if ( bgPixmap.isNull() )
		return;

	int pw = bgPixmap.width();
	int ph = bgPixmap.height();

	int xOrigin = _x/pw*pw - _xval%pw;
	int yOrigin = _y/ph*ph - _yval%ph;

	painter->setClipRect( _x, _y, _w, _h );
	painter->setClipping( TRUE );

	for ( int yp = yOrigin; yp < _y + _h; yp += ph )
	{
		for ( int xp = xOrigin; xp < _x + _w; xp += pw )
		{
			painter->drawPixmap( xp, yp, bgPixmap );
		}
	}

	painter->setClipping( FALSE );
}

bool KHTMLWidget::gotoAnchor( const char *_name )
{
    if ( clue == 0L )
	return FALSE;

	printf( "Finding anchor: \"%s\"\n", _name );
    
    QPoint p( 0, 0 );
    
    HTMLAnchor *anchor = clue->findAnchor( _name, &p );
    if ( anchor == 0L )
	return FALSE;

	printf( "Scrolling to anchor: %d, %d\n", p.x(), p.y() );
    
    emit scrollVert( p.y() );

	// MRJ - I've turned this off for now.  It doesn't produce very nice
	// output.
//    emit scrollHorz( p.x() );

    return TRUE;
}

void KHTMLWidget::select( QPainter *_painter, bool _select )
{
    int tx = x_offset + LEFT_BORDER;
    int ty = -y_offset;

    if ( clue == 0L )
	return;
    
    clue->select( _painter, _select, tx, ty );
}

void KHTMLWidget::selectByURL( QPainter *_painter, const char *_url, bool _select )
{
    int tx = x_offset + LEFT_BORDER;
    int ty = -y_offset;

    if ( clue == 0L )
	return;
    
    clue->selectByURL( _painter, _url, _select, tx, ty );
}

void KHTMLWidget::select( QPainter *_painter, QRegExp& _pattern, bool _select )
{
    int tx = x_offset + LEFT_BORDER;
    int ty = -y_offset;
	bool newPainter = FALSE;

    if ( clue == 0L )
	return;

    if ( _painter == 0L )
    {
	if ( painter == NULL )
	{
		painter = new QPainter();
		painter->begin( this );
		newPainter = TRUE;
	}
    
	clue->select( painter, _pattern, _select, tx, ty );

	if ( newPainter )
	{
		painter->end();
		delete painter;
		painter = NULL;
	}
    }
    else
	clue->select( _painter, _pattern, _select, tx, ty );
}

int KHTMLWidget::docWidth()
{
	if ( clue )
    	return clue->getWidth() + LEFT_BORDER + RIGHT_BORDER;
	else
		return LEFT_BORDER + RIGHT_BORDER;
}

int KHTMLWidget::docHeight()
{
	if ( clue )
    	return clue->getHeight();

	return 0;
}

KHTMLWidget::~KHTMLWidget()
{
	if ( painter )
	{
		painter->end();
		delete painter;
	}
	if (clue)
		delete clue;
    if (ht)
		delete ht;
	if (text)
		delete [] text;
	font_stack.clear();
}

#include "html.moc"

