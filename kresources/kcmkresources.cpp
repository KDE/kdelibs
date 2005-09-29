/*
    This file is part of libkresources.

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qlayout.h>

#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include "configpage.h"

#include "kcmkresources.h"

typedef KGenericFactory<KCMKResources, QWidget> ResourcesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kresources, ResourcesFactory( "kcmkresources" ) )

KCMKResources::KCMKResources( QWidget *parent, const char *name, const QStringList& )
  : KCModule( ResourcesFactory::instance(), parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mConfigPage = new KRES::ConfigPage( this );
  layout->addWidget( mConfigPage );
  connect( mConfigPage, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );
  setButtons( Help | Apply );
  KAboutData *about =
   new KAboutData( I18N_NOOP( "kcmkresources" ),
                   I18N_NOOP( "KDE Resources configuration module" ),
                   0, 0, KAboutData::License_GPL,
                   I18N_NOOP( "(c) 2003 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData(about);
}

void KCMKResources::load()
{
  mConfigPage->load();
}

void KCMKResources::save()
{
  mConfigPage->save();
}

void KCMKResources::defaults()
{
  mConfigPage->defaults();
}

#include "kcmkresources.moc"
