#include <stdio.h>
#include <kmedia2.h>
#include <kcmdlineargs.h>
#include <connect.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include "qiomanager.h"
#include "artskde.h"

using namespace std;
using namespace Arts;


static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("URL to open"), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
	KAboutData aboutData( "kiotest", I18N_NOOP("KIOTest"), I18N_NOOP("0.1"), "", KAboutData::License_GPL, "");
							  
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions(options); 	
	KApplication app;
	QIOManager qiomanager;
	Dispatcher dispatcher(&qiomanager);
	KIOInputStream stream;
	StdoutWriter writer;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if(args->count())
	{
	    if(!stream.openURL(args->arg(0)))
	    {
		printf("can't open url");
		exit(1);
	    }
	}
	else
	    exit(1);
	    
	args->clear();
	
	connect(stream, writer);

	writer.start();
	stream.start();
	
	app.exec();
}
