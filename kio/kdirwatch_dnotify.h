#ifndef _KDIRWATCH_DNOTIFY_H
#define _KDIRWATCH_DNOTIFY_H

class KDirWatchNotifyHandler : public QObject
{
  Q_OBJECT
  
public:
  KDirWatchNotifyHandler();

public slots:
  void slotActivated();

signals:
  void activated();  

public:  
  int mPipe[2];
  QTimer mTimer;
  QSocketNotifier *mSn;
  QIntDict<KDirWatchPrivate::Entry> fd_Entry;
};

#endif