#ifndef kledtest_h
#define kledtest_h

#include <ktmainwindow.h>
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
public:
  KLedTest(QWidget* parent=0)
    : QWidget(parent, 0),
      LedWidth(16),
      LedHeight(10),
      Grid(3)
    {
      y=Grid; index=0;
      for(shape=KLed::NoShape; shape!=KLed::NoOfShapes; shape=(KLed::Shape)(shape+1))
	{
	  x=Grid;
	  for(look=KLed::NoLook; look!=KLed::NoOfLooks; look=(KLed::Look)(look+1))
	    {
	      for(state=KLed::Off; state!=KLed::NoOfStates; state=(KLed::State)(state+1))
		{
		  leds[index]=new KLed(Qt::yellow, state,
				       (KLed::Look)(look+1),
				       (KLed::Shape)(shape+1), this);
		  leds[index]->setGeometry(x, y, LedWidth, LedHeight);
		  ++index;
		  x+=Grid+LedWidth;
		}
	    }
	  y+=Grid+LedHeight;
	}
      setFixedSize(x+Grid, y+Grid);
      connect(&timer, SIGNAL(timeout()), SLOT(timeout()));
      timer.start(500);
    }
  ~KLedTest() {};
public slots:
  void timeout() 
    {
      const int NoOfLeds=sizeof(leds)/sizeof(leds[0]);
      int count;
      // -----
      for(count=0; count<NoOfLeds; ++count)
	{
	  if(leds[count]->state()==KLed::Off)
	    {
	      leds[count]->setState(KLed::On);
	    } else {
	      leds[count]->setState(KLed::Off);
	    }
	}
    }
};

#endif

