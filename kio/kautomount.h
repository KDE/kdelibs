#ifndef __auto_mount_h__
#define __auto_mount_h__

#include <qobject.h>
#include <qstring.h>

namespace KIO {
class Job;
}

class KAutoMount : public QObject
{
  Q_OBJECT
public:
  KAutoMount( bool _readonly, const QString& _format, const QString& _device, const QString& _mountpoint,
              const QString & _desktopFile, bool _show_filemanager_window = true );
    
protected slots:
  void slotResult( KIO::Job * );
    
protected:
  QString m_strDevice;
  bool m_bShowFilemanagerWindow;
  QString m_desktopFile;
};

class KAutoUnmount : public QObject
{
  Q_OBJECT
public:
  KAutoUnmount( const QString & _mountpoint, const QString & _desktopFile );
    
protected slots:
  void slotResult( KIO::Job * );
private:
  QString m_desktopFile;
};

#endif
