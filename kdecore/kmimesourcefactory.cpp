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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "kmimesourcefactory.h"

class KMimeSourceFactoryPrivate
{
public:
  inline KMimeSourceFactoryPrivate (KIconLoader* loader)
	: kil (loader)
  {}
  
  KIconLoader* kil;
};

KMimeSourceFactory::KMimeSourceFactory (KIconLoader* loader)
  : QMimeSourceFactory (),
	d (new KMimeSourceFactoryPrivate (loader))
{
}

KMimeSourceFactory::~KMimeSourceFactory()
{
  delete d;
}

QString KMimeSourceFactory::makeAbsolute (const QString& absOrRelName, const QString& context) const
{
  QString myName;
  QString myContext;

  const int pos = absOrRelName.find ('|');
  if (pos > -1)
	{
	  myContext = absOrRelName.left (pos);
	  myName = absOrRelName.right (absOrRelName.length() - myContext.length() - 1);
	}

  QString result;

  if (myContext == "desktop")
	{
	  result = d->kil->iconPath (myName, KIcon::Desktop);
	}
  else if (myContext == "toolbar")
	{	 
	  result = d->kil->iconPath (myName, KIcon::Toolbar);
	}
  else if (myContext == "maintoolbar")
	{
	  result = d->kil->iconPath (myName, KIcon::MainToolbar);
	}
  else if (myContext == "small")
	{
	  result = d->kil->iconPath (myName, KIcon::Small);
	}
  else if (myContext == "user")
	{	  
	  result = d->kil->iconPath (myName, KIcon::User);
	}

  if (result.isEmpty())
	result = QMimeSourceFactory::makeAbsolute (absOrRelName, context);
  
  return result;
}

void KMimeSourceFactory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

