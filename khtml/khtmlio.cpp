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
*/  
// ------------------------------------------------------------------------
//
// Provides io classes for khtml
// Provides a cache for effective caching of images in memory
//
// $Id$

#include "khtmlio.h"
#include "khtml.h"

//#undef CACHE_DEBUG
#define CACHE_DEBUG

#include <qpixmap.h>
#include <qmovie.h>
#include <qdict.h>
#include <qasyncimageio.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>  

HTMLPendingFile::HTMLPendingFile()
{
}

HTMLPendingFile::HTMLPendingFile( QString _url, HTMLFileRequester *_obj )
{
  m_strURL = _url;
  m_lstClients.append( _obj );
}

/*!
  This Class defines the DataSource for incremental loading of images.
*/
KHTMLImageSource::KHTMLImageSource(QByteArray buf)
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
KHTMLImageSource::~KHTMLImageSource() {}

/**
 * Overload QDataSource::readyToSend() and returns the number
 * of bytes ready to send if not eof instead of returning -1.
*/
int KHTMLImageSource::readyToSend()
{
  int n = buffer.size() - pos;

  if ( !n && !eof )
    return n;

  return n ? n : -1;
}

/*!
  KHTMLImageSource's is rewindable.
*/
bool KHTMLImageSource::rewindable() const
{
    return TRUE;
}

/*!
  Enables rewinding.  No special action is taken.
*/
void KHTMLImageSource::enableRewind(bool on)
{
    rew = on;
}

/*
  Calls reset() on the QIODevice.
*/
void KHTMLImageSource::rewind()
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
void KHTMLImageSource::sendTo(QDataSink* sink, int n)
{
  sink->receive((const uchar*)&buffer.at(pos), n);
  
  pos += n;
}

/**
 * Sets the EOF state.
 */
void KHTMLImageSource::setEOF( bool state )
{
  eof = state;
}

KHTMLCachedImage::KHTMLCachedImage()
    : QObject()
{
    p = 0;
    m = 0;
    typeChecked = false;
    formatType = 0;
    status = KHTMLCache::Unknown;
    size = 0;
    imgSource = 0;
}

KHTMLCachedImage::~KHTMLCachedImage()
{
    if( m ) delete m;
    if( p ) delete p;
}

void 
KHTMLCachedImage::append( HTMLImageRequester *o ) 
{ 
    clients.append( o ); 
    if( status != KHTMLCache::Pending || m )
	notify( o );
}

void 
KHTMLCachedImage::remove( HTMLImageRequester *o ) 
{ 
  clients.remove( o ); 
  if(m && clients.isEmpty() && m->running())
    m->pause();
}

QPixmap*
KHTMLCachedImage::pixmap() 
{
    return m ? (QPixmap*)&m->framePixmap() : p;
}

void
KHTMLCachedImage::computeStatus()
{
    if( size > MAXCACHEABLE || size > KHTMLCache::size()/MAXPERCENT )
    {
	status = KHTMLCache::Uncacheable;
	size = 0;
    }
    else
	status = KHTMLCache::Cached;
}

void 
KHTMLCachedImage::clear()
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
    size = 0;

    // No need to delete imageSource - QMovie does it for us
    imgSource = 0;
}

void
KHTMLCachedImage::load( QString _f )
{
    printf("in KHTMLCache::load()\n");

    clear();

    // remove "file:" prefix
    if(_f.find("file:") == 0)
	_f = _f.mid(5);

    const char *_file = _f.ascii();
    // Workaround for bug in QMovie
    // Load the image in memory to avoid vasting file handles
    struct stat buff;
    stat( _file, &buff );
    int s = buff.st_size;
    FILE *f = fopen( _file, "rb" );
    if( !f )
    {
	warning( "Cache: Could not load %s\n", _file );
	return;
    }
    char *c = new char[ s ];
    fread( c, 1, s, f );
    fclose( f );
    QByteArray arr;
    arr.assign( c, s );
    
    formatType = QImageDecoder::formatName( (const uchar*)arr.data(), arr.size());
    printf("KHTMLCache: image file %s, format %s\n",_file, formatType);
    if ( formatType )
      {
	// set width and height of image
	width  = (uchar)c[6] + ((uchar)c[7]<<8);
	height = (uchar)c[8] + ((uchar)c[9]<<8);
	
	m = new QMovie( arr, 8192 );
	m->connectUpdate( this, SLOT( movieUpdated( const QRect &) ));
	gotFrame = false;
	// well, this is the size of the compressed gif...
	// ... but that's better than nothing
	size = s;
      }
    else
      {
	p = new QPixmap();
	p->loadFromData( arr );	    
	// set size of image. 
	if( p != 0 && !p->isNull() )
	    size = p->width() * p->height() * p->depth() / 8;

	computeStatus();
	notify();
      }
}    

bool
KHTMLCachedImage::data ( QBuffer & _buffer, bool eof )
{
    printf("in KHTMLCache::data()\n");
    if ( !typeChecked )
      {
	clear();
	formatType = QImageDecoder::formatName( (const uchar*)_buffer.buffer().data(), _buffer.size());
	typeChecked = true;
	
	if ( formatType )  // movie format exists
	  {
	    imgSource = new KHTMLImageSource( _buffer.buffer() );
	    m = new QMovie( imgSource );
 	    m->connectUpdate( this, SLOT( movieUpdated( const QRect &) ));
	    gotFrame = false;
	    if(eof) computeStatus();
	    return eof;
	  }
      }
    
    if ( !eof )
      {
	if ( imgSource )
	  imgSource->maybeReady();

	return false;
      }

    if( !formatType )
      {
	p = new QPixmap();
	p->loadFromData( _buffer.buffer() );	    
	// set size of image. 
	if( p && !p->isNull() )
	  size = p->width() * p->height() * p->depth() / 8;

	notify(); // Notify only if we have a pixmap. Movies notifies itself via movieUpdated.
      }
    else
      {
	size = _buffer.size();
	if ( imgSource )
	  {
	    imgSource->setEOF( true );
	    imgSource->maybeReady();
	  }
      }

    computeStatus();
    return true;
}

void 
KHTMLCachedImage::notify( HTMLImageRequester *o )
{
    printf("Cache::notify()\n");

    if ( m )
    {
	if(m->finished())
	    m->restart();
	if(m->paused()) 
	    m->unpause();
    }

    if( o )
    {
	// sanity check...
	if( clients.find( o ) == -1 )
	    clients.append( o );

	if ( m )
	  o->setPixmap( (QPixmap*)&m->framePixmap() );
	else if ( p != 0 && !p->isNull() )
	  o->setPixmap( p );

	return;
    }

    // notify all objects in our list...
    QPixmap* pixmap = 0;

    if ( m )
      pixmap = (QPixmap*)&m->framePixmap();
    else if ( p != 0 && !p->isNull() )
      pixmap = p;

    if ( pixmap )
      for( o = clients.first(); o != 0L; o = clients.next() )
	o->setPixmap( pixmap );
}

void 
KHTMLCachedImage::movieUpdated( const QRect & )
{
    //printf("Cache::movieUpdated()\n");
    HTMLImageRequester *o;

    QPixmap* pixmap = (QPixmap*)&m->framePixmap();
    for( o = clients.first(); o != 0L; o = clients.next() )
	o->pixmapChanged( pixmap );
}

// --------------------------------------------------------------------------

QDict<KHTMLCachedImage> *KHTMLCache::cache = 0;
ImageList *KHTMLCache::lru = 0;

int KHTMLCache::maxSize = DEFCACHESIZE;
int KHTMLCache::actSize = 0;


KHTMLCache::KHTMLCache( KHTMLWidget *w )
{
    htmlWidget = w;
    init();
}

KHTMLCache::~KHTMLCache()
{
}

void
KHTMLCache::init()
{
    if(!cache)
    {
	cache = new QDict<KHTMLCachedImage>(401, false);
	cache->setAutoDelete(true);
    }
    if(!lru)
    {
	lru = new ImageList;
    }
}

void 
KHTMLCache::clear()
{
    if(cache) delete cache;
    cache = 0;
    if(lru) delete lru;
    lru = 0;
}

void
KHTMLCache::requestImage( HTMLImageRequester *obj, QString _url)
{
    // this brings the _url to a standard form...
    KURL kurl( _url );
    if( kurl.isMalformed() )
    {
      printf("Cache: Malformed url: %s\n", _url.latin1() );
      return;
    }
    KHTMLCachedImage *im = cache->find(kurl.url());
    if(!im)
    {
#ifdef CACHE_DEBUG
	printf("Cache: new: %s\n", _url.latin1());
#endif
	im = new KHTMLCachedImage();
	im->status = Pending;
	im->append(obj);
	cache->insert( kurl.url(), im );
	lru->append( kurl.url() );
	    
	if ( kurl.isLocalFile() )
	{
	    im->load( kurl.url() );
	    actSize += im->size;
	}
	else 
	    htmlWidget->requestFile( this, kurl.url() );
	return;
    }

#ifdef CACHE_DEBUG
    if( im->status == Pending )
	printf("Cache: loading in progress: %s\n", kurl.url().data());
    else
	printf("Cache: using cached: %s\n", kurl.url().data());
#endif

    im->append( obj );
    lru->touch( kurl.url() );
}

void
KHTMLCache::fileLoaded( QString _url, QString _file )
{ 
    KHTMLCachedImage *im = cache->find(_url);

    if(!im) 
    {
#ifdef CACHE_DEBUG
	printf("Cache: ERROR loading: %s not found.\n", _url.latin1());
#endif
	return;
    }

    // convert file to pixmap or movie
#ifdef CACHE_DEBUG
    printf("Cache: Loaded %s %s\n", _url.latin1(), _file.latin1() );
#endif
    
    im->load( _file );
    actSize += im->size;
}

bool
KHTMLCache::fileLoaded( QString _url, QBuffer & _buffer, bool eof )
{
#ifdef CACHE_DEBUG
  if ( eof )
    printf("FileLoaded %s\n",_url.latin1());
#endif

    KHTMLCachedImage *im = cache->find(_url);

    if(!im) return false;

#ifdef CACHE_DEBUG
  printf("Processing %s\n",_url.latin1());
#endif

  // convert file to pixmap or movie and store return state
  bool state = im->data( _buffer, eof );

  if( eof )
    actSize += im->size;
  
  return state;
}

void 
KHTMLCache::free( DOMString _url, HTMLImageRequester *obj )
{
    if(!_url.length()) return;
    KURL kurl( _url.string() );
    KHTMLCachedImage *im = cache->find( kurl.url() );

    if(!im) return;
    im->remove( obj );

#ifdef CACHE_DEBUG
    printf("Cache: references( %s ) = %d\n", _url.string().latin1(), 
	   im->count());
#endif

#if 0 // ### how to fix this???
    if(im->count() == 0 && (im->status == Pending ||
			    im->status == Uncacheable ) )
    {
	htmlWidget->cancelRequestFile( _url.string() );
	// remove, since it was not fully loaded
	lru->remove( kurl.url() );
	cache->remove( kurl.url() );
#ifdef CACHE_DEBUG
	printf("Cache: deleted %s\n", kurl.url().latin1());
#endif
    }
#endif
}

void
KHTMLCache::flush()
{
    KHTMLCachedImage *im;
    QString url;

    init();

#ifdef CACHE_DEBUG
    statistics();
    printf("Cache: flush()\n");
#endif
    if( actSize < maxSize ) return;
    
    for ( QStringList::Iterator it = lru->begin(); it != lru->end(); ++it )
    {
	im = cache->find( *it );
	if( im->count() || im->status == Persistent ) 
	    continue; // image is still used or cached permanently

#ifdef CACHE_DEBUG
	printf("Cache: removing %s\n", url.latin1());
#endif
	actSize -= im->size;
	lru->remove( url );
	cache->remove( url );
	if( actSize < maxSize ) break;
    }

#ifdef CACHE_DEBUG
    statistics();
#endif
}

void 
KHTMLCache::preload( QString _url, CacheStatus s)
{
    // better be careful, since this function is static
    init();
    
    KURL kurl( _url );
    if ( kurl.isMalformed() )
    {
	warning("Malformed URL '%s'\n", _url.latin1() );
	return;
    }	    
    if ( !kurl.isLocalFile() )
    {
#ifdef CACHE_DEBUG
	printf("Cache: image to cache is not local file %s\n", _url.latin1());
#endif
	return;
    }

    KHTMLCachedImage *im = cache->find(kurl.url());
    
    if(!im)
    {
#ifdef CACHE_DEBUG
	printf("Cache: *** new cached image %s\n", kurl.path().data());
#endif
	im = new KHTMLCachedImage();
	im->load( kurl.path() );
	actSize += im->size;
	if( s != Unknown )  // specific status required
	    im->status = s;
	cache->insert( kurl.url(), im );
	lru->append( kurl.url() );
    }
}


QPixmap *
KHTMLCache::image( QString _url )
{
    KHTMLCachedImage *im = cache->find(_url);

    if(!im)
    {
#ifdef CACHE_DEBUG
	printf("CACHE: Image not cached: %s\n", _url.latin1());
#endif
	return 0;
    }

    return im->pixmap();
}

void 
KHTMLCache::setSize( int bytes )
{
    maxSize = bytes;
    // may be we need to clear parts of the cache
    flush();
}

void 
KHTMLCache::statistics()
{
  KHTMLCachedImage *im;
    // this function is for debugging purposes only
    init();

    int size = 0;
    int msize = 0;
    int movie = 0;
    QDictIterator<KHTMLCachedImage> it(*cache);
    for(it.toFirst(); it.current(); ++it)
    {
        im = it.current();
	if(im->m != 0)
	  {
	    movie++;
	    msize += im->size;
	  }
	else
	  size += im->size;
    }
    size /= 1024;

    printf("------------------------- image cache statistics -------------------\n");
    printf("Number of items in cache: %d\n", cache->count() );
    printf("Number of items in lru  : %d\n", lru->count() );
    printf("Number of cached images: %d\n", cache->count()-movie);
    printf("Number of cached movies: %d\n", movie);
    printf("calculated allocated space approx. %d kB\n", actSize/1024);
    printf("pixmaps:   allocated space approx. %d kB\n", size);
    printf("movies :   allocated space approx. %d kB\n", msize/1024);
    printf("--------------------------------------------------------------------\n");
}
#include "khtmlio.moc"
