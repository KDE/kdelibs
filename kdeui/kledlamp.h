// CDE style LED lamp widget for Qt
// Richard J. Moore 23.11.96
// Email: moorer@cs.man.ac.uk

#ifndef LED_LAMP_H
#define LED_LAMP_H

#include <qframe.h>

/** 
* KLedLamp provides a CDE style LED lamp widget.
*
* @short A CDE-style LED lamp widget.
* @author Richard J. Moore (moorer@cs.man.ac.uk)
* @version $Id$
*/
class KLedLamp : public QFrame
{
  Q_OBJECT
public:
/** 
* Construct a KLedLamp widget 
*/
  KLedLamp(QWidget *parent=0);

/** 
* Possible states of a KLedLamp 
*/
  enum State { On, Off };

/** 
* Retrieve the state of the KLedLamp 
*/
  State state() const { return s; }
  
/** 
* Set the state of the KLedLamp 
*/
  void setState(State state) { s= state; paintEvent(0); }

/** 
* Toggle the state of the KLedLamp 
*/
  void toggleState() { if (s == On) s= Off; else if (s == Off) s= On; paintEvent(0); }
public slots:
  void toggle() { toggleState(); };
  void on() { setState(On); };
  void off() { setState(Off); };
protected:
  void drawContents(QPainter *);
private:
  const int width;
  const int height;
  const int dx;
  State s;
};


#endif




