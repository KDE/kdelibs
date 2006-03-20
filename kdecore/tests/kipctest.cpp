#include <stdio.h>
#include <stdlib.h>
#include <qobject.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kipc.h>
#include "kipctest.h"

MyObject::MyObject()
    : QObject(0L)
{
    setObjectName(QLatin1String( "testobj" ));
    connect(kapp, SIGNAL(kdisplayPaletteChanged()), SLOT(slotPaletteChanged()));
    connect(kapp, SIGNAL(kdisplayFontChanged()), SLOT(slotFontChanged()));
    connect(kapp, SIGNAL(kdisplayStyleChanged()), SLOT(slotStyleChanged()));
    connect(kapp, SIGNAL(backgroundChanged(int)), SLOT(slotBackgroundChanged(int)));
    connect(kapp, SIGNAL(appearanceChanged()), SLOT(slotAppearanceChanged()));
    connect(kapp, SIGNAL(kipcMessage(int,int)), SLOT(slotMessage(int,int)));
}

int main(int argc, char **argv)
{

    KAboutData about("kipc", "kipc", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;

    if (argc == 3) 
    {
	KIPC::sendMessageAll((KIPC::Message) atoi(argv[1]), atoi(argv[2]));
	return 0;
    }

    MyObject obj;
    return app.exec();
}

#include "kipctest.moc"
