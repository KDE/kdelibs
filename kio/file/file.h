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

class FileProtocol : public QObject, public KIO::SlaveBase
{
  Q_OBJECT
public:
  FileProtocol( const QCString &pool, const QCString &app);
  virtual ~FileProtocol() { }

  virtual void get( const QString& path, const QString& query, bool reload );
  virtual void put( const QString& path, int _mode,
			bool _overwrite, bool _resume );
  virtual void copy( const QString &src, const QString &dest,
                     int mode, bool overwrite );
  virtual void rename( const QString &src, const QString &dest,
                       bool overwrite );

  virtual void stat( const QString& path, const QString& query );
  virtual void listDir( const QString& path, const QString& query );
  virtual void mkdir( const QString& path, int permissions );
  virtual void chmod( const QString& path, int permissions );
  virtual void del( const QString& path, bool isfile);

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

  void createUDSEntry( const QString & filename, const QString & path, KIO::UDSEntry & entry );

  QIntDict<QString> usercache;      // maps long ==> QString *
  QIntDict<QString> groupcache;

};

#endif
