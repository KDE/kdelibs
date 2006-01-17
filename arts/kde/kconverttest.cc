#include <stdio.h>

#include <qfile.h>
#include <qobject.h>

#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include <flowsystem.h>
#include <kplayobject.h>
#include <kartsdispatcher.h>
#include <kplayobjectfactory.h>
#include <kaudioconverter.h>
#include "kconverttest.moc"

using namespace std;
using namespace Arts;

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("URL to open"), 0 },
    KCmdLineLastOption
};

KConvertTest::KConvertTest()
{
}

void KConvertTest::slotRawStreamStart()
{
//	cout << "[START]\n\n" << endl;
}

void KConvertTest::slotNewBlockSize(long blockSize)
{
	m_blockSize = blockSize;
}

void KConvertTest::slotNewBlockPointer(long blockPointer)
{
	m_blockPointer = blockPointer;
}

void KConvertTest::slotNewData()
{
	fwrite((void *) m_blockPointer, 1, m_blockSize, stdout);
}

void KConvertTest::slotRawStreamFinished()
{
//	cout << "\n\n[END]" << endl;
}

int main(int argc, char **argv)
{
	KAboutData aboutData("kconverttest", I18N_NOOP("KConvertTest"), I18N_NOOP("0.1"), "", KAboutData::License_GPL, "");
							  
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options); 	
	KApplication app;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	KUrl url;
	
	if(args->count())
		url = args->arg(0);
	else
		exit(1);

	args->clear();

	KConvertTest *get = new KConvertTest();

	KArtsDispatcher dispatcher;
	KAudioConverter converter;

	// FIXME: crashes
	// converter.setup(44100);
	converter.requestPlayObject(url);

	QObject::connect(&converter, SIGNAL(rawStreamStart()), get, SLOT(slotRawStreamStart()));

	QObject::connect(&converter, SIGNAL(newBlockSize(long)), get, SLOT(slotNewBlockSize(long)));
	QObject::connect(&converter, SIGNAL(newBlockPointer(long)), get, SLOT(slotNewBlockPointer(long)));
	QObject::connect(&converter, SIGNAL(newData()), get, SLOT(slotNewData()));
	
	QObject::connect(&converter, SIGNAL(rawStreamFinished()), get, SLOT(slotRawStreamFinished()));

	converter.start();

	app.exec();
}

