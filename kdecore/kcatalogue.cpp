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
  QCString newFileName = QFile::encodeName( fileName );

  // nothing to do if the file name is already the same
  if ( newFileName == d->domain.filename ) return;

  doUnload();

  if ( !fileName.isEmpty() )
    {
      // set file name
      char *filename = new char[ newFileName.length() + 1 ];
      qstrcpy( filename, newFileName );
      d->domain.filename = filename;
    }
}

const char * KCatalogue::translate(const char * msgid) const
{
  return ::find_msg( &d->domain, msgid );
}

void KCatalogue::doUnload()
{
  // allocated by gettext using malloc!!!
  if ( d->domain.data )
    free( const_cast<void *>(d->domain.data) );
  d->domain.data = 0;

  // free name
  delete const_cast<char *>(d->domain.filename);
  d->domain.filename = 0;

  d->domain.decided = 0;
}
