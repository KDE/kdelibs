#include <qpushbutton.h>

#include <kapp.h>
#include <klineedit.h>
#include <klocale.h>

int main ( int argc, char **argv)
{
    KApplication a(argc, argv, "klineedittest");
    //make a central widget to contain the other widgets
    QWidget * w = new QWidget( );
    // Create the widget with the last argument set to "true" to
    // have KLineEdit automatically handle the completion and
    // rotation signals.
    KLineEdit *l = new KLineEdit( w, "mylineedit" );
    QStringList list;
    list <<i18n("Stone") <<i18n("Tree") <<i18n("Peables") <<i18n("Ocean") <<i18n("Sand") <<i18n("Chips") <<i18n("Computer") <<i18n("Mankind");
    list.sort();
    l->completionObject()->setItems( list );
    // Shows of the value of the returnPressed signals with the QString argument.
    // We simply insert the entered items into the completion object.
    QObject::connect( l, SIGNAL( returnPressed( const QString& ) ), l->completionObject(), SLOT( addItem( const QString& ) ) );
    l->resize(500,30);
    l->setFocus();
    QPushButton * push = new QPushButton( "E&xit", w );
    push->resize(100,30);
    push->move(50,50);
    QObject::connect( push, SIGNAL( clicked() ), &a, SLOT( closeAllWindows() ) );
    a.setMainWidget(w);
    w->show();

    return a.exec();
}
