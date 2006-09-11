#include <stdio.h>
#include <stdlib.h>
#include <qobject.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include "kglobalsettingstest.h"
#include <kdebug.h>

KCmdLineOptions options[] =
{
    { "p", "emit paletteChanged()", 0 },
    { "f", "emit fontChanged()", 0 },
    { "ps", "emit settingsChanged(SETTINGS_PATH)", 0 },
    KCmdLineLastOption
};

MyObject::MyObject()
    : QObject(0)
{
    KGlobalSettings* settings = KGlobalSettings::self();
    connect(settings, SIGNAL(kdisplayPaletteChanged()), SLOT(slotPaletteChanged()));
    connect(settings, SIGNAL(kdisplayFontChanged()), SLOT(slotFontChanged()));
    connect(settings, SIGNAL(kdisplayStyleChanged()), SLOT(slotStyleChanged()));
    connect(settings, SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));
    connect(settings, SIGNAL(appearanceChanged()), SLOT(slotAppearanceChanged()));
}

// We could turn this into an automated test with QSignalSpy, but then
// we wouldn't be able to test that it works between different processes...

int main(int argc, char **argv)
{
    KAboutData about("kglobalsettings", "kglobalsettings", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication app;

    if (args->isSet("p")) {
        KGlobalSettings::self()->emitChange(KGlobalSettings::PaletteChanged);
	return 0;
    } else if (args->isSet("f")) {
        KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);
        return 0;
    } else if (args->isSet("ps")) {
        KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_PATHS);
        return 0;
    }

    MyObject obj;
    kDebug() << "kglobalsettingstest ready..." << endl;
    return app.exec();
}

#include "kglobalsettingstest.moc"
