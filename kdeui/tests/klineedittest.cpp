#include <klined.h>
#include <qpushbutton.h>
#include <kapp.h>

int main ( int argc, char **argv)
{
  KApplication a(argc, argv, "klineedittest");

        //make a central widget to contain the other widgets
        QWidget * w = new QWidget( );
        KLineEdit *l = new KLineEdit( w, "mylineedit" );
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
