
#include <kapp.h>
#include <ktmainwindow.h>
#include <qwidget.h>
#include <qtimer.h>
#include <stdlib.h>
#include "kled.h"
#include "kledtest.h"


int main( int argc, char **argv )
{
    KApplication a( argc, argv );
    KTMainWindow ktw;
    QWidget qw;
    QTimer t_toggle, t_color, t_look;
    //KLed l(KLed::red, &qw);				// create lamp
    //KLed l(KLed::blue, &qw);				// create lamp
    KLed l(Qt::green, &qw);				        // create lamp
    //KLed l(KLed::yellow, &qw);				// create lamp
    //KLed l(KLed::orange, &qw);				// create lamp
    l.resize(16,30);
    //l.setLook(KLed::flat);
    l.setLook(KLed::round);
    l.setShape(KLed::Circular);
    //l.setLook(KLed::sunken);
    //    l.flat = TRUE;
    ktmp tmpobj(&l);
    
    t_toggle.start(1000, FALSE);
    t_color.start(3500, FALSE);
    t_look.start(10000, FALSE);
    QObject::connect(&t_toggle, SIGNAL(timeout()), &l, SLOT(toggle()));
    QObject::connect(&t_color, SIGNAL(timeout()), &tmpobj, SLOT(nextColor()));
    QObject::connect(&t_look, SIGNAL(timeout()), &tmpobj, SLOT(nextLook()));


    a.setMainWidget( &qw );
    ktw.setView(&qw);
    qw.show();					// show widget
    return a.exec();				// go
}

#include "kledtest.moc"

