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

/* KDirWatchPrivate is a singleton and does the watching
 * for every KDirWatch instance in the application.
 */
class KDirWatchPrivate : public QObject
{
  Q_OBJECT

  enum entryStatus { Normal = 0, NonExistent };
  enum entryMode { UnknownMode = 0, StatMode, DNotifyMode, FAMMode };
  enum { NoChange=0, Changed=1, Created=2, Deleted=4 };

public:
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
    QDateTime m_ctime;
    entryStatus m_status;
    entryMode m_mode;
    bool isDir;
    // instances interested in events
    QPtrList<Client> m_clients;
    // nonexistent entries of this directory
    QPtrList<Entry> m_entries;
    QString path;

    int msecLeft, freq;

    void addClient(KDirWatch*);
    void removeClient(KDirWatch*);
    int clients();
    bool isValid() { return m_clients.count() || m_entries.count(); }

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
  void emitEvent(Entry* e, int event, const QString &fileName = QString::null);

public slots:
  void slotRescan();
  void famEventReceived(); // for FAM
  void slotActivated(); // for DNOTIFY

public:
  QTimer *timer;
  EntryMap m_mapEntries;

private:
  int freq;
  int statEntries;
  int m_nfsPollInterval, m_PollInterval;
  bool useStat(Entry*);

  bool delayRemove;
  QPtrList<Entry> removeList;

#ifdef HAVE_FAM
  QSocketNotifier *sn;
  FAMConnection fc;
  bool use_fam;

  void checkFAMEvent(FAMEvent*);
  bool useFAM(Entry*);
#endif

#ifdef HAVE_DNOTIFY
  bool supports_dnotify;
  int mPipe[2];
  QTimer mTimer;
  QSocketNotifier *mSn;
  QIntDict<Entry> fd_Entry;

  static void dnotify_handler(int, siginfo_t *si, void *);
  bool useDNotify(Entry*);
#endif
};

#endif // KDIRWATCH_P_H

