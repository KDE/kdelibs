#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include "KIdleTest.h"

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, new KAboutData("app", "a", ki18n("a"), "1", ki18n("aa"), KAboutData::License_GPL, ki18n("DRF"), ki18n("aaa")));
    KApplication app;

    KIdleTest t;

    return app.exec();
}
