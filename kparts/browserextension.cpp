/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
#include "browserextension.h"

#include <qtimer.h>

using namespace KParts;

const char *OpenURLEvent::s_strOpenURLEvent = "KParts/BrowserExtension/OpenURLevent";

class OpenURLEvent::OpenURLEventPrivate
{
public:
  OpenURLEventPrivate()
  {
  }
  ~OpenURLEventPrivate()
  {
  }
};

OpenURLEvent::OpenURLEvent( ReadOnlyPart *part, const KURL &url, const URLArgs &args )
: Event( s_strOpenURLEvent ), m_part( part ), m_url( url ), m_args( args )
{
//  d = new OpenURLEventPrivate();
}

OpenURLEvent::~OpenURLEvent()
{
//  delete d;
}

namespace KParts
{

struct URLArgsPrivate
{
   QStringList docState;
};

};

URLArgs::URLArgs()
{
  reload = false;
  xOffset = 0;
  yOffset = 0;
  trustedSource = false;
  d = 0;
}

URLArgs::URLArgs( const URLArgs &args )
{
  d = 0;
  (*this) = args;
}

URLArgs &URLArgs::operator=(const URLArgs &args)
{
  if (this == &args) return *this;

  delete d; d= 0;

  reload = args.reload;
  xOffset = args.xOffset;
  yOffset = args.yOffset;
  serviceType = args.serviceType;
  postData = args.postData;
  frameName = args.frameName;
  docState = args.docState;
  trustedSource = args.trustedSource;
  /*
  if ( args.d )
  {
     d = new URLArgsPrivate;
  }*/
  return *this;
}


URLArgs::URLArgs( bool _reload, int _xOffset, int _yOffset, const QString &_serviceType )
{
  reload = _reload;
  xOffset = _xOffset;
  yOffset = _yOffset;
  serviceType = _serviceType;
  d = 0;
}

URLArgs::~URLArgs()
{
  delete d; d = 0;
}

namespace KParts
{

struct WindowArgsPrivate
{
};

};

WindowArgs::WindowArgs()
{
    fullscreen = false;
    menuBarVisible = true;
    toolBarsVisible = true;
    statusBarVisible = true;
    resizable = true;
    d = 0;
}

WindowArgs::WindowArgs( const WindowArgs &args )
{
    d = 0;
    (*this) = args;
}

WindowArgs &WindowArgs::operator=( const WindowArgs &args )
{
    if ( this == &args ) return *this;

    delete d; d = 0;

    geometry = args.geometry;
    fullscreen = args.fullscreen;
    menuBarVisible = args.menuBarVisible;
    toolBarsVisible = args.toolBarsVisible;
    statusBarVisible = args.statusBarVisible;
    resizable = args.resizable;

    /*
    if ( args.d )
    {
      [ ... ]
    }
    */

    return *this;
}

WindowArgs::WindowArgs( const QRect &_geometry, bool _fullscreen, bool _menuBarVisible,
                        bool _toolBarsVisible, bool _statusBarVisible, bool _resizable )
{
    d = 0;
    geometry = _geometry;
    fullscreen = _fullscreen;
    menuBarVisible = _menuBarVisible;
    toolBarsVisible = _toolBarsVisible;
    statusBarVisible = _statusBarVisible;
    resizable = _resizable;
}

namespace KParts
{

class BrowserExtensionPrivate
{
public:
  BrowserExtensionPrivate()
  {
  }
  ~BrowserExtensionPrivate()
  {
  }

  KURL m_delayedURL;
  KParts::URLArgs m_delayedArgs;
};

};

BrowserExtension::BrowserExtension( KParts::ReadOnlyPart *parent,
                                    const char *name )
: QObject( parent, name), m_part( parent )
{
  d = new BrowserExtensionPrivate;

  connect( m_part, SIGNAL( completed() ),
           this, SLOT( slotCompleted() ) );
  connect( this, SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
           this, SLOT( slotOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );
}

BrowserExtension::~BrowserExtension()
{
  delete d;
}

void BrowserExtension::setURLArgs( const URLArgs &args )
{
  m_args = args;
}

URLArgs BrowserExtension::urlArgs()
{
  return m_args;
}

int BrowserExtension::xOffset()
{
  return 0;
}

int BrowserExtension::yOffset()
{
  return 0;
}

void BrowserExtension::saveState( QDataStream &stream )
{
  stream << m_part->url() << (Q_INT32)xOffset() << (Q_INT32)yOffset();
}

void BrowserExtension::restoreState( QDataStream &stream )
{
  KURL u;
  Q_INT32 xOfs, yOfs;
  stream >> u >> xOfs >> yOfs;

  URLArgs args( urlArgs() );
  args.xOffset = xOfs;
  args.yOffset = yOfs;

  setURLArgs( args );

  m_part->openURL( u );
}

void BrowserExtension::slotCompleted()
{
  //empty the argument stuff, to avoid bogus/invalid values when opening a new url
  setURLArgs( URLArgs() );
}

void BrowserExtension::slotOpenURLRequest( const KURL &url, const KParts::URLArgs &args )
{
    d->m_delayedURL = url;
    d->m_delayedArgs = args;
    QTimer::singleShot( 0, this, SLOT( slotEmitOpenURLRequestDelayed() ) );
}

void BrowserExtension::slotEmitOpenURLRequestDelayed()
{
    KURL u = d->m_delayedURL;
    KParts::URLArgs args = d->m_delayedArgs;
    d->m_delayedURL = KURL();
    d->m_delayedArgs = URLArgs();
    emit openURLRequestDelayed( u, args );
    // tricky: do not do anything here! (no access to member variables, etc.)
}

QMap<QCString,QCString> BrowserExtension::actionSlotMap()
{
  QMap<QCString,QCString> res;

  res.insert( "cut", SLOT( cut() ) );
  res.insert( "copy", SLOT( copy() ) );
  res.insert( "paste", SLOT( paste() ) );
  res.insert( "trash", SLOT( trash() ) );
  res.insert( "del", SLOT( del() ) );
  res.insert( "shred", SLOT( shred() ) );
  res.insert( "properties", SLOT( properties() ) );
  res.insert( "editMimeType", SLOT( editMimeType() ) );
  res.insert( "print", SLOT( print() ) );
  // Tricky. Those aren't actions in fact, but simply methods that a browserextension
  // can have or not. No need to return them here.
  //res.insert( "reparseConfiguration", SLOT( reparseConfiguration() ) );
  //res.insert( "refreshMimeTypes", SLOT( refreshMimeTypes() ) );
  // nothing for setSaveViewPropertiesLocally either

  return res;
}

namespace KParts
{

class BrowserHostExtension::BrowserHostExtensionPrivate
{
public:
  BrowserHostExtensionPrivate()
  {
  }
  ~BrowserHostExtensionPrivate()
  {
  }

  KParts::ReadOnlyPart *m_part;
};

};

BrowserHostExtension::BrowserHostExtension( KParts::ReadOnlyPart *parent, const char *name )
 : QObject( parent, name )
{
  d = new BrowserHostExtensionPrivate;
  d->m_part = parent;
}

BrowserHostExtension::~BrowserHostExtension()
{
  delete d;
}

QStringList BrowserHostExtension::frameNames() const
{
  return QStringList();
}

const QList<KParts::ReadOnlyPart> BrowserHostExtension::frames() const
{
  return QList<KParts::ReadOnlyPart>();
}

bool BrowserHostExtension::openURLInFrame( const KURL &, const KParts::URLArgs & )
{
  return false;
}
#include "browserextension.moc"
