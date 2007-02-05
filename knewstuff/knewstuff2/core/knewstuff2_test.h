#ifndef KNEWSTUFF2_TEST_H
#define KNEWSTUFF2_TEST_H

#include <knewstuff2/provider.h>
#include <knewstuff2/providerhandler.h>
#include <knewstuff2/engine.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <qobject.h>

class KNewStuff2Test : public QObject
{
Q_OBJECT
public:
	KNewStuff2Test();
	void providerTest();
	void engineTest();
public slots:
	void slotProvidersLoaded(KNS::Provider::List *list);
	void slotProvidersFailed();
private:
	void quitTest();
};

#endif
