/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2003 Stephan Kulow (coolo@kde.org)
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "ecma/kjs_proxy.h"

#define QT_NO_FONTCONFIG 1
#include <private/qfontengine_p.h>
#include <qfontdatabase.h>
#include <qfont.h>
#include "khtml_settings.h"
#include <qwidget.h>
#include <assert.h>
#include <QX11Info>
#include <QPainter>
#include <fixx11h.h>

struct MetricsInfo {
    const char* name;
    int ascent;
    int descent;
    int leading;
};


#if QT_VERSION >= 0x040100
typedef QFixed QtFontDim;

static int dimToInt(QtFontDim dim) {
    return dim.toInt();
}

#else 
typedef qreal QtFontDim;

static int dimToInt(QtFontDim dim) {
    return (int)dim;
}
#endif

static MetricsInfo compatMetrics[] = {
    {"-Adobe-Courier-Medium-R-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Medium-O-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Bold-R-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Bold-O-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Medium-R-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Medium-O-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Bold-R-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Bold-O-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Medium-R-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Medium-O-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Bold-R-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Bold-O-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Medium-R-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Courier-Medium-O-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Courier-Bold-R-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Courier-Bold-O-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Helvetica-Medium-R-Normal--10-100-75-75-P-56-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Medium-O-Normal--10-100-75-75-P-57-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Bold-R-Normal--10-100-75-75-P-60-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Bold-O-Normal--10-100-75-75-P-60-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Medium-R-Normal--12-120-75-75-P-67-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Medium-O-Normal--12-120-75-75-P-67-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Bold-R-Normal--12-120-75-75-P-70-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Bold-O-Normal--12-120-75-75-P-69-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Medium-R-Normal--18-180-75-75-P-98-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Medium-O-Normal--18-180-75-75-P-98-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Bold-R-Normal--18-180-75-75-P-103-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Bold-O-Normal--18-180-75-75-P-104-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Medium-R-Normal--24-240-75-75-P-130-ISO10646-1", 22, 4, 4},
    {"-Adobe-Helvetica-Medium-O-Normal--24-240-75-75-P-130-ISO10646-1", 22, 4, 4},
    {"-Adobe-Helvetica-Bold-R-Normal--24-240-75-75-P-138-ISO10646-1", 22, 4, 4},
    {"-Adobe-Helvetica-Bold-O-Normal--24-240-75-75-P-138-ISO10646-1", 22, 4, 4},
    {"-Adobe-Courier-Medium-R-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Medium-O-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Bold-R-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Bold-O-Normal--10-100-75-75-M-60-ISO10646-1", 8, 1, 2},
    {"-Adobe-Courier-Medium-R-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Medium-O-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Bold-R-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Bold-O-Normal--12-120-75-75-M-70-ISO10646-1", 10, 2, 2},
    {"-Adobe-Courier-Medium-R-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Medium-O-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Bold-R-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Bold-O-Normal--18-180-75-75-M-110-ISO10646-1", 14, 3, 3},
    {"-Adobe-Courier-Medium-R-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Courier-Medium-O-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Courier-Bold-R-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Courier-Bold-O-Normal--24-240-75-75-M-150-ISO10646-1", 19, 4, 4},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Times-Medium-R-Normal--8-80-75-75-P-44-ISO10646-1", 7, 1, 1},
    {"-Adobe-Helvetica-Medium-R-Normal--10-100-75-75-P-56-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Medium-O-Normal--10-100-75-75-P-57-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Bold-R-Normal--10-100-75-75-P-60-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Bold-O-Normal--10-100-75-75-P-60-ISO10646-1", 10, 1, 2},
    {"-Adobe-Helvetica-Medium-R-Normal--12-120-75-75-P-67-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Medium-O-Normal--12-120-75-75-P-67-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Bold-R-Normal--12-120-75-75-P-70-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Bold-O-Normal--12-120-75-75-P-69-ISO10646-1", 11, 2, 2},
    {"-Adobe-Helvetica-Medium-R-Normal--18-180-75-75-P-98-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Medium-O-Normal--18-180-75-75-P-98-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Bold-R-Normal--18-180-75-75-P-103-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Bold-O-Normal--18-180-75-75-P-104-ISO10646-1", 16, 4, 3},
    {"-Adobe-Helvetica-Medium-R-Normal--24-240-75-75-P-130-ISO10646-1", 22, 4, 4},
    {"-Adobe-Helvetica-Medium-O-Normal--24-240-75-75-P-130-ISO10646-1", 22, 4, 4},
    {"-Adobe-Helvetica-Bold-R-Normal--24-240-75-75-P-138-ISO10646-1", 22, 4, 4},
    {"-Adobe-Helvetica-Bold-O-Normal--24-240-75-75-P-138-ISO10646-1", 22, 4, 4},
    {0, 0, 0, 0}
};

static MetricsInfo* grabMetrics(QString name)
{
    for (int pos = 0; compatMetrics[pos].name; ++pos)
        if (name == QLatin1String(compatMetrics[pos].name))
            return &compatMetrics[pos];
    return 0;
}

class QFakeFontEngine : public QFontEngineXLFD
{
public:
    QString name;

    QFakeFontEngine( XFontStruct *fs, const char *name, int size );
    ~QFakeFontEngine();

    bool  haveMetrics;
    QtFontDim m_ascent, m_descent, m_leading;
    bool  ahem;
    int   pixS;
#if 0
    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
                                         const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    QFontEngine::Error stringToCMap( const QChar *, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool ) const;

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const { return 0; }
    int minRightBearing() const { return 0; }
    int cmap() const;
#endif

    Type type() const
    {
        if (ahem) {
            return QFontEngine::Freetype;
        } else
            return QFontEngine::XLFD; 
    }


    QtFontDim ascent() const 
    {
      if (haveMetrics)
        return m_ascent;
      else
        return QFontEngineXLFD::ascent();
    }

    QtFontDim descent() const 
    {
      if (haveMetrics)
        return m_descent;
      else
        return QFontEngineXLFD::descent();
    }

    QtFontDim leading() const
    {
      if (ahem)
        return 0;
      else if (haveMetrics)
        return m_leading;
      else
        return QFontEngineXLFD::leading();
    }

    bool canRender( const QChar *string,  int len );
};

//OK. This is evil. Since we don't use Xft, we hijack the FreeType painting hook in the X11 engine
//for ahem, as unfortunately the drawing is in the paint engine, and not the font engine in Qt4
class QPaintEngine;
class KDE_EXPORT QX11PaintEngine: public QPaintEngine
{
    void drawFreetype(const QPointF &p, const QTextItemInt &si);
};

void qt_draw_transformed_rect(QPaintEngine *pe, int x, int y, int w, int h, bool fill);

void QX11PaintEngine::drawFreetype(const QPointF &p, const QTextItemInt &si)
{
    if (!si.num_glyphs) return;

    QFakeFontEngine *eng = static_cast<QFakeFontEngine*>(si.fontEngine);

    int x       = int(p.x());
    int y       = int(p.y());
    int pixS    = int(eng->pixS);
    int advance = pixS;
    int ascent  = dimToInt(eng->ascent());
    int descent = dimToInt(eng->descent());
    
    if (si.flags & QTextItem::RightToLeft)
    {
        x       = x + advance * (si.num_glyphs - 1);
        advance = -advance;
    }

    for (int pos = 0; pos < si.num_glyphs; ++pos)
    {
        QRect rect;
        switch (si.glyphs[pos].glyph)
        {
        case ' ':
            rect = QRect();
            break;
        case 'p':
            //Below the baseline, including it
            rect = QRect(x, y, pixS, descent + 1);
            break;
        case 0xC9:
            //Above the baseline
            rect = QRect(x, y - ascent, pixS, ascent);
            break;
        default:
            //Whole block
            rect = QRect(x, y - ascent, pixS, pixS);
        }

        //A bit overkill, but this gets the job done without hassling with private data
        qt_draw_transformed_rect(this, rect.x(), rect.y(), rect.width(), rect.height(), true);

        x += advance;
    }
}


QFakeFontEngine::QFakeFontEngine( XFontStruct *fs, const char *name, int size )
    : QFontEngineXLFD( fs,  name,  0)
{
    pixS = size;
    this->name = QLatin1String(name);
    ahem = this->name.contains("ahem");

    MetricsInfo* metrics = grabMetrics(name);
    if (metrics)
    {
        haveMetrics = true;
        m_ascent  = metrics->ascent;
        m_descent = metrics->descent;
        m_leading = metrics->leading;
    }
    else
        haveMetrics = false;
}

QFakeFontEngine::~QFakeFontEngine()
{
}
#if 0
QFontEngine::Error QFakeFontEngine::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored) const
{
    QFontEngine::Error ret = QFontEngineXLFD::stringToCMap( str, len, glyphs, advances, nglyphs, mirrored );

    if ( ret != NoError )
        return ret;

    *nglyphs = len;

    if ( advances ) {
        for ( int i = 0; i < len; i++ )
            *(advances++) = _size;
    }
    return NoError;
}

glyph_metrics_t QFakeFontEngine::boundingBox( const glyph_t *, const advance_t *, const qoffset_t *, int numGlyphs )
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

glyph_metrics_t QFakeFontEngine::boundingBox( glyph_t )
{
    return glyph_metrics_t( 0, _size, _size, _size, _size, 0 );
}

int QFakeFontEngine::ascent() const
{
    return _size;
}

int QFakeFontEngine::descent() const
{
    return 0;
}

int QFakeFontEngine::leading() const
{
    // the original uses double and creates quite random results depending
    // on the compiler flags
    int l = ( _size * 15 + 50) / 100;
    // only valid on i386 without -O2 assert(l == int(qRound(_size * 0.15)));
    return (l > 0) ? l : 1;
}

int QFakeFontEngine::maxCharWidth() const
{
    return _size;
}

int QFakeFontEngine::cmap() const
{
    return -1;
}

#endif

bool QFakeFontEngine::canRender( const QChar *, int )
{
    return true;
}

static QString courier_pickxlfd( int pixelsize, bool italic, bool bold )
{
    if ( pixelsize >= 24 )
        pixelsize = 24;
    else if ( pixelsize >= 18 )
        pixelsize = 18;
    else if ( pixelsize >= 12 )
        pixelsize = 12;
    else
        pixelsize = 10;

    return QString( "-adobe-courier-%1-%2-normal--%3-*-75-75-m-*-iso10646-1" ).arg( bold ? "bold" : "medium" ).arg( italic ? "o" : "r" ).arg( pixelsize );
}

static QString ahem_pickxlfd( int pixelsize )
{
    return QString( "-misc-ahem-medium-r-normal--%1-*-100-100-c-*-iso10646-1" ).arg( pixelsize );
}

static QString helv_pickxlfd( int pixelsize, bool italic, bool bold )
{
    if ( pixelsize >= 24 )
        pixelsize = 24;
    else if ( pixelsize >= 18 )
        pixelsize = 18;
    else if ( pixelsize >= 12 )
        pixelsize = 12;
    else
        pixelsize = 10;

    return QString( "-adobe-helvetica-%1-%2-normal--%3-*-75-75-p-*-iso10646-1" ).arg( bold ? "bold" : "medium" ).arg( italic ? "o" : "r" ).arg( pixelsize );

}

static QFontEngine* loadFont(const QFontDef& request)
{
    QString xlfd;
    QString family = request.family.toLower();
    if ( family == "adobe courier" || family == "courier" || family == "fixed" ) {
        xlfd = courier_pickxlfd( request.pixelSize, request.style == QFont::StyleItalic, request.weight > 50 );
    }
    else if ( family == "times new roman" || family == "times" )
        xlfd = "-adobe-times-medium-r-normal--8-80-75-75-p-44-iso10646-1";
    else if ( family == "ahem" )
        xlfd = ahem_pickxlfd( request.pixelSize );
    else
        xlfd = helv_pickxlfd( request.pixelSize,  request.style == QFont::StyleItalic, request.weight > 50 );

    QFontEngine *fe = 0;

    XFontStruct *xfs;
    xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(), xlfd.latin1() );
    if (!xfs) // as long as you don't do screenshots, it's maybe fine
	qFatal("we need some fonts. So make sure you have %s installed.", xlfd.latin1());

    unsigned long value;
    if ( !XGetFontProperty( xfs, XA_FONT, &value ) )
        return 0;

    char *n = XGetAtomName( QPaintDevice::x11AppDisplay(), value );
    xlfd = n;
    if ( n )
        XFree( n );

    fe = new QFakeFontEngine( xfs, xlfd.latin1(),request.pixelSize );
    return fe;
}

#if QT_VERSION >= 0x040100 
/* Note: you may want the other path with earlier Qt4.1 snapshots */

KDE_EXPORT
QFontEngine *QFontDatabase::loadXlfd(int screen, int script, 
            const QFontDef &request, int force_encoding_id)
{
    return loadFont(request);
}

extern "C" KDE_EXPORT int FcInit() {
    /* Make sure Qt uses the Xlfd path, which we intercept */
    return 0;
}

#else


KDE_EXPORT
QFontEngine *
QFontDatabase::findFont( int script, const QFontPrivate *fp,
			 const QFontDef &request, int ) {
    QFontEngine* fe = loadFont(request);
    QFontCache::Key key( request, script, fp->screen );
    QFontCache::instance->insertEngine( key, fe );
    return fe;
}

#endif

KDE_EXPORT bool QFontDatabase::isBitmapScalable( const QString &,
				      const QString &) const
{
    return true;
}

KDE_EXPORT bool  QFontDatabase::isSmoothlyScalable( const QString &,
                                         const QString &) const
{
    return true;
}

const QString &KHTMLSettings::availableFamilies()
{
    if ( !avFamilies ) {
        avFamilies = new QString;
        *avFamilies = ",Adobe Courier,Arial,Comic Sans MS,Courier,Helvetica,Times,Times New Roman,Utopia,Fixed,Ahem,";
    }

  return *avFamilies;
}


bool KHTMLSettings::unfinishedImageFrame() const
{
  return false;
}

KDE_EXPORT int QX11Info::appDpiY( int )
{
    return 100;
}

KDE_EXPORT int QX11Info::appDpiX( int )
{
    return 100;
}

KDE_EXPORT void QFont::insertSubstitution(const QString &,
                               const QString &)
{
}

KDE_EXPORT void QFont::insertSubstitutions(const QString &,
                                const QStringList &)
{
}

#include <kprotocolinfo.h>
bool KProtocolInfo::isKnownProtocol( const QString& _protocol )
{
    return ( _protocol == "file" );
}

#include <kprotocolinfofactory.h>

QString KProtocolInfo::exec( const QString& _protocol )
{
    if ( _protocol != "file" )
        return QString();

    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot )
        return QString();

    return prot->m_exec;
}

#include <dcopclient.h>

bool DCOPClient::attach()
{
    return false;
}

bool DCOPClient::isAttached() const
{
    return false;
}

void DCOPClient::processSocketData( int )
{
}

#include <qapplication.h>
#include <qpalette.h>

#if 0
KDE_EXPORT void QApplication::setPalette( const QPalette &, bool ,
                               const char*  )
{
    static bool done = false;
    if (done) return;
    QString xlfd = AHEM;
    XFontStruct *xfs;
    xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(), xlfd.latin1() );
    if (!xfs) // as long as you don't do screenshots, it's maybe fine
	qFatal("We will need some fonts. So make sure you have %s installed.", xlfd.latin1());
    XFreeFont(QPaintDevice::x11AppDisplay(), xfs);
    done = true;
}
#endif

#include <kapplication.h>
void KApplication::dcopFailure( const QString & )
{
    qDebug( "KApplication::dcopFailure" );
}

#include <kparts/historyprovider.h>

bool KParts::HistoryProvider::contains( const QString& t ) const
{
    return ( t == "http://www.kde.org/" || t == "http://www.google.com/");
}


bool KJSCPUGuard::confirmTerminate()
{
    return false;
}

#include <ksslsettings.h>

bool KSSLSettings::warnOnEnter() const       { return false; }
bool KSSLSettings::warnOnUnencrypted() const { return false; }
bool KSSLSettings::warnOnLeave() const       { return false; }

#include <kparts/plugin.h>

KParts::Plugin* KParts::Plugin::loadPlugin( QObject * parent, const char* libname ) { return 0; }

