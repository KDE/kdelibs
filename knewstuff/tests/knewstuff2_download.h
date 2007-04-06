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
class KNSButton;
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
protected slots:
        void resizeEvent(QResizeEvent *event);
        void buttonToggled(bool checked);
private:
        enum CategoryFlag {
            Recent = 0x1,
            Wanted = 0x2,
            Estimated = 0x4
        };
        Q_DECLARE_FLAGS(Category, CategoryFlag)

        void switchCategory(KNewStuff2Download::Category category);

        FeedWidget *m_feedtab;
        KNewStuff2Download::Category m_checkedButton;
	KNS::CoreEngine *m_engine;
	QListWidget *m_providerlist;
	QTabWidget *m_feeds;
	QWidget *m_activefeed;
	KNS::Entry *m_activeentry;
        QFrame *frame;
        KNSButton *recentButton;
        KNSButton *estimatedButton;
        KNSButton *wantedButton;
};

#endif
