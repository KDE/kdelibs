#include <kapplication.h>
#include <kicondialog.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kicondialogtest" );

//    KIconDialog::getIcon(); 

    KIconButton button;
    app.setMainWidget( &button );
    button.show();
 

    return app.exec();
}

/* vim: et sw=4
 */
