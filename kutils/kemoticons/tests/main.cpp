#include <kemoticons.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <qstring.h>

int main(int argc, char **argv)
{
    KAboutData aboutData("kemoticonstest",
                         0,
                         ki18n("KEmoticons Test "),
                         "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;
    
    KEmoticons e;
    QHash<QString, KEmoticonsTheme*> ls = e.getThemeList();
    QStringList txt;
    QHash<QString, KEmoticonsTheme*>::const_iterator i = ls.constBegin();
    for(; i != ls.constEnd(); ++i) {
        txt << i.key();
    }
    KMessageBox::information(0, txt.join("\n"));

    return 0;
}

