#include <kformulaedit.h>
#include <qapplication.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget w;
    w.resize( 640, 480 );

    KFormulaEdit x(&w);
    x.move( 0, 0 );
    x.resize( 640, 480 );

    a.setMainWidget( &w );
    w.show();
    return a.exec();
}                                 

