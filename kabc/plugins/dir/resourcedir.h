#ifndef KABC_RESOURCEDIR_H
#define KABC_RESOURCEDIR_H

#include <kconfig.h>
#include <kdirwatch.h>

#include <sys/types.h>

#include "resource.h"

class QTimer;

namespace KABC {

class FormatPlugin;

/**
  @internal
*/
class ResourceDir : public QObject, public Resource
{
  Q_OBJECT

public:
  ResourceDir( const KConfig* );
  ~ResourceDir();

  virtual void writeConfig( KConfig* );

  virtual bool doOpen();
  virtual void doClose();
  
  virtual Ticket *requestSaveTicket();

  virtual bool load();
  virtual bool save( Ticket * );

  /**
   * Set path to be used for saving.
   */
  void setPath( const QString & );

  /**
   * Return path used for loading and saving the address book.
   */
  QString path() const;

  /**
   * Set the format by name.
   */
  void setFormat( const QString &format );

  /**
   * Returns the format name.
   */
  QString format() const;
  
  /**
   * Remove a addressee from its source.
   * This method is mainly called by KABC::AddressBook.
   */
  virtual void removeAddressee( const Addressee& addr );

  /**
   * This method is called by an error handler if the application
   * crashed
   */
  virtual void cleanUp();

protected slots:
  void pathChanged();

protected:
  bool lock( const QString &path );
  void unlock( const QString &path );

private:
  FormatPlugin *mFormat;

  KDirWatch mDirWatch;

  QString mPath;
  QString mFormatName;
  QString mLockUniqueName;
};

}
#endif
