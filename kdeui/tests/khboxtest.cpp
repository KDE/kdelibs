#include "khboxtest.h"
#include <qpushbutton.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

KHBoxTest::KHBoxTest( QWidget* parentWidget )
    : KHBox( parentWidget )
{
    pbAdd = new QPushButton( "Add a button", this );
    connect( pbAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );
    pbRemove = 0;
}

void KHBoxTest::slotAdd()
{
    if ( !pbRemove ) {
        pbRemove = new QPushButton( "Remove me", this );
        connect( pbRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
        pbAdd->setEnabled( false );
    }
}

void KHBoxTest::slotRemove()
{
    pbAdd->setEnabled( true );
    pbRemove->deleteLater();
    pbRemove = 0;
}

int main( int argc, char ** argv )
{
    KAboutData about("KHBoxTest", "KHBoxTest", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;

    KHBoxTest *toplevel = new KHBoxTest(0);
    toplevel->show();
    app.exec();
}

#include "khboxtest.moc"
