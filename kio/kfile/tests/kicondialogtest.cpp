#include <kapplication.h>
#include <kicondialog.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kicondialogtest" );

    KIconDialog::getIcon(); 

    return 0;
}

/* vim: et sw=4
 */
