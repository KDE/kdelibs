#include <kapplication.h>
#include <kicondialog.h>

int main( int argc, char **argv )
{
    KApplication app;

//    KIconDialog::getIcon(); 

    KIconButton button;
    button.show();
 

    return app.exec();
}

/* vim: et sw=4
 */
