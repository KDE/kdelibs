#include <kformulaedit.h>
#include <kapp.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qaccel.h>

//Simple test for kformula.  Doesn't test evaluation, though.

int main( int argc, char **argv )
{
    KApplication a( argc, argv );

    QWidget w;
    w.resize( 640, 480 );

    QVBoxLayout l(&w, 10);

    QAccel ac(&w);

    ac.insertItem(Qt::CTRL + Qt::Key_Q, 1);
    ac.connectItem(1, qApp, SLOT(quit()));

    KFormulaEdit x(&w);

    l.addWidget(&x, 1);

    QLabel lab("Press CTRL+Q to quit.  Try CTRL + 234568[]./^-_", &w);

    lab.setFont(QFont("courier", 20));

    l.addWidget(&lab, 0);

    a.setMainWidget( &w );
    x.setFocus();
    w.show();
    return a.exec();
}                                 

