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

#include "khtmlview.moc"

#include "misc/loader.h"

#include <qstack.h>
#include <qdragobject.h>

#include <ltdl.h>
#include <kapp.h>
#include <kmimetype.h>

#include <kio/job.h>

#include <assert.h>
#include <stdio.h>

#include <kurl.h>
#include <kapp.h>
#include <kdebug.h>
#include <kcharsets.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kimgio.h>
#include <kstddirs.h>

#include <X11/Xlib.h>

#include "khtmlview.h"
#include "khtmldata.h"
#include "htmlhashes.h"

#include "decoder.h"
#include "html_documentimpl.h"

#include "html_elementimpl.h"

#include "html_miscimpl.h"
#include "html_inlineimpl.h"
#include "dom_elementimpl.h"
#include "dom_textimpl.h"

#include "dom/dom2_range.h"

#include "kjs.h"

#include "rendering/render_object.h"
#include <qdatetime.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpalette.h>
#include <qevent.h>
#include <qfont.h>
#include <qfontinfo.h>
#include <qlist.h>
#include <qpoint.h>
#include <qrect.h>
#include <qregexp.h>
#include <qscrollview.h>
#include <qtimer.h>
#include <qwidget.h>

#include "khtml_part.h"

#define SCROLLBARWIDTH 16

#define PAINT_BUFFER_HEIGHT 150

template class QList<KHTMLView>;

QList<KHTMLView> *KHTMLView::lstViews = 0L;

using namespace DOM;

QPixmap* KHTMLView::paintBuffer = 0L;


KHTMLView::KHTMLView( KHTMLPart *part, QWidget *parent, const char *name)
    : QScrollView( parent, name)
{
//    _parent = 0;
//    m_strFrameName       = QString::null;

    m_part = part;

    // initialize QScrollview

    enableClipper(true);

    viewport()->setMouseTracking(true);
    viewport()->setBackgroundMode(NoBackground);

    kimgioRegister();

    setCursor(arrowCursor);
    /*
    _isFrame = false;
    _isSelected = false;
    */
    paintBuffer = new QPixmap();

    setFocusPolicy(QWidget::WheelFocus);

    init();

    viewport()->show();
}
/*
KHTMLView::KHTMLView( QWidget *parent, KHTMLView *_parent_browser, QString name )
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

    setFocusPolicy(QWidget::WheelFocus);

    paintBuffer = new QPixmap();

    if(_parent) setURLCursor(_parent->urlCursor());

    init();
}
*/
KHTMLView::~KHTMLView()
{
//  slotStop();
  lstViews->removeRef( this );
  if(lstViews->isEmpty())
  {
      delete lstViews;
      lstViews = 0;
      if(paintBuffer) delete paintBuffer;
      paintBuffer = 0;
  }

  //  clear();

}

void KHTMLView::init()
{
/*
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
*/
    if ( lstViews == 0L )
	lstViews = new QList<KHTMLView>;
    lstViews->setAutoDelete( FALSE );
    lstViews->append( this );

    //  m_lstChildren.setAutoDelete( true );

//  setFocusPolicy( QWidget::StrongFocus );
  viewport()->setFocusPolicy( QWidget::WheelFocus );
  /*
  document = 0;
  decoder = 0;
  */
  _marginWidth = 5;
  _marginHeight = 5;
  _width = width()- SCROLLBARWIDTH - 2*marginWidth();
  /*
  findPos = -1;
  findNode = 0;

  bIsTextSelected = false;
  */
  resizeContents(clipper()->width(), clipper()->height());

  selection = 0;

  khtml::Cache::init();
}

void KHTMLView::clear()
{
/*
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

    m_lstChildren.clear();

    if ( bIsTextSelected )
    {
	bIsTextSelected = false;
	emit textSelected( false );
    }


    _baseURL = QString::null;
    _baseTarget = QString::null;

    findPos = -1;
    findNode = 0;
*/
    resizeContents(clipper()->width(), clipper()->height());

    pressed = false;

    setVScrollBarMode(Auto);
    setHScrollBarMode(Auto);

    if(selection) delete selection;
    selection = 0;
}
/*
void KHTMLView::setFollowsLinks( bool follow )
{
    _followLinks = follow;
}

bool KHTMLView::followsLinks()
{
    return _followLinks;
}

void KHTMLView::enableImages( bool enable )
{
    m_bEnableImages = enable;
}

bool KHTMLView::imagesEnabled() const
{
    return m_bEnableImages;
}

void KHTMLView::enableJava( bool enable )
{
    _javaEnabled = enable;
}

bool KHTMLView::javaEnabled() const
{
    return _javaEnabled;
}

void KHTMLView::enableJScript( bool enable )
{
    _jScriptEnabled = enable;
}

bool KHTMLView::jScriptEnabled() const
{
    return _jScriptEnabled;
}

void KHTMLView::executeScript(const QString &c)
{
    if(!_jScriptEnabled) return;
    jScript()->evaluate(c.unicode(), c.length());
}


KHTMLView* KHTMLView::topView()
{
  KHTMLView *v = this;

  while( v->parentView() )
    v = v->parentView();

  return v;
}

KHTMLView* KHTMLView::findFrame( const QString &_name )
{
    KHTMLView *v;

    if ( _name == "_top" || _name == "_TOP" )
    {
	v = this;

	while ( v->parentView() )
	    v = v->parentView();
	
	return v;
    }
    else if ( _name == "_self" || _name == "_SELF" )
    {
	return this;
    }
    else if ( _name == "_parent" || _name == "_PARENT" )
    {
	if ( parentView() )
	    return parentView();
    }
    else if ( _name == "_blank" || _name == "_BLANK" )
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

KHTMLView* KHTMLView::createFrame( QWidget *_parent, QString _name )
{
    KHTMLView *child = new KHTMLView( _parent, this, _name );
    Child *c = new Child(child, false);
    m_lstChildren.append(c);
    return child;
}

KHTMLView* KHTMLView::getFrame( QString _name )
{
    Child *c = m_lstChildren.first();
    while(c)
    {	
	if( c->m_pBrowser->frameName() == _name) return c->m_pBrowser;
	c = m_lstChildren.next();
    }
    return 0;
}


void KHTMLView::begin( const QString &_url, int _x_offset, int _y_offset )
{
    debug("KHTMLView::begin(....)");

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

    document = new HTMLDocumentImpl(this);
    document->ref();
    document->attach(this);
    document->setURL(m_strURL);
    document->open();
    // clear widget
    resizeContents(0, 0);
//    setBackgroundMode(PaletteBackground);
//    viewport()->repaint(false);

    m_bParsing = true;
}

void KHTMLView::slotStop()
{
  printf("----> KHTMLView::slotStop()\n");

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

  // ### cancel all file requests

  if(!m_bComplete)
  {
      emit canceled();
      if ( _parent )
	  _parent->childCompleted( this );
  }
  m_bComplete = true;

}
void KHTMLView::slotReload()
{
  // Reloads everything including the framesets
  if ( !m_strURL.isEmpty() )
    openURL( m_strURL, true );
}

void KHTMLView::slotReloadFrames()
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
void KHTMLView::openURL( const QString &_url, bool _reload, int _xoffset, int _yoffset, const char* _post_data )
{

    printf("======================>>>>>>>> openURL this=%p, url=%s\n", this, debugString(_url));
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
  connect( job, SIGNAL( sigData( int, const char*, int ) ), this, SLOT( slotData( int, const char*, int ) ) );
   connect( job, SIGNAL( sigError( int, int, const char* ) ), this, SLOT( slotError( int, int, const char* ) ) );

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

void KHTMLView::slotPost( int id )
{
    KIOJob* job = KIOJob::find( id );
#ifdef AFTER_KRASH_API
    job->data(post_data);
#else
    job->data((const char *)post_data, post_data.length());
#endif
}

void KHTMLView::slotFinished( int /*_id */ /* )
{
  kDebugInfo(1202,"SLOT_FINISHED 1");

  m_strWorkingURL = "";

  if ( m_bParsing )
  {
    kDebugInfo(1202,"SLOT_FINISHED 2");
    end();
  }

  m_jobId = 0;
  m_bParsing = false;
  m_bComplete = true;
}

void KHTMLView::slotData( int /*_id*/ /*, const char *_p, int _len )
{
    //if(_id != m_jobId) return; // the data is still from the previous page.

    kDebugInfo(1202,"SLOT_DATA %d", _len);

  /** DEBUG **/ /*
  assert( (int)strlen(_p ) <= _len );
  /** End DEBUG **/

  // The first data ?
/*
  if ( !m_strWorkingURL.isEmpty() )
  {
    kDebugInfo(1202,"BEGIN...");
    m_lstChildren.clear();
    m_bParsing = true;
    begin( m_strWorkingURL, m_iNextXOffset, m_iNextYOffset );
    m_strWorkingURL = "";
  }

  write( _p, _len );
}


void KHTMLView::slotError( int /*_id*/ /*, int _err, const char *_text )
{
  if ( _err == KIO::ERR_WARNING )
    return; //let's ignore warnings for now

  kDebugInfo(1202,"+++++++++++++ ERROR %d, %s ", _err, _text);

  slotStop();

  emit error( _err, _text );

  // !!!!!! HACK !!!!!!!!!!
  kioErrorDialog( _err, _text );

  kDebugInfo(1202,"+++++++++++ RETURN from error ++++++++++");

  emit canceled();
}

void KHTMLView::urlSelected( const QString &_url, int _button, const QString &_target )
{
    if ( !m_bComplete )
	slotStop();

  // Security
  KURL u1( _url );
  KURL u2( m_strURL );
  if ( ::strcmp( u1.protocol(), "cgi" ) == 0 &&
       ::strcmp( u2.protocol(), "file" ) != 0 && ::strcmp( u2.protocol(), "cgi" ) != 0 )
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
      if ( ::strcmp( target.latin1(), "_parent" ) == 0 )
      {
	  KHTMLView *v = parentView();
	  if ( !v )
	      v = this;
	  v->openURL( url );
	  if(v->_parent) emit v->topView()->started( url );
	  emit urlClicked( url, target, _button );
	  return;
      }
      else if ( ::strcmp( target.latin1(), "_top" ) == 0 )
      {
	  kDebugInfo(1202,"OPENING top %s", url.ascii());
	  topView()->openURL( url );
	  emit urlClicked( url, target, _button );
	  kDebugInfo(1202,"OPENED top");
	  return;
      }
      else if ( ::strcmp( target.latin1(), "_blank" ) == 0 )
      {
	  emit newWindow( url );
	  return;
      }
      else if ( ::strcmp( target.latin1(), "_self" ) == 0 )
      {
	  openURL( url );
	  emit urlClicked( url, target, _button );
	  if(_parent) emit topView()->started( url );
	  return;
      }

      KHTMLView *v = topView()->findChildView( target );
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

void KHTMLView::slotFormSubmitted( const QString &_method, const QString &_url,
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

QString KHTMLView::completeURL( const QString &_url, const QString &target )
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
    orig.setEncodedPathAndQuery(_url);
    return orig.url();
}

KHTMLView* KHTMLView::findChildView( const QString &_target )
{
  QListIterator<Child> it( m_lstChildren );
  for( ; it.current(); ++it )
  {
    if ( it.current()->m_pBrowser->frameName() &&
	 ::strcmp( it.current()->m_pBrowser->frameName(), _target ) == 0 )
      return it.current()->m_pBrowser;
  }

  QListIterator<Child> it2( m_lstChildren );
  for( ; it2.current(); ++it2 )
  {
    KHTMLView *b = it2.current()->m_pBrowser->findChildView( _target );
    if ( b )
      return b;
  }

  return 0L;
}

void KHTMLView::childCompleted( KHTMLView *_browser )
{
  /** DEBUG **/ /*
  kDebugInfo(1202,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  kDebugInfo(1202,"--------------- ChildFinished %p ----------------------",this);
  /** End DEBUG **/
/*
  QListIterator<Child> it( m_lstChildren );
  for( ; it.current(); ++it )
  {
    if ( it.current()->m_pBrowser == _browser )
      it.current()->m_bReady = true;
  }

  checkCompleted();

  kDebugInfo(1202,"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}

void KHTMLView::write( const char *_str, int _len)
{
    if(!decoder) decoder = new KHTMLDecoder();

    if ( _len == 0 )
	return;
    if ( _len == -1 )
      _len = strlen(_str);

    QString decoded = decoder->decode( _str, _len );
    document->write( decoded );
}

void KHTMLView::write( const QString &_str)
{
    if ( _str == QString::null )
	return;

    document->write( _str );

}

void KHTMLView::end()
{
  /** DEBUG **/ /*
  if ( !_parent )
  {
    kDebugInfo(1202,"--------------------------------------------------");
    kDebugInfo(1202,"--------------- DocFinished %p ----------------------",this);
    kDebugInfo(1202,"--------------------------------------------------");
  }
  else
    kDebugInfo(1202,"########### SUB-DocFinished %p ##############",this);
  /** End DEBUG **/
/*
  m_bParsing = false;

  document->close();

  KURL u(m_strURL);
  if ( !u.htmlRef().isEmpty() )
      gotoAnchor( u.htmlRef() );
  else
      setContentsPos( m_iNextXOffset, m_iNextYOffset );

  checkCompleted();
}

void KHTMLView::checkCompleted()
{
    if(m_bParsing) return;

    // Are all children complete now ?
    QListIterator<Child> it2( m_lstChildren );
    for( ; it2.current(); ++it2 )
	if ( !it2.current()->m_bReady )
	    return;

    // ### check all additional data downloaded

    m_bComplete = true;
    emit completed();
    if ( _parent )
	_parent->childCompleted( this );
}
*/
void KHTMLView::resizeEvent ( QResizeEvent * event )
{

//    printf("resizeEvent\n");
    layout();

    //    if(document && document->body())
    //	resizeContents(document->renderer()->width(), document->renderer()->height());

    DOM::HTMLDocumentImpl *doc = m_part->docImpl();

    if ( doc && doc->body() )
      resizeContents( doc->renderer()->width(), doc->renderer()->height() );

    QScrollView::resizeEvent(event);

    //emit resized( event->size() );
}



void KHTMLView::viewportPaintEvent ( QPaintEvent* pe  )
{
    QRect r = pe->rect();

    NodeImpl *body = 0;

    if( m_part->docImpl() )
	body = m_part->docImpl()->body();

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
	QPainter p(paintBuffer);
	p.fillRect(r.x(), r.y(), ew, eh, kapp->palette().normal().brush(QColorGroup::Background));
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

	tp->fillRect(ex, ey+py, ew, ph, kapp->palette().normal().brush(QColorGroup::Background));

	m_part->docImpl()->renderer()->print(tp, ex, ey+py, ew, ph, 0, 0);

	tp->end();
	delete tp;

    	//printf("bitBlt x=%d,y=%d,sw=%d,sh=%d\n",ex,ey+py,ew,ph);
	bitBlt(viewport(),r.x(),r.y()+py,paintBuffer,0,0,ew,ph,Qt::CopyROP);
	
	py += PAINT_BUFFER_HEIGHT;
    }

    // ### print selection

    //if(selection) {
    //}

    //printf("TIME: print() dt=%d\n",qt.elapsed());
}

void KHTMLView::layout()
{
    //### take care of frmaes (hide scrollbars,...)

    if( m_part->docImpl() )
    {	
        DOM::HTMLDocumentImpl *document = m_part->docImpl();

	NodeImpl *body = document->body();
	if(body && body->id() == ID_FRAMESET)
	{
	    setVScrollBarMode(AlwaysOff);
	    setHScrollBarMode(AlwaysOff);
	    _width = width();
	
	    document->renderer()->setMinWidth(_width);
	    document->renderer()->layout(true);
	    return;
	}

	int w = width() - SCROLLBARWIDTH - 2*marginWidth();

//	if(w < _width-5 || w > _width + 5)
    	if (w!=_width)
	{
	    //printf("layouting document\n");

	    _width = w;

	    QTime qt;
	    qt.start();

	    document->renderer()->setMinWidth(_width);
	    document->renderer()->layout(true);
	    resizeContents(document->renderer()->width(), document->renderer()->height());
	    printf("TIME: layout() dt=%d\n",qt.elapsed());

	    viewport()->repaint(false);
	}
    }
    else
    {
	_width = width() - SCROLLBARWIDTH - 2*marginWidth();
    }
}

void KHTMLView::paintElement( khtml::RenderObject *o, int xPos, int yPos )
{
    int yOff = contentsY();
    if(yOff > yPos + o->height() ||
       yOff + visibleHeight() < yPos)
	return;

    QWidget *vp = viewport();
    QPainter p(vp);
    int xOff = contentsX()+vp->x();
    yOff += vp->y();
    p.translate( -xOff, -yOff );

    o->printObject( &p , xOff, yOff, vp->width(), vp->height(),
		    xPos , yPos );
}

//
// Event Handling
//
/////////////////

void KHTMLView::viewportMousePressEvent( QMouseEvent *_mouse )
{
    if(!m_part->docImpl()) return;

    if(m_part->mousePressHook(_mouse)) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    //printf("\nmousePressEvent: x=%d, y=%d\n", xm, ym);


    // Make this frame the active one
    // ### need some visual indication for the active frame.
    /* ### use PartManager (Simon)
    if ( _isFrame && !_isSelected )
    {
	printf("activating frame!\n");
	topView()->setFrameSelected(this);
    }*/

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MousePress, 0, 0, url, innerNode, offset );

    if(url != 0)
    {
	//printf("mouseEvent: overURL %s\n", url.string().latin1());
	m_strSelectedURL = url.string();
    }
    else
	m_strSelectedURL = QString::null;

    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
    {
    	pressed = TRUE;
	if(_mouse->button() == LeftButton) {
	    if(selection) delete selection;
	    selection = 0;
    	    if(innerNode) {
		selection = new Range;
		try {
		    selection->setStart(innerNode, offset);
		    selection->setEnd(innerNode, offset);
		}
		catch(...)
		{
		    printf("selection: catched range exception\n");
		}
	    }
	    // ### emit some signal
	}
    }

    if( _mouse->button() == RightButton )
    {
    //	QPoint p(xm, ym);
	
	//	emit popupMenu(m_strSelectedURL,mapToGlobal(p));
	m_part->popupMenu( m_strSelectedURL );
    }
}

void KHTMLView::viewportMouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if(!m_part->docImpl()) return;
    if(m_part->mouseDoubleClickHook(_mouse)) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmouseDblClickEvent: x=%d, y=%d\n", xm, ym);

    DOMString url;
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick, 0, 0, url, innerNode, offset );

    // ###
    //if ( url.length() )
    //emit doubleClick( url.string(), _mouse->button() );
}

void KHTMLView::viewportMouseMoveEvent( QMouseEvent * _mouse )
{
    if(!m_part->docImpl()) return;
    if(m_part->mouseMoveHook(_mouse)) return;

    // drag of URL

    if(pressed && !m_strSelectedURL.isEmpty())
    {
	QStrList uris;
	KURL u( m_part->completeURL( m_strSelectedURL) );
	uris.append(u.url().ascii());
	QDragObject *d = new QUriDrag(uris, this);
	QPixmap p = KMimeType::pixmapForURL(u, 0, KIconLoader::Medium);
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
    NodeImpl *innerNode=0;
    long offset=0;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseMove, 0, 0, url, innerNode, offset );

    if ( !pressed && url.length() )
    {
	QString surl = url.string();
	if ( overURL.isEmpty() )
	{
	    setCursor( linkCursor );
	    overURL = surl;
	    m_part->overURL( overURL );
	    //	    emit onURL( overURL );
	}
	else if ( overURL != surl )
	{
	//	    emit onURL( surl );
	    m_part->overURL( overURL );
	    overURL = surl;
	}
	return;
    }
    else if( overURL.length() && !url.length() )
    {
	setCursor( arrowCursor );
	//	emit onURL( 0 );
	m_part->overURL( QString::null );
	overURL = "";
    }

    // selection stuff
    if( pressed ) {
	try {
	    selection->setEnd(innerNode, offset);
	}
	catch(...)
	{
	    printf("selection: catched range exception\n");
	}
    }
}

void KHTMLView::viewportMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( !m_part->docImpl() ) return;
    if(m_part->mouseReleaseHook(_mouse)) return;

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

    //printf("\nmouseReleaseEvent: x=%d, y=%d\n", xm, ym);

    DOMString url=0;
    NodeImpl *innerNode=0;
    long offset;
    m_part->docImpl()->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseRelease, 0, 0, url, innerNode, offset );

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

	//	urlSelected( m_strSelectedURL.data(), _mouse->button(), pressedTarget.data() );
	m_part->urlSelected( m_strSelectedURL, _mouse->button(), pressedTarget );
   }

    try {
	selection->setEnd(innerNode, offset);
    }
    catch(...)
    {
	printf("selection: catched range exception\n");
    }

    // ### delete selection in case start and end position are at the same point
}

void KHTMLView::keyPressEvent( QKeyEvent *_ke )
{
    if(m_part->keyPressHook(_ke)) return;


    int offs = (clipper()->height() < 30) ? clipper()->height() : 30;
    switch ( _ke->key() )
    {
    case Key_Down:
    case Key_J:
	scrollBy( 0, 10 );
	break;

    case Key_Next:
    case Key_Space:
	scrollBy( 0, clipper()->height() - offs );
	break;

    case Key_Up:
    case Key_K:
	scrollBy( 0, -10 );
	break;

    case Key_Prior:
    case Key_Backspace:
	scrollBy( 0, -clipper()->height() + offs );
	break;

    case Key_Right:
    case Key_L:
	scrollBy( 10, 0 );
	break;	

    case Key_Left:
    case Key_H:
	scrollBy( -10, 0 );
	break;

    default:
	QScrollView::keyPressEvent( _ke );
    }
}

// ---------------------------------- selection ------------------------------------------------

/*
QString KHTMLView::selectedText()
{
    // ###
    return QString::null;
}

bool
KHTMLView::gotoAnchor( const QString &_name )
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

void KHTMLView::findTextBegin()
{
    findPos = -1;
    findNode = 0;
}

bool KHTMLView::findTextNext( const QRegExp &exp )
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
		findNode->renderer()->absolutePosition(x, y);
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


bool KHTMLView::isFrameSet()
{
    if(!document || !document->body()) return false;

    if(document->body()->id() == ID_FRAMESET) return true;
    return false;
}

const QString &KHTMLView::baseUrl()
{
  return _baseURL;
}

void KHTMLView::setBaseUrl(const QString &base)
{
  _baseURL = base;
}

KJSProxy *KHTMLView::jScript()
{
    if(!_jScriptEnabled) return 0;
    if(!_jscript)
    {
      if(!lt_dl_initialized)
      {
	lt_dlinit();
	lt_dl_initialized = true;
      }
      // locate module
      QString module = KGlobal::dirs()->findResource("lib", "kjs_html.la");
      if(module.isNull())
      {
	fprintf(stderr, "didn't find kjs module\n");
	return 0;
      }
      // try to obtain a handle on the module
      lt_dlhandle handle = lt_dlopen(module.ascii());
      if(!handle)
      {
	fprintf(stderr, "error loading kjs module: %s\n", lt_dlerror());
	return 0;
      }
      // look for plain C init function
      lt_ptr_t sym = lt_dlsym(handle, "kjs_html_init");
      if (lt_dlerror() != 0L)
      {
	fprintf(stderr, "error finding init symbol: %s\n", lt_dlerror());
	return 0;
      }

      typedef KJSProxy* (*initFunction)(HTMLDocument);
      initFunction initSym = (initFunction) sym;
      _jscript = (*initSym)(htmlDocument());
    }

    return _jscript;
}


void KHTMLView::setFrameSelected(KHTMLView *w)
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

KHTMLView *KHTMLView::selectedFrame()
{
    if(_isSelected) return this;

    Child *c = m_lstChildren.first();
    while(c)
    {	
	KHTMLView *w = c->m_pBrowser->selectedFrame();
	if(w) return w;
	c = m_lstChildren.next();
    }
    return 0;
}

// ####
bool KHTMLView::setCharset(const QString &name, bool /*override*/ /*)
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

    // ### defaultSettings->charset = f.charSet();
    return true;
}

*/


bool KHTMLView::focusNextPrevChild( bool next )
{
    printf("focusNextPrev %d\n",next);
//    return true;    // ### temporary fix for qscrollview focus bug
    	    	    // as a side effect, disables tabbing between form elements
		    // -antti

    return QScrollView::focusNextPrevChild( next );
}

