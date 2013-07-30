#include "QDebug"
#include <QWidget>
#include <QApplication>
#include <QtCore/QVariant>
#include <QPen>


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QWidget widget(0);
    widget.setGeometry(45, 54, 120, 80);
    widget.show();

    // qDebug() << &widget;

    QRect r(9,12,58,234);
    QRegion reg(r);
    reg += QRect(1,60,200,59);
    // qDebug() << reg;

    QVariant v = QPen( Qt::red );
    // qDebug() << "Variant: " << v;

    return 0;
}
