/*
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)

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

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#define CACHE_DEBUG

#include "loader.h"

// up to which size is a picture for sure cacheable
#define MAXCACHEABLE 10*1024
// max. size of a single picture in percent of the total cache size
// to be cacheable
#define MAXPERCENT 10
// default cache size
#define DEFCACHESIZE 512*1024
// maximum number of files the loader will try to load in parallel
#define MAX_REQUEST_JOBS 4


#include <qtextstream.h>
#include <qasyncio.h>
#include <qasyncimageio.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qmovie.h>

#include <kio/job.h>
#include <kdebug.h>

#include "css/css_stylesheetimpl.h"

using namespace khtml;
using namespace DOM;

void CachedObject::computeStatus()
{
    if( m_size > MAXCACHEABLE || m_size > Cache::size()/MAXPERCENT )
    {
	m_status = Uncacheable;
	m_size = 0;
    }
    else
	m_status = Cached;
}

void CachedObject::setRequest(Request *_request)
{
    m_request = _request;
    if (canDelete() && m_free)
	delete this;
}

bool CachedObject::canDelete()
{
    return (m_clients.count() == 0 && !m_request);
}

// -------------------------------------------------------------------------------------------

CachedCSSStyleSheet::CachedCSSStyleSheet(const DOMString &url, const DOMString &baseURL, bool reload)
    : CachedObject(url, CSSStyleSheet, reload)
{
    // load the file
    Cache::loader()->load(this, baseURL, false);
    loading = true;
}

CachedCSSStyleSheet::~CachedCSSStyleSheet()
{
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "CachedCSSStyleSheet::~CachedCSSStyleSheet() " << url().string() << endl;
#endif
}

void CachedCSSStyleSheet::ref(CachedObjectClient *c)
{
    // make sure we don't get it twice...
    m_clients.remove(c);
    m_clients.append(c);

    if(!loading) c->setStyleSheet( m_url, m_sheet );
}

void CachedCSSStyleSheet::deref(CachedObjectClient *c)
{
    m_clients.remove(c);
    if ( canDelete() && m_free )
      delete this;
}

void CachedCSSStyleSheet::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    buffer.open( IO_ReadOnly );
    QTextStream t( &buffer );
    QString data = t.read();

    m_sheet = DOMString(data);
    loading = false;

    checkNotify();
}	

void CachedCSSStyleSheet::checkNotify()
{
    if(loading) return;

#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "CachedCSSStyleSheet:: finishedLoading " << m_url.string() << endl;
#endif

    CachedObjectClient *c;
    for ( c = m_clients.first(); c != 0; c = m_clients.next() )
	c->setStyleSheet( m_url, m_sheet );
}


void CachedCSSStyleSheet::error( int /*err*/, const char */*text*/ )
{
    loading = false;
    checkNotify();
}

// -------------------------------------------------------------------------------------------

CachedScript::CachedScript(const DOMString &url, const DOMString &baseURL, bool reload)
    : CachedObject(url, Script, reload)
{
    // load the file
    Cache::loader()->load(this, baseURL, false);
    loading = true;
}

CachedScript::~CachedScript()
{
}

void CachedScript::ref(CachedObjectClient *c)
{
    // make sure we don't get it twice...
    m_clients.remove(c);
    m_clients.append(c);

    if(!loading) c->notifyFinished(this);
}

void CachedScript::deref(CachedObjectClient *c)
{
    m_clients.remove(c);
    if ( canDelete() && m_free )
      delete this;
}

void CachedScript::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    buffer.open( IO_ReadOnly );
    QTextStream t( &buffer );
    QString data = t.read();

    m_script = DOMString(data);
    loading = false;

    checkNotify();
}	

void CachedScript::checkNotify()
{
    if(loading) return;

    CachedObjectClient *c;
    for ( c = m_clients.first(); c != 0; c = m_clients.next() )
	c->notifyFinished(this);
}


void CachedScript::error( int /*err*/, const char */*text*/ )
{
    loading = false;
    checkNotify();
}

// ------------------------------------------------------------------------------------------

namespace khtml
{
    /*
     * Defines the DataSource for incremental loading of images.
     */
    class ImageSource : public QDataSource
    {
    public:
	ImageSource(QByteArray buf);
	~ImageSource();

	/**
	 * Overload QDataSource::readyToSend() and returns the number
	 * of bytes ready to send if not eof instead of returning -1.
	 */
	int readyToSend();

	/*!
	  Reads and sends a block of data.
	*/
	void sendTo(QDataSink*, int count);

	/*!
	  KHTMLImageSource's is rewindable.
	*/
	bool rewindable() const;

	/*!
	  Enables rewinding.  No special action is taken.
	*/
	void enableRewind(bool on);

	/*
	  Calls reset() on the QIODevice.
	*/
	void rewind();

	/**
	 * Sets the EOF state.
	 */
	void setEOF( bool state );

    private:

	QByteArray buffer;
	bool rew;
	int pos;
	bool eof;
    };

};

/*!
  This Class defines the DataSource for incremental loading of images.
*/
ImageSource::ImageSource(QByteArray buf)
{
  buffer = buf;
  rew = false;
  pos = 0;
  eof = false;
}

/*!
  Destroys the QIODeviceSource, deleting the QIODevice from which it was
  constructed.
*/
ImageSource::~ImageSource() {}

/**
 * Overload QDataSource::readyToSend() and returns the number
 * of bytes ready to send if not eof instead of returning -1.
*/
int ImageSource::readyToSend()
{
  int n = buffer.size() - pos;

  if ( !n && !eof )
    return n;

  return n ? n : -1;
}

/*!
  ImageSource's is rewindable.
*/
bool ImageSource::rewindable() const
{
    return TRUE;
}

/*!
  Enables rewinding.  No special action is taken.
*/
void ImageSource::enableRewind(bool on)
{
    rew = on;
}

/*
  Calls reset() on the QIODevice.
*/
void ImageSource::rewind()
{
  pos = 0;
  if (!rew) {
    QDataSource::rewind();
  } else
    ready();
}

/*!
  Reads and sends a block of data.
*/
void ImageSource::sendTo(QDataSink* sink, int n)
{
  sink->receive((const uchar*)&buffer.at(pos), n);

  pos += n;
}

/**
 * Sets the EOF state.
 */
void ImageSource::setEOF( bool state )
{
  eof = state;
}

// -------------------------------------------------------------------------------------

CachedImage::CachedImage(const DOMString &url, const DOMString &baseURL, bool reload)
    : QObject(), CachedObject(url, Image, reload)
{
    p = 0;
    m = 0;
    bg = 0;
    typeChecked = false;
    formatType = 0;
    m_status = Unknown;
    m_size = 0;
    imgSource = 0;
    gotFrame = false;
    m_baseURL = baseURL;

    if ( Cache::autoloadImages() )
      load();
}

CachedImage::~CachedImage()
{
    if( m ) delete m;
    if( p ) delete p;
    if (bg ) delete bg;

#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "CachedImage::~CachedImage() " << url().string() << endl;
#endif
}

void CachedImage::ref( CachedObjectClient *c )
{
    // make sure we don't get it twice...
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "CachedImage::ref() " << endl;
#endif
    m_clients.remove(c);
    m_clients.append(c);

    if( m_status != Pending || m )
	notify( c );
}

void CachedImage::deref( CachedObjectClient *c )
{
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "CachedImage::deref() " << endl;
#endif
    m_clients.remove( c );
    if(m && m_clients.isEmpty() && m->running())
	m->pause();

    if ( canDelete() && m_free )
      delete this;
}

static bool
fixBackground(QPixmap &bgPixmap)
{
#define BGMINWIDTH	50
#define BGMINHEIGHT	25
   if (bgPixmap.isNull())
       return false;
   int w = bgPixmap.width();
   int h = bgPixmap.height();
   if (w < BGMINWIDTH)
   {
       int factor = ((BGMINWIDTH+w-1) / w);
       QPixmap newPixmap(w * factor, h);
       QPainter p;
       p.begin(&newPixmap);
       for(int i=0; i < factor; i++)
           p.drawPixmap(i*w, 0, bgPixmap);
       p.end();
       const QBitmap *mask = bgPixmap.mask();
       if (mask)
       {
          QBitmap newBitmap( w * factor, h);
          p.begin(&newBitmap);
          for(int i=0; i < factor; i++)
             p.drawPixmap(i*w, 0, *mask);
          p.end();
          newPixmap.setMask(newBitmap);
       }
       bgPixmap = newPixmap;
       w = w * factor;
   }
#ifdef CACHE_DEBUG
   else {
       kdDebug( 6060 ) << "Not scaling in X-dir" << endl;
   }
#endif
   if (h < BGMINHEIGHT)
   {
       int factor = ((BGMINHEIGHT+h-1) / h);
       QPixmap newPixmap(w, h*factor);
       QPainter p;
       p.begin(&newPixmap);
       for(int i=0; i < factor; i++)
          p.drawPixmap(0, i*h, bgPixmap);
       p.end();
       const QBitmap *mask = bgPixmap.mask();
       if (mask)
       {
          QBitmap newBitmap( w, h*factor);
          p.begin(&newBitmap);
          for(int i=0; i < factor; i++)
             p.drawPixmap(0, i*h, *mask);
          p.end();
          newPixmap.setMask(newBitmap);
       }
       bgPixmap = newPixmap;
       h = h * factor;
   }
#ifdef CACHE_DEBUG
   else {
       kdDebug( 6060 ) << "Not scaling in Y-dir" << endl;
   }
#endif
   return true;
}


const QPixmap &CachedImage::tiled_pixmap() const
{
    const QPixmap &r = pixmap();

    if (r.isNull()) return r;

    if (bg)
       return *bg;

    if ((r.width() < BGMINWIDTH) || (r.height() < BGMINHEIGHT))
    {
       delete bg;
       bg = new QPixmap(r);
       fixBackground(*bg);
       return *bg;
    }
    return r;
}

const QPixmap &CachedImage::pixmap( ) const
{
    return m ? m->framePixmap() : ( p ? *p : *Cache::nullPixmap );
}

void CachedImage::notify( CachedObjectClient *c )
{
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "Cache::notify()" << endl;
#endif

    if ( m )
    {
	if(m->finished())
	    m->restart();
	if(m->paused())
	    m->unpause();
    }

    if( c )
    {
	// sanity check...
	if( m_clients.find( c ) == -1 )
	    m_clients.append( c );

	if ( m )
	  c->setPixmap( m->framePixmap(), this );
	else if ( p != 0 && !p->isNull() )
	  c->setPixmap( *p, this );

	return;
    }

    // notify all objects in our list...
    QPixmap pixmap;

    if ( m )
      pixmap = m->framePixmap();
    else if ( p != 0 && !p->isNull() )
      pixmap = *p;

    if ( !pixmap.isNull() )
    {
	QList<CachedObjectClient> updateList;
	CachedObjectClient *c;
        for ( c = m_clients.first(); c != 0; c = m_clients.next() ) {
#ifdef CACHE_DEBUG	
	    kdDebug( 6060 ) << "CachedImage: manually setting pixmap (" << c << ")" << endl;
#endif
            bool manualUpdate = false; // set the pixmap, dont update yet.
	    c->setPixmap( pixmap, this, &manualUpdate );
            if (manualUpdate)
               updateList.append(c);
	}
        for ( c = updateList.first(); c != 0; c = updateList.next() ) {
            bool manualUpdate = true; // Update!
            // Actually we want to do c->updateSize()
            // This is a terrible hack which does the same.
            // updateSize() does not exist in CachecObjectClient only
            // in RenderBox()
	    c->setPixmap( pixmap, this, &manualUpdate );
	}
    }
}

void CachedImage::movieUpdated( const QRect & )
{
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "Cache::movieUpdated()" << endl;
#endif
    QPixmap pixmap = m->framePixmap();
    QList<CachedObjectClient> updateList;
    CachedObjectClient *c;
    for ( c = m_clients.first(); c != 0; c = m_clients.next() ) {
#ifdef CACHE_DEBUG	
        kdDebug( 6060 ) << "CachedImage: manually setting pixmap (" << c << ")" << endl;
#endif
        bool manualUpdate = false; // set the pixmap, dont update yet.
        c->setPixmap( pixmap, this, &manualUpdate );
        if (manualUpdate)
           updateList.append(c);
    }
    for ( c = updateList.first(); c != 0; c = updateList.next() ) {
        bool manualUpdate = true; // Update!
        // Actually we want to do c->updateSize()
        // This is a terrible hack which does the same.
        // updateSize() does not exist in CachecObjectClient only
        // in RenderBox()
        c->setPixmap( pixmap, this, &manualUpdate );
   }
}

void CachedImage::clear()
{
    if( m ) {
	delete m;
	m = 0;
    }
    if( p ) {
	delete p;
	p = 0;
    }

    formatType = 0;

    typeChecked = false;
    m_size = 0;

    // No need to delete imageSource - QMovie does it for us
    imgSource = 0;
    gotFrame = false;
}

void CachedImage::data ( QBuffer &_buffer, bool eof )
{
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "in CachedImage::data()" << endl;
#endif
    if ( !typeChecked )
    {
	clear();
	formatType = QImageDecoder::formatName( (const uchar*)_buffer.buffer().data(), _buffer.size());
	typeChecked = true;
	
	if ( formatType )  // movie format exists
	{
	    imgSource = new ImageSource( _buffer.buffer() );
	    m = new QMovie( imgSource );
 	    m->connectUpdate( this, SLOT( movieUpdated( const QRect &) ));
	    gotFrame = false;
	    if(eof) computeStatus();
	    return;
	}
    }

    if ( !eof )
    {
	if ( imgSource )
	    imgSource->maybeReady();

	return;
    }

    if( !formatType )
    {
	p = new QPixmap();
	p->loadFromData( _buffer.buffer() );	
	// set size of image.
	if( p && !p->isNull() )
	    m_size = p->width() * p->height() * p->depth() / 8;

	notify(); // Notify only if we have a pixmap. Movies notifies itself via movieUpdated.
    }
    else
    {
	m_size = _buffer.size();
	if ( imgSource )
	{
	    imgSource->maybeReady();
	    imgSource->setEOF( true );
	}
	notify();
    }

    computeStatus();
    return;
}

void CachedImage::error( int /*err*/, const char */*text*/ )
{
#ifdef CACHE_DEBUG
    kdDebug(6060) << "CahcedImage::error" << endl;
#endif
    p = 0;

    notify();
}

void CachedImage::load()
{
  Cache::loader()->load(this, m_baseURL, true);
}


// ------------------------------------------------------------------------------------------

Request::Request(CachedObject *_object, const DOM::DOMString &baseURL, bool _incremental)
{
    object = _object;
    object->setRequest(this);
    incremental = _incremental;
    m_baseURL = baseURL;
}

Request::~Request()
{
    object->setRequest(0);
}

// ------------------------------------------------------------------------------------------

DocLoader::DocLoader()
{
  reloading = false;
}

DocLoader::~DocLoader()
{

}

CachedImage *DocLoader::requestImage( const DOM::DOMString &url, const DOM::DOMString &baseUrl)
{
    if (reloading) {
	QString fullURL = Cache::completeURL( url, baseUrl ).url();
	if (!reloadedURLs.contains(fullURL)) {
	    CachedObject *existing = Cache::cache->find(fullURL);
	    if (existing)
		Cache::removeCacheEntry(existing);
	    return Cache::requestImage(url,baseUrl,true);
	}
    }

    return Cache::requestImage(url,baseUrl,false);
}

CachedCSSStyleSheet *DocLoader::requestStyleSheet( const DOM::DOMString &url, const DOM::DOMString &baseUrl)
{
    if (reloading) {
	QString fullURL = Cache::completeURL( url, baseUrl ).url();
	if (!reloadedURLs.contains(fullURL)) {
	    CachedObject *existing = Cache::cache->find(fullURL);
	    if (existing)
		Cache::removeCacheEntry(existing);
	    return Cache::requestStyleSheet(url,baseUrl,true);
	}
    }

    return Cache::requestStyleSheet(url,baseUrl,false);
}

CachedScript *DocLoader::requestScript( const DOM::DOMString &url, const DOM::DOMString &baseUrl)
{
    if (reloading) {
	QString fullURL = Cache::completeURL( url, baseUrl ).url();
	if (!reloadedURLs.contains(fullURL)) {
	    CachedObject *existing = Cache::cache->find(fullURL);
	    if (existing)
		Cache::removeCacheEntry(existing);
	    return Cache::requestScript(url,baseUrl,true);
	}
    }

    return Cache::requestScript(url,baseUrl,false);
}


// ------------------------------------------------------------------------------------------

Loader::Loader() : QObject()
{
}

Loader::~Loader()
{
  m_requestsPending.setAutoDelete( true );
  m_requestsLoading.setAutoDelete( true );

  m_requestsPending.clear();
  m_requestsLoading.clear();
}

void Loader::load(CachedObject *object, const DOMString &baseURL, bool incremental)
{
    Request *req = new Request(object, baseURL, incremental);
    m_requestsPending.append(req);

    servePendingRequests();
}

void Loader::servePendingRequests()
{
  if ( m_requestsLoading.count() >= MAX_REQUEST_JOBS )
    return;
  if ( m_requestsPending.count() == 0 )
      return;

  // get the first pending request
  Request *req = m_requestsPending.take(0);

#ifdef CACHE_DEBUG
  kdDebug( 6060 ) << "starting Loader url=" << req->object->url().string() << endl;
#endif

  KIO::Job* job = KIO::get( req->object->url().string(), req->object->reload(), false /*no GUI*/);

  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotFinished( KIO::Job * ) ) );
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray &)),
           SLOT( slotData( KIO::Job*, const QByteArray &)));

  m_requestsLoading.insert(job, req);
}

void Loader::slotFinished( KIO::Job* job )
{
  Request *r = m_requestsLoading.take( job );

  if ( !r )
    return;

  if (job->error())
    r->object->error( job->error(), job->errorText() );
  else {
    r->object->data(r->m_buffer, true);

#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "Loader:: JOB FINISHED " << r->object->url().string() << endl;
#endif

    servePendingRequests();
  }

  emit requestDone( r->m_baseURL, r->object );

  delete r;
}

void Loader::slotData( KIO::Job*job, const QByteArray &data )
{
    Request *r = m_requestsLoading[job];
    if(!r) return;
    if ( !r->m_buffer.isOpen() )
	r->m_buffer.open( IO_WriteOnly );
    r->m_buffer.writeBlock( data.data(), data.size() );

    if(r->incremental)
	r->object->data( r->m_buffer, false );
}

int Loader::numRequests( const DOMString &baseURL )
{
  int res = 0;

  QListIterator<Request> pIt( m_requestsPending );
  for (; pIt.current(); ++pIt )
    if ( pIt.current()->m_baseURL == baseURL )
      res++;

  QPtrDictIterator<Request> lIt( m_requestsLoading );
  for (; lIt.current(); ++lIt )
    if ( lIt.current()->m_baseURL == baseURL )
      res++;

  return res;
}

int Loader::numRequests( const DOMString &baseURL, CachedObject::Type type )
{
  int res = 0;

  QListIterator<Request> pIt( m_requestsPending );
  for (; pIt.current(); ++pIt )
    if ( pIt.current()->m_baseURL == baseURL && pIt.current()->object->type() == type )
      res++;

  QPtrDictIterator<Request> lIt( m_requestsLoading );
  for (; lIt.current(); ++lIt )
    if ( lIt.current()->m_baseURL == baseURL && pIt.current()->object->type() == type )
      res++;

  return res;
}

void Loader::cancelRequests( const DOMString &baseURL )
{
    //kdDebug( 6060 ) << "void Loader::cancelRequests( " << baseURL.string() << " )" << endl;

    //kdDebug( 6060 ) << "got " << m_requestsPending.count() << " pending requests" << endl;

  QListIterator<Request> pIt( m_requestsPending );
  while ( pIt.current() )
  {
    if ( pIt.current()->m_baseURL == baseURL )
    {
	//kdDebug( 6060 ) << "cancelling pending request for " << pIt.current()->object->url().string() << endl;

      Cache::removeCacheEntry( pIt.current()->object );

      m_requestsPending.remove( pIt );
    }
    else
      ++pIt;
  }

  //kdDebug( 6060 ) << "got " << m_requestsLoading.count() << "loading requests" << endl;

  QPtrDictIterator<Request> lIt( m_requestsLoading );
  while ( lIt.current() )
  {
    if ( lIt.current()->m_baseURL == baseURL )
    {
	//kdDebug( 6060 ) << "cancelling loading request for " << lIt.current()->object->url().string() << endl;

      KIO::Job *job = static_cast<KIO::Job *>( lIt.currentKey() );

      Cache::removeCacheEntry( lIt.current()->object );

      m_requestsLoading.remove( lIt.currentKey() );

      job->kill();
    }
    else
      ++lIt;
  }
}

// ----------------------------------------------------------------------------

QDict<CachedObject> *Cache::cache = 0;
Cache::LRUList *Cache::lru = 0;
Loader *Cache::m_loader = 0;

int Cache::maxSize = DEFCACHESIZE;
int Cache::actSize = 0;

QPixmap *Cache::nullPixmap = 0;

bool Cache::s_autoloadImages = true;

unsigned long Cache::s_ulRefCnt = 0;

void Cache::ref()
{
  s_ulRefCnt++;
  init();
}

void Cache::deref()
{
  s_ulRefCnt--;
  if ( s_ulRefCnt == 0 )
    clear();
}

void Cache::init()
{
    if(!cache)
    {
	cache = new QDict<CachedObject>(401, false);
	cache->setAutoDelete(true);
    }
    if(!lru)
    {
	lru = new LRUList;
    }
    if(!nullPixmap)
	nullPixmap = new QPixmap;

    if(!m_loader)
	m_loader = new Loader();
}

void Cache::clear()
{
    //kdDebug( 6060 ) << "Cache::clear()" << endl;
    if(cache) delete cache;
    cache = 0;
    if(lru) delete lru;
    lru = 0;

    if ( nullPixmap )
      delete nullPixmap;
    nullPixmap = 0;

    if ( m_loader )
      delete m_loader;

    m_loader = 0;
}

CachedImage *Cache::requestImage( const DOMString & url, const DOMString &baseUrl, bool reload )
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
#ifdef CACHE_DEBUG
      kdDebug( 6060 ) << "Cache: Malformed url: " << kurl.url() << endl;
#endif
      return 0;
    }

    CachedObject *o = 0;
    if (!reload)
	o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
	kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
	CachedImage *im = new CachedImage(kurl.url(), baseUrl, reload);
	cache->insert( kurl.url(), im );
	lru->append( kurl.url() );
	return im;
    }

    if(!o->type() == CachedObject::Image)
    {
#ifdef CACHE_DEBUG
	kdDebug( 6060 ) << "Cache::Internal Error in requestImage url=" << kurl.url() << "!" << endl;
#endif
	return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
	kdDebug( 6060 ) << "Cache: loading in progress: " << kurl.url() << endl;
    else
	kdDebug( 6060 ) << "Cache: using cached: " << kurl.url() << endl;
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedImage *>(o);
}

CachedCSSStyleSheet *Cache::requestStyleSheet( const DOMString & url, const DOMString &baseUrl, bool reload)
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
      kdDebug( 6060 ) << "Cache: Malformed url: " << kurl.url() << endl;
      return 0;
    }

    CachedObject *o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
	kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
	CachedCSSStyleSheet *sheet = new CachedCSSStyleSheet(kurl.url(), baseUrl, reload);
	cache->insert( kurl.url(), sheet );
	lru->append( kurl.url() );
	return sheet;
    }

    if(!o->type() == CachedObject::CSSStyleSheet)
    {
#ifdef CACHE_DEBUG
	kdDebug( 6060 ) << "Cache::Internal Error in requestStyleSheet url=" << kurl.url() << "!" << endl;
#endif
	return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
	kdDebug( 6060 ) << "Cache: loading in progress: " << kurl.url() << endl;
    else
	kdDebug( 6060 ) << "Cache: using cached: " << kurl.url() << endl;
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedCSSStyleSheet *>(o);
}

CachedScript *Cache::requestScript( const DOM::DOMString &url, const DOM::DOMString &baseUrl, bool reload)
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
      kdDebug( 6060 ) << "Cache: Malformed url: " << kurl.url() << endl;
      return 0;
    }

    CachedObject *o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
	kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
	CachedScript *script = new CachedScript(kurl.url(), baseUrl, reload);
	cache->insert( kurl.url(), script );
	lru->append( kurl.url() );
	return script;
    }

    if(!o->type() == CachedObject::Script)
    {
#ifdef CACHE_DEBUG
	kdDebug( 6060 ) << "Cache::Internal Error in requestScript url=" << kurl.url() << "!" << endl;
#endif
	return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
	kdDebug( 6060 ) << "Cache: loading in progress: " << kurl.url() << endl;
    else
	kdDebug( 6060 ) << "Cache: using cached: " << kurl.url() << endl;
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedScript *>(o);
}

void Cache::flush()
{
    CachedObject *o;
    QString url;

    init();

#ifdef CACHE_DEBUG
    statistics();
    kdDebug( 6060 ) << "Cache: flush()" << endl;
#endif
    if( actSize < maxSize ) return;

    for ( QStringList::Iterator it = lru->begin(); it != lru->end(); ++it )
    {
	o = cache->find( *it );
	if( !o->canDelete() || o->status() == CachedObject::Persistent )
	    continue; // image is still used or cached permanently

#ifdef CACHE_DEBUG
	kdDebug( 6060 ) << "Cache: removing " << url << endl;
#endif
	actSize -= o->size();
	lru->remove( url );
	cache->remove( url );
	if( actSize < maxSize ) break;
    }

#ifdef CACHE_DEBUG
    statistics();
#endif
}

void Cache::setSize( int bytes )
{
    maxSize = bytes;
    // may be we need to clear parts of the cache
    flush();
}

void Cache::statistics()
{
    CachedObject *o;
    // this function is for debugging purposes only
    init();

    int size = 0;
    int msize = 0;
    int movie = 0;
    int stylesheets = 0;
    QDictIterator<CachedObject> it(*cache);
    for(it.toFirst(); it.current(); ++it)
    {
        o = it.current();
	if(o->type() == CachedObject::Image)	
	{
	    CachedImage *im = static_cast<CachedImage *>(o);
	    if(im->m != 0)
	    {
		movie++;
		msize += im->size();
	    }
	}
	else
	{
	    if(o->type() == CachedObject::CSSStyleSheet)
		stylesheets++;
	
	    size += o->size();
	}
    }
    size /= 1024;

    kdDebug( 6060 ) << "------------------------- image cache statistics -------------------" << endl;
    kdDebug( 6060 ) << "Number of items in cache: " << cache->count() << endl;
    kdDebug( 6060 ) << "Number of items in lru  : " << lru->count() << endl;
    kdDebug( 6060 ) << "Number of cached images: " << cache->count()-movie << endl;
    kdDebug( 6060 ) << "Number of cached movies: " << movie << endl;
    kdDebug( 6060 ) << "Number of cached stylesheets: " << stylesheets << endl;
    kdDebug( 6060 ) << "calculated allocated space approx. " << actSize/1024 << " kB" << endl;
    kdDebug( 6060 ) << "pixmaps:   allocated space approx. " << size << " kB" << endl;
    kdDebug( 6060 ) << "movies :   allocated space approx. " << msize/1024 << " kB" << endl;
    kdDebug( 6060 ) << "--------------------------------------------------------------------" << endl;
}

KURL Cache::completeURL( const DOMString &_url, const DOMString &_baseUrl )
{
    QString url = _url.string();
    QString baseUrl = _baseUrl.string();
    KURL orig(baseUrl);
    if(_url[(unsigned int)0] != '/')
    {
	KURL u( orig, url );
	return u;
    }	
    orig.setEncodedPathAndQuery(url);
    return orig;
}

void Cache::removeCacheEntry( CachedObject *object )
{
  QString key = object->url().string();

  // this indicates the deref() method of CachedObject to delete itself when the reference counter
  // drops down to zero
  object->setFree( true );

  // if the object is still referenced, then don't really kill it but let it get killed
  // when its reference counter drops down to zero
  if ( !object->canDelete() )
  {
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "cache object for " << key << " is still referenced. Killing it softly..." << endl;
#endif
    cache->setAutoDelete( false );
  }


  if ( cache->remove( key ) )
  {
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "removed cache entry for " << key << " from cache dict" << endl;
#endif
   }

  cache->setAutoDelete( true );

  LRUList::Iterator it = lru->find( key );
  if ( it != lru->end() )
  {
    lru->remove( it );
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "removed cache entry for " << key << " from lru" << endl;
#endif
  }
}

void Cache::autoloadImages( bool enable )
{
  if ( enable == s_autoloadImages )
    return;

  s_autoloadImages = enable;

  QDictIterator<CachedObject> it( *cache );
  for (; it.current(); ++it )
    if ( it.current()->type() == CachedObject::Image )
    {
      CachedImage *img = static_cast<CachedImage *>( it.current() );

      CachedObject::Status status = img->status();
      if ( status != CachedObject::Unknown ||
	   status == CachedObject::Cached ||
	   status == CachedObject::Uncacheable ||
	   status == CachedObject::Pending )
        continue;
	
      img->load();
    }
}

bool Cache::autoloadImages()
{
  return s_autoloadImages;
}

#include "loader.moc"
