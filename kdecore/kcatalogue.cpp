#include "kcatalogue.h"

#include <kdebug.h>

#include <stdlib.h>

#include <qfile.h>

char *find_msg(struct loaded_l10nfile *domain_file,
	       const char *msgid);

#ifndef KDE_USE_FINAL // with --enable-final, we're getting this from libintl.cpp
struct loaded_l10nfile
{
  const char *filename;
  int decided;

  const void *data;

  struct loaded_l10nfile *next;
  struct loaded_l10nfile *successor[1];

  loaded_l10nfile() : filename(0), decided(0), data(0), next(0) {}
};
#endif

class KCataloguePrivate
{
public:
  QString name;

  loaded_l10nfile domain;
};

KCatalogue::KCatalogue(const QString & name)
  : d( new KCataloguePrivate )
{
  d->name = name;
}

KCatalogue::KCatalogue(const KCatalogue & rhs)
{
  d = new KCataloguePrivate;

  *this = rhs;
}

KCatalogue & KCatalogue::operator=(const KCatalogue & rhs)
{
  d->name = rhs.d->name;
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

void KCatalogue::setFileName( const QString & fileName )
{
  // nothing to do if the file name is already the same
  //  if ( this->fileName() == fileName ) return;

  doUnload();

  QCString newFileName = QFile::encodeName( fileName );

  if ( !fileName.isEmpty() )
    {
      // set file name
      char *filename = new char[ newFileName.length() + 1 ];
      ::strcpy( filename, newFileName );
      d->domain.filename = filename;
    }
}

QString KCatalogue::fileName() const
{
  return QFile::decodeName( d->domain.filename );
}

const char * KCatalogue::translate(const char * msgid) const
{
  return ::find_msg( &d->domain, msgid );
}

void KCatalogue::doUnload()
{
  // allocated by gettext using malloc!!!
  //  if ( d->domain.data )
  //  free( const_cast<void *>(d->domain.data) );
  d->domain.data = 0;

  // free name
  delete [] const_cast<char *>(d->domain.filename);
  d->domain.filename = 0;

  d->domain.decided = 0;
}
