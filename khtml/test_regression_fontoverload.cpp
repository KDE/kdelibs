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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "ecma/kjs_proxy.h"
#define QT_NO_XRANDR 1
#define QT_NO_XFTFREETYPE 1
#include <private/qfontengine_p.h>
#include <qfontdatabase.h>
#include <qfont.h>
#include "khtml_settings.h"
#include <qwidget.h>
#include <assert.h>

class QFakeFontEngine : public QFontEngineXLFD
{
public:
    QFakeFontEngine( XFontStruct *fs, const char *name, int size );
    ~QFakeFontEngine();
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
    bool canRender( const QChar *string,  int len );
};

QFakeFontEngine::QFakeFontEngine( XFontStruct *fs, const char *name, int size )
    : QFontEngineXLFD( fs,  name,  0)
{

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

QFontEngine *
QFontDatabase::findFont( QFont::Script script, const QFontPrivate *fp,
			 const QFontDef &request, int )
{
    QString xlfd;
    QString family = request.family.lower();
    if ( family == "adobe courier" || family == "courier" || family == "fixed" ) {
        xlfd = courier_pickxlfd( request.pixelSize, request.italic, request.weight > 50 );
    }
    else if ( family == "Times New Roman" || family == "times" )
        xlfd = "-adobe-times-medium-r-normal--8-80-75-75-p-44-iso10646-1";
    else
        xlfd = helv_pickxlfd( request.pixelSize, request.italic, request.weight > 50 );

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

    // fe->setScale( scale );

    QFontCache::Key key( request, script, fp->screen );
    QFontCache::instance->insertEngine( key, fe );
    return fe;
}

bool QFontDatabase::isBitmapScalable( const QString &,
				      const QString &) const
{
    return true;
}

bool  QFontDatabase::isSmoothlyScalable( const QString &,
                                         const QString &) const
{
    return true;
}

const QString &KHTMLSettings::availableFamilies()
{
    if ( !avFamilies ) {
        avFamilies = new QString;
        *avFamilies = ",Adobe Courier,Arial,Comic Sans MS,Courier,Helvetica,Times,Times New Roman,Utopia,Fixed,";
    }

  return *avFamilies;
}

int QPaintDevice::x11AppDpiY( int )
{
    return 100;
}

int QPaintDevice::x11AppDpiX( int )
{
    return 100;
}

void QFont::insertSubstitution(const QString &,
                               const QString &)
{
}

void QFont::insertSubstitutions(const QString &,
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
        return QString::null;

    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot )
        return QString::null;

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

void QApplication::setPalette( const QPalette &, bool ,
                               const char*  )
{
}

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

