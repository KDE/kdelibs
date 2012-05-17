#include "khboxtest.h"
#include <QPushButton>
#include <QApplication>


KHBoxTest::KHBoxTest( QWidget* parentWidget )
    : KHBox( parentWidget )
{
    pbAdd = new QPushButton( QLatin1String("Add a button"), this );
    connect( pbAdd, SIGNAL(clicked()), this, SLOT(slotAdd()) );
    pbRemove = 0;
}

void KHBoxTest::slotAdd()
{
    if ( !pbRemove ) {
        pbRemove = new QPushButton( QLatin1String("Remove me"), this );
        connect( pbRemove, SIGNAL(clicked()), this, SLOT(slotRemove()) );
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
    QApplication app(argc, argv);

    KHBoxTest *toplevel = new KHBoxTest(0);
    toplevel->show();
    app.exec();
}

