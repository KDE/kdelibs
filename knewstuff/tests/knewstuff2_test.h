#ifndef KNEWSTUFF2_TEST_H
#define KNEWSTUFF2_TEST_H

#include <knewstuff2/provider.h>
#include <knewstuff2/entry.h>

#include <qobject.h>

namespace KNS
{
	class Engine;
};

class KNewStuff2Test : public QObject
{
Q_OBJECT
public:
	KNewStuff2Test();
	void entryTest();
	void providerTest();
	void engineTest();
public slots:
	void slotProviderLoaded(KNS::Provider *provider);
	void slotProvidersFailed();
	void slotEntryLoaded(KNS::Entry *entry);
	void slotEntriesFailed();
	void slotPayloadLoaded(KUrl payload);
	void slotPayloadFailed();
private:
	void quitTest();
	KNS::Engine *m_engine;
};

#endif
