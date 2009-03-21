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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.

    // regarding the LRU:
    // http://www.is.kyusan-u.ac.jp/~chengk/pub/papers/compsac00_A07-07.pdf
*/

#undef CACHE_DEBUG
//#define CACHE_DEBUG

#ifdef CACHE_DEBUG
#define CDEBUG kDebug(6060)
#else
#define CDEBUG kDebugDevNull()
#endif

#undef LOADER_DEBUG
//#define LOADER_DEBUG

//#define PRELOAD_DEBUG

#include "loader.h"
#include "seed.h"
#include <imload/image.h>
#include <imload/imagepainter.h>

#include <assert.h>

// default cache size
#define DEFCACHESIZE 2096*1024
#define MAX_JOB_COUNT 32

//#include <qasyncio.h>
//#include <qasyncimageio.h>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/QMovie>
#include <QtGui/QWidget>
#include <QtCore/QDebug>
#include <kauthorized.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/jobclasses.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <scheduler.h>
#include <kdebug.h>

#include <khtml_global.h>
#include <khtml_part.h>

#ifdef IMAGE_TITLES
#include <qfile.h>
#include <kfilemetainfo.h>
#include <ktemporaryfile.h>
#endif

#include "html/html_documentimpl.h"
#include "css/css_stylesheetimpl.h"
#include "xml/dom_docimpl.h"

#include "blocked_icon.cpp"

#include <QPaintEngine>

using namespace khtml;
using namespace DOM;
using namespace khtmlImLoad;

#define MAX_LRU_LISTS 20
struct LRUList {
    CachedObject* m_head;
    CachedObject* m_tail;

    LRUList() : m_head(0), m_tail(0) {}
};

static LRUList m_LRULists[MAX_LRU_LISTS];
static LRUList* getLRUListFor(CachedObject* o);

CachedObjectClient::~CachedObjectClient()
{
}

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
    if (m_preloadResult == PreloadNotReferenced) {
        if (isLoaded())
            m_preloadResult = PreloadReferencedWhileComplete;
        else if (m_prospectiveRequest)
            m_preloadResult = PreloadReferencedWhileLoading;
        else
            m_preloadResult = PreloadReferenced;
    }
    // unfortunately we can be ref'ed multiple times from the
    // same object,  because it uses e.g. the same foreground
    // and the same background picture. so deal with it.
    // Hence the use of a QHash rather than of a QSet.
    m_clients.insertMulti(c,c);
    Cache::removeFromLRUList(this);
    m_accessCount++;
}

void CachedObject::deref(CachedObjectClient *c)
{
    assert( c );
    assert( m_clients.count() );
    assert( !canDelete() );
    assert( m_clients.contains( c ) );

    Cache::flush();

    m_clients.take(c);

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

    // BOM
    if ( s >= 3 &&
         d[0] == 0xef && d[1] == 0xbb && d[2] == 0xbf)
         return QTextCodec::codecForMib( 106 ); // UTF-8

    if ( s >= 2 && ((d[0] == 0xff && d[1] == 0xfe) ||
                    (d[0] == 0xfe && d[1] == 0xff)))
        return QTextCodec::codecForMib( 1000 ); // UCS-2

    // Link or @charset
    if(!charset.isEmpty())
    {
	QTextCodec* c = KGlobal::charsets()->codecForName(charset);
        if(c->mibEnum() == 11)  {
            // iso8859-8 (visually ordered)
            c = QTextCodec::codecForName("iso8859-8-i");
        }
        return c;
    }

    // Default
    return QTextCodec::codecForMib( 4 ); // latin 1
}

// -------------------------------------------------------------------------------------------

CachedCSSStyleSheet::CachedCSSStyleSheet(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy,
					 const char *accept)
    : CachedObject(url, CSSStyleSheet, _cachePolicy, 0)
{
    // Set the type we want (probably css or xml)
    QString ah = QLatin1String( accept );
    if ( !ah.isEmpty() )
        ah += ',';
    ah += "*/*;q=0.1";
    setAccept( ah );
    m_hadError = false;
    m_wasBlocked = false;
    m_err = 0;
    // load the file
    Cache::loader()->load(dl, this, false, true /*highPriority*/);
    m_loading = true;
}

CachedCSSStyleSheet::CachedCSSStyleSheet(const DOMString &url, const QString &stylesheet_data)
    : CachedObject(url, CSSStyleSheet, KIO::CC_Verify, stylesheet_data.length())
{
    m_loading = false;
    m_status = Persistent;
    m_sheet = DOMString(stylesheet_data);
}

bool khtml::isAcceptableCSSMimetype( const DOM::DOMString& mimetype )
{
    // matches Mozilla's check (cf. nsCSSLoader.cpp)
    return mimetype.isEmpty() || mimetype == "text/css" || mimetype == "application/x-unknown-content-type";
}

void CachedCSSStyleSheet::ref(CachedObjectClient *c)
{
    CachedObject::ref(c);

    if (!m_loading) {
	if (m_hadError)
	    c->error( m_err, m_errText );
	else
	    c->setStyleSheet( m_url, m_sheet, m_charset, m_mimetype );
    }
}

void CachedCSSStyleSheet::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    setSize(buffer.buffer().size());

//     QString charset = checkCharset( buffer.buffer() );
    QTextCodec* c = 0;
    if (!m_charset.isEmpty()) {
        c = KGlobal::charsets()->codecForName(m_charset);
        if(c->mibEnum() == 11)  c = QTextCodec::codecForName("iso8859-8-i");
    }
    else {
        c = codecForBuffer( m_charsetHint, buffer.buffer() );
        m_charset = c->name();
    }
    QString data = c->toUnicode( buffer.buffer().data(), m_size );
    // workaround Qt bugs
    m_sheet = static_cast<QChar>(data[0]) == QChar::ByteOrderMark ? DOMString(data.mid( 1 ) ) : DOMString(data);
    m_loading = false;

    checkNotify();
}

void CachedCSSStyleSheet::checkNotify()
{
    if(m_loading || m_hadError) return;

    CDEBUG << "CachedCSSStyleSheet:: finishedLoading " << m_url.string() << endl;

    for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
        it.next().value()->setStyleSheet( m_url, m_sheet, m_charset, m_mimetype );
}


void CachedCSSStyleSheet::error( int err, const char* text )
{
    m_hadError = true;
    m_err = err;
    m_errText = text;
    m_loading = false;

    for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
        it.next().value()->error( m_err, m_errText );
}

#if 0
QString CachedCSSStyleSheet::checkCharset(const QByteArray& buffer ) const
{
    int s = buffer.size();
    if (s <= 12) return m_charset;

    // @charset has to be first or directly after BOM.
    // CSS 2.1 says @charset should win over BOM, but since more browsers support BOM
    // than @charset, we default to that.
    const char* d = (const char*) buffer.data();
    if (strncmp(d, "@charset \"",10) == 0)
    {
        // the string until "; is the charset name
        char *p = strchr(d+10, '"');
        if (p == 0) return m_charset;
        QString charset = QString::fromAscii(d+10, p-(d+10));
	return charset;
    }
    return m_charset;
}
#endif

// -------------------------------------------------------------------------------------------

CachedScript::CachedScript(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy, const char*)
    : CachedObject(url, Script, _cachePolicy, 0)
{
    // It's javascript we want.
    // But some websites think their scripts are <some wrong mimetype here>
    // and refuse to serve them if we only accept application/x-javascript.
    setAccept( QLatin1String("*/*") );
    // load the file
    Cache::loader()->load(dl, this, false);
    m_loading = true;
    m_hadError = false;
}

CachedScript::CachedScript(const DOMString &url, const QString &script_data)
    : CachedObject(url, Script, KIO::CC_Verify, script_data.length())
{
    m_hadError = false;
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
    m_script = static_cast<QChar>(data[0]) == QChar::ByteOrderMark ? DOMString(data.mid( 1 ) ) : DOMString(data);
    m_loading = false;
    checkNotify();
}

void CachedScript::checkNotify()
{
    if(m_loading) return;

    for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
        it.next().value()->notifyFinished(this);
}

void CachedScript::error( int /*err*/, const char* /*text*/ )
{
    m_hadError = true;
    m_loading  = false;
    checkNotify();
}

// ------------------------------------------------------------------------------------------

static QString buildAcceptHeader()
{
    return "image/png, image/jpeg, video/x-mng, image/jp2, image/gif;q=0.5,*/*;q=0.1";
}

// -------------------------------------------------------------------------------------

CachedImage::CachedImage(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy, const char*)
    : QObject(), CachedObject(url, Image, _cachePolicy, 0)
{
    static const QString &acceptHeader = KGlobal::staticQString( buildAcceptHeader() );

    i = new khtmlImLoad::Image(this);
    //p = 0;
    //pixPart = 0;
    bg = 0;
    scaled = 0;
    bgColor = qRgba( 0, 0, 0, 0 );
    typeChecked = false;
    isFullyTransparent = false;
    monochrome = false;
    formatType = 0;
    m_status = Unknown;
    imgSource = 0;
    setAccept( acceptHeader );
    i->setShowAnimations(dl->showAnimations());
    m_loading = true;

    if ( KHTMLGlobal::defaultHTMLSettings()->isAdFiltered( url.string() ) ) {
        m_wasBlocked = true;
        CachedObject::finish();
    }
}

CachedImage::~CachedImage()
{
    clear();
    delete i;
}

void CachedImage::ref( CachedObjectClient *c )
{
    CachedObject::ref(c);

#ifdef LOADER_DEBUG
    kDebug(6060) << " image "<<this<<" ref'd by client " << c << "\n";
#endif

    // for mouseovers, dynamic changes
    //### having both makes no sense
    if ( m_status >= Persistent && !pixmap_size().isNull() ) {
#ifdef LOADER_DEBUG
        kDebug(6060) << "Notifying finished size:" <<
            i->size().width() << ", " << i->size().height() << endl;
#endif
        c->updatePixmap( QRect(QPoint(0, 0), pixmap_size()),
                         this );
        c->notifyFinished( this );
    }
}

void CachedImage::deref( CachedObjectClient *c )
{
    CachedObject::deref(c);
/*    if(m && m_clients.isEmpty() && m->running())
        m->pause();*/
}

#define BGMINWIDTH      32
#define BGMINHEIGHT     32

QPixmap CachedImage::tiled_pixmap(const QColor& newc, int xWidth, int xHeight)
{

    // no error indication for background images
    if(m_hadError||m_wasBlocked) return *Cache::nullPixmap;

    // If we don't have size yet, nothing to draw yet
    if (i->size().width() == 0 || i->size().height() == 0)
        return *Cache::nullPixmap;

#ifdef __GNUC__
#warning "Needs some additional performance work"
#endif

    static QRgb bgTransparent = qRgba( 0, 0, 0, 0 );

    QSize s(pixmap_size());
    int w = xWidth;
    int h = xHeight;

    if (w == -1) xWidth = w = s.width();
    if (h == -1) xHeight = h = s.height();

    if ( ( (bgColor != bgTransparent) && (bgColor != newc.rgba()) ) ||
         ( bgSize != QSize(xWidth, xHeight)) )
    {
        delete bg; bg = 0;
    }

    if (bg)
        return *bg;

    const QPixmap* src; //source for pretiling, if any

    const QPixmap &r = pixmap(); //this is expensive
    if (r.isNull()) return r;

    //See whether we should scale
    if (xWidth != s.width() || xHeight != s.height()) {
        src = scaled_pixmap(xWidth, xHeight);
    } else {
        src = &r;
    }

    bgSize = QSize(xWidth, xHeight);

    //See whether we can - and should - pre-blend
    // ### this needs serious investigations. Not likely to help with transparent bgColor,
    // won't work with CSS3 multiple backgrounds. Does it help at all in Qt4? (ref: #114938)
    if (newc.isValid() && (r.hasAlpha() || r.hasAlphaChannel())) {
        bg = new QPixmap(xWidth, xHeight);
        bg->fill(newc);
        QPainter p(bg);
        p.drawPixmap(0, 0, *src);
        bgColor = newc.rgba();
        src     = bg;
    } else {
        bgColor = bgTransparent;
    }

    //See whether to pre-tile.
    if ( w*h < 8192 )
    {
        if ( r.width() < BGMINWIDTH )
            w = ((BGMINWIDTH-1) / xWidth + 1) * xWidth;
        if ( r.height() < BGMINHEIGHT )
            h = ((BGMINHEIGHT-1) / xHeight + 1) * xHeight;
    }

    if ( w != xWidth  || h != xHeight )
    {
        // kDebug() << "pre-tiling " << s.width() << "," << s.height() << " to " << w << "," << h;
        QPixmap* oldbg = bg;
        bg = new QPixmap(w, h);
        if (src->hasAlpha() || src->hasAlphaChannel()) {
            if (newc.isValid() && (bgColor != bgTransparent))
                bg->fill( bgColor );
            else
                bg->fill( Qt::transparent );
        }

        QPainter p(bg);
        p.drawTiledPixmap(0, 0, w, h, *src);
        p.end();

        if ( src == oldbg )
            delete oldbg;
    } else if (src && !bg) {
        // we were asked for the entire pixmap. Cache it.
        // ### goes against imload stuff, but it's far too expensive
        //     to recreate the full pixmap each time it's requested as
        //     we don't know what portion of it will be used eventually
        //     (by e.g. paintBackgroundExtended). It could be a few pixels of
        //     a huge image. See #140248/#1 for an obvious example.
        //     Imload probably needs to handle all painting in paintBackgroundExtended.
        bg = new QPixmap(*src);
    }

    if (bg)
        return *bg;

    return *src;
}


QPixmap* CachedImage::scaled_pixmap( int xWidth, int xHeight )
{
    // no error indication for background images
    if(m_hadError||m_wasBlocked) return Cache::nullPixmap;

    // If we don't have size yet, nothing to draw yet
    if (i->size().width() == 0 || i->size().height() == 0)
        return Cache::nullPixmap;

    if (scaled) {
        if (scaled->width() == xWidth && scaled->height() == xHeight)
            return scaled;
        delete scaled;
    }

    //### this is quite awful performance-wise. It should avoid
    // alpha if not needed, and go to pixmap, etc.
    QImage im(xWidth, xHeight, QImage::Format_ARGB32_Premultiplied);

    QPainter paint(&im);
    paint.setCompositionMode(QPainter::CompositionMode_Source);
    ImagePainter pi(i, QSize(xWidth, xHeight));
    pi.paint(0, 0, &paint);
    paint.end();

    scaled = new QPixmap(QPixmap::fromImage(im));

    return scaled;
}

QPixmap CachedImage::pixmap( ) const
{
    if (m_hadError)
        return *Cache::brokenPixmap;

    if(m_wasBlocked)
        return *Cache::blockedPixmap;

    int w = i->size().width();
    int h = i->size().height();

    if (i->hasAlpha() && !QApplication::desktop()->paintEngine()->hasFeature(QPaintEngine::PorterDuff)) {
        QImage im(w, h, QImage::Format_ARGB32_Premultiplied);
        QPainter paint(&im);
        paint.setCompositionMode(QPainter::CompositionMode_Source);
        ImagePainter pi(i);
        pi.paint(0, 0, &paint);
        paint.end();
        return QPixmap::fromImage( im );
    } else {
        QPixmap pm(w, h);
        if (i->hasAlpha())
            pm.fill(Qt::transparent);
        QPainter paint(&pm);
        paint.setCompositionMode(QPainter::CompositionMode_Source);
        ImagePainter pi(i);
        pi.paint(0, 0, &paint);
        paint.end();
        return pm;
    }
}


QSize CachedImage::pixmap_size() const
{
    if (m_wasBlocked) return Cache::blockedPixmap->size();
    if (m_hadError)   return Cache::brokenPixmap->size();
    if (i)            return i->size();
    return QSize();
}


void CachedImage::imageHasGeometry(khtmlImLoad::Image* /*img*/, int width, int height)
{
#ifdef LOADER_DEBUG
    kDebug(6060) << this << " got geometry "<< width << "x" << height;
#endif
    do_notify(QRect(0, 0, width, height));
}

void CachedImage::imageChange     (khtmlImLoad::Image* /*img*/, QRect region)
{
#ifdef LOADER_DEBUG
    kDebug(6060) << "Image " << this << " change " <<
        region.x() << "," << region.y() << ":" << region.width() << "x" << region.height() << endl;
#endif
    //### this is overly conservative -- I guess we need to also specify reason,
    //e.g. repaint vs. changed !!!
    delete bg;
    bg = 0;

    do_notify(region);
}

void CachedImage::doNotifyFinished()
{
    for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
    {
        it.next().value()->notifyFinished(this);
    }
}

void CachedImage::imageError(khtmlImLoad::Image* /*img*/)
{
    error(0, 0);
}


void CachedImage::imageDone(khtmlImLoad::Image* /*img*/)
{
#ifdef LOADER_DEBUG
    kDebug(6060)<<"Image is done:" << this;
#endif
    m_status = Persistent;
    m_loading = false;
    doNotifyFinished();
}

// QRect CachedImage::valid_rect() const
// {
//     if (m_wasBlocked) return Cache::blockedPixmap->rect();
//     return (m_hadError ? Cache::brokenPixmap->rect() : m ? m->frameRect() : ( p ? p->rect() : QRect()) );
// }


void CachedImage::do_notify(const QRect& r)
{
    for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
    {
#ifdef LOADER_DEBUG
        kDebug(6060) << " image "<<this<<" notify of geom client " << it.peekNext() << "\n";
#endif
        it.next().value()->updatePixmap( r, this);
    }
}


// void CachedImage::movieUpdated( const QRect& r )
// {
// #ifdef LOADER_DEBUG
//     qDebug("movie updated %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
//            m->framePixmap().size().width(), m->framePixmap().size().height());
// #endif
//
//     do_notify(m->framePixmap(), r);
// }
#if 0

void CachedImage::movieStatus(int status)
{
#warning QMovie emits different signals now and requires different ways to call
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
                pix->convertFromImage( p->convertToImage().convertDepth( 1 ), Qt::MonoOnly|Qt::AvoidDither );
                pix->setMask( p->mask() );
                delete p;
                p = pix;
                monochrome = false;
            }
        }
        for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
            it.next().value()->notifyFinished( this );
	m_status = Cached; //all done
    }

    if((status == QMovie::EndOfFrame) || (status == QMovie::EndOfMovie))
    {
#ifdef LOADER_DEBUG
        QRect r(valid_rect());
        qDebug("movie Status frame update %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
               pixmap().size().width(), pixmap().size().height());
#endif
        do_notify(pixmap(), valid_rect());
    }
}
#endif

// void CachedImage::movieResize(const QSize& /*s*/)
// {
//     do_notify(m->framePixmap(), QRect());
// }

void CachedImage::setShowAnimations( KHTMLSettings::KAnimationAdvice showAnimations )
{
    if (i)
        i->setShowAnimations(showAnimations);
}

// void CachedImage::deleteMovie()
// {
//     delete m; m = 0;
// }

void CachedImage::pauseAnimations()
{
//    if ( m ) m->setPaused( true );
}

void CachedImage::resumeAnimations()
{
//    if ( m ) m->setPaused( false );
}

void CachedImage::clear()
{
    delete i;   i = new khtmlImLoad::Image(this);
    delete scaled;  scaled = 0;
    bgColor = qRgba( 0, 0, 0, 0xff );
    delete bg;  bg = 0;
    bgSize = QSize(-1,-1);

    formatType = 0;
    typeChecked = false;
    setSize(0);

    // No need to delete imageSource - QMovie does it for us
    imgSource = 0;
}

void CachedImage::data ( QBuffer &_buffer, bool eof )
{
#ifdef LOADER_DEBUG
    kDebug( 6060 ) << this << "in CachedImage::data(buffersize " << _buffer.buffer().size() <<", eof=" << eof << " pos:" << _buffer.pos();
#endif
    i->processData((uchar*)_buffer.data().data(), _buffer.pos());

    _buffer.close();

    if (eof)
        i->processEOF();

    //if ( !typeChecked )
    //{
        // don't attempt incremental loading if we have all the data already
      //  assert(!eof);

#ifdef __GNUC__
#warning QImage* requires heavy porting
#endif
#if 0
        formatType = QImageDecoder::formatName( (const uchar*)_buffer.buffer().data(), _buffer.size());
        if ( formatType && strcmp( formatType, "PNG" ) == 0 )
            formatType = 0; // Some png files contain multiple images, we want to show only the first one

        typeChecked = true;

        if ( formatType )  // movie format exists
        {
            // imgSource = new ImageSource( _buffer.buffer());
            // m = new QMovie( imgSource, 8192 );
	    m = new QMovie( _buffer);
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
            kDebug(6060) << "CachedImage::data(): reloading as pixmap:";
#endif
            p = new QPixmap;
            {
            	QBuffer buffer(_buffer.buffer());
            	buffer.open(IO_ReadOnly);
                QImageIO io( &buffer, 0 );
                io.setGamma(2.2); // hardcoded "reasonable value"
                bool result = io.read();
                if (result) p->convertFromImage(io.image(), 0);
            }

            // set size of image.
#ifdef CACHE_DEBUG
            kDebug(6060) << "CachedImage::data(): image is null: " << p->isNull();
#endif
            if(p->isNull())
            {
                m_hadError = true;
                do_notify(pixmap(), QRect(0, 0, 16, 16)); // load "broken image" icon
            }
            else
                do_notify(*p, p->rect());

            for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
                it.next().value()->notifyFinished( this );
            m_status = Cached; //all done
        }
#endif
    //}
}

void CachedImage::finish()
{
    //Status oldStatus = m_status;
    CachedObject::finish();
/*    if ( oldStatus != m_status ) {
	const QPixmap &pm = pixmap();
	do_notify( pm, pm.rect() );
    }*/
    m_loading = false;
    QSize s = pixmap_size();
    setSize( s.width() * s.height() * 2);
}


void CachedImage::error( int /*err*/, const char* /*text*/ )
{
    clear();
    typeChecked = true;
    m_hadError = true;
    m_loading = false;
    do_notify(QRect(0, 0, 16, 16));
    for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
        it.next().value()->notifyFinished(this);
}

// -------------------------------------------------------------------------------------------

CachedSound::CachedSound(DocLoader* dl, const DOMString &url, KIO::CacheControl _cachePolicy, const char*)
    : CachedObject(url, Sound, _cachePolicy, 0)
{
    setAccept( QLatin1String("*/*") ); // should be whatever phonon would accept...
    Cache::loader()->load(dl, this, false);
    m_loading = true;
}

void CachedSound::ref(CachedObjectClient *c)
{
    CachedObject::ref(c);

    if(!m_loading) c->notifyFinished(this);
}

void CachedSound::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    setSize(buffer.buffer().size());

    m_sound = buffer.buffer();
    m_loading = false;
    checkNotify();
}

void CachedSound::checkNotify()
{
    if(m_loading) return;

    for (QHashIterator<CachedObjectClient*,CachedObjectClient*> it( m_clients ); it.hasNext();)
        it.next().value()->notifyFinished(this);
}

void CachedSound::error( int /*err*/, const char* /*text*/ )
{
    m_loading = false;
    checkNotify();
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
    clearPreloads();
    Cache::loader()->cancelRequests( this );
    Cache::docloader->removeAll( this );
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
    kDebug(6061) << "docLoader: " << m_expireDate - time(0) << " seconds left until reload required.\n";
#endif
}

void DocLoader::insertCachedObject( CachedObject* o ) const
{
    m_docObjects.insert( o );
}

bool DocLoader::needReload(CachedObject *existing, const QString& fullURL)
{
    bool reload = false;
    if (m_cachePolicy == KIO::CC_Verify ||    
        // During a softReload, we favour using cached images
        // over forcibly re-downloading them.
        (existing->type() == CachedObject::Image && m_part->browserExtension()->browserArguments().softReload))
    {
       if (!m_reloadedURLs.contains(fullURL))
       {
          if (existing && existing->isExpired() && !existing->isPreloaded())
          {
             Cache::removeCacheEntry(existing);
             m_reloadedURLs.append(fullURL);
             reload = true;
          }
       }
    }
    else if ((m_cachePolicy == KIO::CC_Reload) || (m_cachePolicy == KIO::CC_Refresh))
    {
       if (!m_reloadedURLs.contains(fullURL))
       {
          if (existing && !existing->isPreloaded())
          {
             Cache::removeCacheEntry(existing);
          }
          if (!existing || !existing->isPreloaded()) {
              m_reloadedURLs.append(fullURL);
              reload = true;
          }
       }
    }
    return reload;
}

void DocLoader::registerPreload(CachedObject* resource)
{
    if (!resource || resource->isLoaded() || m_preloads.contains(resource))
        return;
    resource->increasePreloadCount();
    m_preloads.insert(resource);
    resource->setProspectiveRequest();
#ifdef PRELOAD_DEBUG
    fprintf(stderr, "PRELOADING %s\n", resource->url().string().toLatin1().data());
#endif
}
 
void DocLoader::clearPreloads()
{
    printPreloadStats();
    QSet<CachedObject*>::iterator end = m_preloads.end();
    for (QSet<CachedObject*>::iterator it = m_preloads.begin(); it != end; ++it) {
        CachedObject* res = *it;
        res->decreasePreloadCount();
        if (res->preloadResult() == CachedObject::PreloadNotReferenced || res->hadError())
            Cache::removeCacheEntry(res);
    }
    m_preloads.clear();
}

void DocLoader::printPreloadStats()
{
#ifdef PRELOAD_DEBUG
    unsigned scripts = 0;
    unsigned scriptMisses = 0;
    unsigned stylesheets = 0;
    unsigned stylesheetMisses = 0;
    unsigned images = 0;
    unsigned imageMisses = 0;
    QSet<CachedObject*>::iterator end = m_preloads.end();
    for (QSet<CachedObject*>::iterator it = m_preloads.begin(); it != end; ++it) {
        CachedObject* res = *it;
        if (res->preloadResult() == CachedObject::PreloadNotReferenced)
            fprintf(stderr,"!! UNREFERENCED PRELOAD %s\n", res->url().string().toLatin1().data());
        else if (res->preloadResult() == CachedObject::PreloadReferencedWhileComplete)
            fprintf(stderr,"HIT COMPLETE PRELOAD %s\n", res->url().string().toLatin1().data());
        else if (res->preloadResult() == CachedObject::PreloadReferencedWhileLoading)
            fprintf(stderr,"HIT LOADING PRELOAD %s\n", res->url().string().toLatin1().data());
        
        if (res->type() == CachedObject::Script) {
            scripts++;
            if (res->preloadResult() < CachedObject::PreloadReferencedWhileLoading)
                scriptMisses++;
        } else if (res->type() == CachedObject::CSSStyleSheet) {
            stylesheets++;
            if (res->preloadResult() < CachedObject::PreloadReferencedWhileLoading)
                stylesheetMisses++;
        } else {
            images++;
            if (res->preloadResult() < CachedObject::PreloadReferencedWhileLoading)
                imageMisses++;
        }
    }
    if (scripts)
        fprintf(stderr, "SCRIPTS: %d (%d hits, hit rate %d%%)\n", scripts, scripts - scriptMisses, (scripts - scriptMisses) * 100 / scripts);
    if (stylesheets)
        fprintf(stderr, "STYLESHEETS: %d (%d hits, hit rate %d%%)\n", stylesheets, stylesheets - stylesheetMisses, (stylesheets - stylesheetMisses) * 100 / stylesheets);
    if (images)
        fprintf(stderr, "IMAGES:  %d (%d hits, hit rate %d%%)\n", images, images - imageMisses, (images - imageMisses) * 100 / images);
#endif
}

#define DOCLOADER_SECCHECK(doRedirectCheck) \
    KUrl fullURL (m_doc->completeURL( url.string() )); \
    if ( !fullURL.isValid() || \
         ( m_part && m_part->onlyLocalReferences() && fullURL.protocol() != "file" && fullURL.protocol() != "data") || \
         doRedirectCheck && (m_doc && !KAuthorized::authorizeUrlAction("redirect", m_doc->URL(), fullURL))) \
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
    if ( s && !charset.isEmpty() ) {
        s->setCharsetHint( charset );
    }
    return s;
}

CachedScript *DocLoader::requestScript( const DOM::DOMString &url, const QString& charset)
{
    DOCLOADER_SECCHECK(true);
    if ( ! KHTMLGlobal::defaultHTMLSettings()->isJavaScriptEnabled(fullURL.host()) ||
           KHTMLGlobal::defaultHTMLSettings()->isAdFiltered(fullURL.url()))
	return 0L;

    CachedScript* s = Cache::requestObject<CachedScript, CachedObject::Script>( this, fullURL, 0 );
    if ( s && !charset.isEmpty() )
        s->setCharset( charset );
    return s;
}

CachedSound *DocLoader::requestSound( const DOM::DOMString &url )
{
    DOCLOADER_SECCHECK(true);
    CachedSound* s = Cache::requestObject<CachedSound, CachedObject::Sound>( this, fullURL, 0 );
    return s;
}

#undef DOCLOADER_SECCHECK

void DocLoader::setAutoloadImages( bool enable )
{
    if ( enable == m_bautoloadImages )
        return;

    m_bautoloadImages = enable;

    if ( !m_bautoloadImages ) return;

    for ( QSetIterator<CachedObject*> it( m_docObjects ); it.hasNext(); )
    {
        CachedObject* cur = it.next();
        if ( cur->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>(cur) );

            CachedObject::Status status = img->status();
            if ( status != CachedObject::Unknown )
                continue;

            Cache::loader()->load(this, img, true);
        }
    }
}

void DocLoader::setShowAnimations( KHTMLSettings::KAnimationAdvice showAnimations )
{
    if ( showAnimations == m_showAnimations ) return;
    m_showAnimations = showAnimations;

    for ( QSetIterator<CachedObject*> it( m_docObjects ); it.hasNext(); )
    {
        CachedObject* cur = it.next();
        if ( cur->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( cur ) );

            img->setShowAnimations( m_showAnimations );
        }
    }
}

void DocLoader::pauseAnimations()
{
    for ( QSetIterator<CachedObject*> it( m_docObjects ); it.hasNext(); )
    {
        CachedObject* cur = it.next();
        if ( cur->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( cur ) );

            img->pauseAnimations();
        }
    }
}

void DocLoader::resumeAnimations()
{
    for ( QSetIterator<CachedObject*> it( m_docObjects ); it.hasNext(); )
    {
        CachedObject* cur = it.next();
        if ( cur->type() == CachedObject::Image )
        {
            CachedImage *img = const_cast<CachedImage*>( static_cast<const CachedImage *>( cur ) );

            img->resumeAnimations();
        }
    }
}

// ------------------------------------------------------------------------------------------

Loader::Loader() : QObject()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT( servePendingRequests() ) );
    m_highPriorityRequestPending = 0;
}

Loader::~Loader()
{
    delete m_highPriorityRequestPending;
    qDeleteAll(m_requestsPending);
    qDeleteAll(m_requestsLoading);
}

void Loader::load(DocLoader* dl, CachedObject *object, bool incremental, bool highPriority)
{
    Request *req = new Request(dl, object, incremental);
    if (highPriority && !m_highPriorityRequestPending) {
        m_highPriorityRequestPending = req;
    } else {
        if (highPriority) {
            m_requestsPending.prepend(req);
        } else {
            m_requestsPending.append(req);
        }
        highPriority = false;
    }

    emit requestStarted( req->m_docLoader, req->object );

    if (highPriority) {
        servePendingRequests();
    } else {
        m_timer.setSingleShot(true);
        m_timer.start(0);
    }
}

void Loader::servePendingRequests()
{
    while ( (m_highPriorityRequestPending != 0 || m_requestsPending.count() != 0) && (m_requestsLoading.count() < MAX_JOB_COUNT) )
    {
        // get the first pending request
        Request *req = m_highPriorityRequestPending ? m_highPriorityRequestPending : m_requestsPending.takeFirst();

#ifdef LOADER_DEBUG
  kDebug( 6060 ) << "starting Loader url=" << req->object->url().string();
#endif

        KUrl u(req->object->url().string());
        KIO::TransferJob* job = KIO::get( u, KIO::NoReload, KIO::HideProgressInfo /*no GUI*/);

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
                    job->ui()->setWindow (part->widget()->topLevelWidget());
            }
        }

        connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotFinished( KJob * ) ) );
        connect( job, SIGNAL( mimetype( KIO::Job *, const QString& ) ), this, SLOT( slotMimetype( KIO::Job *, const QString& ) ) );
        connect( job, SIGNAL( data( KIO::Job*, const QByteArray &)),
                 SLOT( slotData( KIO::Job*, const QByteArray &)));

        if ( req->object->schedule() )
            KIO::Scheduler::scheduleJob( job );

        m_requestsLoading.insertMulti(job, req);
        if (m_highPriorityRequestPending) {
            m_highPriorityRequestPending = 0;
            break;
        }
    }
}

void Loader::slotMimetype( KIO::Job *j, const QString& s )
{
    KIO::TransferJob* job = static_cast<KIO::TransferJob*>(j);
    Request *r = m_requestsLoading.value( j );
    if (!r)
        return;
    CachedObject *o = r->object;
    
    // Mozilla plain ignores any  mimetype that doesn't have / in it, and handles it as "",
    // including when being picky about mimetypes. Match that for better compatibility with broken servers.
    if (s.contains('/'))
        o->m_mimetype = s;
    else
        o->m_mimetype = "";
}

void Loader::slotFinished( KJob* job )
{
  KIO::TransferJob* j = static_cast<KIO::TransferJob*>(job);
  Request *r = m_requestsLoading.take( j );

  if ( !r )
    return;

  if (j->error() || j->isErrorPage())
  {
#ifdef LOADER_DEBUG
      kDebug(6060) << "Loader::slotFinished, with error. job->error()= " << j->error() << " job->isErrorPage()=" << j->isErrorPage();
#endif
      r->object->error( job->error(), job->errorText().toAscii().constData() );
      emit requestFailed( r->m_docLoader, r->object );
  }
  else
  {
      QString cs = j->queryMetaData("charset");
      if (!cs.isEmpty()) r->object->setCharset(cs);
      r->object->data(r->m_buffer, true);
      emit requestDone( r->m_docLoader, r->object );
      time_t expireDate = j->queryMetaData("expire-date").toLong();
#ifdef LOADER_DEBUG
      kDebug(6060) << "Loader::slotFinished, url = " << j->url().url();
#endif
      r->object->setExpireDate( expireDate );

      if ( r->object->type() == CachedObject::Image ) {
          QString fn = j->queryMetaData("content-disposition");
          static_cast<CachedImage*>( r->object )->setSuggestedFilename(fn);
#ifdef IMAGE_TITLES
          static_cast<CachedImage*>( r->object )->setSuggestedTitle(fn);
          KTemporaryFile tf;
          tf.open();
          tf.write((const char*)r->m_buffer.buffer().data(), r->m_buffer.size());
          tf.flush();
          KFileMetaInfo kfmi(tf.fileName());
          if (!kfmi.isEmpty()) {
              KFileMetaInfoItem i = kfmi.item("Name");
              if (i.isValid()) {
                  static_cast<CachedImage*>(r->object)->setSuggestedTitle(i.string());
              } else {
                  i = kfmi.item("Title");
                  if (i.isValid()) {
                      static_cast<CachedImage*>(r->object)->setSuggestedTitle(i.string());
                  }
              }
          }
#endif
      }
  }

  r->object->finish();

#ifdef LOADER_DEBUG
  kDebug( 6060 ) << "Loader:: JOB FINISHED " << r->object << ": " << r->object->url().string();
#endif

  delete r;

  if ( (m_highPriorityRequestPending != 0 || m_requestsPending.count() != 0) && (m_requestsLoading.count() < MAX_JOB_COUNT / 2) ) {
      m_timer.setSingleShot(true);
      m_timer.start(0);
  }
}

void Loader::slotData( KIO::Job*job, const QByteArray &data )
{
    Request *r = m_requestsLoading[job];
    if(!r) {
        kDebug( 6060 ) << "got data for unknown request!";
        return;
    }

    if ( !r->m_buffer.isOpen() )
        r->m_buffer.open( QIODevice::WriteOnly );

    r->m_buffer.write( data.data(), data.size() );

    if(r->incremental)
        r->object->data( r->m_buffer, false );
}

int Loader::numRequests( DocLoader* dl ) const
{
    int res = 0;
    if (m_highPriorityRequestPending && m_highPriorityRequestPending->m_docLoader == dl)
        res++;

    foreach( Request* req, m_requestsPending )
        if ( req->m_docLoader == dl )
            res++;

    foreach( Request* req, m_requestsLoading)
        if ( req->m_docLoader == dl )
            res++;

    return res;
}

void Loader::cancelRequests( DocLoader* dl )
{
    if (m_highPriorityRequestPending && m_highPriorityRequestPending->m_docLoader == dl) {
        CDEBUG << "canceling high priority pending request for " << m_highPriorityRequestPending->object->url().string() << endl;
        Cache::removeCacheEntry( m_highPriorityRequestPending->object );
        delete m_highPriorityRequestPending;
        m_highPriorityRequestPending = 0;
    }

    QMutableLinkedListIterator<Request*> pIt( m_requestsPending );
    while ( pIt.hasNext() ) {
        Request* cur = pIt.next();
        if ( cur->m_docLoader == dl )
        {
            CDEBUG << "canceling pending request for " << cur->object->url().string() << endl;
            Cache::removeCacheEntry( cur->object );
            pIt.remove();
            delete cur;
        }
    }

    //kDebug( 6060 ) << "got " << m_requestsLoading.count() << "loading requests";

    QMutableHashIterator<KIO::Job*,Request*> lIt( m_requestsLoading );
    while ( lIt.hasNext() )
    {
        lIt.next();
        if ( lIt.value()->m_docLoader == dl )
        {
            //kDebug( 6060 ) << "canceling loading request for " << lIt.current()->object->url().string();
            KIO::Job *job = static_cast<KIO::Job *>( lIt.key() );
            Cache::removeCacheEntry( lIt.value()->object );
            delete lIt.value();
            lIt.remove();
            job->kill();
        }
    }
}

KIO::Job *Loader::jobForRequest( const DOM::DOMString &url ) const
{
    QHashIterator<KIO::Job*,Request*> it( m_requestsLoading );
    while (it.hasNext())
    {
        it.next();
        if ( it.value()->object && it.value()->object->url() == url )
            return static_cast<KIO::Job *>( it.key() );
    }

    return 0;
}

// ----------------------------------------------------------------------------


QHash<QString,CachedObject*> *Cache::cache;
QLinkedList<DocLoader*>    *Cache::docloader;
QLinkedList<CachedObject*> *Cache::freeList;
Loader *Cache::m_loader;

int Cache::maxSize = DEFCACHESIZE;
int Cache::totalSizeOfLRU;

QPixmap *Cache::nullPixmap;
QPixmap *Cache::brokenPixmap;
QPixmap *Cache::blockedPixmap;

void Cache::init()
{
    if ( !cache )
        cache = new QHash<QString,CachedObject*>();

    if ( !docloader )
        docloader = new QLinkedList<DocLoader*>;

    if ( !nullPixmap )
        nullPixmap = new QPixmap;

    if ( !brokenPixmap )
        brokenPixmap = new QPixmap(KHTMLGlobal::iconLoader()->loadIcon("image-missing", KIconLoader::Desktop, 16, KIconLoader::DisabledState));

    if ( !blockedPixmap ) {
        blockedPixmap = new QPixmap();
        blockedPixmap->loadFromData(blocked_icon_data, blocked_icon_len);
    }

    if ( !m_loader )
        m_loader = new Loader();

    if ( !freeList )
        freeList = new QLinkedList<CachedObject*>;
}

void Cache::clear()
{
    if ( !cache ) return;
#ifdef CACHE_DEBUG
    kDebug( 6060 ) << "Cache: CLEAR!";
    statistics();
#endif

#ifndef NDEBUG
    bool crash = false;
    foreach (CachedObject* co, *cache) {
        if (!co->canDelete()) {
            kDebug( 6060 ) << " Object in cache still linked to";
            kDebug( 6060 ) << " -> URL: " << co->url();
            kDebug( 6060 ) << " -> #clients: " << co->count();
            crash = true;
//         assert(co->canDelete());
        }
    }
    foreach (CachedObject* co, *freeList) {
        if (!co->canDelete()) {
            kDebug( 6060 ) << " Object in freelist still linked to";
            kDebug( 6060 ) << " -> URL: " << co->url();
            kDebug( 6060 ) << " -> #clients: " << co->count();
            crash = true;
            /*
            foreach (CachedObjectClient* cur, (*co->m_clients)))
            {
                if (dynamic_cast<RenderObject*>(cur)) {
                    kDebug( 6060 ) << " --> RenderObject";
                } else
                    kDebug( 6060 ) << " --> Something else";
            }*/
        }
//         assert(freeList->current()->canDelete());
    }
    assert(!crash);
#endif
    qDeleteAll(*cache);
    delete cache; cache = 0;
    delete nullPixmap; nullPixmap = 0;
    delete brokenPixmap; brokenPixmap = 0;
    delete blockedPixmap; blockedPixmap = 0;
    delete m_loader;  m_loader = 0;
    delete docloader; docloader = 0;
    qDeleteAll(*freeList);
    delete freeList; freeList = 0;
}

template<typename CachedObjectType, enum CachedObject::Type CachedType>
CachedObjectType* Cache::requestObject( DocLoader* dl, const KUrl& kurl, const char* accept )
{
    KIO::CacheControl cachePolicy = dl->cachePolicy();

    QString url = kurl.url();
    CachedObject* o = cache->value(url);

    if ( o && o->type() != CachedType ) {
        removeCacheEntry( o );
        o = 0;
    }

    if ( o && dl->needReload( o, url ) ) {
        o = 0;
        assert( !cache->contains( url ) );
    }

    if(!o)
    {
#ifdef CACHE_DEBUG
        kDebug( 6060 ) << "Cache: new: " << kurl.url();
#endif
        CachedObjectType* cot = new CachedObjectType(dl, url, cachePolicy, accept);
        cache->insert( url, cot );
        if ( cot->allowInLRUList() )
            insertInLRUList( cot );
        o = cot;
    }
#ifdef CACHE_DEBUG
    else {
    kDebug( 6060 ) << "Cache: using pending/cached: " << kurl.url();
    }
#endif


    dl->insertCachedObject( o );

    return static_cast<CachedObjectType *>(o);
}

void Cache::preloadStyleSheet( const QString &url, const QString &stylesheet_data)
{
    if (cache->contains(url))
        removeCacheEntry(cache->value(url));

    CachedCSSStyleSheet *stylesheet = new CachedCSSStyleSheet(url, stylesheet_data);
    cache->insert( url, stylesheet );
}

void Cache::preloadScript( const QString &url, const QString &script_data)
{
    if (cache->contains(url))
        removeCacheEntry(cache->value(url));

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

    QMutableLinkedListIterator<CachedObject*> it(*freeList);
    while ( it.hasNext() ) {
        CachedObject* p = it.next();
        if ( p->canDelete() ) {
            it.remove();
            delete p;
        }
    }
}

void Cache::setSize( int bytes )
{
    maxSize = bytes;
    flush(true /* force */);
}

void Cache::statistics()
{
    // this function is for debugging purposes only
    init();

    int size = 0;
    int msize = 0;
    int movie = 0;
    int images = 0;
    int scripts = 0;
    int stylesheets = 0;
    int sound = 0;
    foreach (CachedObject* o, *cache)
    {
        switch(o->type()) {
        case CachedObject::Image:
        {
            //CachedImage *im = static_cast<CachedImage *>(o);
            images++;
            /*if(im->m != 0)
            {
                movie++;
                msize += im->size();
            }*/
            break;
        }
        case CachedObject::CSSStyleSheet:
            stylesheets++;
            break;
        case CachedObject::Script:
            scripts++;
            break;
        case CachedObject::Sound:
            sound++;
            break;
        }
        size += o->size();
    }
    size /= 1024;

    kDebug( 6060 ) << "------------------------- image cache statistics -------------------";
    kDebug( 6060 ) << "Number of items in cache: " << cache->count();
    kDebug( 6060 ) << "Number of cached images: " << images;
    kDebug( 6060 ) << "Number of cached movies: " << movie;
    kDebug( 6060 ) << "Number of cached scripts: " << scripts;
    kDebug( 6060 ) << "Number of cached stylesheets: " << stylesheets;
    kDebug( 6060 ) << "Number of cached sounds: " << sound;
    kDebug( 6060 ) << "pixmaps:   allocated space approx. " << size << " kB";
    kDebug( 6060 ) << "movies :   allocated space approx. " << msize/1024 << " kB";
    kDebug( 6060 ) << "--------------------------------------------------------------------";
}

void Cache::removeCacheEntry( CachedObject *object )
{
    QString key = object->url().string();

    cache->remove( key );
    removeFromLRUList( object );

    foreach( DocLoader* dl, *docloader )
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

void CachedObjectClient::updatePixmap(const QRect&, CachedImage *) {}
void CachedObjectClient::setStyleSheet(const DOM::DOMString &/*url*/, const DOM::DOMString &/*sheet*/, const DOM::DOMString &/*charset*/, const DOM::DOMString &/*mimetype*/) {}
void CachedObjectClient::notifyFinished(CachedObject * /*finishedObj*/) {}
void CachedObjectClient::error(int /*err*/, const QString &/*text*/) {}

#undef CDEBUG

#include "loader.moc"
