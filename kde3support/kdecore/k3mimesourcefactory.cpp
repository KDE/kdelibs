/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
*/

#include "k3mimesourcefactory.h"

#include <kdebug.h>
#include <kiconloader.h>

class K3MimeSourceFactoryPrivate
{
public:
  inline K3MimeSourceFactoryPrivate (KIconLoader* loader)
	: kil (loader)
  {}

  KIconLoader* kil;
};

void K3MimeSourceFactory::install()
{
    // Set default mime-source factory
    // XXX: This is a hack. Make our factory the default factory, but add the
    // previous default factory to the list of factories. Why? When the default
    // factory can't resolve something, it iterates in the list of factories.
    // But it QWhatsThis only uses the default factory. So if there was already
    // a default factory (which happens when using an image library using uic),
    // we prefer KDE's factory and so we put that old default factory in the
    // list and use KDE as the default. This may speed up things as well.
    Q3MimeSourceFactory* oldDefaultFactory = Q3MimeSourceFactory::takeDefaultFactory();
    K3MimeSourceFactory* mimeSourceFactory = new K3MimeSourceFactory(KIconLoader::global());
    Q3MimeSourceFactory::setDefaultFactory( mimeSourceFactory );
    if ( oldDefaultFactory ) {
        Q3MimeSourceFactory::addFactory( oldDefaultFactory );
    }
}

K3MimeSourceFactory::K3MimeSourceFactory (KIconLoader* loader)
  : Q3MimeSourceFactory (),
	d (new K3MimeSourceFactoryPrivate (loader ? loader : KIconLoader::global()))
{
}

K3MimeSourceFactory::~K3MimeSourceFactory()
{
  delete d;
}

QString K3MimeSourceFactory::makeAbsolute (const QString& absOrRelName, const QString& context) const
{
  QString myName;
  QString myContext;

  const int pos = absOrRelName.indexOf ('|');
  if (pos > -1)
	{
	  myContext = absOrRelName.left (pos);
	  myName = absOrRelName.right (absOrRelName.length() - myContext.length() - 1);
	}

  QString result;

  if (myContext == "desktop")
	{
	  result = d->kil->iconPath (myName, KIconLoader::Desktop);
	}
  else if (myContext == "toolbar")
	{
	  result = d->kil->iconPath (myName, KIconLoader::Toolbar);
	}
  else if (myContext == "maintoolbar")
	{
	  result = d->kil->iconPath (myName, KIconLoader::MainToolbar);
	}
  else if (myContext == "small")
	{
	  result = d->kil->iconPath (myName, KIconLoader::Small);
	}
  else if (myContext == "user")
	{
	  result = d->kil->iconPath (myName, KIconLoader::User);
	}

  if (result.isEmpty())
	result = Q3MimeSourceFactory::makeAbsolute (absOrRelName, context);

  return result;
}

void K3MimeSourceFactory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

