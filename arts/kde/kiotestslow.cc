#include <stdio.h>
#include <kmedia2.h>
#include <kcmdlineargs.h>
#include <connect.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <stdsynthmodule.h>
#include "qiomanager.h"
#include "artskde.h"

using namespace std;
using namespace Arts;

namespace Arts {
/* simulate slow receiver */
class KIOTestSlow_impl : public KIOTestSlow_skel,
                         public TimeNotify,
						 public StdSynthModule
{
	int pos;
	list< DataPacket<mcopbyte>* > q;
	InputStream _inputStream;

public:
	InputStream inputStream() { return _inputStream; }
	void inputStream(InputStream i) { _inputStream = i; }

	KIOTestSlow_impl()
	{
		Dispatcher::the()->ioManager()->addTimer(10, this);
		pos = 0;
	}
	void notifyTime()
	{
		if(!_inputStream.isNull() && _inputStream.eof())
		{
			printf("\n[*EOF*] remaining = %d packets\n");
			_inputStream = InputStream::null();
			return;
		}

		int TODO = 100;
		do {
			if(q.empty())
				return;

			DataPacket<mcopbyte> *p = q.front();
			char ch = p->contents[pos++];
			if(p->size == pos)
			{
				p->processed();
				q.pop_front();
				pos = 0;
			}

			if(ch == '\n')
			{
				long size = 0;
				list<DataPacket<mcopbyte>*>::iterator i;
				for(i = q.begin(); i != q.end(); i++)
					size += (*i)->size;
				printf("\n[queued %8ld] ",size-pos);
			}
			else
				putchar(ch);

		} while(TODO-- > 0);
	}
	void process_data(DataPacket<mcopbyte> *p)
	{
		if(p->size == 0)
			p->processed();
		else
			q.push_back(p);
	}
};
REGISTER_IMPLEMENTATION(KIOTestSlow_impl);
};

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("URL to open"), 0 },
    KCmdLineLastOption
};

#undef USE_FILEINPUTSTREAM

int main(int argc, char **argv)
{
	KAboutData aboutData( "kiotestslow", I18N_NOOP("KIOTest"), I18N_NOOP("0.1"), "", KAboutData::License_GPL, "");
							  
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions(options); 	
	KApplication app;
	QIOManager qiomanager;
	Dispatcher dispatcher(&qiomanager);
#ifndef USE_FILEINPUTSTREAM
	KIOInputStream stream;
#else
	FileInputStream stream;
#endif
	KIOTestSlow writer;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if(args->count())
	{
#ifdef USE_FILEINPUTSTREAM
	    if(!stream.open(args->arg(0)))
#else
	    if(!stream.openURL(args->arg(0)))
#endif
	    {
		printf("can't open url");
		exit(1);
	    }
	}
	else
	    exit(1);
	    
	args->clear();

	writer.inputStream(stream);
	connect(stream, writer);

	writer.start();
	stream.start();
	
	app.exec();
}
