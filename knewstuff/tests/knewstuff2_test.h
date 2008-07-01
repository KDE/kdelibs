#ifndef KNEWSTUFF2_TEST_TEST_H
#define KNEWSTUFF2_TEST_TEST_H

#include <knewstuff2/core/provider.h>
#include <knewstuff2/core/entry.h>

#include <QtCore/QObject>

namespace KNS
{
class CoreEngine;
}

class KNewStuff2Test : public QObject
{
    Q_OBJECT
public:
    KNewStuff2Test();
    void setTestAll(bool testall);
    void entryTest();
    void providerTest();
    void engineTest();
public slots:
    void slotProviderLoaded(KNS::Provider *provider);
    void slotProvidersFailed();
    void slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void slotEntriesFailed();
    void slotEntriesFinished();
    void slotPayloadLoaded(KUrl payload);
    void slotPayloadFailed();
    void slotInstallationFinished();
    void slotInstallationFailed();
private:
    void quitTest();
    KNS::CoreEngine *m_engine;
    bool m_testall;
};

#endif
