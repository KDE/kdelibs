#ifndef KNEWSTUFF2_DOWNLOAD_H
#define KNEWSTUFF2_DOWNLOAD_H

#include <knewstuff2/provider.h>
#include <knewstuff2/entry.h>

#include <qwidget.h>

namespace KNS
{
	class Engine;
};

class QListWidget;
class QTabWidget;

class KNewStuff2Download : public QWidget
{
Q_OBJECT
public:
	KNewStuff2Download();
	void run();
public slots:
	void slotProviderLoaded(KNS::Provider *provider);
	void slotProvidersFailed();
	void slotEntryLoaded(KNS::Entry *entry);
	void slotEntriesFailed();
	void slotPreviewLoaded(KUrl payload);
	void slotPreviewFailed();
	void slotPayloadLoaded(KUrl payload);
	void slotPayloadFailed();
	void slotInstall();
private:
	KNS::Engine *m_engine;
	QListWidget *m_providerlist;
	QTabWidget *m_feeds;
	QWidget *m_activefeed;
	KNS::Entry *m_activeentry;
};

#endif
