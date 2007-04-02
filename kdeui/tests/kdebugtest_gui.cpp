#include "kdebug.h"
#include <QtGui/QWidget>


int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  TestWidget widget(0);
  widget.setGeometry(45, 54, 120, 80);
  widget.show();

  kDebug() << &widget;

  QRegion reg(r);
  reg += QRect(1,60,200,59);
  kDebug() << reg << endl;

  QVariant v = QPen( Qt::red );
  kDebug() << "Variant: " << v << endl;

  return 0;;
}
