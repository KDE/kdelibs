#ifndef KCATALOGUE_H
#define KCATALOGUE_H

#include <qstring.h>

struct loaded_l10nfile;

class KCataloguePrivate;

class KCatalogue
{
public:
  KCatalogue(const QString & name);
  virtual ~KCatalogue();

  QString name() const;
  void setFileName( const QString & fileName );
  const char * translate( const char * msgid ) const;

private:
  KCatalogue(const KCatalogue &);
  KCatalogue & operator = ( const KCatalogue &);

  void doUnload();

private:
  KCataloguePrivate * d;
};

#endif
