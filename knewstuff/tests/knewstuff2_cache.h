#ifndef KNEWSTUFF2_TEST_CACHE_H
#define KNEWSTUFF2_TEST_CACHE_H

#include <knewstuff2/core/entry.h>

#include <QtCore/QObject>

namespace KNS
{
class CoreEngine;
class Feed;
class Provider;
}

class KNewStuff2Cache : public QObject
{
    Q_OBJECT
public:
    KNewStuff2Cache();
    void run();
public slots:
    void slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void slotEntriesFailed();
    void slotEntriesFinished();
private:
    void quitTest();
    KNS::CoreEngine *m_engine;
};

#endif
