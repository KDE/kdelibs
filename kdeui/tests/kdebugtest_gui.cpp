#include "kdebug.h"
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtCore/QVariant>
#include <QtGui/QPen>


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QWidget widget(0);
    widget.setGeometry(45, 54, 120, 80);
    widget.show();

    kDebug() << &widget;

    QRect r(9,12,58,234);
    QRegion reg(r);
    reg += QRect(1,60,200,59);
    kDebug() << reg;

    QVariant v = QPen( Qt::red );
    kDebug() << "Variant: " << v;

    return 0;
}
