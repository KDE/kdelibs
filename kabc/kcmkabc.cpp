/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <qlayout.h>

#include <klocale.h>
#include <kaboutdata.h>
#include <kresources/resourcesconfigpage.h>

#include "kcmkabc.h"

KCMkabc::KCMkabc( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mConfigPage = new KRES::ResourcesConfigPage( "contact", "kabcrc", this );
  layout->addWidget( mConfigPage );
  connect( mConfigPage, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );
}

void KCMkabc::load()
{
  mConfigPage->load();
}

void KCMkabc::save()
{
  mConfigPage->save();
}

void KCMkabc::defaults()
{
  mConfigPage->defaults();
}

const KAboutData* KCMkabc::aboutData() const
{
   KAboutData *about =
   new KAboutData( I18N_NOOP( "kcmkabc" ), I18N_NOOP( "Address book configuration module" ),
                   0, 0, KAboutData::License_GPL,
                   I18N_NOOP( "(c), 2003 Tobias Koenig" ) );

   about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

   return about;
}


extern "C"
{
  KCModule *create_kabc( QWidget *parent, const char * ) {
    return new KCMkabc( parent, "kcmkabc" );
  }
}

#include "kcmkabc.moc"

