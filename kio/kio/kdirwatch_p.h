/* Private Header for class of KDirWatchPrivate
 *
 * this separate header file is needed for MOC processing
 * because KDirWatchPrivate has signals and slots
 */

#ifndef _KDIRWATCH_P_H
#define _KDIRWATCH_P_H

#ifdef HAVE_FAM
#include <fam.h>
#endif

enum directoryStatus { Normal = 0, NonExistent };

class KDirWatchPrivate : public QObject
{
  Q_OBJECT

public:
  class Entry
  {
  public:
    time_t m_ctime;
    int m_clients;
    directoryStatus m_status;
    QPtrList<Entry> m_entries;
    QString path;

#ifdef HAVE_FAM
    FAMRequest fr;
#endif

#ifdef HAVE_DNOTIFY
    int dn_fd;
    bool dn_dirty;
    void propagate_dirty();
#endif
  };

  typedef QMap<QString,Entry> EntryMap;

  KDirWatchPrivate(int);
  ~KDirWatchPrivate();

  void resetList (bool);
  void addDir(const QString&, Entry*);
  void removeDir(const QString&, Entry*);
  bool stopDirScan( const QString& );
  bool restartDirScan( const QString& );
  void stopScan();
  void startScan(bool, bool);

  Entry* entry(const QString&);

public slots:
  void slotRescan();
  void famEventReceived(); // for FAM
  void slotActivated(); // for DNOTIFY

signals:
  // to be passed to KDirWatch
  void dirty (const QString& dir);
  void fileDirty (const QString& _file);
  void deleted (const QString& dir);

public:
  QTimer *timer;
  EntryMap m_mapDirs;

private:
  int freq;

#ifdef HAVE_FAM
  QSocketNotifier *sn;
  FAMConnection fc;
  bool use_fam;
  bool emitEvents;

  void checkFAMEvent(FAMEvent*);
#endif

#ifdef HAVE_DNOTIFY
  bool supports_dnotify;
  int mPipe[2];
  QTimer mTimer;
  QSocketNotifier *mSn;
  QIntDict<Entry> fd_Entry;  

  static void dnotify_handler(int, siginfo_t *si, void *);
#endif
};

#endif // KDIRWATCH_P_H

