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
#ifndef _khtml_loader_h
#define _khtml_loader_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "loader_client.h"
#ifdef HAVE_LIBJPEG
#include "loader_jpeg.h"
#endif

#include <qlist.h>
#include <qobject.h>
#include <qptrdict.h>
#include <qdict.h>
#include <qpixmap.h>
#include <qbuffer.h>
#include <qstringlist.h>

#include <kurl.h>

#include <dom/dom_string.h>

class QMovie;

namespace KIO {
  class Job;
  class TransferJob;
}

namespace DOM
{
    class CSSStyleSheetImpl;
};

namespace khtml
{
    class CachedObject;
    class Request;

    /**
     * @internal
     *
     * A cached object. Classes who want to use this object should derive
     * from CachedObjectClient, to get the function calls in case the requested data has arrived.
     *
     * This class also does the actual communication with kio and loads the file.
     */
    class CachedObject
    {
    public:
	enum Type {
	    Image,
	    CSSStyleSheet,
	    Script
	};
	
	enum Status {
	    NotCached,    // this URL is not cached
	    Unknown,      // let imagecache decide what to do with it
	    New,          // inserting new image
	    Persistent,   // never delete this pixmap
	    Cached,       // regular case
	    Pending,      // only partially loaded
	    Uncacheable   // to big to be cached,
	};  	          // will be destroyed as soon as possible

	CachedObject(const DOM::DOMString &url, Type type, bool _reload)
	{
	    m_url = url;
	    m_type = type;
	    m_status = Pending;
	    m_size = 0;
	    m_free = false;
	    m_reload = _reload;
	}
	virtual ~CachedObject() {}

	virtual void data( QBuffer &buffer, bool eof) = 0;
	virtual void error( int err, const char *text ) = 0;
	
	const DOM::DOMString &url() { return m_url; }
	Type type() { return m_type; }
	
	virtual void ref(CachedObjectClient *consumer) = 0;
	virtual void deref(CachedObjectClient *consumer) = 0;
	
	int count() { return m_clients.count(); }

	void setStatus(Status s) { m_status = s; }
	Status status() { return m_status; }

	int size() { return m_size; }

	/*
	 * computes the status of an object after loading.
	 * the result depends on the objects size and the size of the cache
	 */
	void computeStatus();

        /*
         * Called by the cache if the object has been removed from the cache dict
         * while still being referenced. This means the object should kill itself
         * if its reference counter drops down to zero.
         */
        void setFree( bool b ) { m_free = b; }

        bool reload() { return m_reload; }

        void setRequest(Request *_request);

        bool canDelete();

        /*
         * List of acceptable mimetypes seperated by ",". A mimetype may contain a wildcard.
         */
        // e.g. "text/*"
        QString accept() { return m_accept; }
        void setAccept(const QString &_accept) { m_accept = _accept; }

    protected:
	DOM::DOMString m_url;
	Type m_type;
	Status m_status;
	QList<CachedObjectClient> m_clients;
	int m_size;
        bool m_free;
        bool m_reload;
        Request *m_request;
        QString m_accept;
    };


    /**
     * a cached style sheet
     */
    class CachedCSSStyleSheet : public CachedObject
    {
    public:
	CachedCSSStyleSheet(const DOM::DOMString &url, const DOM::DOMString &baseURL, bool reload);
	virtual ~CachedCSSStyleSheet();
	
	const DOM::DOMString &sheet() const { return m_sheet; }

	virtual void ref(CachedObjectClient *consumer);
	virtual void deref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );

	void checkNotify();

    protected:
	DOM::DOMString m_sheet;
	bool loading;
    };
	
    /**
     * a cached script
     */
    class CachedScript : public CachedObject
    {
    public:
	CachedScript(const DOM::DOMString &url, const DOM::DOMString &baseURL, bool reload);
	virtual ~CachedScript();
	
	const DOM::DOMString &script() const { return m_script; }

	virtual void ref(CachedObjectClient *consumer);
	virtual void deref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );

	void checkNotify();

    protected:
	DOM::DOMString m_script;
	bool loading;
    };

    class ImageSource;

    /**
     * a cached image
     */
    class CachedImage : public QObject, public CachedObject
    {
	Q_OBJECT
    public:
	CachedImage(const DOM::DOMString &url, const DOM::DOMString &baseURL, bool reload);
	virtual ~CachedImage();
	
	const QPixmap &pixmap() const;
	const QPixmap &tiled_pixmap() const;

        QSize pixmap_size() const;    // returns the size of the complete (i.e. when finished) loading
        QRect valid_rect() const;     // returns the rectangle of pixmap that has been loaded already

        void ref(CachedObjectClient *consumer);
	virtual void deref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );
	
	void load();

    private slots:
	/**
	 * gets called, whenever a QMovie changes frame
	 */
	void movieUpdated( const QRect &rect );
        void movieStatus(int);

    private:
        void do_notify(const QPixmap& p, const QRect& r);

    public:
	QMovie* m;
        QPixmap* p;

    protected:
	void clear();

	int width;
	int height;

	// Is the name of the movie format type
	const char* formatType;

	// Is set if movie format type ( incremental/animation) was checked
	bool typeChecked;
	
        // Used to cache a tiled version of the image
	mutable QPixmap *bg;
        QPixmap* pixPart;

	ImageSource* imgSource;

	DOM::DOMString m_baseURL;
    };

    /**
     * @internal
     *
     * Manages the loading of scripts/images/stylesheets for a particular document
     */
    class DocLoader
    {
    public:
 	DocLoader();
 	~DocLoader();
 	
	CachedImage *requestImage( const DOM::DOMString &url, const DOM::DOMString &baseUrl);
	CachedCSSStyleSheet *requestStyleSheet( const DOM::DOMString &url, const DOM::DOMString &baseUrl);
	CachedScript *requestScript( const DOM::DOMString &url, const DOM::DOMString &baseUrl);
	bool reloading;
	QStringList reloadedURLs;
    };

    /**
     * @internal
     */
    class Request
    {
    public:
	Request(CachedObject *_object, const DOM::DOMString &baseURL, bool _incremental);
	~Request();
	bool incremental;
	QBuffer m_buffer;
	CachedObject *object;
	DOM::DOMString m_baseURL;
    };

	
	
    /**
     * @internal
     */
    class Loader : public QObject
    {
	Q_OBJECT

    public:
	Loader();
	~Loader();

	void load(CachedObject *object, const DOM::DOMString &baseURL, bool incremental = true);

        int numRequests( const DOM::DOMString &baseURL );
        int numRequests( const DOM::DOMString &baseURL, CachedObject::Type type );

        void cancelRequests( const DOM::DOMString &baseURL );	

    signals:
	void requestDone( const DOM::DOMString &baseURL, khtml::CachedObject *obj );

    protected slots:
	void slotFinished( KIO::Job * );
	void slotData( KIO::Job *, const QByteArray & );

    private:
	void servePendingRequests();
	
	QList<Request> m_requestsPending;
	QPtrDict<Request> m_requestsLoading;
#ifdef HAVE_LIBJPEG        
        KJPEGFormatType m_jpegloader;
#endif        
    };


    /**
     * @internal
     *
     * Provides a cache/loader for objects needed for displaying the html page.
     * At the moment these are stylesheets, scripts and images
     */
    class Cache
    {
	friend class DocLoader;
    public:

        static void ref();
	static void deref();

	/**
	 * init the cache in case it's not already. This needs to get called once
	 * before using it.
	 */
	static void init();

	/**
	 * Ask the cache for some url. Will return a cachedObject, and
	 * load the requested data in case it's not cahced
	 */
	static CachedImage *requestImage( const DOM::DOMString &url, const DOM::DOMString &baseUrl, bool reload = false);

	/**
	 * Ask the cache for some url. Will return a cachedObject, and
	 * load the requested data in case it's not cahced
	 */
	static CachedCSSStyleSheet *requestStyleSheet( const DOM::DOMString &url, const DOM::DOMString &baseUrl, bool reload = false);

	/**
	 * Ask the cache for some url. Will return a cachedObject, and
	 * load the requested data in case it's not cahced
	 */
	static CachedScript *requestScript( const DOM::DOMString &url, const DOM::DOMString &baseUrl, bool reload = false);
	
	/**
	 * sets the size of the cache. This will only hod approximately, since the size some
	 * cached objects (like stylesheets) take up in memory is not exaclty known.
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

	static Loader *loader() { return m_loader; }

	static KURL completeURL(const DOM::DOMString &url, const DOM::DOMString &baseUrl);
	
    	static QPixmap *nullPixmap;

        static void removeCacheEntry( CachedObject *object );

        static void autoloadImages( bool enable );
	static bool autoloadImages();

    protected:
	/*
	 * @internal
	 */
	class LRUList : public QStringList
	{
	public:
	    /**
	     * implements the LRU list
	     * The least recently used item is at the beginning of the list.
	     */
	    void touch( const QString &url )
	    {
		remove( url );
		append( url );
	    }
	};


	static QDict<CachedObject> *cache;
	static LRUList *lru;

	static int maxSize;
	static int actSize;

	static Loader *m_loader;

        static bool s_autoloadImages;

        static unsigned long s_ulRefCnt;
    };

};

#endif
