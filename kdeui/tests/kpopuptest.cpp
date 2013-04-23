#include <QApplication>
#include <QWidget>
#include <QCursor>
#include <QKeyEvent>
#include <QPainter>
#include <QMenu>

class DemoWidget : public QWidget {
private:
    QMenu *menu;

void mousePressEvent(QMouseEvent *)
{
    menu->popup(QCursor::pos());
}

void paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.drawText(32, 32, "Press a Mouse Button!");
}

public:
    DemoWidget() : QWidget()
    {
        menu = new QMenu("Popup Menu:");
        menu->addAction( "Item1" );
        menu->addAction( "Item2" );
        menu->addSeparator();
        QAction *a = new QAction( "Quit", this );
        menu->addAction( a );
        connect( a, SIGNAL(triggered()), qApp, SLOT(quit()));
    }
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName("test");
    QApplication app(argc, argv);
    DemoWidget w;
    w.setFont(QFont("helvetica", 12, QFont::Bold));
    w.show();
    return app.exec();
}

