#include <qpushbutton.h>

#include <kapp.h>
#include <klineedit.h>
#include <kcompletion.h>

int main ( int argc, char **argv)
{
    KApplication a(argc, argv, "klineedittest");
    //make a central widget to contain the other widgets
    QWidget * w = new QWidget( );
    KLineEdit *l = new KLineEdit( w, "mylineedit" );
    l->enableCompletion();
    // Shows of the value of the returnPressed signals with the QString argument.
    // We simply insert the entered items into the completion object.
    QObject::connect( l, SIGNAL( returnPressed( const QString& ) ), l->completionObject(), SLOT( addItem( const QString& ) ) );
    // Un-comment the the code below to stop KLineEdit from handling rotation
    // signals internally.
    // l->setHandleRotationSignals( false );
    l->resize(500,30);
    l->setFocus();
    QPushButton * push = new QPushButton( "Exit", w );
    push->resize(100,30);
    push->move(50,50);
    QObject::connect( push, SIGNAL( clicked() ), &a, SLOT( closeAllWindows() ) );
    a.setMainWidget(w);
    w->show();

    return a.exec();
}
