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
// Handles khtml's io, up to now mainly loading of files
// Provides a cache for effective caching of images in memory
//
// $Id$

#ifndef KHTMLIO_H
#define KHTMLIO_H
#include "khtmlobj.h"

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

class KHTMLWidget;

class KHTMLCachedImage
{
public:

    KHTMLCachedImage();
    ~KHTMLCachedImage();

    // just for convenience
    void append( HTMLObject *o ); 
    void remove( HTMLObject *o ) { clients.remove( o ); }
    QPixmap *pixmap();
    int count() { return clients.count(); }

    /**
     * tell the HTMLObjects, that the image is ready.
     * if o = 0  notify all objects
     */
    void notify(HTMLObject *o = 0);

    /**
     * computes the status of an image after loading.
     * the result depends on the image size and the size of the cache
     */
    void computeStatus();

    /**
     * load a local file
     */
    void load( const char *_filename );

    /**
     * update the object from the buffer
     */
    bool data( QBuffer &buffer, bool eof = false );

    void clear();

    QPixmap *p;
    QMovie *m;

    QList<HTMLObject> clients;
    int status;
    int size;
};

class ImageList : public QStrList
{
public:
    /**
     * implements the LRU list
     * The least recently used item is at the beginning of the list.
     */
    void touch( const char *url )
    {
	remove( url );
	append( url );
    }
};

class KHTMLCache : public HTMLObject
{
public:
    KHTMLCache( KHTMLWidget *w );
    virtual ~KHTMLCache();

    enum Status { NotCached,    // this URL is not cached
		  Unknown,      // let imagecache decide what to do with it
		  New,          // inserting new image
		  Persistent,   // never delete this pixmap
		  Cached,       // regular case
		  Pending,      // only partially loaded
		  Uncacheable };// to big to be cached,
                                // will be destroyed as soon as possible
    
    /**
     * called from the KHTMLWidget to say that new data has arrived
     * if eof it TRUE, the url is completely loaded.
     */
    virtual bool fileLoaded( const char *_url, QBuffer &buffer, 
			     bool eof = false);
    /**
     * called from KHTMLWidget to say, that the image is loaded
     */
    virtual void fileLoaded( const char *_url, const char *localfile );

    /** if an htmlimage is destructed it calls
     * this function, to tell the cache, the image is not used
     * anymore.
     */
    void free( const char *_url, HTMLObject *o = 0);

    /** this is called from the KHTMLWidget to indicate that a
     * HTMLObject needs an Image
     */
    void requestImage( HTMLObject *obj, const char * _url );

    static void setStatus( const char * _url, Status s );
    static int status( const char *_url );

    /**
     * preload an image into the cache. 
     * Set Status to Persistent, if you don't wan't it to be 
     * removed in any case.
     */
    static void preload( const char *_url, Status s = Unknown );

    /**
     * get the pixmap belonging to an url. 
     * @return 0 if the image is not cached.
     */
    static QPixmap *image( const char * _url );

    static void setSize( int bytes );
    static int size() { return maxSize; };

    static void statistics();
    static void flush();

    /**
     * clears the cache
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

