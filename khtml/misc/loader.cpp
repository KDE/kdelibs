/*
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001-2003 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2003 Apple Computer, Inc.

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

    // regarding the LRU:
    // http://www.is.kyusan-u.ac.jp/~chengk/pub/papers/compsac00_A07-07.pdf
*/

#undef CACHE_DEBUG
//#define CACHE_DEBUG

#ifdef CACHE_DEBUG
#define CDEBUG kdDebug(6060)
#else
#define CDEBUG kndDebug()
#endif

#undef LOADER_DEBUG
//#define LOADER_DEBUG

#include <assert.h>

#include "misc/loader.h"
#include "misc/seed.h"

// default cache size
#define DEFCACHESIZE 2096*1024
#define MAX_JOB_COUNT 32

#include <qasyncio.h>
#include <qasyncimageio.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qmovie.h>
#include <qwidget.h>

#include <kapplication.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <scheduler.h>
#include <kdebug.h>
#include "khtml_factory.h"
#include "khtml_part.h"

#include "html/html_documentimpl.h"
#include "css/css_stylesheetimpl.h"
#include "xml/dom_docimpl.h"

using namespace khtml;
using namespace DOM;

#define MAX_LRU_LISTS 20
struct LRUList {
    CachedObject* m_head;
    CachedObject* m_tail;

    LRUList() : m_head(0), m_tail(0) {}
};

static LRUList m_LRULists[MAX_LRU_LISTS];
static LRUList* getLRUListFor(CachedObject* o);

CachedObject::~CachedObject()
{
    Cache::removeFromLRUList(this);
}

void CachedObject::finish()
{
    m_status = Cached;
}

bool CachedObject::isExpired() const
{
    if (!m_expireDate) return false;
    time_t now = time(0);
    return (difftime(now, m_expireDate) >= 0);
}

void CachedObject::setRequest(Request *_request)
{
    if ( _request && !m_request )
        m_status = Pending;

    if ( allowInLRUList() )
        Cache::removeFromLRUList( this );

    m_request = _request;

    if ( allowInLRUList() )
        Cache::insertInLRUList( this );
}

void CachedObject::ref(CachedObjectClient *c)
{
    // unfortunately we can be ref'ed multiple times from the
    // same object,  because it uses e.g. the same foreground
    // and the same background picture. so deal with it.
    m_clients.insert(c,c);
    Cache::removeFromLRUList(this);
    m_accessCount++;
}

void CachedObject::deref(CachedObjectClient *c)
{
    assert( c );
    assert( m_clients.count() );
    assert( !canDelete() );
    assert( m_clients.find( c ) );

    Cache::flush();

    m_clients.remove(c);

    if (allowInLRUList())
        Cache::insertInLRUList(this);
}

void CachedObject::setSize(int size)
{
    bool sizeChanged;

    if ( !m_next && !m_prev && getLRUListFor(this)->m_head != this )
        sizeChanged = false;
    else
        sizeChanged = ( size - m_size ) != 0;

    // The object must now be moved to a different queue,
    // since its size has been changed.
    if ( sizeChanged  && allowInLRUList())
        Cache::removeFromLRUList(this);

    m_size = size;

    if ( sizeChanged && allowInLRUList())
        Cache::insertInLRUList(this);
}

QTextCodec* CachedObject::codecForBuffer( const QString& charset, const QByteArray& buffer ) const
{
    // we don't use heuristicContentMatch here since it is a) far too slow and
    // b) having too much functionality for our case.

    uchar* d = ( uchar* ) buffer.data();
    int s = buffer.size();

    if ( s >= 3 &&
         d[0] == 0xef && d[1] == 0xbb && d[2] == 0xbf)
         return QTextCodec::codecForMib( 106 ); // UTF-8

    if ( s >= 2 && ((d[0] == 0xff && d[1] == 0xfe) ||
                    (d[0] == 0xfe && d[1] == 0xff)))
        return QTextCodec::codecForMib( 1000 ); // UCS-2

    if(!charset.isEmpty())
    {
	QTextCodec* c = KGlobal::charsets()->codecForName(charset);
        if(c->mibEnum() == 11)  {
            // iso8859-8 (visually ordered)
            c = QTextCodec::codecForName("iso8859-8-i");
        }
        return c;
    }

    return QTextCodec::codecForMib(4); // latin-1
}

// -------------------------------------------------------------------------------------------

CachedCSSStyleSheet::CachedCSSStyleSheet(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy,
					 const char *accept)
    : CachedObject(url, CSSStyleSheet, _cachePolicy, 0)
{
    // Set the type we want (probably css or xml)
    QString ah = QString::fromLatin1( accept );
    if ( !ah.isEmpty() )
        ah += ",";
    ah += "*/*;q=0.1";
    setAccept( ah );
    m_hadError = false;
    m_err = 0;
    // load the file
    Cache::loader()->load(dl, this, false);
    m_loading = true;
}

CachedCSSStyleSheet::CachedCSSStyleSheet(const DOMString &url, const QString &stylesheet_data)
    : CachedObject(url, CSSStyleSheet, KIO::CC_Verify, stylesheet_data.length())
{
    m_loading = false;
    m_status = Persistent;
    m_sheet = DOMString(stylesheet_data);
}


void CachedCSSStyleSheet::ref(CachedObjectClient *c)
{
    CachedObject::ref(c);

    if (!m_loading) {
	if (m_hadError)
	    c->error( m_err, m_errText );
	else
	    c->setStyleSheet( m_url, m_sheet );
    }
}

void CachedCSSStyleSheet::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    setSize(buffer.buffer().size());

    QTextCodec* c = codecForBuffer( m_charset, buffer.buffer() );
    QString data = c->toUnicode( buffer.buffer().data(), m_size );
    // workaround Qt bugs
    m_sheet = data[0] == QChar::byteOrderMark ? DOMString(data.mid( 1 ) ) : DOMString(data);
    m_loading = false;

    checkNotify();
}

void CachedCSSStyleSheet::checkNotify()
{
    if(m_loading || m_hadError) return;

    CDEBUG << "CachedCSSStyleSheet:: finishedLoading " << m_url.string() << endl;

    // it() first increments, then returnes the current item.
    // this avoids skipping an item when setStyleSheet deletes the "current" one.
    for (QPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->setStyleSheet( m_url, m_sheet );
}


void CachedCSSStyleSheet::error( int err, const char* text )
{
    m_hadError = true;
    m_err = err;
    m_errText = text;
    m_loading = false;

    // it() first increments, then returnes the current item.
    // this avoids skipping an item when setStyleSheet deletes the "current" one.
    for (QPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->error( m_err, m_errText );
}

// -------------------------------------------------------------------------------------------

CachedScript::CachedScript(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy, const char*)
    : CachedObject(url, Script, _cachePolicy, 0)
{
    // It's javascript we want.
    // But some websites think their scripts are <some wrong mimetype here>
    // and refuse to serve them if we only accept application/x-javascript.
    setAccept( QString::fromLatin1("*/*") );
    // load the file
    Cache::loader()->load(dl, this, false);
    m_loading = true;
}

CachedScript::CachedScript(const DOMString &url, const QString &script_data)
    : CachedObject(url, Script, KIO::CC_Verify, script_data.length())
{
    m_loading = false;
    m_status = Persistent;
    m_script = DOMString(script_data);
}

void CachedScript::ref(CachedObjectClient *c)
{
    CachedObject::ref(c);

    if(!m_loading) c->notifyFinished(this);
}

void CachedScript::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    setSize(buffer.buffer().size());

    QTextCodec* c = codecForBuffer( m_charset, buffer.buffer() );
    QString data = c->toUnicode( buffer.buffer().data(), m_size );
    m_script = data[0] == QChar::byteOrderMark ? DOMString(data.mid( 1 ) ) : DOMString(data);
    m_loading = false;
    checkNotify();
}

void CachedScript::checkNotify()
{
    if(m_loading) return;

    for (QPtrDictIterator<CachedObjectClient> it( m_clients); it.current();)
        it()->notifyFinished(this);
}

void CachedScript::error( int /*err*/, const char* /*text*/ )
{
    m_loading = false;
    checkNotify();
}

// ------------------------------------------------------------------------------------------

namespace khtml
{

class ImageSource : public QDataSource
{
public:
    ImageSource(QByteArray buf)
        : buffer( buf ), pos( 0 ), eof( false ), rew(false ), rewable( true )
        {}

    int readyToSend()
    {
       if(eof && pos == buffer.size())
           return -1;

        return  buffer.size() - pos;
    }

    void sendTo(QDataSink* sink, int n)
    {
        sink->receive((const uchar*)&buffer.at(pos), n);

        pos += n;

        // buffer is no longer needed
        if(eof && pos == buffer.size() && !rewable)
        {
            buffer.resize(0);
            pos = 0;
        }
    }

    /**
     * Sets the EOF state.
     */
    void setEOF( bool state ) { eof = state; }

    bool rewindable() const { return rewable; }
    void enableRewind(bool on) { rew = on; }

    /*
      Calls reset() on the QIODevice.
    */
    void rewind()
    {
        pos = 0;
        if (!rew) {
            QDataSource::rewind();
        } else
            ready();
    }

    /*
      Indicates that the buffered data is no longer
      needed.
    */
    void cleanBuffer()
    {
        // if we need to be able to rewind, buffer is needed
        if(rew)
            return;

        rewable = false;

        // buffer is no longer needed
        if(eof && pos == buffer.size())
        {
            buffer.resize(0);
            pos = 0;
        }
    }

    QByteArray buffer;
    unsigned int pos;
private:
    bool eof     : 1;
    bool rew     : 1;
    bool rewable : 1;
};

} // end namespace

static QString buildAcceptHeader()
{
    return "image/png, image/jpeg, video/x-mng, image/jp2, image/gif;q=0.5,*/*;q=0.1";
}

// -------------------------------------------------------------------------------------

CachedImage::CachedImage(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy, const char*)
    : QObject(), CachedObject(url, Image, _cachePolicy, 0)
{
    static const QString &acceptHeader = KGlobal::staticQString( buildAcceptHeader() );

    m = 0;
    p = 0;
    pixPart = 0;
    bg = 0;
    bgColor = qRgba( 0, 0, 0, 0xFF );
    typeChecked = false;
    isFullyTransparent = false;
    monochrome = false;
    formatType = 0;
    m_status = Unknown;
    imgSource = 0;
    setAccept( acceptHeader );
    m_showAnimations = dl->showAnimations();
}

CachedImage::~CachedImage()
{
    clear();
}

void CachedImage::ref( CachedObjectClient *c )
{
    CachedObject::ref(c);

    if( m ) {
        m->unpause();
        if( m->finished() || m_clients.count() == 1 )
            m->restart();
    }

    // for mouseovers, dynamic changes
    if ( m_status >= Persistent && !valid_rect().isNull() ) {
        c->setPixmap( pixmap(), valid_rect(), this);
        c->notifyFinished( this );
    }
}

void CachedImage::deref( CachedObjectClient *c )
{
    CachedObject::deref(c);
    if(m && m_clients.isEmpty() && m->running())
        m->pause();
}

#define BGMINWIDTH      32
#define BGMINHEIGHT     32


const QPixmap &CachedImage::tiled_pixmap(const QColor& newc)
{
    static QRgb bgTransparant = qRgba( 0, 0, 0, 0xFF );
    if ( (bgColor != bgTransparant) && (bgColor != newc.rgb()) ) {
        delete bg; bg = 0;
    }

    if (bg)
        return *bg;

    const QPixmap &r = pixmap();

    if (r.isNull()) return r;

    // no error indication for background images
    if(m_hadError) return *Cache::nullPixmap;

    bool isvalid = newc.isValid();
    QSize s(pixmap_size());
    int w = r.width();
    int h = r.height();
    if ( w*h < 8192 )
    {
        if ( r.width() < BGMINWIDTH )
            w = ((BGMINWIDTH  / s.width())+1) * s.width();
        if ( r.height() < BGMINHEIGHT )
            h = ((BGMINHEIGHT / s.height())+1) * s.height();
    }

#ifdef Q_WS_X11
    if ( r.hasAlphaChannel() && 
         ((w != r.width()) || (h != r.height())) )
    {
        bg = new QPixmap(w, h);
        //Tile horizontally on the first stripe
        for (int x = 0; x < w; x += r.width())
            copyBlt(bg, x, 0, &r, 0, 0, r.width(), r.height());
        
        //Copy first stripe down
        for (int y = r.height(); y < h; y += r.height())
            copyBlt(bg, 0, y, bg, 0, 0, w, r.height());
        
        return *bg;
    }
#endif    
    
    if ( 
#ifdef Q_WS_X11    
        !r.hasAlphaChannel() &&
#endif
        ( (w != r.width()) || (h != r.height()) || (isvalid && r.mask())) )
    {
        QPixmap pix = r;
        if ( w != r.width() || (isvalid && pix.mask()))
        {
            bg = new QPixmap(w, r.height());
            QPainter p(bg);
            if(isvalid) p.fillRect(0, 0, w, r.height(), newc);
            p.drawTiledPixmap(0, 0, w, r.height(), pix);
            p.end();

            if(!isvalid && pix.mask())
            {
                // unfortunately our anti-transparency trick doesn't work here
                // we need to create a mask.
                QBitmap newmask(w, r.height());
                QPainter pm(&newmask);
                pm.drawTiledPixmap(0, 0, w, r.height(), *pix.mask());
                bg->setMask(newmask);
                bgColor = bgTransparant;
            }
            else
                bgColor= newc.rgb();
            pix = *bg;
        }
        if ( h != r.height() )
        {
            delete bg;
            bg = new QPixmap(w, h);
            QPainter p(bg);
            if(isvalid) p.fillRect(0, 0, w, h, newc);
            p.drawTiledPixmap(0, 0, w, h, pix);
            if(!isvalid && pix.mask())
            {
                // unfortunately our anti-transparency trick doesn't work here
                // we need to create a mask.
                QBitmap newmask(w, h);
                QPainter pm(&newmask);
                pm.drawTiledPixmap(0, 0, w, h, *pix.mask());
                bg->setMask(newmask);
                bgColor = bgTransparant;
            }
            else
                bgColor= newc.rgb();
        }
        return *bg;
    }

    return r;
}

const QPixmap &CachedImage::pixmap( ) const
{
    if(m_hadError)
        return *Cache::brokenPixmap;

    if(m)
    {
        if(m->framePixmap().size() != m->getValidRect().size())
        {   
            // pixmap is not yet completely loaded, so we
            // return a clipped version. asserting here
            // that the valid rect is always from 0/0 to fullwidth/ someheight
            if(!pixPart) pixPart = new QPixmap();

            (*pixPart) = m->framePixmap();
            if (m->getValidRect().size().isValid())
                pixPart->resize(m->getValidRect().size());
            else
                pixPart->resize(0, 0);
            return *pixPart;
        }
        else
            return m->framePixmap();
    }
    else if(p)
        return *p;

    return *Cache::nullPixmap;
}


QSize CachedImage::pixmap_size() const
{
    return (m_hadError ? Cache::brokenPixmap->size() : m ? m->framePixmap().size() : ( p ? p->size() : QSize()));
}


QRect CachedImage::valid_rect() const
{
    return (m_hadError ? Cache::brokenPixmap->rect() : m ? m->getValidRect() : ( p ? p->rect() : QRect()) );
}


void CachedImage::do_notify(const QPixmap& p, const QRect& r)
{
    for (QPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->setPixmap( p, r, this);
}


void CachedImage::movieUpdated( const QRect& r )
{
#ifdef LOADER_DEBUG
    qDebug("movie updated %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
           m->framePixmap().size().width(), m->framePixmap().size().height());
#endif

    do_notify(m->framePixmap(), r);
}

void CachedImage::movieStatus(int status)
{
#ifdef LOADER_DEBUG
    qDebug("movieStatus(%d)", status);
#endif

    // ### the html image objects are supposed to send the load event after every frame (according to
    // netscape). We have a problem though where an image is present, and js code creates a new Image object,
    // which uses the same CachedImage, the one in the document is not supposed to be notified

    // just another Qt 2.2.0 bug. we cannot call
    // QMovie::frameImage if we're after QMovie::EndOfMovie
    if(status == QMovie::EndOfFrame)
    {
        const QImage& im = m->frameImage();
        monochrome = ( ( im.depth() <= 8 ) && ( im.numColors() - int( im.hasAlphaBuffer() ) <= 2 ) );
        for (int i = 0; monochrome && i < im.numColors(); ++i)
            if (im.colorTable()[i] != qRgb(0xff, 0xff, 0xff) &&
                im.colorTable()[i] != qRgb(0x00, 0x00, 0x00))
                monochrome = false;
        if( (im.width() < 5 || im.height() < 5) && im.hasAlphaBuffer()) // only evaluate for small images
        {
            QImage am = im.createAlphaMask();
            if(am.depth() == 1)
            {
                bool solid = false;
                for(int y = 0; y < am.height(); y++)
                    for(int x = 0; x < am.width(); x++)
                        if(am.pixelIndex(x, y)) {
                            solid = true;
                            break;
                        }
                isFullyTransparent = (!solid);
            }
        }

        // we have to delete our tiled bg variant here
        // because the frame has changed (in order to keep it in sync)
        delete bg;
        bg = 0;
    }

    if((status == QMovie::EndOfMovie && (!m || m->frameNumber() <= 1)) ||
       ((status == QMovie::EndOfLoop) && (m_showAnimations == KHTMLSettings::KAnimationLoopOnce)) ||
       ((status == QMovie::EndOfFrame) && (m_showAnimations == KHTMLSettings::KAnimationDisabled))
      )
    {
        if(imgSource)
        {
            setShowAnimations( KHTMLSettings::KAnimationDisabled );

            // monochrome alphamasked images are usually about 10000 times
            // faster to draw, so this is worth the hack
            if (p && monochrome && p->depth() > 1)
            {
                QPixmap* pix = new QPixmap;
                pix->convertFromImage( p->convertToImage().convertDepth( 1 ), MonoOnly|AvoidDither );
                if ( p->mask() )
                    pix->setMask( *p->mask() );
                delete p;
                p = pix;
                monochrome = false;
            }
        }
        for (QPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
            it()->notifyFinished( this );
    }

#if 0
    if((status == QMovie::EndOfFrame) || (status == QMovie::EndOfMovie))
    {
#ifdef LOADER_DEBUG
        QRect r(valid_rect());
        qDebug("movie Status frame update %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
               pixmap().size().width(), pixmap().size().height());
#endif
        do_notify(pixmap(), valid_rect());
    }
#endif
}

void CachedImage::movieResize(const QSize& /*s*/)
{
    do_notify(m->framePixmap(), QRect());
}

void CachedImage::setShowAnimations( KHTMLSettings::KAnimationAdvice showAnimations )
{
    m_showAnimations = showAnimations;
    if ( (m_showAnimations == KHTMLSettings::KAnimationDisabled) && imgSource ) {
        imgSource->cleanBuffer();
        delete p;
        p = new QPixmap(m->framePixmap());
        m->disconnectUpdate( this, SLOT( movieUpdated( const QRect &) ));
        m->disconnectStatus( this, SLOT( movieStatus( int ) ));
        m->disconnectResize( this, SLOT( movieResize( const QSize& ) ) );
        QTimer::singleShot(0, this, SLOT( deleteMovie()));
        imgSource = 0;
    }
}

void CachedImage::deleteMovie()
{
    delete m; m = 0;
}

void CachedImage::clear()
{
    delete m;   m = 0;
    delete p;   p = 0;
    delete bg;  bg = 0;
    bgColor = qRgba( 0, 0, 0, 0xff );
    delete pixPart; pixPart = 0;

    formatType = 0;
    typeChecked = false;
    setSize(0);

    // No need to delete imageSource - QMovie does it for us
    imgSource = 0;
}

void CachedImage::data ( QBuffer &_buffer, bool eof )
{
#ifdef LOADER_DEBUG
    kdDebug( 6060 ) << this << "in CachedImage::data(buffersize " << _buffer.buffer().size() <<", eof=" << eof << endl;
#endif
    if ( !typeChecked )
    {
        // don't attempt incremental loading if we have all the data already
        assert(!eof);

        formatType = QImageDecoder::formatName( (const uchar*)_buffer.buffer().data(), _buffer.size());
        if ( formatType && strcmp( formatType, "PNG" ) == 0 )
            formatType = 0; // Some png files contain multiple images, we want to show only the first one

        typeChecked = true;

        if ( formatType )  // movie format exists
        {
            imgSource = new ImageSource( _buffer.buffer());
            m = new QMovie( imgSource, 8192 );
            m->connectUpdate( this, SLOT( movieUpdated( const QRect &) ));
            m->connectStatus( this, SLOT( movieStatus(int)));
            m->connectResize( this, SLOT( movieResize( const QSize& ) ) );
        }
    }

    if ( imgSource )
    {
        imgSource->setEOF(eof);
        imgSource->maybeReady();
    }

    if(eof)
    {
        // QMovie currently doesn't support all kinds of image formats
        // so we need to use a QPixmap here when we finished loading the complete
        // picture and display it then all at once.
        if(typeChecked && !formatType)
        {
#ifdef CACHE_DEBUG
            kdDebug(6060) << "CachedImage::data(): reloading as pixmap:" << endl;
#endif
            p = new QPixmap( _buffer.buffer() );
            // set size of image.
#ifdef CACHE_DEBUG
            kdDebug(6060) << "CachedImage::data(): image is null: " << p->isNull() << endl;
#endif
            if(p->isNull())
            {
                m_hadError = true;
                do_notify(pixmap(), QRect(0, 0, 16, 16)); // load "broken image" icon
            }
            else
                do_notify(*p, p->rect());

            for (QPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
                it()->notifyFinished( this );
        }
    }
}

void CachedImage::finish()
{
    Status oldStatus = m_status;
    CachedObject::finish();
    if ( oldStatus != m_status ) {
	const QPixmap &pm = pixmap();
	do_notify( pm, pm.rect() );
    }
    QSize s = pixmap_size();
    setSize( s.width() * s.height() * 2);
}


void CachedImage::error( int /*err*/, const char* /*text*/ )
{
    clear();
    typeChecked = true;
    m_hadError = true;
    m_loading = false;
    do_notify(pixmap(), QRect(0, 0, 16, 16));
    for (QPtrDictIterator<CachedObjectClient> it( m_clients ); it.current();)
        it()->notifyFinished(this);
}

// ------------------------------------------------------------------------------------------

Request::Request(DocLoader* dl, CachedObject *_object, bool _incremental)
{
    object = _object;
    object->setRequest(this);
    incremental = _incremental;
    m_docLoader = dl;
}

Request::~Request()
{
    object->setRequest(0);
}

// ------------------------------------------------------------------------------------------

DocLoader::DocLoader(KHTMLPart* part, DocumentImpl* doc)
{
    m_cachePolicy = KIO::CC_Verify;
    m_expireDate = 0;
    m_creationDate = time(0);
    m_bautoloadImages = true;
    m_showAnimations = KHTMLSettings::KAnimationEnabled;
    m_part = part;
    m_doc = doc;

    Cache::docloader->append( this );
}

DocLoader::~DocLoader()
{
    Cache::loader()->cancelRequests( this );
    Cache::docloader->remove( this );
}

void DocLoader::setCacheCreationDate(time_t _creationDate)
{
    if (_creationDate)
       m_creationDate = _creationDate;
    else
       m_creationDate = time(0); // Now
}

void DocLoader::setExpireDate(time_t _expireDate, bool relative)
{
    if (relative)
       m_expireDate = _expireDate + m_creationDate; // Relative date
    else
       m_expireDate = _expireDate; // Absolute date
#ifdef CACHE_DEBUG
    kdDebug(6061) << "docLoader: " << m_expireDate - time(0) << " seconds left until reload required.\n";
#endif
}

void DocLoader::insertCachedObject( CachedObject* o ) const
{
    m_docObjects.insert( o, o );
    if ( m_docObjects.count() > 3 * m_docObjects.size() )
        m_docObjects.resize(khtml::nextSeed( m_docObjects.size() ) );
}

bool DocLoader::needReload(const KURL &fullURL)
{
    bool reload = false;
    if (m_cachePolicy == KIO::CC_Verify)
    {
       if (!m_reloadedURLs.contains(fullURL.url()))
       {
          CachedObject *existing = Cache::cache->find(fullURL.url());
          if (existing && existing->isExpired())
          {
             Cache::removeCacheEntry(existing);
             m_reloadedURLs.append(fullURL.url());
             reload = true;
          }
       }
    }
    else if ((m_cachePolicy == KIO::CC_Reload) || (m_cachePolicy == KIO::CC_Refresh))
    {
       if (!m_reloadedURLs.contains(fullURL.url()))
       {
          CachedObject *existing = Cache::cache->find(fullURL.url());
          if (existing)
          {
             Cache::removeCacheEntry(existing);
          }
          m_reloadedURLs.append(fullURL.url());
          reload = true;
       }
    }
    return reload;
}

#define DOCLOADER_SECCHECK(doRedirectCheck) \
    KURL fullURL (m_doc->completeURL( url.string() )); \
    if ( !fullURL.isValid() || \
         ( m_part && m_part->onlyLocalReferences() && fullURL.protocol() != "file" && fullURL.protocol() != "data") || \
         doRedirectCheck && ( kapp && m_doc && !kapp->authorizeURLAction("redirect", m_doc->URL(), fullURL))) \
         return 0L;

CachedImage *DocLoader::requestImage( const DOM::DOMString &url)
{
    DOCLOADER_SECCHECK(true);

    CachedImage* i = Cache::requestObject<CachedImage, CachedObject::Image>( this, fullURL, 0);

    if (i && i->status() == CachedObject::Unknown && autoloadImages())
        Cache::loader()->load(this, i, true);

    return i;
}

CachedCSSStyleSheet *DocLoader::requestStyleSheet( const DOM::DOMString &url, const QString& charset,
						   const char *accept, bool userSheet )
{
    DOCLOADER_SECCHECK(!userSheet);

    CachedCSSStyleSheet* s = Cache::requestObject<CachedCSSStyleSheet, CachedObject::CSSStyleSheet>( this, fullURL, accept );
    if ( s ) {
        s->setCharset( charset );
    }
    return s;
}

CachedScript *DocLoader::requestScript( const DOM::DOMString &url, const QString& charset)
{
    DOCLOADER_SECCHECK(true);

    CachedScript* s = Cache::requestObject<CachedScript, CachedObject::Script>( this, fullURL, 0 );
    if ( s )
        s->setCharset( charset );
    return s;
}

#undef DOCLOADER_SECCHECK

void DocLoader::setAutoloadImages( bool enable )
{
    if ( enable == m_bautoloadImages )
        return;

    m_bautoloadImages = enable;

    if ( !m_bautoloadImages ) return;

    for ( QPtrDictIterator<CachedObject> it( m_docObjects ); it.current(); ++it )
        if ( it.current()->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( it.current()) );

            CachedObject::Status status = img->status();
            if ( status != CachedObject::Unknown )
                continue;

            Cache::loader()->load(this, img, true);
        }
}

void DocLoader::setShowAnimations( KHTMLSettings::KAnimationAdvice showAnimations )
{
    if ( showAnimations == m_showAnimations ) return;
    m_showAnimations = showAnimations;

    for ( QPtrDictIterator<CachedObject> it( m_docObjects ); it.current(); ++it )
        if ( it.current()->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( it.current() ) );

            img->setShowAnimations( m_showAnimations );
        }
}

// ------------------------------------------------------------------------------------------

Loader::Loader() : QObject()
{
    m_requestsPending.setAutoDelete( true );
    m_requestsLoading.setAutoDelete( true );
    connect(&m_timer, SIGNAL(timeout()), this, SLOT( servePendingRequests() ) );
}

void Loader::load(DocLoader* dl, CachedObject *object, bool incremental)
{
    Request *req = new Request(dl, object, incremental);
    m_requestsPending.append(req);

    emit requestStarted( req->m_docLoader, req->object );

    m_timer.start(0, true);
}

void Loader::servePendingRequests()
{
    while ( (m_requestsPending.count() != 0) && (m_requestsLoading.count() < MAX_JOB_COUNT) )
    {
        // get the first pending request
        Request *req = m_requestsPending.take(0);

#ifdef LOADER_DEBUG
  kdDebug( 6060 ) << "starting Loader url=" << req->object->url().string() << endl;
#endif

        KURL u(req->object->url().string());
        KIO::TransferJob* job = KIO::get( u, false, false /*no GUI*/);

        job->addMetaData("cache", KIO::getCacheControlString(req->object->cachePolicy()));
        if (!req->object->accept().isEmpty())
            job->addMetaData("accept", req->object->accept());
        if ( req->m_docLoader )
        {
            job->addMetaData( "referrer",  req->m_docLoader->doc()->URL().url() );

            KHTMLPart *part = req->m_docLoader->part();
            if (part )
            {
                job->addMetaData( "cross-domain", part->toplevelURL().url() );
                if (part->widget())
                    job->setWindow (part->widget()->topLevelWidget());
            }
        }

        connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotFinished( KIO::Job * ) ) );
        connect( job, SIGNAL( data( KIO::Job*, const QByteArray &)),
                 SLOT( slotData( KIO::Job*, const QByteArray &)));

        if ( req->object->schedule() )
            KIO::Scheduler::scheduleJob( job );

        m_requestsLoading.insert(job, req);
    }
}

void Loader::slotFinished( KIO::Job* job )
{
  Request *r = m_requestsLoading.take( job );
  KIO::TransferJob* j = static_cast<KIO::TransferJob*>(job);

  if ( !r )
    return;

  if (j->error() || j->isErrorPage())
  {
#ifdef LOADER_DEBUG
      kdDebug(6060) << "Loader::slotFinished, with error. job->error()= " << j->error() << " job->isErrorPage()=" << j->isErrorPage() << endl;
#endif
      r->object->error( job->error(), job->errorText().ascii() );
      emit requestFailed( r->m_docLoader, r->object );
  }
  else
  {
      r->object->data(r->m_buffer, true);
      emit requestDone( r->m_docLoader, r->object );
      time_t expireDate = j->queryMetaData("expire-date").toLong();
#ifdef LOADER_DEBUG
      kdDebug(6060) << "Loader::slotFinished, url = " << j->url().url() << endl;
#endif
      r->object->setExpireDate( expireDate );
  }

  if ( r->object->type() == CachedObject::Image )
      static_cast<CachedImage*>( r->object )->setSuggestedFilename( j->queryMetaData( "content-disposition" ) );

  r->object->finish();

#ifdef LOADER_DEBUG
  kdDebug( 6060 ) << "Loader:: JOB FINISHED " << r->object << ": " << r->object->url().string() << endl;
#endif

  delete r;

  if ( (m_requestsPending.count() != 0) && (m_requestsLoading.count() < MAX_JOB_COUNT / 2) )
      m_timer.start(0, true);
}

void Loader::slotData( KIO::Job*job, const QByteArray &data )
{
    Request *r = m_requestsLoading[job];
    if(!r) {
        kdDebug( 6060 ) << "got data for unknown request!" << endl;
        return;
    }

    if ( !r->m_buffer.isOpen() )
        r->m_buffer.open( IO_WriteOnly );

    r->m_buffer.writeBlock( data.data(), data.size() );

    if(r->incremental)
        r->object->data( r->m_buffer, false );
}

int Loader::numRequests( DocLoader* dl ) const
{
    int res = 0;

    QPtrListIterator<Request> pIt( m_requestsPending );
    for (; pIt.current(); ++pIt )
        if ( pIt.current()->m_docLoader == dl )
            res++;

    QPtrDictIterator<Request> lIt( m_requestsLoading );
    for (; lIt.current(); ++lIt )
        if ( lIt.current()->m_docLoader == dl )
            res++;

    return res;
}

void Loader::cancelRequests( DocLoader* dl )
{
    QPtrListIterator<Request> pIt( m_requestsPending );
    while ( pIt.current() ) {
        if ( pIt.current()->m_docLoader == dl )
        {
            CDEBUG << "canceling pending request for " << pIt.current()->object->url().string() << endl;
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
        if ( lIt.current()->m_docLoader == dl )
        {
            //kdDebug( 6060 ) << "canceling loading request for " << lIt.current()->object->url().string() << endl;
            KIO::Job *job = static_cast<KIO::Job *>( lIt.currentKey() );
            Cache::removeCacheEntry( lIt.current()->object );
            m_requestsLoading.remove( lIt.currentKey() );
            job->kill();
            //emit requestFailed( dl, pIt.current()->object );
        }
        else
            ++lIt;
    }
}

KIO::Job *Loader::jobForRequest( const DOM::DOMString &url ) const
{
    QPtrDictIterator<Request> it( m_requestsLoading );

    for (; it.current(); ++it )
    {
        CachedObject *obj = it.current()->object;

        if ( obj && obj->url() == url )
            return static_cast<KIO::Job *>( it.currentKey() );
    }

    return 0;
}

// ----------------------------------------------------------------------------


QDict<CachedObject> *Cache::cache;
QPtrList<DocLoader>* Cache::docloader;
QPtrList<CachedObject> *Cache::freeList;
Loader *Cache::m_loader;

int Cache::maxSize = DEFCACHESIZE;
int Cache::totalSizeOfLRU;

QPixmap *Cache::nullPixmap;
QPixmap *Cache::brokenPixmap;

void Cache::init()
{
    if ( !cache )
        cache = new QDict<CachedObject>(401, true);

    if ( !docloader )
        docloader = new QPtrList<DocLoader>;

    if ( !nullPixmap )
        nullPixmap = new QPixmap;

    if ( !brokenPixmap )
        brokenPixmap = new QPixmap(KHTMLFactory::instance()->iconLoader()->loadIcon("file_broken", KIcon::Desktop, 16, KIcon::DisabledState));

    if ( !m_loader )
        m_loader = new Loader();

    if ( !freeList ) {
        freeList = new QPtrList<CachedObject>;
        freeList->setAutoDelete(true);
    }
}

void Cache::clear()
{
    if ( !cache ) return;
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "Cache: CLEAR!" << endl;
    statistics();
#endif
    cache->setAutoDelete( true );

#ifndef NDEBUG
    for (QDictIterator<CachedObject> it(*cache); it.current(); ++it)
        assert(it.current()->canDelete());
    for (QPtrListIterator<CachedObject> it(*freeList); it.current(); ++it)
        assert(it.current()->canDelete());
#endif

    delete cache; cache = 0;
    delete nullPixmap; nullPixmap = 0;
    delete brokenPixmap; brokenPixmap = 0;
    delete m_loader;  m_loader = 0;
    delete docloader; docloader = 0;
    delete freeList; freeList = 0;
}

template<typename CachedObjectType, enum CachedObject::Type CachedType>
CachedObjectType* Cache::requestObject( DocLoader* dl, const KURL& kurl, const char* accept )
{
    KIO::CacheControl cachePolicy = dl ? dl->cachePolicy() : KIO::CC_Verify;

    CachedObject* o = cache->find(kurl.url());

    if ( o && o->type() != CachedType ) {
        removeCacheEntry( o );
        o = 0;
    }

    if ( o && dl->needReload( kurl ) ) {
        o = 0;
        assert( cache->find( kurl.url() ) == 0 );
    }

    if(!o)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
        CachedObjectType* cot = new CachedObjectType(dl, kurl.url(), cachePolicy, accept);
        cache->insert( kurl.url(), cot );
        if ( cot->allowInLRUList() )
            insertInLRUList( cot );
        o = cot;
    }
#ifdef CACHE_DEBUG
    else {
    kdDebug( 6060 ) << "Cache: using pending/cached: " << kurl.url() << endl;
    }
#endif


    dl->insertCachedObject( o );

    return static_cast<CachedObjectType *>(o);
}

void Cache::preloadStyleSheet( const QString &url, const QString &stylesheet_data)
{
    CachedObject *o = cache->find(url);
    if(o)
        removeCacheEntry(o);

    CachedCSSStyleSheet *stylesheet = new CachedCSSStyleSheet(url, stylesheet_data);
    cache->insert( url, stylesheet );
}

void Cache::preloadScript( const QString &url, const QString &script_data)
{
    CachedObject *o = cache->find(url);
    if(o)
        removeCacheEntry(o);

    CachedScript *script = new CachedScript(url, script_data);
    cache->insert( url, script );
}

void Cache::flush(bool force)
{
    init();

    if ( force || totalSizeOfLRU > maxSize + maxSize/4) {
        for ( int i = MAX_LRU_LISTS-1; i >= 0 && totalSizeOfLRU > maxSize; --i )
            while ( totalSizeOfLRU > maxSize && m_LRULists[i].m_tail )
                removeCacheEntry( m_LRULists[i].m_tail );

#ifdef CACHE_DEBUG
        statistics();
#endif
    }

    for ( CachedObject* p = freeList->first(); p; p = freeList->next() ) {
        if ( p->canDelete() )
            freeList->remove();
    }

}

void Cache::setSize( int bytes )
{
    maxSize = bytes;
    flush(true /* force */);
}

void Cache::statistics()
{
    CachedObject *o;
    // this function is for debugging purposes only
    init();

    int size = 0;
    int msize = 0;
    int movie = 0;
    int images = 0;
    int scripts = 0;
    int stylesheets = 0;
    QDictIterator<CachedObject> it(*cache);
    for(it.toFirst(); it.current(); ++it)
    {
        o = it.current();
        switch(o->type()) {
        case CachedObject::Image:
        {
            CachedImage *im = static_cast<CachedImage *>(o);
            images++;
            if(im->m != 0)
            {
                movie++;
                msize += im->size();
            }
            break;
        }
        case CachedObject::CSSStyleSheet:
            stylesheets++;
            break;
        case CachedObject::Script:
            scripts++;
            break;
        }
        size += o->size();
    }
    size /= 1024;

    kdDebug( 6060 ) << "------------------------- image cache statistics -------------------" << endl;
    kdDebug( 6060 ) << "Number of items in cache: " << cache->count() << endl;
    kdDebug( 6060 ) << "Number of cached images: " << images << endl;
    kdDebug( 6060 ) << "Number of cached movies: " << movie << endl;
    kdDebug( 6060 ) << "Number of cached scripts: " << scripts << endl;
    kdDebug( 6060 ) << "Number of cached stylesheets: " << stylesheets << endl;
    kdDebug( 6060 ) << "pixmaps:   allocated space approx. " << size << " kB" << endl;
    kdDebug( 6060 ) << "movies :   allocated space approx. " << msize/1024 << " kB" << endl;
    kdDebug( 6060 ) << "--------------------------------------------------------------------" << endl;
}

void Cache::removeCacheEntry( CachedObject *object )
{
    QString key = object->url().string();

    cache->remove( key );
    removeFromLRUList( object );

    for (const DocLoader* dl=docloader->first(); dl; dl=docloader->next() )
        dl->removeCachedObject( object );

    if ( !object->free() ) {
        Cache::freeList->append( object );
        object->m_free = true;
    }
}

static inline int FastLog2(unsigned int j)
{
   unsigned int log2;
   log2 = 0;
   if (j & (j-1))
       log2 += 1;
   if (j >> 16)
       log2 += 16, j >>= 16;
   if (j >> 8)
       log2 += 8, j >>= 8;
   if (j >> 4)
       log2 += 4, j >>= 4;
   if (j >> 2)
       log2 += 2, j >>= 2;
   if (j >> 1)
       log2 += 1;

   return log2;
}

static LRUList* getLRUListFor(CachedObject* o)
{
    int accessCount = o->accessCount();
    int queueIndex;
    if (accessCount == 0) {
        queueIndex = 0;
   } else {
        int sizeLog = FastLog2(o->size());
        queueIndex = sizeLog/o->accessCount() - 1;
        if (queueIndex < 0)
            queueIndex = 0;
        if (queueIndex >= MAX_LRU_LISTS)
            queueIndex = MAX_LRU_LISTS-1;
    }
   return &m_LRULists[queueIndex];
}

void Cache::removeFromLRUList(CachedObject *object)
{
    CachedObject *next = object->m_next;
    CachedObject *prev = object->m_prev;

    LRUList* list = getLRUListFor(object);
    CachedObject *&head = getLRUListFor(object)->m_head;

    if (next == 0 && prev == 0 && head != object) {
        return;
    }

    object->m_next = 0;
    object->m_prev = 0;

    if (next)
        next->m_prev = prev;
    else if (list->m_tail == object)
       list->m_tail = prev;

    if (prev)
        prev->m_next = next;
    else if (head == object)
        head = next;

    totalSizeOfLRU -= object->size();
}

void Cache::insertInLRUList(CachedObject *object)
{
    removeFromLRUList(object);

    assert( object );
    assert( !object->free() );
    assert( object->canDelete() );
    assert( object->allowInLRUList() );

    LRUList* list = getLRUListFor(object);

    CachedObject *&head = list->m_head;

    object->m_next = head;
    if (head)
        head->m_prev = object;
    head = object;

    if (object->m_next == 0)
        list->m_tail = object;

    totalSizeOfLRU += object->size();
}

// --------------------------------------

void CachedObjectClient::setPixmap(const QPixmap &, const QRect&, CachedImage *) {}
void CachedObjectClient::setStyleSheet(const DOM::DOMString &/*url*/, const DOM::DOMString &/*sheet*/) {}
void CachedObjectClient::notifyFinished(CachedObject * /*finishedObj*/) {}
void CachedObjectClient::error(int /*err*/, const QString &/*text*/) {}

#undef CDEBUG

#include "loader.moc"
