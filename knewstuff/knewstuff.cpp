/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "engine.h"

#include "knewstuff.h"

using namespace KNS;

KAction* KNS::standardAction(const QString& what,
                             const QObject *recvr,
                             const char *slot, KActionCollection* parent,
                             const char *name)
{
    return new KAction(i18n("Download New %1").arg(what), "knewstuff",
                       0, recvr, slot, parent, name);
}

KNewStuff::KNewStuff( const QString &type, QWidget *parentWidget )
{
    mEngine = new Engine( this, type, parentWidget );
}

KNewStuff::KNewStuff( const QString &type, const QString &providerList, QWidget *parentWidget )
{
  mEngine = new Engine( this, type, providerList, parentWidget );
}

QString KNewStuff::type() const
{
  return mEngine->type();
}

QWidget *KNewStuff::parentWidget() const
{
  return mEngine->parentWidget();
}

KNewStuff::~KNewStuff()
{
  delete mEngine;
}

void KNewStuff::download()
{
  mEngine->download();
}

QString KNewStuff::downloadDestination( Entry * )
{
  return KGlobal::dirs()->saveLocation( "tmp" ) +
         KApplication::randomString( 10 );
}

void KNewStuff::upload()
{
  mEngine->upload();
}

void KNewStuff::upload( const QString &fileName, const QString previewName )
{
  mEngine->upload(fileName, previewName);
}
