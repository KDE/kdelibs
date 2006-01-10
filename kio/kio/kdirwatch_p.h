/* Private Header for class of KDirWatchPrivate
 *
 * this separate header file is needed for MOC processing
 * because KDirWatchPrivate has signals and slots
 */

#ifndef _KDIRWATCH_P_H
#define _KDIRWATCH_P_H

#include <q3intdict.h>

#ifdef HAVE_FAM
#include <limits.h>
#include <fam.h>
#endif

#include <ctime>

#define invalid_ctime ((time_t)-1)

/* KDirWatchPrivate is a singleton and does the watching
 * for every KDirWatch instance in the application.
 */
class KDirWatchPrivate : public QObject
{
  Q_OBJECT
public:

  enum entryStatus { Normal = 0, NonExistent };
  enum entryMode { UnknownMode = 0, StatMode, DNotifyMode, INotifyMode, FAMMode };
  enum { NoChange=0, Changed=1, Created=2, Deleted=4 };

  struct Client {
    KDirWatch* instance;
    int count;
    // did the instance stop watching
    bool watchingStopped;
    // events blocked when stopped
    int pending;
  };

  class Entry
  {
  public:
    // the last observed modification time
    time_t m_ctime;
    // the last observed link count
    int m_nlink;
    entryStatus m_status;
    entryMode m_mode;
    bool isDir;
    // instances interested in events
    QList<Client *> m_clients;
    // nonexistent entries of this directory
    QList<Entry *> m_entries;
    QString path;

    int msecLeft, freq;

    void addClient(KDirWatch*);
    void removeClient(KDirWatch*);
    int clients();
    bool isValid() { return m_clients.count() || m_entries.count(); }

    bool dirty;
    void propagate_dirty();

#ifdef HAVE_FAM
    FAMRequest fr;
#endif

#ifdef HAVE_DNOTIFY
    int dn_fd;
#endif
#ifdef HAVE_INOTIFY
    int wd;
#endif
  };

  typedef QMap<QString,Entry> EntryMap;

  KDirWatchPrivate();
  ~KDirWatchPrivate();

  void resetList (KDirWatch*,bool);
  void useFreq(Entry* e, int newFreq);
  void addEntry(KDirWatch*,const QString&, Entry*, bool);
  void removeEntry(KDirWatch*,const QString&, Entry*);
  bool stopEntryScan(KDirWatch*, Entry*);
  bool restartEntryScan(KDirWatch*, Entry*, bool );
  void stopScan(KDirWatch*);
  void startScan(KDirWatch*, bool, bool);

  void removeEntries(KDirWatch*);
  void statistics();

  Entry* entry(const QString&);
  int scanEntry(Entry* e);
  void emitEvent(Entry* e, int event, const QString &fileName = QString());

  // Memory management - delete when last KDirWatch gets deleted
  void ref() { m_ref++; }
  bool deref() { return ( --m_ref == 0 ); }

 static bool isNoisyFile( const char *filename );

public slots:
  void slotRescan();
  void famEventReceived(); // for FAM
  void slotActivated(); // for DNOTIFY
  void slotRemoveDelayed();

public:
  QTimer *timer;
  EntryMap m_mapEntries;

  int freq;
  int statEntries;
  int m_nfsPollInterval, m_PollInterval;
  int m_ref;
  bool useStat(Entry*);

  bool delayRemove;
  QList<Entry *> removeList;

  bool rescan_all;
  QTimer rescan_timer;

#ifdef HAVE_FAM
  QSocketNotifier *sn;
  FAMConnection fc;
  bool use_fam;

  void checkFAMEvent(FAMEvent*);
  bool useFAM(Entry*);
#endif

#if defined(HAVE_DNOTIFY) || defined(HAVE_INOTIFY)
   QSocketNotifier *mSn;
#endif

#ifdef HAVE_DNOTIFY
  bool supports_dnotify;
  int mPipe[2];
  Q3IntDict<Entry> fd_Entry;

  static void dnotify_handler(int, siginfo_t *si, void *);
  static void dnotify_sigio_handler(int, siginfo_t *si, void *);
  bool useDNotify(Entry*);
#endif

#ifdef HAVE_INOTIFY
  bool supports_inotify;
  int m_inotify_fd;

  bool useINotify(Entry*);
#endif
};

#endif // KDIRWATCH_P_H

