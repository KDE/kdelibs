#include <ktopwidget.h>
#include <qwidget.h>
#include <qtimer.h>
#include <stdlib.h>
#include <kled.h>

class ktmp : public QObject
{
Q_OBJECT
public:
ktmp(KLed *l) : led(l), ledcolor(l->getColor()), ledlook(l->getLook()) {};
~ktmp() {};

 public slots:
 void nextColor() {
   ledcolor=(KLed::Color)((++(int)ledcolor)%5);
   led->setColor(ledcolor);
   led->repaint(); }
  void nextLook() { 
    ledlook = (KLed::Look)((++(int)ledlook)%3);
    led->setLook(ledlook);
    led->repaint(); }

private:
  KLed        *led;
  KLed::Color ledcolor;
  KLed::Look  ledlook;
};
