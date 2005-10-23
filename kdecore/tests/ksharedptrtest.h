#ifndef KSHAREDPTRTEST_H
#define KSHAREDPTRTEST_H

#include <qobject.h>

class KSharedPtrTest : public QObject
{
	Q_OBJECT
	private slots:
		void testAll();
};

#endif /* KSHAREDPTRTEST_H */
