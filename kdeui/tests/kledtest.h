#ifndef kledtest_h
#define kledtest_h

#include <qwidget.h>
#include <qtimer.h>
#include <stdlib.h>
#include <kled.h>

class KLedTest : public QWidget
{
  Q_OBJECT
protected:
  QTimer timer;
  KLed *leds[KLed::NoOfShapes*KLed::NoOfLooks*KLed::NoOfStates];
  const int LedWidth;
  const int LedHeight;
  const int Grid;
  KLed::Shape shape;
  KLed::Look look;
  KLed::State state;
  int x, y, index;


  QTimer t_toggle, t_color, t_look;
  //KLed *l;				// create lamp
  //KLed *l;				// create lamp
  KLed *l;				// create lamp
  //KLed *l;				// create lamp
  //KLed *l;				// create lamp
  //KLed::Color ledcolor;
  KLed::Look  ledlook;


public:

  KLedTest(QWidget* parent=0);
  ~KLedTest();

  bool kled_round;


public slots:
  void timeout();

// void nextColor();
  void nextLook();


};

#endif

