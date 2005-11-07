#ifndef _OPT_KDECVS_SRC_KDELIBS_DCOP_TEST_TESTER_H
#define _OPT_KDECVS_SRC_KDELIBS_DCOP_TEST_TESTER_H
#include "test_stub.h"
#include <qobject.h>
#include <qtextstream.h>



class Driver : public QObject, public Test_stub
{
	Q_OBJECT

	public:
		Driver(const char*);
	public slots:
		void test();

	private:
	int count;
	QTextStream output;
};

#endif // _OPT_KDE-CVS_SRC_KDELIBS_DCOP_TEST_TESTER_H
