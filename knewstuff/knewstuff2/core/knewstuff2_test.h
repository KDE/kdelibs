#ifndef KNEWSTUFF2_TEST_H
#define KNEWSTUFF2_TEST_H

#include <knewstuff2/provider.h>
#include <knewstuff2/entry.h>

#include <qobject.h>

class KNewStuff2Test : public QObject
{
Q_OBJECT
public:
	KNewStuff2Test();
	void entryTest();
	void providerTest();
	void engineTest();
public slots:
	void slotProvidersLoaded(KNS::Provider::List *list);
	void slotProvidersFailed();
	void slotEntriesLoaded(KNS::Entry::List *list);
	void slotEntriesFailed();
private:
	void quitTest();
};

#endif
