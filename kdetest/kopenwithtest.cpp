#include <qapplication.h>
#include <qwidget.h>
#include <qdir.h>
#include <kio_openwith.h>

class DemoWidget : public QWidget {
    
void paintEvent(QPaintEvent *)
{
}

public:
    DemoWidget() : QWidget()
    {
        OpenWithDlg* dlg = new OpenWithDlg("OpenWith_Text", "OpenWith_Value", this);

        if(dlg->exec())
          ;
        
    }       
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    DemoWidget w;
    app.setMainWidget(&w);
    w.show();
    
    return app.exec();
}
    
