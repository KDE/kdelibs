/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "khtml.moc"

#include <kio_job.h>
#include <kio_cache.h>
#include <kio_error.h>

#include <assert.h>

#include <kurl.h>
#include <kapp.h>
#include <kdebug.h>
#include <khtml.h>
#include <khtmldata.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kimgio.h>

#include "khtmldecoder.h"
#include "html_documentimpl.h"

#include "khtmlio.h"
#include "html_elementimpl.h"

#include "kjs.h"

#define SCROLLBARWIDTH 16

QList<KHTMLWidget> *KHTMLWidget::lstViews = 0L;

using namespace DOM;

KHTMLWidget::KHTMLWidget( QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    init();

    // initialize QScrollview
    enableClipper(true);
    viewport()->setMouseTracking(true);

    kimgioRegister();
}

KHTMLWidget::KHTMLWidget( QWidget *parent, KHTMLWidget *_parent_browser, const char *name )
    : QScrollView( parent, name)
{
    init();
    _parent     = _parent_browser;
    m_strFrameName       = name;

  // initialize QScrollview
    enableClipper(true);
    viewport()->setMouseTracking(true);

    kimgioRegister();
}

KHTMLWidget::~KHTMLWidget()
{
  slotStop();
  lstViews->removeRef( this );

  clear();

  if(cache) delete cache;
  if(defaultSettings) delete defaultSettings;
}

void KHTMLWidget::init()
{
    _parent     = 0;
  m_strFrameName       = QString::null;

  m_bStartedRubberBand = false;
  m_pRubberBandPainter = 0L;
  m_bRubberBandVisible = false;
  m_jobId              = 0;
  m_bParsing           = false;
  m_bComplete          = true;
  m_bReload            = false;
  m_bEnableImages      = true;
  _javaEnabled = false;
  _jScriptEnabled = false;
  jscript = 0;

    if ( lstViews == 0L )
	lstViews = new QList<KHTMLWidget>;
    lstViews->setAutoDelete( FALSE );
    lstViews->append( this );

  m_lstURLRequestJobs.setAutoDelete( true );
  m_lstChildren.setAutoDelete( true );

  setFocusPolicy( QWidget::StrongFocus );

  document = 0;
  decoder = 0;
  cache = new KHTMLCache(this);
  defaultSettings = new HTMLSettings;
  settings = 0;
  _width = width()- SCROLLBARWIDTH - 10;
}

void KHTMLWidget::clear()
{
    if(document)
    {		
	document->detach();
	document->deref();
    }
    document = 0;
    if(decoder) delete decoder;
    decoder = 0;
    delete jscript;
    jscript = 0;
    if ( settings ) delete settings;
    settings = 0;

    m_lstChildren.clear();

    if ( bIsTextSelected )
    {
	bIsTextSelected = false;
	emit textSelected( false );
    }

    pressed = false;
}

void KHTMLWidget::setFollowsLinks( bool follow )
{
    _followLinks = follow;
}

bool KHTMLWidget::followsLinks()
{
    return _followLinks;
}

void KHTMLWidget::enableImages( bool enable )
{
    m_bEnableImages = enable;
}

bool KHTMLWidget::imagesEnabled() const
{
    return m_bEnableImages;
}

void KHTMLWidget::enableJava( bool enable )
{
    _javaEnabled = enable;
}

bool KHTMLWidget::javaEnabled() const
{
    return _javaEnabled;
}

void KHTMLWidget::enableJScript( bool enable )
{
    _jScriptEnabled = enable;
}

bool KHTMLWidget::jScriptEnabled() const
{
    return _jScriptEnabled;
}

void KHTMLWidget::executeScript(const QString &c)
{
    if(!_jScriptEnabled) return;
    if(!jscript) jscript = new KJSWorld(this);
    jscript->evaluate((KJS::UnicodeChar*)c.unicode(), c.length());
}


KHTMLWidget* KHTMLWidget::topView()
{
  KHTMLWidget *v = this;

  while( v->parentView() )
    v = v->parentView();

  return v;
}

KHTMLWidget* KHTMLWidget::findView( const QString &_name )
{
    KHTMLWidget *v;

    if ( _name == "_top" )
    {
	v = this;

	while ( v->parentView() )
	    v = v->parentView();
	
	return v;
    }
    else if ( _name == "_self"  )
    {
	return this;
    }
    else if ( _name == "_parent" )
    {
	if ( parentView() )
	    return parentView();
    }
    else if ( _name == "_blank" )
    {
	return 0;
    }

    for ( v = lstViews->first(); v != 0; v = lstViews->next() )
    {
	if ( v->frameName() )
	{
	    // debugT("Comparing '%s' '%s'\n", _name, v->getFrameName() );
	    if ( v->frameName() == _name  )
		return v;
	}
    }

    return 0;
}

KHTMLWidget* KHTMLWidget::createFrame( QWidget *_parent, const char *_name )
{
    return new KHTMLWidget( _parent, this, _name );
}

void KHTMLWidget::begin( const QString &_url, int _x_offset, int _y_offset )
{
    debug("KHTMLWidget::begin(....)");

    clear();

    //emit scrollHorz( _x_offset );
    //emit scrollVert( _y_offset );

    // ###
    //stopParser();
    m_strURL = _url;

    if ( !_url.isEmpty() )
    {
    
	KURL::List lst = KURL::split( m_strURL );
	assert ( !lst.isEmpty() );
	QString baseurl = (*lst.begin()).url();

        // Set a default title
        KURL title(baseurl);
        title.setRef(QString::null);
        title.setQuery(QString::null);
        emit setTitle( title.url().data() );
    }
    else
    {
        emit setTitle( "* Unknown *" );
    }

    document = new HTMLDocumentImpl(this, cache);
    document->ref();
    document->open();
    // clear widget
    resizeContents(0, 0);
    setBackgroundMode(PaletteBackground);
    viewport()->repaint(true);

    // ###
    //emit documentStarted();

    settings = new HTMLSettings( *defaultSettings);

    m_bParsing = true;
}

void KHTMLWidget::slotStop()
{
  if ( !m_strWorkingURL.isEmpty() )
  {
    m_strWorkingURL = "";

    if ( m_bParsing )
    {
      end();
      m_bParsing = false;
    }
  }

  m_bComplete = true;
  m_jobId = 0;

  // ### cancel all file requests

  m_lstURLRequestJobs.clear();
  m_lstPendingURLRequests.clear();

  emit canceled();
  if ( _parent )
    _parent->childCompleted( this );
}

void KHTMLWidget::slotReload()
{
  // Reloads everything including the framesets
  if ( !m_strURL.isEmpty() )
    openURL( m_strURL, true );
}

void KHTMLWidget::slotReloadFrames()
{
  // Does not reload framesets but all frames
  if ( isFrame() )
  {
    Child *c;
    for ( c = m_lstChildren.first(); c != 0L; c = m_lstChildren.next() )
    {
      c->m_bReady = false;
      c->m_pBrowser->slotReloadFrames();
    }
  }
  else
    slotReload();
}

void KHTMLWidget::openURL( const QString &_url, bool _reload, int _xoffset, int _yoffset, const char* /*_post_data*/ )
{
  // Check URL
  if ( KURL::split( _url ).isEmpty() )
  {
    emit error( KIO::ERR_MALFORMED_URL, _url );
    return;
  }

  if ( m_jobId )
  {
    KIOJob* job = KIOJob::find( m_jobId );
    if ( job )
      job->kill();
    m_jobId = 0;
  }

  if ( m_bParsing )
  {
    end();
    m_bParsing = false;
  }

  slotStop();

  m_strWorkingURL = _url;
  m_iNextYOffset = _yoffset;
  m_iNextXOffset = _xoffset;

  m_bReload = _reload;

  KIOCachedJob* job = new KIOCachedJob;

  job->setGUImode( KIOJob::NONE );

  // ###
  connect( job, SIGNAL( sigFinished( int ) ), this, SLOT( slotFinished( int ) ) );
  //connect( job, SIGNAL( sigRedirection( int, const char* ) ), this, SLOT( slotRedirection( int, const char* ) ) );
  connect( job, SIGNAL( sigData( int, const char*, int ) ), this, SLOT( slotData( int, const char*, int ) ) );
  // connect( job, SIGNAL( sigError( int, int, const char* ) ), this, SLOT( slotError( int, int, const char* ) ) );

  m_jobId = job->id();
  // TODO
  /* if ( _post_data )
    job->post( _url, _post_data );
  else */
  job->get( _url, _reload );

  m_bComplete = false;

  emit started( m_strWorkingURL );
}

void KHTMLWidget::slotFinished( int /*_id*/ )
{
  kdebug(0,1202,"SLOT_FINISHED 1");

  kdebug(0,1202,"SLOT_FINISHED 2");
  m_strWorkingURL = "";

  if ( m_bParsing )
  {
    kdebug(0,1202,"SLOT_FINISHED 3");
    end();
  }

  m_jobId = 0;
  m_bParsing = false;
}

void KHTMLWidget::slotRedirection( int /*_id*/, const QString &_url )
{
  // We get this only !before! we receive the first data
  assert( !m_strWorkingURL.isEmpty() );
  m_strWorkingURL = _url;
}

void KHTMLWidget::slotData( int /*_id*/, const char *_p, int _len )
{
  kdebug(0,1202,"SLOT_DATA %d", _len);

  /** DEBUG **/
  assert( (int)strlen(_p ) <= _len );
  /** End DEBUG **/

  // The first data ?
  if ( !m_strWorkingURL.isEmpty() )
  {
    kdebug(0,1202,"BEGIN...");
    m_lstChildren.clear();
    m_bParsing = true;
    begin( m_strWorkingURL, m_iNextXOffset, m_iNextYOffset );
    m_strWorkingURL = "";
  }

  write( _p );
}

void KHTMLWidget::data( HTMLURLRequestJob *job, const char *_data, int _len, bool _eof )
{

    printf("HTMLWidget::data()\n");
    HTMLURLRequest *p = job->m_req;
    if ( !p )
    {	
	printf("no such request!!!!!\n");
	return;
    }

    if ( !p->m_buffer.isOpen() )
	p->m_buffer.open( IO_WriteOnly );
    p->m_buffer.writeBlock( _data, _len );

  HTMLURLRequester* o;
  for( o = p->m_lstClients.first(); o != 0L; o = p->m_lstClients.next() )
    o->fileLoaded( p->m_strURL, p->m_buffer, _eof );

  if ( _eof )
    {
	p->m_buffer.close();
	m_lstURLRequestJobs.remove( job );
    }
  else
    return;

  /// ### FIXME: parser has to finish too...
  if ( m_lstURLRequestJobs.count() == 0 )
  {
      //emit documentDone();
      cache->flush();
  }
}


void KHTMLWidget::slotError( int /*_id*/, int _err, const QString &_text )
{
  if ( _err == KIO::ERR_WARNING )
    return; //let's ignore warnings for now

  kdebug(0,1202,"+++++++++++++ ERROR %d, %s ", _err, _text.data());

  slotStop();

  emit error( _err, _text );

  // !!!!!! HACK !!!!!!!!!!
  kioErrorDialog( _err, _text.data() );

  kdebug(0,1202,"+++++++++++ RETURN from error ++++++++++");

  // emit canceled();
}

//
// File handling
//
///////////////////////////////////////////////////
// ### remove bool ???
void KHTMLWidget::requestFile( HTMLURLRequester *_obj, const QString &_url,
			       bool )
{
  printf("==== REQUEST %s  ====\n", _url.latin1() );

  HTMLURLRequest *r = m_lstPendingURLRequests[ _url ];
  if ( r )
  {
    r->m_lstClients.append( _obj );
    return;
  }

  r = new HTMLURLRequest( _url, _obj );
  m_lstPendingURLRequests.insert( _url, r );

    servePendingURLRequests();
}

void KHTMLWidget::cancelRequestFile( HTMLURLRequester *_obj )
{
  QDictIterator<HTMLURLRequest> it( m_lstPendingURLRequests );
  for( ; it.current(); ++it )
  {
    it.current()->m_lstClients.remove( _obj );
    if ( it.current()->m_lstClients.count() == 0 )
    {
	QString tmp = completeURL( it.current()->m_strURL );
	m_lstPendingURLRequests.remove( it.current()->m_strURL );
    }
  }
}

void KHTMLWidget::cancelAllRequests()
{
  m_lstPendingURLRequests.clear();
}

void KHTMLWidget::servePendingURLRequests()
{
  if ( m_lstURLRequestJobs.count() == MAX_REQUEST_JOBS )
    return;
  if ( m_lstPendingURLRequests.count() == 0 )
      return;

  printf("starting URLRequestJob\n");
  QDictIterator<HTMLURLRequest> it( m_lstPendingURLRequests );
  HTMLURLRequest *req = it.current();
  HTMLURLRequestJob* j = new HTMLURLRequestJob( this, req, m_bReload );
  m_lstURLRequestJobs.append( j );
  m_lstPendingURLRequests.remove(req->m_strURL);
}

void KHTMLWidget::urlRequestFinished( HTMLURLRequestJob* _request )
{
    m_lstURLRequestJobs.remove( _request );
    servePendingURLRequests();
}

void KHTMLWidget::urlSelected( const QString &_url, int _button, const QString &_target )
{
//   if ( !m_bComplete )
//     slotStop();

  // Security
  KURL u1( _url );
  KURL u2( m_strURL );
  if ( strcmp( u1.protocol(), "cgi" ) == 0 &&
       strcmp( u2.protocol(), "file" ) != 0 && strcmp( u2.protocol(), "cgi" ) != 0 )
  {
    KMessageBox::error( 0,
			i18n( "This page is untrusted\nbut it contains a link to your local file system."),
			i18n( "Security Alert" ));
    return;
  }

  if ( _url.isNull() )
    return;

  QString url = completeURL( _url );

  if(!_followLinks)
  {
      emit urlClicked( url, _target, _button );
      return;
  }

  if ( !_target.isNull() && !_target.isEmpty() && _button == LeftButton )
  {
    if ( strcmp( _target, "_parent" ) == 0 )
    {
      KHTMLWidget *v = parentView();
      if ( !v )
	v = this;
      v->openURL( url );
      emit urlClicked( url, _target, _button );
      return;
    }
    else if ( strcmp( _target, "_top" ) == 0 )
    {
      kdebug(0,1202,"OPENING top %s", url.ascii());
      topView()->openURL( url );
      emit urlClicked( url, _target, _button );
      kdebug(0,1202,"OPENED top");
      return;
    }
    else if ( strcmp( _target, "_blank" ) == 0 )
    {
      emit newWindow( url );
      return;
    }
    else if ( strcmp( _target, "_self" ) == 0 )
    {
      openURL( url );
      emit urlClicked( url, _target, _button );
      return;
    }

    KHTMLWidget *v = topView()->findChildView( _target );
    if ( !v )
      v = findView( _target );
    if ( v )
    {
      v->openURL( url );
      emit urlClicked( url, _target, _button );
      return;
    }
    else
    {
      emit newWindow( url );
      return;
    }
  }
  else if ( _button == MidButton )
  {
      emit newWindow( url );
      return;
  }
  else if ( _button == LeftButton )
  {
    // Test whether both URLs differ in the Reference only.
    KURL u1( url );
    if ( u1.isMalformed() )
    {
      kioErrorDialog( KIO::ERR_MALFORMED_URL, url );
      return;
    }

    // if only the reference differs, then we just go to the new anchor
    if ( urlcmp( url, m_strURL, TRUE, TRUE ) )
    {
      QString anchor = u1.htmlRef();
      kdebug(0,1202,"Going to anchor %s", anchor.ascii());
      // ###
      gotoAnchor( anchor );
      emit urlClicked( url, _target, _button );
      return;
    }

    openURL( url );	
  }
}

void KHTMLWidget::slotFormSubmitted( const QString &_method, const QString &_url,
				     const char *_data, const QString &_target )
{
    // ### add target!!!

    QString url = completeURL( _url );

  KURL u( _url );
  if ( u.isMalformed() )
  {
    emit error( KIO::ERR_MALFORMED_URL, url );
    return;
  }

  if ( strcasecmp( _method, "GET" ) == 0 )
  {
    // GET
    QString query = u.query();
    if ( !query.isEmpty() )
    {
      u.setQuery( query + "&" + _data );
    }
    else
    {
      u.setQuery( _data );
    }

    openURL( u.url() );
  }
  else
  {
    // POST
    openURL( url, false, 0, 0, _data );
  }
}

void KHTMLWidget::setDefaultTextColors( const QColor& _textc, const QColor& _linkc, const QColor& _vlinkc )
{
    defaultSettings->fontBaseColor = _textc;
    defaultSettings->linkColor = _linkc;
    defaultSettings->vLinkColor = _vlinkc;

    Child *c;
    for ( c = m_lstChildren.first(); c != 0L; c = m_lstChildren.next() )
	c->m_pBrowser->setDefaultTextColors( _textc, _linkc, _vlinkc );
}

void KHTMLWidget::setDefaultBGColor( const QColor& bgcolor )
{
    defaultSettings->bgColor = bgcolor;

    Child *c;
    for ( c = m_lstChildren.first(); c != 0L; c = m_lstChildren.next() )
	c->m_pBrowser->setDefaultBGColor( bgcolor );
}

QString KHTMLWidget::completeURL( const QString &_url )
{
  KURL orig( m_strURL );

  KURL u( orig, _url );
  return u.url();
}

KHTMLWidget* KHTMLWidget::findChildView( const QString &_target )
{
  QListIterator<Child> it( m_lstChildren );
  for( ; it.current(); ++it )
  {
    if ( it.current()->m_pBrowser->frameName() &&
	 strcmp( it.current()->m_pBrowser->frameName(), _target ) == 0 )
      return it.current()->m_pBrowser;
  }

  QListIterator<Child> it2( m_lstChildren );
  for( ; it2.current(); ++it2 )
  {
    KHTMLWidget *b = it2.current()->m_pBrowser->findChildView( _target );
    if ( b )
      return b;
  }

  return 0L;
}

void KHTMLWidget::childCompleted( KHTMLWidget *_browser )
{
  /** DEBUG **/
  kdebug(0,1202,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  kdebug(0,1202,"--------------- ChildFinished %p ----------------------",this);
  /** End DEBUG **/

  QListIterator<Child> it( m_lstChildren );
  for( ; it.current(); ++it )
  {
    if ( it.current()->m_pBrowser == _browser )
      it.current()->m_bReady = true;
  }

  // ###
  //if ( m_bComplete )
  // slotDocumentFinished( this );

  kdebug(0,1202,"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}

void KHTMLWidget::write( const char *_str)
{
    if(!decoder) decoder = new KHTMLDecoder();

    if ( _str == 0 )
	return;

    QString decoded = decoder->decode( _str );
    document->write( decoded );
}

void KHTMLWidget::write( const QString &_str)
{
    if ( _str == QString::null )
	return;

    document->write( _str );

}

void KHTMLWidget::end()
{
  /** DEBUG **/
  if ( !_parent )
  {
    kdebug(0,1202,"--------------------------------------------------");
    kdebug(0,1202,"--------------- DocFinished %p ----------------------",this);
    kdebug(0,1202,"--------------------------------------------------");
  }
  else
    kdebug(0,1202,"########### SUB-DocFinished %p ##############",this);
  /** End DEBUG **/

  m_bComplete = true;

  // Are all children complete now ?
  QListIterator<Child> it2( m_lstChildren );
  for( ; it2.current(); ++it2 )
    if ( !it2.current()->m_bReady )
      return;

  emit completed();
  if ( _parent )
    _parent->childCompleted( this );
}

void KHTMLWidget::resizeEvent ( QResizeEvent * event )
{
    printf("resizeEvent\n");
    QScrollView::resizeEvent(event);
    layout();
    //emit resized( event->size() );
}

void KHTMLWidget::drawContents ( QPainter * p, int clipx,
				 int clipy, int clipw, int cliph )

{
    if(!document) return;
    NodeImpl *body = document->body();
    if(!body) return;
    // ### fix this for frames...
    body->print(p, clipx, clipy, clipw, cliph, 0, 0);
}

void KHTMLWidget::layout()
{
    //### take care of frmaes (hide scrollbars,...)

    if(!document) return;
    NodeImpl *body = document->body();
    if(!body) return;

    int w = width() - SCROLLBARWIDTH - 10;
    if(w < _width-5 || w > _width + 10)
    {
	printf("layouting document\n");

	_width = w;

	document->setAvailableWidth(_width);
	document->layout(true);
	resizeContents(document->getWidth(), document->getHeight());
	viewport()->repaint(true);
    }
}

void KHTMLWidget::paintElement( NodeImpl *e, bool recursive )
{
    int xPos, yPos;
    e->getAbsolutePosition(xPos, yPos);
    int yOff = contentsY();
    if(yOff > yPos+e->getDescent() ||
       yOff+visibleHeight() < yPos-e->getAscent())
	return;

    QWidget *vp = viewport();
    QPainter p(vp);
    int xOff = contentsX()+vp->x();
    yOff += vp->y();
    p.translate( -xOff, -yOff );
    if(recursive)
	e->print(&p, xOff, yOff, vp->width(), vp->height(),
		 xPos - e->getXPos(), yPos - e->getYPos());
    else
	e->printObject( &p , xOff, yOff, vp->width(), vp->height(),
			xPos , yPos );
}

//
// Event Handling
//
/////////////////

void KHTMLWidget::viewportMousePressEvent( QMouseEvent *_mouse )
{
    if(!document) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmousePressEvent: x=%d, y=%d\n", xm, ym);


    // Make this frame the active one
    if ( _isFrame && !_isSelected )
    {
	// find top level frame
	KHTMLWidget *w = this;
	while(w->isFrame())
	    w = static_cast<KHTMLWidget *>(w->parentWidget());
	// ####
	//w->setSelected(this);
    }

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MousePress, 0, 0, url );

    if(url != 0)
    {
	printf("mouseEvent: overURL %s\n", url.string().latin1());
	m_strSelectedURL = url.string();
    }
    else
	m_strSelectedURL = QString::null;

#if 0
    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
    {
    	pressed = TRUE;
	// deselect all currently selected text
	if ( bIsTextSelected )
	{
	    bIsTextSelected = false;
	    selectText( 0, 0, 0, 0 );	// deselect all text
	    emit textSelected( false );
	}
	// start point for text selection
	selectPt1.setX( _mouse->pos().x() + contentsX());
	selectPt1.setY( _mouse->pos().y() + contentsY());
    }
    press_x = _mouse->pos().x();
    press_y = _mouse->pos().y();
#endif

#if 0
    HTMLObject *obj;

    obj = clue->checkPoint( _mouse->pos().x() + contentsX() - leftBorder,
	    _mouse->pos().y() + contentsY() - topBorder );

    m_strSelectedURL = "";
    pressedTarget = "";

    if ( obj != 0)
    {
	if ( obj->getURL().length() )
	{
	    // Save data. Perhaps the user wants to start a drag.
	    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
	    {
		m_strSelectedURL = obj->getURL();
		pressedTarget = obj->getTarget();
	    }
	
	    // Does the parent want to process the event now ?
	    if ( htmlView )	
	    {
		if ( htmlView->mousePressedHook( obj->getURL(), obj->getTarget(),
						 _mouse, obj->isSelected() ) )
		    return;
	    }
	
	    if ( _mouse->button() == RightButton )
	    {
		emit popupMenu(obj->getURL(),mapToGlobal( _mouse->pos() ));
		return;
	    }
	    return;
	}
    }

    if ( htmlView )	
      if ( htmlView->mousePressedHook( 0, 0L, _mouse, FALSE ) )
	return;
    if ( _mouse->button() == RightButton )
	emit popupMenu( 0, mapToGlobal( _mouse->pos() ) );
#endif
}

void KHTMLWidget::viewportMouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if(!document) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmouseDblClickEvent: x=%d, y=%d\n", xm, ym);

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick, 0, 0, url );

    // ###
    //if ( url.length() )
    //emit doubleClick( url.string(), _mouse->button() );
}

void KHTMLWidget::viewportMouseMoveEvent( QMouseEvent * _mouse )
{
    if(!document) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseMove, 0, 0, url );

    if ( !pressed && url.length() )
    {
	QString surl = url.string();
	if ( overURL.isEmpty() )
	{
	    setCursor( linkCursor );
	    overURL = surl;
	    emit onURL( overURL );
	}
	else if ( overURL != surl )
	{
	    emit onURL( surl );
	    overURL = surl;
	}
	return;
    }
    else if( overURL.length() && !url.length() )
    {
	setCursor( arrowCursor );
	emit onURL( 0 );
	overURL = "";
    }

#if 0
    // debugT(">>>>>>>>>>>>>>>>>>> Move detected <<<<<<<<<<<<<<<<<<<\n");

    // Does the parent want to process the event now ?
    if ( htmlView )
    {
	if ( htmlView->mouseMoveHook( _mouse ) )
	    return;
    }

    // text selection
    if ( pressed && m_strSelectedURL.isEmpty() )
    {
	QPoint point = _mouse->pos();
	if ( point.y() > height() )
	    point.setY( height() );
	else if ( point.y() < 0 )
	    point.setY( 0 );
	selectPt2.setX( point.x() + contentsX() - leftBorder );
	selectPt2.setY( point.y() + contentsY() - topBorder );
	if ( selectPt2.y() < selectPt1.y() )
	{
	    selectText( selectPt2.x(), selectPt2.y(),
		selectPt1.x(), selectPt1.y() );
	}
	else
	{
	    selectText( selectPt1.x(), selectPt1.y(),
		selectPt2.x(), selectPt2.y() );
	}

	// Do we need to scroll because the user has moved the mouse
	// outside the widget bounds?
	if ( _mouse->pos().y() > height() )
	{
	    autoScrollY( 100, 20 );
	    connect( this, SIGNAL( scrollVert(int) ),
		SLOT( slotUpdateSelectText(int) ) );
	}
	else if ( _mouse->pos().y() < 0 )
	{
	    autoScrollY( 100, -20 );
	    connect( this, SIGNAL( scrollVert(int) ),
		SLOT( slotUpdateSelectText(int) ) );
	}
	else
	{
	    stopAutoScrollY();
	    disconnect( this, SLOT( slotUpdateSelectText(int) ) );
	}
    }

    // Drags are only started with the left mouse button ...
    // if ( _mouse->button() != LeftButton )
    // return;

    // debugT("Testing m_strSelectedURL.isEmpty()\n");
    if ( m_strSelectedURL.isEmpty() )
	return;

    int x = _mouse->pos().x();
    int y = _mouse->pos().y();

    // debugT("Testing Drag\n");

    // Did the user start a drag?
    if ( abs( x - press_x ) > 5 || abs( y - press_y ) > 5 )
    {
	// Does the parent want to process the event now ?
	if ( htmlView )
        {
	    if ( htmlView->dndHook( m_strSelectedURL.data() ) )
		return;
	}
	
	QStrList uris;
	uris.append(m_strSelectedURL.data());

	QUriDrag *ud = new QUriDrag(uris, this);
	ud->setPixmap(dndDefaultPixmap);
	ud->dragCopy();
    }
#endif
}

void KHTMLWidget::viewportMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( !document ) return;

    if ( pressed )
    {
	// in case we started an autoscroll in MouseMove event
	// ###
	//stopAutoScrollY();
	disconnect( this, SLOT( slotUpdateSelectText(int) ) );
    }

    // Used to prevent mouseMoveEvent from initiating a drag before
    // the mouse is pressed again.
    pressed = false;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmouseReleaseEvent: x=%d, y=%d\n", xm, ym);

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseRelease, 0, 0, url );

#if 0
    if ( clue->mouseEvent( _mouse->x() + contentsX() - leftBorder,
	    _mouse->y() + contentsY() - topBorder, _mouse->button(),
	    _mouse->state() ) )
	return;

    // Does the parent want to process the event now ?
    if ( htmlView )
    {
      if ( htmlView->mouseReleaseHook( _mouse ) )
	return;
    }

    // emit textSelected() if necessary
    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
    {
	if ( bIsTextSelected )
	{
	    debugM( "Text Selected\n" );
	    emit textSelected( true );
	}
    }
    if ( clue == 0 )
	return;
#endif
    if ( m_strSelectedURL.isNull() )
	return;

     if ( m_strSelectedURL.data()[0] == '#' )
    	gotoAnchor( m_strSelectedURL.data() + 1 );
     else
    if ( _mouse->button() != RightButton )
    {
	printf("m_strSelectedURL='%s'\n",m_strSelectedURL.data());
	// ### FIXME
	QString pressedTarget;
	urlSelected( m_strSelectedURL.data(), _mouse->button(), pressedTarget.data() );
    }
}

QString KHTMLWidget::selectedText()
{
    // ###
    return QString::null;
}

void
KHTMLWidget::setFontSizes(const int *newFontSizes, const int *newFixedFontSizes)
{
    defaultSettings->setFontSizes(newFontSizes, newFixedFontSizes);
}

void
KHTMLWidget::fontSizes(int *newFontSizes, int *newFixedFontSizes)
{
    defaultSettings->getFontSizes(newFontSizes, newFixedFontSizes);
}

void
KHTMLWidget::resetFontSizes(void)
{
    defaultSettings->resetFontSizes();
}

void
KHTMLWidget::setStandardFont( const QString &name )
{	
    defaultSettings->fontBaseFace = name;
}

void
KHTMLWidget::setFixedFont( const QString &name )
{	
    defaultSettings->fixedFontFace = name;
}

void
KHTMLWidget::setUnderlineLinks( bool ul )
{
    defaultSettings->underlineLinks = ul;
}

bool
KHTMLWidget::gotoAnchor( const QString &/*_name*/ )
{
    // ### FIXME
}

void KHTMLWidget::findTextBegin()
{
    // ###
}

bool KHTMLWidget::findTextNext( const QRegExp &/*exp*/ )
{
    // ###
    return false;
}
