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

class BrowserExtensionPrivate
{
public:
  BrowserExtensionPrivate()
  {
  }
  ~BrowserExtensionPrivate()
  {
  }
};

};

BrowserExtension::BrowserExtension( KParts::ReadOnlyPart *parent,
				    const char *name )
: QObject( parent, name), m_part( parent )
{
  d = new BrowserExtensionPrivate;

  connect( m_part, SIGNAL( completed() ),
	   this, SLOT( slotCompleted() ) );
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

QMap<QCString,QCString> BrowserExtension::actionSlotMap()
{
  QMap<QCString,QCString> res;

  res.insert( "cut", SLOT( cut() ) );
  res.insert( "copy", SLOT( copy() ) );
  res.insert( "pastecut", SLOT( pastecut() ) );
  res.insert( "pastecopy", SLOT( pastecopy() ) );
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
