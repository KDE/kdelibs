#include <kapplication.h>
#include <knotifydialog.h>

int main( int argc, char **argv )
{
    KApplication app;
    KNotifyDialog *dlg = new KNotifyDialog();
    dlg->addApplicationEvents( "kwin" );
    return dlg->exec();
}
