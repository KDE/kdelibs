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
// Provides a cache for effective caching of images in memory
//
// $Id$

#ifndef KHTMLIO_H
#define KHTMLIO_H

// up to which size is a picture for sure cacheable
#define MAXCACHEABLE 10*1024
// max. size of a single picture in percent of the total cache size
// to be cacheable
#define MAXPERCENT 10
// default cache size
#define DEFCACHESIZE 512*1024

class QPixmap;
class QMovie;
#include <qdict.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qasyncio.h>
#include <qbuffer.h>
#include <qstring.h>
#include <qlist.h>

#include "dom_string.h"
using namespace DOM;

class KHTMLWidget;

class HTMLFileRequester 
{
public:
    /********************************
     * These two functions are overloaded by objects that need to have a remote
     * file downloaded, e.g. HTMLMap.
     *
     * fileLoaded is called when the requested file has arrived.
     */
    virtual void fileLoaded( QString /*_url*/, 
			     QString /*localfile*/ ) = 0;
    virtual bool fileLoaded( QString /* _url */, QBuffer& /* _buffer */, 
			     bool /* eof */ = false ) = 0;
};

class HTMLImageRequester
{
public:
    virtual void setPixmap( QPixmap *p ) = 0;
    virtual void pixmapChanged( QPixmap *p ) = 0;
};

struct HTMLPendingFile
{
public:
  HTMLPendingFile();
  HTMLPendingFile( const QString _url, HTMLFileRequester *_obj );

  QBuffer m_buffer;
  QString m_strURL;
  QList<HTMLFileRequester> m_lstClients;
};

struct HTMLPageInfo
{
public:
  QStringList imageURLs;
  QStringList hrefURLs;
};

/**
 * Defines the DataSource for incremental loading of images.
 */
class KHTMLImageSource : public QDataSource
{
 public:
  KHTMLImageSource(QByteArray buf);
  ~KHTMLImageSource();
 
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

/** 
 * contains one cached image
 */
class KHTMLCachedImage : public QObject
{
    Q_OBJECT
public:

    KHTMLCachedImage();
    ~KHTMLCachedImage();

    // just for convenience
    void append( HTMLImageRequester *o ); 
    void remove( HTMLImageRequester *o );
    QPixmap* pixmap();
    int count() { return clients.count(); }

    /**
     * tell the HTMLImageRequesters, that the image is ready.
     * if o = 0  notify all objects
     */
    void notify(HTMLImageRequester *o = 0);

    /**
     * computes the status of an image after loading.
     * the result depends on the image size and the size of the cache
     */
    void computeStatus();

    /**
     * load a local file
     */
    void load( QString _filename );

    /**
     * update the object from the buffer
     */
    bool data( QBuffer &buffer, bool eof = false );

    void clear();

public slots:
    /**
     * gets called, whenever a QMovie changes frame
     */
    void movieUpdated( const QRect &rect );
 
public:
    QPixmap *p;
    QMovie *m;
    int width;
    int height;

    QList<HTMLImageRequester> clients;
    int status;
    int size;
    bool gotFrame;

private:
    // Is the name of the movie format type
    const char* formatType;

    // Is set if movie format type ( incremental/animation) was checked
    bool typeChecked;

    KHTMLImageSource* imgSource;
};

class ImageList : public QStringList
{
public:
    /**
     * implements the LRU list
     * The least recently used item is at the beginning of the list.
     */
    void touch( QString url )
    {
	remove( url );
	append( url );
    }
};

class KHTMLCache : public HTMLFileRequester
{
public:
    KHTMLCache( KHTMLWidget *w );
    virtual ~KHTMLCache();

    enum CacheStatus { NotCached,    // this URL is not cached
		  Unknown,      // let imagecache decide what to do with it
		  New,          // inserting new image
		  Persistent,   // never delete this pixmap
		  Cached,       // regular case
		  Pending,      // only partially loaded
		  Uncacheable };// to big to be cached,
                                // will be destroyed as soon as possible
    
    /**
     * called from the KHTMLWidget to say that new data has arrived
     * if eof is TRUE, the url is completely loaded.
     */
    virtual bool fileLoaded( QString _url, QBuffer &buffer, 
			     bool eof = false);
    /**
     * called from KHTMLWidget to say, that the image is loaded
     */
    virtual void fileLoaded( QString _url, QString localfile );

    /** 
     * if an htmlimage is destructed it calls
     * this function, to tell the cache, the image is not used
     * anymore.
     */
    static void free( DOMString _url, HTMLImageRequester *o = 0);

    /** 
     * this is called from the KHTMLWidget to indicate that a
     * HTMLImageRequester needs an Image
     */
    void requestImage( HTMLImageRequester *obj, QString _url );

    /** sets the status of an URL */
    static void setStatus( QString _url, CacheStatus s );
    /** returns the current status of an image */
    static int status( QString _url );

    /**
     * preload an image into the cache. 
     * Set Status to Persistent, if you don't wan't it to be 
     * removed in any case.
     */
    static void preload( QString _url, CacheStatus s = Unknown );

    /**
     * get the pixmap belonging to an url. 
     * @return 0 if the image is not cached.
     */
    static QPixmap *image( QString _url );

    /**
     * sets the size of the cache
     */
    static void setSize( int bytes );
    /** returns the size of the cache */
    static int size() { return maxSize; };

    /** prints some statistics to stdout */
    static void statistics();

    /** clean up cache */
    static void flush();

    /**
     * clears the cache
     * Warning: call this only at the end of your program, to clean
     * up memory (useful for finding memory holes)
     */
    static void clear();

protected:
    static void init();

    static QDict<KHTMLCachedImage> *cache;
    static ImageList *lru;

    static int maxSize;
    static int actSize;
    
    KHTMLWidget *htmlWidget;

};


#endif
