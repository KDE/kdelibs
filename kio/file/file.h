#ifndef __file_h__
#define __file_h__ "$Id$"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>

#include <qintdict.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kio/global.h>
#include <kio/slavebase.h>

class FileProtocol : public KIO::SlaveBase
{
public:
  FileProtocol( KIO::Connection *_conn = 0);
  virtual ~FileProtocol() { }

  /*
  virtual void slotOpenConnection(const QString&, const QString&, const QString&) {}
  virtual void slotCloseConnection() {}
  */

  //virtual void testDir( const QString& path );

  virtual void get( const QString& path, const QString& query, bool reload );
  virtual void put( const QString& path, int _mode,
			bool _overwrite, bool _resume );
  virtual void copy( const QString &src, const QString &dest,
                     int mode, bool overwrite );
  virtual void rename( const QString &src, const QString &dest,
                       bool overwrite );

  virtual void stat( const QString& path );
  virtual void listDir( const QString& path );
  virtual void mkdir( const QString& path, int permissions );
  virtual void chmod( const QString& path, int permissions );
  virtual void del( const QString& path, bool isfile);

  /**
   * Special commands supported by this slave:
   * 1 - mount
   * 2 - unmount
   */
  virtual void special( const QByteArray &data);
  void unmount( const QString& point );
  void mount( bool _ro, const char *_fstype, const QString& dev, const QString& point );

protected:

  void createUDSEntry( const QString & filename, const QString & path, KIO::UDSEntry & entry );

  FILE* m_fPut;
  QIntDict<QString> usercache;      // maps long ==> QString *
  QIntDict<QString> groupcache;

};

#endif
