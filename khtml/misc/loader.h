/*
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001-2003 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2003 Apple Computer, Inc

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

#include <time.h>

#include "loader_client.h"
#ifdef HAVE_LIBJPEG
#include "loader_jpeg.h"
#endif

#include <stdlib.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qptrdict.h>
#include <qdict.h>
#include <qpixmap.h>
#include <qbuffer.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qtimer.h>

#include <kurl.h>
#include <kio/global.h>

#include <khtml_settings.h>
#include <dom/dom_string.h>

class QMovie;
class KHTMLPart;

namespace KIO {
  class Job;
  class TransferJob;
}

namespace DOM
{
    class CSSStyleSheetImpl;
    class DocumentImpl;
}

namespace khtml
{
    class CachedObject;
    class Request;
    class DocLoader;

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
	    Unknown,      // let imagecache decide what to do with it
	    New,          // inserting new image
            Pending,      // only partially loaded
	    Persistent,   // never delete this pixmap
	    Cached        // regular case
	};

	CachedObject(const DOM::DOMString &url, Type type, KIO::CacheControl _cachePolicy, int size)
            : m_url(url), m_type(type), m_cachePolicy(_cachePolicy),
              m_expireDate(0), m_size(size)
	{
	    m_status = Pending;
            m_accessCount = 0;
	    m_cachePolicy = _cachePolicy;
	    m_request = 0;
            m_deleted = false;
            m_free = false;
            m_hadError = false;
            m_prev = m_next = 0;
	}
        virtual ~CachedObject();

	virtual void data( QBuffer &buffer, bool eof) = 0;
	virtual void error( int err, const char *text ) = 0;

	const DOM::DOMString &url() const { return m_url; }
	Type type() const { return m_type; }

	virtual void ref(CachedObjectClient *consumer);
	virtual void deref(CachedObjectClient *consumer);

	int count() const { return m_clients.count(); }
        int accessCount() const { return m_accessCount; }

	void setStatus(Status s) { m_status = s; }
	Status status() const { return m_status; }

        QTextCodec* codecForBuffer( const QString& charset, const QByteArray& buffer ) const;

	int size() const { return m_size; }

        bool free() const { return m_free; }

        KIO::CacheControl cachePolicy() const { return m_cachePolicy; }

        void setRequest(Request *_request);

        bool canDelete() const { return (m_clients.count() == 0 && !m_request); }

	void setExpireDate(time_t _expireDate) {  m_expireDate = _expireDate; }

	bool isExpired() const;

        virtual bool schedule() const { return false; }
	virtual void finish();

        /**
         * List of acceptable mimetypes separated by ",". A mimetype may contain a wildcard.
         */
        // e.g. "text/*"
        QString accept() const { return m_accept; }
        void setAccept(const QString &_accept) { m_accept = _accept; }

    protected:
        void setSize(int size);
        QPtrDict<CachedObjectClient> m_clients;
	DOM::DOMString m_url;
        QString m_accept;
        Request *m_request;
	Type m_type;
	Status m_status;
        int m_accessCount;
	KIO::CacheControl m_cachePolicy;
	time_t m_expireDate;
	int m_size;
        bool m_deleted : 1;
        bool m_loading : 1;
        bool m_free : 1;
	bool m_hadError : 1;

    private:
        bool allowInLRUList() const { return canDelete() && !m_free && status() != Persistent; }
        CachedObject* m_next;
        CachedObject* m_prev;
        friend class Cache;
    };


    /**
     * a cached style sheet. also used for loading xml documents.
     *
     * ### rename to CachedTextDoc or something since it's more generic than just for css
     */
    class CachedCSSStyleSheet : public CachedObject
    {
    public:
	CachedCSSStyleSheet(DocLoader* dl, const DOM::DOMString &url, KIO::CacheControl cachePolicy,
			    const char *accept);
	CachedCSSStyleSheet(const DOM::DOMString &url, const QString &stylesheet_data);

	const DOM::DOMString &sheet() const { return m_sheet; }

	virtual void ref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );

        virtual bool schedule() const { return true; }
        void setCharset( const QString& charset ) { m_charset = charset; }

    protected:
        void checkNotify();

	DOM::DOMString m_sheet;
        QString m_charset;
	int m_err;
	QString m_errText;
    };

    /**
     * a cached script
     */
    class CachedScript : public CachedObject
    {
    public:
	CachedScript(DocLoader* dl, const DOM::DOMString &url, KIO::CacheControl cachePolicy, const char* accept );
	CachedScript(const DOM::DOMString &url, const QString &script_data);

	const DOM::DOMString &script() const { return m_script; }

	virtual void ref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );

        virtual bool schedule() const { return false; }

	void checkNotify();

        bool isLoaded() const { return !m_loading; }
        void setCharset( const QString& charset ) { m_charset = charset; }

    protected:
        QString m_charset;
	DOM::DOMString m_script;
    };

    class ImageSource;

    /**
     * a cached image
     */
    class CachedImage : public QObject, public CachedObject
    {
	Q_OBJECT
    public:
	CachedImage(DocLoader* dl, const DOM::DOMString &url, KIO::CacheControl cachePolicy, const char* accept);
	virtual ~CachedImage();

	const QPixmap &pixmap() const;
	const QPixmap &tiled_pixmap(const QColor& bg);

        QSize pixmap_size() const;    // returns the size of the complete (i.e. when finished) loading
        QRect valid_rect() const;     // returns the rectangle of pixmap that has been loaded already

        void ref(CachedObjectClient *consumer);
	virtual void deref(CachedObjectClient *consumer);

	virtual void data( QBuffer &buffer, bool eof );
	virtual void error( int err, const char *text );

        bool isTransparent() const { return isFullyTransparent; }
        bool isErrorImage() const { return m_hadError; }

        void setShowAnimations( KHTMLSettings::KAnimationAdvice );

        virtual bool schedule() const { return true; }

	virtual void finish();

    protected:
	void clear();

    private slots:
	/**
	 * gets called, whenever a QMovie changes frame
	 */
	void movieUpdated( const QRect &rect );
        void movieStatus(int);
        void movieResize(const QSize&);
        void deleteMovie();

    private:
        void do_notify(const QPixmap& p, const QRect& r);

	QMovie* m;
        QPixmap* p;
	QPixmap* bg;
        QRgb bgColor;
        mutable QPixmap* pixPart;

        ImageSource* imgSource;
        const char* formatType;  // Is the name of the movie format type

	int width;
	int height;

	// Is set if movie format type ( incremental/animation) was checked
	bool typeChecked : 1;
        bool isFullyTransparent : 1;
        bool monochrome : 1;
        KHTMLSettings::KAnimationAdvice m_showAnimations : 2;

        friend class Cache;
    };

    /**
     * @internal
     *
     * Manages the loading of scripts/images/stylesheets for a particular document
     */
    class DocLoader
    {
    public:
 	DocLoader(KHTMLPart*, DOM::DocumentImpl*);
 	~DocLoader();

	CachedImage *requestImage( const DOM::DOMString &url);
	CachedCSSStyleSheet *requestStyleSheet( const DOM::DOMString &url, const QString& charset,
						const char *accept = "text/css");
        CachedScript *requestScript( const DOM::DOMString &url, const QString& charset);

	bool autoloadImages() const { return m_bautoloadImages; }
        KIO::CacheControl cachePolicy() const { return m_cachePolicy; }
        KHTMLSettings::KAnimationAdvice showAnimations() const { return m_showAnimations; }
        time_t expireDate() const { return m_expireDate; }
        KHTMLPart* part() const { return m_part; }
        DOM::DocumentImpl* doc() const { return m_doc; }

        void setCacheCreationDate( time_t );
        void setExpireDate( time_t, bool relative );
        void setAutoloadImages( bool );
        void setCachePolicy( KIO::CacheControl cachePolicy ) { m_cachePolicy = cachePolicy; }
        void setShowAnimations( KHTMLSettings::KAnimationAdvice );
        void insertCachedObject( CachedObject* o ) const;
        void removeCachedObject( CachedObject* o) const { m_docObjects.remove( o ); }

    private:
        bool needReload(const KURL &fullUrl);

        friend class Cache;
        friend class DOM::DocumentImpl;

        QStringList m_reloadedURLs;
        mutable QPtrDict<CachedObject> m_docObjects;
	time_t m_expireDate;
	time_t m_creationDate;
	KIO::CacheControl m_cachePolicy;
        bool m_bautoloadImages : 1;
        KHTMLSettings::KAnimationAdvice m_showAnimations : 2;
        KHTMLPart* m_part;
        DOM::DocumentImpl* m_doc;
    };

    /**
     * @internal
     */
    class Request
    {
    public:
	Request(DocLoader* dl, CachedObject *_object, bool _incremental);
	~Request();
	bool incremental;
	QBuffer m_buffer;
	CachedObject *object;
        DocLoader* m_docLoader;
    };

    /**
     * @internal
     */
    class Loader : public QObject
    {
	Q_OBJECT

    public:
	Loader();

	void load(DocLoader* dl, CachedObject *object, bool incremental = true);

        int numRequests( DocLoader* dl ) const;
        void cancelRequests( DocLoader* dl );

        // may return 0L
        KIO::Job *jobForRequest( const DOM::DOMString &url ) const;

    signals:
        void requestStarted( khtml::DocLoader* dl, khtml::CachedObject* obj );
	void requestDone( khtml::DocLoader* dl, khtml::CachedObject *obj );
	void requestFailed( khtml::DocLoader* dl, khtml::CachedObject *obj );

    protected slots:
	void slotFinished( KIO::Job * );
	void slotData( KIO::Job *, const QByteArray & );
	void servePendingRequests();

    protected:
	QPtrList<Request> m_requestsPending;
	QPtrDict<Request> m_requestsLoading;
#ifdef HAVE_LIBJPEG
        KJPEGFormatType m_jpegloader;
#endif
        QTimer m_timer;
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

        template<typename CachedObjectType, enum CachedObject::Type CachedType>
        static CachedObjectType* requestObject( DocLoader* dl, const KURL& kurl, const char* accept );

    public:
	/**
	 * init the cache in case it's not already. This needs to get called once
	 * before using it.
	 */
	static void init();

	/**
	 * Ask the cache for some url. Will return a cachedObject, and
	 * load the requested data in case it's not cached
         * if the DocLoader is zero, the url must be full-qualified.
         * Otherwise, it is automatically base-url expanded
	 */
// 	static CachedImage *requestImage(const KURL& url)
//         { return Cache::requestObject<CachedImage, CachedObject::Image>( 0, url, 0 ); }

        /**
         * Pre-loads a stylesheet into the cache.
         */
        static void preloadStyleSheet(const QString &url, const QString &stylesheet_data);

        /**
         * Pre-loads a script into the cache.
         */
        static void preloadScript(const QString &url, const QString &script_data);

	static void setSize( int bytes );
	static int size() { return maxSize; };
	static void statistics();
	static void flush(bool force=false);

	/**
	 * clears the cache
	 * Warning: call this only at the end of your program, to clean
	 * up memory (useful for finding memory holes)
	 */
	static void clear();

	static Loader *loader() { return m_loader; }

    	static QPixmap *nullPixmap;
        static QPixmap *brokenPixmap;
        static int cacheSize;

        static void removeCacheEntry( CachedObject *object );

    private:

        static void checkLRUAndUncacheableListIntegrity();

        friend class CachedObject;

	static QDict<CachedObject> *cache;
        static QPtrList<DocLoader>* docloader;
        static QPtrList<CachedObject> *freeList;
        static void insertInLRUList(CachedObject*);
        static void removeFromLRUList(CachedObject*);

        static int totalSizeOfLRU;
	static int maxSize;

	static Loader *m_loader;
    };

} // namespace

#endif
