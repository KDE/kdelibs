#include <kdebug.h>
#include <qwidget.h>
#include <kinstance.h>
#include <iostream.h>
#include <qapplication.h>
#include <kdebugclasses.h>

class TestWidget : public QWidget
{

public:
  TestWidget(QWidget* parent, const char* name)
    : QWidget(parent, name)
  {
    kdDebug().form("mytest %s", "hello") << endl;
    QString test = "%20C this is a string";
    kdDebug(150) << test << endl;
    kdDebug() << k_lineinfo << "error on this line" << endl;
    kdDebug(2 == 2) << "this is right " << perror << endl;
    kdDebug() << "Before instance creation" << endl;
    kdDebug(1202) << "Before instance creation" << endl;
    KInstance i("kdebugtest");
    kdDebug(1) << "kDebugInfo with inexisting area number" << endl;
    kdDebug(1202) << "This number has a value of " << 5 << endl;
    // kdDebug() << "This number should come out as appname " << 5 << " " << "test" << endl;
    kdWarning() << "1+1 = " << 1+1+1 << endl;
    kdError(1+1 != 2) << "there is something really odd!" << endl;
    QString s = "mystring";
    kdDebug() << s << endl;
    kdError(1202) << "Error !!!" << endl;
    kdError() << "Error with no area" << endl;
    
    kdDebug() << "Printing a null QWidget pointer: " << (QWidget*)0 << endl;

    kdDebug() << "char " << '^' << " " << char(26) << endl;
    QPoint p(0,9);
    kdDebug() << p << endl;

    QRect r(9,12,58,234);
    kdDebug() << r << endl;

    QRegion reg(r);
    reg += QRect(1,60,200,59);
    kdDebug() << reg << endl;

    QStringList sl;
    sl << "hi" << "this" << "list" << "is" << "short";
    kdDebug() << sl << endl;

  }
  void resizeEvent(QResizeEvent*)
  {
    kdDebug() << this << endl;
  }    
};

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  TestWidget widget(0, "NoNameWidget");
  widget.setGeometry(45, 54, 120, 80);
  widget.show();
  app.setMainWidget(&widget);
  app.exec();
  return 0;
}

