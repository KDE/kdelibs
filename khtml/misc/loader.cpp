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

#include <qtextstream.h>
#include <qasyncio.h>
#include <qasyncimageio.h>

#include <kio/job.h>
#include <kdebug.h>

#include "css/css_stylesheetimpl.h"
using namespace khtml;
using namespace DOM;

#include <stdio.h>

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


// -------------------------------------------------------------------------------------------

CachedCSSStyleSheet::CachedCSSStyleSheet(const DOMString &url, const DOMString &baseURL)
    : CachedObject(url, CSSStyleSheet)
{
    // load the file
    Cache::loader()->load(this, baseURL, false);
    loading = true;
}

CachedCSSStyleSheet::~CachedCSSStyleSheet()
{
  kdDebug( 300 ) << "CachedCSSStyleSheet::~CachedCSSStyleSheet() " << url().string() << endl;
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
    if ( m_clients.count() == 0 && m_free )
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

    printf("CachedCSSStyleSheet:: finishedLoading %s\n", m_url.string().ascii());

    CachedObjectClient *c;
    for ( c = m_clients.first(); c != 0; c = m_clients.next() )
	c->setStyleSheet( m_url, m_sheet );
}


void CachedCSSStyleSheet::error( int /*err*/, const char */*text*/ )
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

CachedImage::CachedImage(const DOMString &url, const DOMString &baseURL)
    : QObject(), CachedObject(url, Image)
{
    p = 0;
    m = 0;
    typeChecked = false;
    formatType = 0;
    m_status = Unknown;
    m_size = 0;
    imgSource = 0;
    gotFrame = false;

    Cache::loader()->load(this, baseURL, true);
}

CachedImage::~CachedImage()
{
    if( m ) delete m;
    if( p ) delete p;
    kdDebug( 300 ) << "CachedImage::~CachedImage() " << url().string() << endl;
}

void CachedImage::ref( CachedObjectClient *c )
{
    // make sure we don't get it twice...
    m_clients.remove(c);
    m_clients.append(c);

    if( m_status != Pending || m )
	notify( c );
}

void CachedImage::deref( CachedObjectClient *c )
{
    m_clients.remove( c );
    if(m && m_clients.isEmpty() && m->running())
	m->pause();
    
    if ( m_clients.count() == 0 && m_free )
      delete this;
}

const QPixmap &CachedImage::pixmap() const
{
    return m ? m->framePixmap() : ( p ? *p : *Cache::nullPixmap );
}

void CachedImage::notify( CachedObjectClient *c )
{
    printf("Cache::notify()\n");

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
	  c->setPixmap( m->framePixmap() );
	else if ( p != 0 && !p->isNull() )
	  c->setPixmap( *p );

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
	CachedObjectClient *c;
        for ( c = m_clients.first(); c != 0; c = m_clients.next() )
	    c->setPixmap( pixmap );
    }
}

void CachedImage::movieUpdated( const QRect & )
{
    //printf("Cache::movieUpdated()\n");
    QPixmap pixmap = m->framePixmap();
    CachedObjectClient *c;
    for ( c = m_clients.first(); c != 0; c = m_clients.next() )
	c->setPixmap( pixmap );
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
    printf("in CachedImage::data()\n");
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
	    imgSource->setEOF( true );
	    imgSource->maybeReady();
	}
    }

    computeStatus();
    return;
}

void CachedImage::error( int /*err*/, const char */*text*/ )
{
    p = 0;

    notify();
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

  printf("starting Loader url=%s\n", req->object->url().string().ascii());

  KIO::Job* job = KIO::get( req->object->url().string(), false);
  //job->setGUImode( KIOJob::NONE );

  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotFinished( KIO::Job * ) ) );
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray &)),
           SLOT( slotData( KIO::Job*, const QByteArray &)));

  m_requestsLoading.insert(job, req);
}

void Loader::slotFinished( KIO::Job* job )
{
  if (job->error())
  {
    Request *r = m_requestsLoading[job];
    if(!r) return;

    r->object->error( job->error(), job->errorText() );
  } else {
    Request *r = m_requestsLoading.take( job );
    if(!r) return;
    r->object->data(r->m_buffer, true);
    printf("Loader:: JOB FINISHED %s\n", r->object->url().string().ascii());

    servePendingRequests();
  }
  emit requestDone();
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

void Loader::cancelRequests( const DOMString &baseURL )
{
  kdDebug( 300 ) << "void Loader::cancelRequests( " << baseURL.string() << " )" << endl;
  
  kdDebug( 300 ) << "got " << m_requestsPending.count() << " pending requests" << endl;
  
  QListIterator<Request> pIt( m_requestsPending );
  while ( pIt.current() )
  {
    if ( pIt.current()->m_baseURL == baseURL )
    {
      kdDebug( 300 ) << "cancelling pending request for " << pIt.current()->object->url().string() << endl;
      
      Cache::removeCacheEntry( pIt.current()->object );
      
      m_requestsPending.remove( pIt );
    }
    else
      ++pIt;
  }
  
  kdDebug( 300 ) << "got " << m_requestsLoading.count() << "loading requests" << endl;
  
  QPtrDictIterator<Request> lIt( m_requestsLoading );
  while ( lIt.current() )
  {
    if ( lIt.current()->m_baseURL == baseURL )
    {
      kdDebug( 300 ) << "cancelling loading request for " << lIt.current()->object->url().string() << endl;
      
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
    kdDebug( 300 ) << "Cache::clear()" << endl; 
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

CachedImage *Cache::requestImage( const DOMString & url, const DOMString &baseUrl )
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
      printf("Cache: Malformed url: %s\n", kurl.url().latin1() );
      return 0;
    }

    CachedObject *o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
	printf("Cache: new: %s\n", kurl.url().latin1());
#endif
	CachedImage *im = new CachedImage(kurl.url(), baseUrl);
	cache->insert( kurl.url(), im );
	lru->append( kurl.url() );
	return im;
    }

    if(!o->type() == CachedObject::Image)
    {
	printf("Cache::Internal Error in requestImage url=%s!\n", kurl.url().ascii());
	return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
	printf("Cache: loading in progress: %s\n", kurl.url().data());
    else
	printf("Cache: using cached: %s\n", kurl.url().data());
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedImage *>(o);
}

CachedCSSStyleSheet *Cache::requestStyleSheet( const DOMString & url, const DOMString &baseUrl)
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
      printf("Cache: Malformed url: %s\n", kurl.url().latin1() );
      return 0;
    }

    CachedObject *o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
	printf("Cache: new: %s\n", kurl.url().latin1());
#endif
	CachedCSSStyleSheet *sheet = new CachedCSSStyleSheet(kurl.url(), baseUrl);
	cache->insert( kurl.url(), sheet );
	lru->append( kurl.url() );
	return sheet;
    }

    if(!o->type() == CachedObject::CSSStyleSheet)
    {
	printf("Cache::Internal Error in requestStyleSheet url=%s!\n", kurl.url().ascii());
	return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
	printf("Cache: loading in progress: %s\n", kurl.url().data());
    else
	printf("Cache: using cached: %s\n", kurl.url().data());
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedCSSStyleSheet *>(o);
}

void Cache::flush()
{
    CachedObject *o;
    QString url;

    init();

#ifdef CACHE_DEBUG
    statistics();
    printf("Cache: flush()\n");
#endif
    if( actSize < maxSize ) return;

    for ( QStringList::Iterator it = lru->begin(); it != lru->end(); ++it )
    {
	o = cache->find( *it );
	if( o->count() || o->status() == CachedObject::Persistent )
	    continue; // image is still used or cached permanently

#ifdef CACHE_DEBUG
	printf("Cache: removing %s\n", url.latin1());
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

    printf("------------------------- image cache statistics -------------------\n");
    printf("Number of items in cache: %d\n", cache->count() );
    printf("Number of items in lru  : %d\n", lru->count() );
    printf("Number of cached images: %d\n", cache->count()-movie);
    printf("Number of cached movies: %d\n", movie);
    printf("Number of cached stylesheets: %d\n", stylesheets);
    printf("calculated allocated space approx. %d kB\n", actSize/1024);
    printf("pixmaps:   allocated space approx. %d kB\n", size);
    printf("movies :   allocated space approx. %d kB\n", msize/1024);
    printf("--------------------------------------------------------------------\n");
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
  if ( object->count() > 0 )
  {
    kdDebug( 300 ) << "cache object for " << key << " is still referenced. Killing it softly..." << endl;
    cache->setAutoDelete( false );
  }
  
  if ( cache->remove( key ) )
    kdDebug( 300 ) << "removed cache entry for " << key << " from cache dict" << endl;
  
  cache->setAutoDelete( true );
    
  LRUList::Iterator it = lru->find( key );
  if ( it != lru->end() )
  {
    lru->remove( it );
    kdDebug( 300 ) << "removed cache entry for " << key << " from lru" << endl;
  }
} 

#include "loader.moc"
