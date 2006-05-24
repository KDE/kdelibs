#ifndef KSHAREDPTRTEST_H
#define KSHAREDPTRTEST_H

#include <qobject.h>

class KSharedPtrTest : public QObject
{
	Q_OBJECT
	private Q_SLOTS:
		void testWithStrings();
		void testDeletion();
		void testDifferentTypes();
		void testOrdering();
};

#endif /* KSHAREDPTRTEST_H */
