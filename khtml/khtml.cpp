/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "khtml.moc"

#include <qstack.h>
#include <qdragobject.h>

#include <kapp.h>

#include <kio_job.h>
#include <kio_cache.h>
#include <kio_error.h>

#include <assert.h>

#include <kurl.h>
#include <kapp.h>
#include <kdebug.h>
#include "khtml.h"
#include "khtmldata.h"
#include "khtmlattrs.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kimgio.h>
#include <kstddirs.h>

#include "khtmldecoder.h"
#include "html_documentimpl.h"

#include "khtmlio.h"
#include "html_elementimpl.h"

#include "html_miscimpl.h"
#include "html_inlineimpl.h"
#include "dom_elementimpl.h"
#include "dom_textimpl.h"

#include "kjs.h"

#include <X11/Xlib.h>

#define SCROLLBARWIDTH 16

#define PAINT_BUFFER_HEIGHT 150

QList<KHTMLWidget> *KHTMLWidget::lstViews = 0L;

using namespace DOM;

QPixmap* KHTMLWidget::paintBuffer = 0L;

KHTMLWidget::KHTMLWidget( QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
    _parent = 0;
    m_strFrameName       = QString::null;

    // initialize QScrollview
    enableClipper(true);
    viewport()->setMouseTracking(true);
    viewport()->setBackgroundMode(NoBackground);

    kimgioRegister();

    setCursor(arrowCursor);
    _isFrame = false;
    _isSelected = false;

    paintBuffer = new QPixmap();

    init();

}

KHTMLWidget::KHTMLWidget( QWidget *parent, KHTMLWidget *_parent_browser, QString name )
    : QScrollView( parent, name.latin1())
{
    _parent     = _parent_browser;
    m_strFrameName       = name;

    printf("new frame, name=%s\n", name.latin1());
  // Initialize QScrollview
    enableClipper(true);
    viewport()->setMouseTracking(true);
    viewport()->setBackgroundMode(NoBackground);

    kimgioRegister();

    setCursor(arrowCursor);
    _isFrame = true;
    _isSelected = false;

    paintBuffer = new QPixmap();

    if(_parent) setURLCursor(_parent->urlCursor());

    init();
}

KHTMLWidget::~KHTMLWidget()
{
  slotStop();
  lstViews->removeRef( this );
  if(lstViews->isEmpty())
  {
      delete lstViews;
      lstViews = 0;
      if(paintBuffer) delete paintBuffer;
      paintBuffer = 0;
  }

  clear();

  if(cache) delete cache;
  if(defaultSettings) delete defaultSettings;
}

void KHTMLWidget::init()
{
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
  _followLinks = true;
  _jscript = 0;

    if ( lstViews == 0L )
	lstViews = new QList<KHTMLWidget>;
    lstViews->setAutoDelete( FALSE );
    lstViews->append( this );

  m_lstURLRequestJobs.setAutoDelete( true );
  //m_lstPendingURLRequests.setAutoDelete( true );
  m_lstChildren.setAutoDelete( true );

//  setFocusPolicy( QWidget::StrongFocus );
  viewport()->setFocusPolicy( QWidget::WheelFocus );

  document = 0;
  decoder = 0;
  cache = new KHTMLCache(this);
  defaultSettings = new HTMLSettings;
  _settings = 0;

  _marginWidth = 5;
  _marginHeight = 5;
  _width = width()- SCROLLBARWIDTH - 2*marginWidth();

  findPos = -1;
  findNode = 0;

  resizeContents(clipper()->width(), clipper()->height());
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
    delete _jscript;
    _jscript = 0;
    if ( _settings ) delete _settings;
    _settings = 0;

    m_lstChildren.clear();

    if ( bIsTextSelected )
    {
	bIsTextSelected = false;
	emit textSelected( false );
    }

    pressed = false;

    _baseURL = QString::null;
    _baseTarget = QString::null;

    findPos = -1;
    findNode = 0;

    resizeContents(clipper()->width(), clipper()->height());

    m_strRedirectUrl = QString::null;
    m_delayRedirect = 0;

    setVScrollBarMode(Auto);
    setHScrollBarMode(Auto);

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
    if(!_jscript) _jscript = new KJSWorld(this);
    _jscript->evaluate((KJS::UnicodeChar*)c.unicode(), c.length());
}


KHTMLWidget* KHTMLWidget::topView()
{
  KHTMLWidget *v = this;

  while( v->parentView() )
    v = v->parentView();

  return v;
}

KHTMLWidget* KHTMLWidget::findFrame( const QString &_name )
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

    // ### FIXME: get preferencies right in case of two identical names.
    //            see html docs for rules.
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

KHTMLWidget* KHTMLWidget::createFrame( QWidget *_parent, QString _name )
{
    KHTMLWidget *child = new KHTMLWidget( _parent, this, _name );
    Child *c = new Child(child, false);
    m_lstChildren.append(c);
    return child;
}

KHTMLWidget* KHTMLWidget::getFrame( QString _name )
{
    Child *c = m_lstChildren.first();
    while(c)
    {	
	if( c->m_pBrowser->frameName() == _name) return c->m_pBrowser;
	c = m_lstChildren.next();
    }
    return 0;
}


void KHTMLWidget::begin( const QString &_url, int _x_offset, int _y_offset )
{
    debug("KHTMLWidget::begin(....)");

    clear();

    m_iNextXOffset = _x_offset;
    m_iNextYOffset = _y_offset;

    // ###
    //stopParser();
    m_strURL = _url;

    if ( !_url.isEmpty() )
    {

	KURL::List lst = KURL::split( m_strURL );
	QString baseurl;
	if ( !lst.isEmpty() )
	    baseurl = (*lst.begin()).url();
	else
	    baseurl = QString::null;

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

    if(!_settings) _settings = new HTMLSettings( *defaultSettings);


    document = new HTMLDocumentImpl(this, cache);
    document->ref();
    document->setURL(m_strURL);
    document->open();
    // clear widget
    resizeContents(0, 0);
//    setBackgroundMode(PaletteBackground);
//    viewport()->repaint(false);

    m_bParsing = true;
}

void KHTMLWidget::slotStop()
{
  printf("----> KHTMLWidget::slotStop()\n");

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

  if ( !m_strWorkingURL.isEmpty() )
    m_strWorkingURL = "";

  // cancel all file requests
  m_lstURLRequestJobs.clear();
  m_lstPendingURLRequests.clear();

  if(!m_bComplete)
  {
      emit canceled();
      if ( _parent )
	  _parent->childCompleted( this );
  }
  m_bComplete = true;

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

// ### fix after krash: post_data -> QCString
void KHTMLWidget::openURL( const QString &_url, bool _reload, int _xoffset, int _yoffset, const char* _post_data )
{

    printf("======================>>>>>>>> openURL this=%p, url=%s\n", this, _url.ascii());
  // Check URL
  if ( KURL::split( _url ).isEmpty() )
  {
    emit error( KIO::ERR_MALFORMED_URL, _url );
    return;
  }

  m_iNextYOffset = _yoffset;
  m_iNextXOffset = _xoffset;

  m_bReload = _reload;

  // ### hack...
  bool frameset = false;
  if(document)
  {
      NodeImpl *body = document->body();
      if(body && body->id() == ID_FRAMESET) frameset = true;
  }

  if ( !m_bReload && !frameset && urlcmp( _url, m_strURL, true, true )
       && !_post_data )
  {
    KURL u( _url );

    emit started( _url );

    if ( !u.htmlRef().isEmpty() )
      gotoAnchor( u.htmlRef() );
    else
      setContentsPos( 0, 0 );

    m_bComplete = true;
    m_bParsing = false;

    emit completed();

    return;
  }

  slotStop();

  KIOCachedJob *job = new KIOCachedJob;
  m_jobId = job->id();

  job->setGUImode( KIOJob::NONE );

  // ###
  connect( job, SIGNAL( sigFinished( int ) ), this, SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigRedirection( int, const char* ) ), this, SLOT( slotRedirection( int, const char* ) ) );
  connect( job, SIGNAL( sigData( int, const char*, int ) ), this, SLOT( slotData( int, const char*, int ) ) );
  // connect( job, SIGNAL( sigError( int, int, const char* ) ), this, SLOT( slotError( int, int, const char* ) ) );

  if ( _post_data )
  {
      post_data = _post_data;
      connect( job, SIGNAL( sigReady( int ) ),
	       this, SLOT( slotPost( int ) ) );
      job->put( _url, -1, true, false, strlen(_post_data) );
  }	
  else
  {
      job->get( _url, _reload );
      post_data = 0;
  }

  m_bComplete = false;
  m_strWorkingURL = _url;

  emit started( m_strWorkingURL );
}

void KHTMLWidget::slotPost( int id )
{
    KIOJob* job = KIOJob::find( id );
#ifdef AFTER_KRASH_API
    job->data(post_data);
#else
    job->data((const char *)post_data, post_data.length());
#endif
}

void KHTMLWidget::slotFinished( int /*_id*/ )
{
  kdebug(0,1202,"SLOT_FINISHED 1");

  m_strWorkingURL = "";

  if ( m_bParsing )
  {
    kdebug(0,1202,"SLOT_FINISHED 2");
    end();
  }

  m_jobId = 0;
  m_bParsing = false;
  m_bComplete = true;
}

void KHTMLWidget::slotRedirection( int /*_id*/, const char *_url )
{
  // We get this only !before! we receive the first data
  assert( !m_strWorkingURL.isEmpty() );
  m_strWorkingURL = _url;
}

void KHTMLWidget::slotData( int /*_id*/, const char *_p, int _len )
{
    //if(_id != m_jobId) return; // the data is still from the previous page.

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
	//m_lstURLRequestJobs.remove( job );
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
    if ( !m_bComplete )
	slotStop();

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

  QString target = _target;
  if(target.isEmpty()) target = _baseTarget;

  QString url = completeURL( _url, target );

  printf("urlSelected: url=%s target=%s\n", url.latin1(), target.latin1());

  if(!_followLinks)
  {
      emit urlClicked( url, target, _button );
      return;
  }

  // ### the started signals are just a hack to get history right...

  if ( !target.isNull() && !target.isEmpty() && _button == LeftButton )
  {
      printf("searching target frame\n");
      if ( strcmp( target.latin1(), "_parent" ) == 0 )
      {
	  KHTMLWidget *v = parentView();
	  if ( !v )
	      v = this;
	  v->openURL( url );
	  if(v->_parent) emit v->topView()->started( url );
	  emit urlClicked( url, target, _button );
	  return;
      }
      else if ( strcmp( target.latin1(), "_top" ) == 0 )
      {
	  kdebug(0,1202,"OPENING top %s", url.ascii());
	  topView()->openURL( url );
	  emit urlClicked( url, target, _button );
	  kdebug(0,1202,"OPENED top");
	  return;
      }
      else if ( strcmp( target.latin1(), "_blank" ) == 0 )
      {
	  emit newWindow( url );
	  return;
      }
      else if ( strcmp( target.latin1(), "_self" ) == 0 )
      {
	  openURL( url );
	  emit urlClicked( url, target, _button );
	  if(_parent) emit topView()->started( url );
	  return;
      }

      printf("searching...\n");
      KHTMLWidget *v = topView()->findChildView( target );
      if ( !v )
	  v = findFrame( target );
      if ( v )
      {
	  v->openURL( url );
	  emit urlClicked( url, target, _button );
	  if(v->_parent) emit v->topView()->started( url );
	  return;
      }
      else
      {
	  printf("couldn't find target!\n");
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

      openURL( url );	
      if(_parent) emit topView()->started( url );
  }
}

void KHTMLWidget::slotFormSubmitted( const QString &_method, const QString &_url,
				     const char *_data, const QString &_target )
{
  QString target = _target;
  if(target.isEmpty()) target = _baseTarget;

  QString url = completeURL( _url, target );

  KURL u( url );
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
    printf("setting default text colors\n");

    defaultSettings->fontBaseColor = _textc;
    defaultSettings->linkColor = _linkc;
    defaultSettings->vLinkColor = _vlinkc;

    Child *c;
    for ( c = m_lstChildren.first(); c != 0L; c = m_lstChildren.next() )
	c->m_pBrowser->setDefaultTextColors( _textc, _linkc, _vlinkc );
}

void KHTMLWidget::setDefaultBGColor( const QColor& bgcolor )
{
  printf("setting default bgColor\n");
    defaultSettings->bgColor = bgcolor;

    Child *c;
    for ( c = m_lstChildren.first(); c != 0L; c = m_lstChildren.next() )
	c->m_pBrowser->setDefaultBGColor( bgcolor );
}

QString KHTMLWidget::completeURL( const QString &_url, const QString &target )
{
    KURL orig;
    if(_url[0] == '#' && !target.isEmpty() && findFrame(target))
    {
	orig = KURL(findFrame(target)->url());
    }
    else if(_baseURL.isEmpty())
    {
	orig = KURL( m_strURL );
    }
    else
	orig = KURL( _baseURL );
    if(_url[0] != '/')
    {
	KURL u( orig, _url );
	return u.url();
    }	
    orig.setPath(_url);
    return orig.url();
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

  if ( !m_bParsing )
      emit completed();

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

  document->close();

  KURL u(m_strURL);
  if ( !u.htmlRef().isEmpty() )
      gotoAnchor( u.htmlRef() );
  else
      setContentsPos( m_iNextXOffset, m_iNextYOffset );

  if(!m_strRedirectUrl.isEmpty())
  {
      QTimer::singleShot(1000*m_delayRedirect, this, SLOT(slotRedirect()));
      return;
  }

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
    layout();

    if(document && document->body())
	resizeContents(document->getWidth(), document->getHeight());
    QScrollView::resizeEvent(event);

    //emit resized( event->size() );
}



void KHTMLWidget::viewportPaintEvent ( QPaintEvent* pe  )
{
    QRect r = pe->rect();

    NodeImpl *body = 0;

    if(document)
	body = document->body();

    QRect rr(
	-viewport()->x(), -viewport()->y(),
	clipper()->width(), clipper()->height()
    );
    r &= rr;
    int ex = r.x() + viewport()->x() + contentsX();;
    int ey = r.y() + viewport()->y() + contentsY();;
    int ew = r.width();
    int eh = r.height();

    if(!body)
    {
	QPainter p(viewport());

	p.fillRect(r.x(), r.y(), ew, eh, kapp->palette().normal().brush(QColorGroup::Background));
	return;
    }
    //printf("viewportPaintEvent x=%d,y=%d,w=%d,h=%d\n",ex,ey,ew,eh);

    if ( paintBuffer->width() < width() )
    {
        paintBuffer->resize(width(),PAINT_BUFFER_HEIGHT);
    }

    QTime qt;
    qt.start();

    int py=0;
    while (py < eh)
    {
	QPainter* tp = new QPainter;
	tp->begin( paintBuffer );
	tp->translate(-ex,-ey-py);

    	int ph = eh-py<PAINT_BUFFER_HEIGHT ? eh-py : PAINT_BUFFER_HEIGHT;	

	// ### fix this for frames...

	body->print(tp, ex, ey+py, ew, ph, 0, 0);

	tp->end();
	delete tp;

//    	printf("bitBlt x=%d,y=%d,sw=%d,sh=%d\n",ex,ey+py,ew,ph);
	bitBlt(viewport(),r.x(),r.y()+py,paintBuffer,0,0,ew,ph,Qt::CopyROP);
	
	py += PAINT_BUFFER_HEIGHT;
    }

    printf("TIME: print() dt=%d\n",qt.elapsed());
}

void KHTMLWidget::layout()
{
    //### take care of frmaes (hide scrollbars,...)

    if(document)
    {	
	NodeImpl *body = document->body();
	if(body && body->id() == ID_FRAMESET)
	{
	    setVScrollBarMode(AlwaysOff);
	    setHScrollBarMode(AlwaysOff);
	    _width = width();
	
	    document->setAvailableWidth(_width);
	    document->layout(true);
	    return;
	}

	int w = width() - SCROLLBARWIDTH - 2*marginWidth();

//	if(w < _width-5 || w > _width + 5)
    	if (w!=_width)
	{
	    printf("layouting document\n");

	    _width = w;

	    document->setAvailableWidth(_width);
	    document->layout(true);
	    resizeContents(document->getWidth(), document->getHeight());
	    viewport()->repaint(false);
	}
    }
    else
    {
	_width = width() - SCROLLBARWIDTH - 2*marginWidth();
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
    // ### need some visual indication for the active frame.
    if ( _isFrame && !_isSelected )
    {
	printf("activating frame!\n");
	topView()->setFrameSelected(this);
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

    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
    {
    	pressed = TRUE;
    }
#if 0
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

    // drag of URL
    if(pressed && !m_strSelectedURL.isEmpty())
    {
	QStrList uris;
	KURL u(completeURL(m_strSelectedURL));
	uris.append(u.url().ascii());
	QDragObject *d = new QUriDrag(uris, this);
	QPixmap p(locate( "data", "khtml/pics/khtml_dnd.png"));
	if(p.isNull()) printf("null pixmap\n");
	d->setPixmap(p);
	d->drag();

	// when we finish our drag, we need to undo our mouse press
	pressed = false;
    m_strSelectedURL = "";
	return;
    }

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
	//disconnect( this, SLOT( slotUpdateSelectText(int) ) );
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
    if ( m_strSelectedURL.isEmpty() )
	return;

    if ( _mouse->button() != RightButton )
    {
	KURL u(m_strSelectedURL);
	QString pressedTarget;
	if(u.protocol() == "target")
	{
	    m_strSelectedURL = u.ref();
	    pressedTarget = u.host();
	}	
	printf("m_strSelectedURL='%s' target=%s\n",m_strSelectedURL.data(), pressedTarget.latin1());

	urlSelected( m_strSelectedURL.data(), _mouse->button(), pressedTarget.data() );
    }
}

void KHTMLWidget::keyPressEvent( QKeyEvent *_ke )
{
    switch ( _ke->key() )
    {
    case Key_Down:
    case Key_J:
	scrollBy( 0, 10 );
	flushKeys();
	break;

    case Key_Next:
    case Key_Space:
	scrollBy( 0, clipper()->height()*8/10 );
	flushKeys();
	break;

    case Key_Up:
    case Key_K:
	scrollBy( 0, -10 );
	flushKeys();
	break;

    case Key_Prior:
    case Key_Backspace:
	scrollBy( 0, -clipper()->height()*8/10 );
	flushKeys();
	break;

    case Key_Right:
    case Key_L:
	scrollBy( 10, 0 );
	flushKeys();
	break;	

    case Key_Left:
    case Key_H:
	scrollBy( -10, 0 );
	flushKeys();
	break;

    default:
	QScrollView::keyPressEvent( _ke );
    }
}

// Little routine from Alessandro Russo to flush extra keypresses from
// the event queue
void KHTMLWidget::flushKeys()
{
    XEvent ev_return;
    Display *dpy = qt_xdisplay();
    while ( XCheckTypedEvent( dpy, KeyPress, &ev_return ) );
}

// ---------------------------------- selection ------------------------------------------------


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
KHTMLWidget::gotoAnchor( const QString &_name )
{
    printf("gotoAnchor(%s)\n", _name.latin1());
    HTMLCollectionImpl *anchors =
	new HTMLCollectionImpl(document, HTMLCollectionImpl::DOC_ANCHORS);
    anchors->ref();
    NodeImpl *n = anchors->namedItem(_name);
    anchors->deref();

    if(!n)
    {
	n = document->getElementById(_name);
	if(n) printf("found element with matching id\n");
    }
	
    if(!n) return false;
    printf("found anchor %p!\n", n);

    int x = 0, y = 0;
    HTMLAnchorElementImpl *a = static_cast<HTMLAnchorElementImpl *>(n);
    a->getAnchorPosition(x, y);
    printf("going to %d/%d\n", x, y);
    setContentsPos(x-50, y-50);
    return true;
}

void KHTMLWidget::findTextBegin()
{
    findPos = -1;
    findNode = 0;
}

bool KHTMLWidget::findTextNext( const QRegExp &exp )
{
    if(!findNode) findNode = document->body();

    if(findNode->id() == ID_FRAMESET) return false;

    while(1)
    {
	if(findNode->id() == ID_TEXT)
	{
	    DOMStringImpl *t = (static_cast<TextImpl *>(findNode))->string();
	    QConstString s(t->s, t->l);
	    findPos = s.string().find(exp, findPos+1);
	    if(findPos != -1)
	    {
		int x = 0, y = 0;
		findNode->getAbsolutePosition(x, y);
		setContentsPos(x-50, y-50);
	    }
	}
	findPos = -1;
	NodeImpl *next = findNode->firstChild();
	if(!next) next = findNode->nextSibling();
	if(!next) next = findNode->parentNode();
    }
    return false;
}

#define INFO_NONE (int)0
#define INFO_FRAMESET (int)1
#define INFO_FRAME (int)2
#define INFO_FORM (int)3

void KHTMLWidget::saveState( QDataStream &stream )
{
    if(m_strURL.isEmpty() && !m_strWorkingURL.isEmpty())
	stream << m_strWorkingURL;
    else
	stream << m_strURL;
    stream << (int)contentsX() << (int)contentsY();

    if(!m_bComplete || !document || !document->body())
    {
	printf("-------- saving page ---------\n");
	stream << INFO_NONE;
	return;
    }

    int id = document->body()->id();

    if(id  == ID_FRAMESET)
    {	
	// did I already tell you I hate frames...
	printf("------------------ saving frameset --------------------------\n");
	stream << INFO_FRAMESET;
	
	NodeImpl *current = document->body()->firstChild();

	// go through the doc. Save every frame.
	QStack<NodeImpl> nodeStack;

	while(1)
	{
	    if(!current)
	    {
		if(nodeStack.isEmpty()) break;
		current = nodeStack.pop();
		current = current->nextSibling();
	    }
	    else
	    {
		if(current->id() == ID_FRAME)
		{
		    printf("saving frame %p\n", current);
		    stream << INFO_FRAME;
		    HTMLFrameElementImpl *f = static_cast<HTMLFrameElementImpl *>(current);
		    stream << f->view->frameName();
		    f->view->saveState(stream);
		}

		NodeImpl *child = current->firstChild();
		if(child)
		{	
		    nodeStack.push(current);
		    current = child;
		}
		else
		    current = current->nextSibling();
	    }
	}
	stream << INFO_NONE; // end of frameset
    }
    else if(id == ID_BODY)
    {
	printf("-------- saving page 2 ---------\n");
	stream << INFO_NONE;
    }
    else
	printf("error in KHTMLWidget::saveState()\n");
}

void KHTMLWidget::restoreState( QDataStream &stream )
{
    int x, y, info;
    QString u;
    stream >> u;
    stream >> x >> y;
    stream >> info; // do we have additional info?

    printf("restoring url=%s\n", u.ascii());

    if(info == INFO_NONE)
    {
	printf("------------------ restoring page ----------------------------\n");
	openURL( u, false, x, y );
	return;
    }
    if (info == INFO_FRAMESET)
    {
	printf("------------------ restoring framed page ----------------------------\n");
	while(1)
	{
	    stream >> info;
	    if(info != INFO_FRAME) break;

	    QString name;
	    stream >> name;

	    KHTMLWidget *w = getFrame(name);
	    if(!w)
	    {
		printf("have to vreate the frame!!!\n");
		w = createFrame(viewport(), name);
	    }
	    w->resize(500,100);
	    w->restoreState(stream);
	}

	if ( !urlcmp( u, m_strURL, true, true ) )
	    openURL(u, false, 0, 0);
    }

    layout();
}

bool KHTMLWidget::isFrameSet()
{
    if(!document || !document->body()) return false;

    if(document->body()->id() == ID_FRAMESET) return true;
    return false;
}

HTMLSettings *KHTMLWidget::settings()
{
    // ### check all settings stuff in khtml.cpp for memory leaks...
    if(!_settings) _settings = new HTMLSettings(*defaultSettings);
    return _settings;
}

const QString &KHTMLWidget::baseUrl()
{
  return _baseURL;
}

void KHTMLWidget::setBaseUrl(const QString &base)
{
  _baseURL = base;
}

KJSWorld *KHTMLWidget::jScript()
{
    if(!_jScriptEnabled) return 0;
    if(!_jscript) _jscript = new KJSWorld(this);
    return _jscript;
}


void KHTMLWidget::setFrameSelected(KHTMLWidget *w)
{
    if(w == this)
    {
	_isSelected = true;
	printf("selecting frame: this=%p\n", this);
    }
    else
	_isSelected = false;

    Child *c = m_lstChildren.first();
    while(c)
    {	
	c->m_pBrowser->setFrameSelected(w);
	c = m_lstChildren.next();
    }
}

KHTMLWidget *KHTMLWidget::selectedFrame()
{
    if(_isSelected) return this;

    Child *c = m_lstChildren.first();
    while(c)
    {	
	KHTMLWidget *w = c->m_pBrowser->selectedFrame();
	if(w) return w;
	c = m_lstChildren.next();
    }
    return 0;
}

// ####
bool KHTMLWidget::setCharset(const QString &name, bool /*override*/)
{
    // ### hack: FIXME
    KCharsets *c = KGlobal::charsets();
    if(!c->isAvailable(name))
    {
	printf("charset not available!\n");
	return false;
    }

    QFont f;
    c->setQFont(f, name);

    QFontInfo fi(f);
    printf("font has charset %d, real %d\n", f.charSet(), fi.charSet());

    defaultSettings->charset = f.charSet();
    return true;
}

void KHTMLWidget::scheduleRedirection(int delay, const QString & url)
{
    m_delayRedirect = delay;
    m_strRedirectUrl = url;
}

void KHTMLWidget::slotRedirect()
{
    urlSelected(m_strRedirectUrl, LeftButton, QString::null);
    m_strRedirectUrl == QString::null;
    m_delayRedirect = 0;
}
