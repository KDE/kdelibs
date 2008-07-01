#include "kdxsview.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
    KAboutData about("kdxspreview",
                     0,
                     ki18n("KDXS Preview"),
                     "0.1",
                     ki18n("KNewstuff2 DXS Preview"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2005, 2006 Josef Spillner"),
                     KLocalizedString(),
                     0,
                     "spillner@kde.org");

    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;

    KNS::KDXSView view;
    return view.exec();
}
