#ifndef KABC_RESOURCENET_H
#define KABC_RESOURCENET_H

#include <kconfig.h>

#include <sys/types.h>

#include "resource.h"

class QTimer;

namespace KABC {

class FormatPlugin;

/**
  @internal
*/
class ResourceNet : public QObject, public Resource
{
  Q_OBJECT

public:
  ResourceNet( AddressBook *, const KConfig * );
  ResourceNet( AddressBook *, const KURL &url, FormatPlugin *format=0 );
  ~ResourceNet();

  bool open();
  void close();
  
  Ticket *requestSaveTicket();

  bool load();
  bool save( Ticket * );

  /**
   * Set url of directory to be used for saving.
   */
  void setUrl( const KURL & );

  /**
   * Return url of directory used for loading and saving the address book.
   */
  KURL url() const;

  /**
   * Returns a unique identifier.
   */
  virtual QString identifier() const;

  /**
   * Remove a addressee from its source.
   * This method is mainly called by KABC::AddressBook.
   */
  void removeAddressee( const Addressee& addr );

private:
  void init( const KURL &url, FormatPlugin *format );

  FormatPlugin *mFormat;

  KURL mUrl;
};

}
#endif
