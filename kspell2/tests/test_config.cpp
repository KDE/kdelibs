#include "kspell_configdialog.h"

#include <kapplication.h>
#include <kdebug.h>
using namespace KSpell2;

int main( int argc, char** argv )
{
    KApplication app(argc, argv, "KSpell2Test");

    SettingsDialog *dialog = new SettingsDialog( 0 );

    dialog->show();

    app.setMainWidget( dialog );

    return app.exec();
}
