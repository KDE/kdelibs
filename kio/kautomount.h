#ifndef __auto_mount_h__
#define __auto_mount_h__

#include <qobject.h>
#include <qstring.h>

class KAutoMount : public QObject
{
  Q_OBJECT
public:
  KAutoMount( bool _readonly, const char *_format, const char *_device, const char *_mountpoint,
	      bool _show_filemanager_window = true );
    
public slots:
  void slotFinished( int );
  void slotError( int, int _id, const char *_text );
    
protected:
  QString m_strDevice;
  bool m_bShowFilemanagerWindow;
};

class KAutoUnmount : public QObject
{
  Q_OBJECT
public:
  KAutoUnmount( const char *_mountpoint );
    
public slots:
  void slotFinished( int );
  void slotError( int, int _id, const char *_text );
};

#endif
