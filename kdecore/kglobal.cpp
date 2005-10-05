/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/*
 * kglobal.cpp -- Implementation of class KGlobal.
 * Author:	Sirtaj Singh Kang
 * Generated:	Sat May  1 02:08:43 EST 1999
 */

#undef QT3_SUPPORT

#include <qglobal.h>
#include <qlist.h>
#include <qset.h>
#include <qwindowdefs.h>
#include "kglobal.h"

#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <qapplication.h>
#include "kstaticdeleter.h"

#include <qfont.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif

#include <qcolormap.h>
#include <qwidget.h>

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   qFatal("Fatal error: you need to have a KInstance object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

static void kglobal_init();

KStandardDirs *KGlobal::dirs()
{
    MYASSERT(_instance);

    return _instance->dirs();
}

KConfig	*KGlobal::config()
{
    MYASSERT(_instance);

    return _instance->config();
}

KSharedConfig *KGlobal::sharedConfig()
{
    MYASSERT(_instance);

    return _instance->sharedConfig();
}

KIconLoader *KGlobal::iconLoader()
{
    MYASSERT(_instance);

    return _instance->iconLoader();
}

KInstance *KGlobal::instance()
{
    MYASSERT(_instance);
    return _instance;
}

KLocale	*KGlobal::locale()
{
    if( _locale == 0 ) {
	if (!_instance)
	   return 0;
        kglobal_init();

        // will set _locale if it works - otherwise 0 is returned
        KLocale::initInstance();
    }

    return _locale;
}

KCharsets *KGlobal::charsets()
{
    if( _charsets == 0 ) {
        _charsets =new KCharsets();
        kglobal_init();
    }

    return _charsets;
}

void KGlobal::setActiveInstance(KInstance *i)
{
    _activeInstance = i;
    if (i && _locale)
	_locale->setActiveCatalog(QString::fromUtf8(i->instanceName()));
}

/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString("myText");
 */
const QString &
KGlobal::staticQString(const char *str)
{
   return staticQString(QLatin1String(str));
}

class KStringDict : public QSet<QString>
{
public:
   KStringDict() { }
};

/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString(i18n("My Text"));
 */
const QString &
KGlobal::staticQString(const QString &str)
{
    if (!_stringDict) {
      _stringDict = new KStringDict;
      kglobal_init();
    }

   return *_stringDict->insert(str);
}

class KStaticDeleterList: public QList<KStaticDeleterBase*>
{
public:
   KStaticDeleterList() { }
};

void
KGlobal::registerStaticDeleter(KStaticDeleterBase *obj)
{
   if (!_staticDeleters)
      kglobal_init();
   if (_staticDeleters->indexOf(obj) == -1)
      _staticDeleters->append(obj);
}

void
KGlobal::unregisterStaticDeleter(KStaticDeleterBase *obj)
{
   if (_staticDeleters)
      _staticDeleters->removeAll(obj);
}

void
KGlobal::deleteStaticDeleters()
{
    if (!KGlobal::_staticDeleters)
        return;

    for(;_staticDeleters->count();)
    {
        _staticDeleters->takeFirst()->destructObject();
    }

    delete KGlobal::_staticDeleters;
    KGlobal::_staticDeleters = 0;
}

// The Variables

KStringDict     *KGlobal::_stringDict   = 0;
KInstance       *KGlobal::_instance     = 0;
KInstance       *KGlobal::_activeInstance = 0;
KLocale         *KGlobal::_locale	= 0;
KCharsets       *KGlobal::_charsets	= 0;
KStaticDeleterList *KGlobal::_staticDeleters = 0;

__attribute__((destructor))
static void kglobal_freeAll()
{
    delete KGlobal::_locale;
    KGlobal::_locale = 0;
    delete KGlobal::_charsets;
    KGlobal::_charsets = 0;
    delete KGlobal::_stringDict;
    KGlobal::_stringDict = 0;
    KGlobal::deleteStaticDeleters();
    // so that we don't hold a reference and see memory leaks :/
    KGlobal::setActiveInstance(0);
}

static void kglobal_init()
{
    if (KGlobal::_staticDeleters)
        return;

    KGlobal::_staticDeleters = new KStaticDeleterList;
}

int kasciistricmp( const char *str1, const char *str2 )
{
    const unsigned char *s1 = (const unsigned char *)str1;
    const unsigned char *s2 = (const unsigned char *)str2;
    int res;
    unsigned char c1, c2;

    if ( !s1 || !s2 )
        return s1 ? 1 : (s2 ? -1 : 0);
    if ( !*s1 || !*s2 )
        return *s1 ? 1 : (*s2 ? -1 : 0);
    for (;*s1; ++s1, ++s2) {
        c1 = *s1; c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 'a' - 'A';

        if ((res = c1 - c2))
            break;
    }
    return *s1 ? res : (*s2 ? -1 : 0);
}

#ifdef Q_WS_X11
//static GC*	app_gc_ro	= 0;		// read-only GC
static GC*	app_gc_tmp	= 0;		// temporary GC
//static GC*	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC*	app_gc_tmp_m	= 0;		// temporary GC (monochrome)

static GC create_gc( int scrn, bool monochrome )
{
    GC gc;
    if ( monochrome ) {
	Pixmap pm = XCreatePixmap( QX11Info::display(), RootWindow( QX11Info::display(), scrn ), 8, 8, 1 );
	gc = XCreateGC( QX11Info::display(), pm, 0, 0 );
	XFreePixmap( QX11Info::display(), pm );
    } else {
	if ( QX11Info::appDefaultVisual( scrn ) ) {
	    gc = XCreateGC( QX11Info::display(), RootWindow( QX11Info::display(), scrn ), 0, 0 );
	} else {
	    Window w;
	    XSetWindowAttributes a;
            QColormap colormap = QColormap::instance( scrn );
	    a.background_pixel = colormap.pixel( Qt::black );
	    a.border_pixel = a.background_pixel;
	    a.colormap = QX11Info::appColormap( scrn );
	    w = XCreateWindow( QX11Info::display(), RootWindow( QX11Info::display(), scrn ), 0, 0, 100, 100,
			       0, QX11Info::appDepth( scrn ), InputOutput,
			       (Visual*)QX11Info::appVisual( scrn ),
			       CWBackPixel|CWBorderPixel|CWColormap, &a );
	    gc = XCreateGC( QX11Info::display(), w, 0, 0 );
	    XDestroyWindow( QX11Info::display(), w );
	}
    }
    XSetGraphicsExposures( QX11Info::display(), gc, False );
    return gc;
}

// #### remove me, shouldn't be necessary anymore, rumors have it
// xlib caches GCs client side
KDE_EXPORT GC kde_xget_temp_gc( int scrn, bool monochrome );		// get temporary GC
GC kde_xget_temp_gc( int scrn, bool monochrome )		// get temporary GC
{
    // #####
    int screenCount = ScreenCount(QX11Info::display());
    if ( scrn < 0 || scrn >= screenCount ) {
	qFatal("invalid screen (tmp) %d %d", scrn, screenCount );
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m )			// create GC for bitmap
	    memset( (app_gc_tmp_m = new GC[screenCount]), 0, screenCount * sizeof( GC ) );
	if ( !app_gc_tmp_m[scrn] )
	    app_gc_tmp_m[scrn] = create_gc( scrn, true );
	gc = app_gc_tmp_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_tmp )
	    memset( (app_gc_tmp = new GC[screenCount]), 0, screenCount * sizeof( GC ) );
	if ( !app_gc_tmp[scrn] )
	    app_gc_tmp[scrn] = create_gc( scrn, false );
	gc = app_gc_tmp[scrn];
    }
    return gc;
}
#endif
