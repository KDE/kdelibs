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

#define QT_NO_XFTFREETYPE 1
#include <private/qfontengine_p.h>
#include <qfontdatabase.h>
#include <qfont.h>
#include "khtml_settings.h"
#include <qwidget.h>

#if 0
class QFakeFontEngine : public QFontEngineXLFD
{
public:
    QFakeFontEngine( XFontStruct *fs, const char *name, int size );
    ~QFakeFontEngine();

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const;

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const { return 0; }
    int minRightBearing() const { return 0; }

#ifdef Q_WS_X11
    int cmap() const;
#endif
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;
    inline int size() const { return _size; }

private:
    int _size;
};

QFakeFontEngine::QFakeFontEngine( XFontStruct *fs, const char *name, int size )
    : QFontEngineXLFD( fs,  name,  0), _size( size )
{

}

QFakeFontEngine::~QFakeFontEngine()
{
}

QFontEngine::Error QFakeFontEngine::stringToCMap( const QChar *, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    memset( glyphs, 0, len * sizeof( glyph_t ) );
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
    int l = qRound( _size * 0.15 );
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

const char *QFakeFontEngine::name() const
{
    return "null";
}

bool QFakeFontEngine::canRender( const QChar *, int )
{
    return TRUE;
}

QFontEngine::Type QFakeFontEngine::type() const
{
    return Box;
}


QFontEngine *
QFontDatabase::findFont( QFont::Script script, const QFontPrivate *fp,
			 const QFontDef &request, int )
{
    QCString xlfd = "-";
    xlfd += "*";
    xlfd += "-";
    xlfd += "*"; // request.family.isEmpty() ? "*" : request.family.latin1();

    xlfd += "-";
    xlfd += "*";
    xlfd += "-";
    xlfd += ( request.italic ? "i" : "r" );

    xlfd += "-";
    xlfd += "*";
    // ### handle add-style
    xlfd += "-*-";

    xlfd += QString::number( request.pixelSize ).latin1();
    xlfd += "-";
    xlfd += "*";
    xlfd += "-";
    xlfd += "*";
    xlfd += "-";
    xlfd += "*";
    xlfd += "-";

    // ### handle cell spaced fonts
    xlfd += "*";
    xlfd += "-";
    xlfd += "*";
    xlfd += "-";
    xlfd += "*-*";

    qDebug( "findFont '%s' '%s' %d xlfd: '%s' italic=", request.addStyle.latin1(), request.family.latin1(), request.pixelSize, xlfd.data() );
    QFontEngine *fe = 0;

    XFontStruct *xfs;
    if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(), xlfd.data() ) ) ) {
        qDebug( "no font found" );
    }

    fe = new QFakeFontEngine( xfs, xlfd.data(),request.pixelSize );

    // fe->setScale( scale );

    QFontCache::Key key( request, script, fp->screen );
    QFontCache::instance->insertEngine( key, fe );
    return fe;
}

#else

QFontEngine *
QFontDatabase::findFont( QFont::Script script, const QFontPrivate *fp,
			 const QFontDef &request, int )
{
    qDebug( "findFont script=%d family='%s' pixelsize=%d italic=%d fixed=%d weight=%d stretch=%d styleHint=%d" ,
            script,
            request.family.latin1(),
            request.pixelSize,
            request.italic,
            request.fixedPitch,
            request.weight,
            request.stretch,
            request.styleHint
            );

    QFontEngine *fe = new QFontEngineBox( request.pixelSize );
    QFontCache::Key key( request, script, fp->screen );
    QFontCache::instance->insertEngine( key, fe );
    return fe;
}
#endif

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

#include <qapplication.h>
#include <qpalette.h>

void QApplication::setPalette( const QPalette &palette, bool informWidgets,
                               const char* className )
{
      qDebug("QApplication::setPalette");
}

