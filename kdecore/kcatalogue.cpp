/* This file is part of the KDE libraries
   Copyright (c) 2001 Hans Petter Bieker <bieker@kde.org>

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

#include <config.h>

#include <qfile.h>

#include <kdebug.h>

#include "kcatalogue.h"
#include "kstandarddirs.h"

char *k_nl_find_msg(struct kde_loaded_l10nfile *domain_file,
	       const char *msgid);
void k_nl_unload_domain (struct loaded_domain *domain);

#ifndef KDE_USE_FINAL // with --enable-final, we're getting this from libintl.cpp
struct kde_loaded_l10nfile
{
  const char *filename;
  int decided;

  const void *data;

  kde_loaded_l10nfile() : filename(0), decided(0), data(0) {}
};
#endif

class KCataloguePrivate
{
public:
  QString name;
  QString language;
  int	  pluralType;

  kde_loaded_l10nfile domain;
};

KCatalogue::KCatalogue(const QString & name, const QString & language )
  : d( new KCataloguePrivate )
{
  d->name = name;
  d->language = language;
  // at the moment we do not know more. To find out the plural type we first have to look into
  // kdelibs.mo for the language. And for this we already need a catalog object. So this data
  // has to be set after we have the first catalog objects.
  d->pluralType = -1; 

  QString path = QString::fromLatin1("%1/LC_MESSAGES/%2.mo")
    .arg( d->language )
    .arg( d->name );

  setFileName( locate( "locale", path ) );
    
}

KCatalogue::KCatalogue(const KCatalogue & rhs)
  : d( new KCataloguePrivate )
{
  *this = rhs;
}

KCatalogue & KCatalogue::operator=(const KCatalogue & rhs)
{
  d->name       = rhs.d->name;
  d->language   = rhs.d->language;
  d->pluralType = rhs.d->pluralType;
  setFileName( rhs.fileName() );

  return *this;
}

KCatalogue::~KCatalogue()
{
  doUnload();

  delete d;
}

QString KCatalogue::name() const
{
  return d->name;
}

QString KCatalogue::language() const
{
  return d->language;
}	  

void KCatalogue::setPluralType( int pluralType) 
{
  d->pluralType = pluralType;
}

int KCatalogue::pluralType() const
{
  return d->pluralType;
}

  
void KCatalogue::setFileName( const QString & fileName )
{
  // nothing to do if the file name is already the same
  if ( this->fileName() == fileName ) return;

  doUnload();

  QCString newFileName = QFile::encodeName( fileName );

  if ( !fileName.isEmpty() )
    {
      // set file name
      char *filename = new char[ newFileName.length() + 1 ];
      ::qstrcpy( filename, newFileName );
      d->domain.filename = filename;
    }
}

QString KCatalogue::fileName() const
{
  return QFile::decodeName( d->domain.filename );
}

const char * KCatalogue::translate(const char * msgid) const
{
  return ::k_nl_find_msg( &d->domain, msgid );
}

void KCatalogue::doUnload()
{
  // use gettext's unloader
  if ( d->domain.data )
    ::k_nl_unload_domain( (struct loaded_domain *)d->domain.data );
  d->domain.data = 0;

  // free name
  delete [] const_cast<char *>(d->domain.filename);
  d->domain.filename = 0;

  d->domain.decided = 0;
}
