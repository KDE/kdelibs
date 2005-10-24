#ifndef KSHAREDPTRTEST_H
#define KSHAREDPTRTEST_H

#include <qobject.h>

class KSharedPtrTest : public QObject
{
	Q_OBJECT
	private slots:
		void testWithStrings();
		void testDeletion();
		void testDifferentTypes();
};

#endif /* KSHAREDPTRTEST_H */
