
#include <kthemebase.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kipc.h>

static const char *description =
	I18N_NOOP("KDE tool to change themes");

static const char *version = "v0.0.1";

static KCmdLineOptions options[] =
{
   { "+Theme", I18N_NOOP("Input file containing a theme"), 0 },
   { "apply", I18N_NOOP("Apply to all apps immediately"), 0 },
   { 0, 0, 0 }
};

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "settheme", description, version );

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app; // needed now because of properties.
    puts("settheme: (C)2000 MIT License - D.M. Duley (mosfet@kde.org)");

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    bool apply = false;

    if (args->isSet("apply"))
    {
      apply = true;
    }

    if (args->count() < 1)
    {
        KCmdLineArgs::usage(i18n("You must specify an input file!\n"));
    }

    QFileInfo fi(args->arg(0));

    if (!fi.exists())
    {
        puts("settheme: The input file does not exist.");
        return(2);
    }

    KThemeBase::applyConfigFile(args->arg(0));

#ifdef Q_WS_X11 // FIXME
    if (apply)
      KIPC::sendMessageAll(KIPC::StyleChanged);
#endif

    return(0);
}
