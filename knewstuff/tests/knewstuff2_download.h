#ifndef KNEWSTUFF2_TEST_DOWNLOAD_H
#define KNEWSTUFF2_TEST_DOWNLOAD_H

#include <knewstuff2/core/provider.h>
#include <knewstuff2/core/entry.h>

#include <QtGui/QWidget>

namespace KNS
{
class CoreEngine;
}

class QListWidget;
class QTabWidget;
class FeedWidget;
class QFrame;

class KNewStuff2Download : public QWidget
{
    Q_OBJECT
public:
    KNewStuff2Download();
    void run();
public slots:
    void slotProviderLoaded(KNS::Provider *provider);
    void slotProvidersFailed();
    void slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void slotEntriesFailed();
    void slotPreviewLoaded(KUrl payload);
    void slotPreviewFailed();
    void slotPayloadLoaded(KUrl payload);
    void slotPayloadFailed();
    void slotInstall();
private:
    FeedWidget *m_feedtab;
    KNS::CoreEngine *m_engine;
    QListWidget *m_providerlist;
    QTabWidget *m_feeds;
    QWidget *m_activefeed;
    KNS::Entry *m_activeentry;
    QFrame *frame;
};

#endif
