#ifndef kledtest_h
#define kledtest_h

#include <ktmainwindow.h>
#include <qwidget.h>
#include <qtimer.h>
#include <stdlib.h>
#include <kled.h>

class ktmp : public QObject
{
  Q_OBJECT
public:
  ktmp(KLed *l) : led(l), ledcolor(l->color()), ledlook(l->look()) {};
  ~ktmp() {};
  
public slots:
  void nextColor() 
    {
      // int tmp = (static_cast<int>(ledcolor) + 1) % 5;
      // ledcolor=static_cast<KLed::Color>(tmp);
      // led->setColor(ledcolor);
      // led->repaint(); 
    }
  void nextLook() 
    { 
      int tmp = (static_cast<int>(ledlook) + 1) % 3;
      led->setLook(static_cast<KLed::Look>(tmp));
      led->repaint(false); 
    }
protected:
  KLed *led;
  QColor ledcolor;
  KLed::Look  ledlook;
};

#endif

