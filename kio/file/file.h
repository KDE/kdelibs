#ifndef __file_h__
#define __file_h__ "$Id$"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>

#include <qobject.h>
#include <qintdict.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kio/global.h>
#include <kio/slavebase.h>

// Note that this header file is installed, so think twice
// before breaking binary compatibility (read: it is forbidden :)

class FileProtocol : public QObject, public KIO::SlaveBase
{
  Q_OBJECT
public:
  FileProtocol( const QCString &pool, const QCString &app);
  virtual ~FileProtocol() { }

  virtual void get( const KURL& url );
  virtual void put( const KURL& url, int _mode,
		    bool _overwrite, bool _resume );
  virtual void copy( const KURL &src, const KURL &dest,
                     int mode, bool overwrite );
  virtual void rename( const KURL &src, const KURL &dest,
                       bool overwrite );
  virtual void symlink( const QString &target, const KURL &dest,
                        bool overwrite );

  virtual void stat( const KURL& url );
  virtual void listDir( const KURL& url );
  virtual void mkdir( const KURL& url, int permissions );
  virtual void chmod( const KURL& url, int permissions );
  virtual void del( const KURL& url, bool isfile);

  /**
   * Special commands supported by this slave:
   * 1 - mount
   * 2 - unmount
   * 3 - shred
   */
  virtual void special( const QByteArray &data);
  void unmount( const QString& point );
  void mount( bool _ro, const char *_fstype, const QString& dev, const QString& point );

protected slots:
  void slotProcessedSize( unsigned long _bytes );
  void slotInfoMessage( const QString & msg );

protected:

  bool createUDSEntry( const QString & filename, const QCString & path, KIO::UDSEntry & entry, short int details );

  QIntDict<QString> usercache;      // maps long ==> QString *
  QIntDict<QString> groupcache;

  class FileProtocolPrivate;
  FileProtocolPrivate *d;
};

#endif
