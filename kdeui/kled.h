/* This file is part of the KDE libraries
    Copyright (C) 1998 Jörg Habenicht (j.habenicht@europemail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef LED_H
#define LED_H

#include <qwidget.h>
#include <qcolor.h>

/** 
* KLed displays a round light emmiting diode.
*
* It is configurable to 5 colors and the 2 on/off states.
*
* It may display itself in a performant flat view, a round view with
* light spot or a round view sunken in the screen.
*
* @short A round led widget
* @author Jörg Habenicht
*/
class KLed : public QWidget
{
Q_OBJECT

public:

  /**
   * color of the led
   * @short led color
   */
  enum Color { yellow, orange, red, green, blue };

  /**
   * Status of the light is on/off
   * @short led on/off
   */
  enum State { Off=0, On=1 };

  /**
   * Displays a flat, round or round-sunken led.
   * Displaying the led flat is less time and color consuming,
   * but not so nice to see.
   *
   * The sunken led itself is (for shure) smaller than the round led
   * because of the 3 shading circles and is
   * most time consuming. Makes sense for led > 15x15 pixels.
   *
   * timings:
   * ( AMD K5/133, Diamond Stealth 64 PCI Graphics, widgetsize 29x29 )
   *  - flat approx. 0.7 msec per paint 
   *  - round approx. 2.9 msec per paint
   *  - sunken approx. 3.3 msec per paint
   *
   * The widget will be updated the next repaining event.
   * @short led look
   */
  enum Look  { flat, round, sunken };

  /**
   * Constructor with the ledcolor, the parent widget, and the name.
   *
   * The State will be defaulted On and the Look round
   *
   * @param ledcolor initializes with Color
   * @param parent   will be handed over to QWidget
   * @param name     will be handed over to QWidget
   * @short Constructor
   */
KLed(KLed::Color ledcolor, QWidget *parent = 0, const char *name = 0);

  /**
   * Constructor with the ledcolor, ledstate, ledlook,
   * the parent widget, and the name.
   *
   * Differs from above only in the parameters, which configure all settings
   *
   * @param ledcolor initializes with Color
   * @param state    sets the State
   * @param look     sets the Look
   * @param parent   will be handed over to QWidget
   * @param name     will be handed over to QWidget
   * @short Constructor
   */
KLed(KLed::Color ledcolor, KLed::State state, KLed::Look look, QWidget *parent = 0, const char *name = 0);

  /**
   * Hands back the current state of the widget (on/off)
   * see enum State
   * @short returns led state
   */
  State getState() const { return led_state; }

  /**
   * Hands back the color of the widget
   * see enum Color
   * @short returns led color
   */
  Color getColor() const { return led_color; }

  /**
   * Hands back the look of the widget
   * see enum Look
   * @short returns led look
   */
  Look getLook() const { return led_look; }

  /**
   * sets the state of the widget to On or Off.
   * The widget will be painted immediately.
   * see also: on(), off(), toggle(), toggleState()
   *
   * @param state the led state on or off
   * @short set led state
   */
  void setState( State state ) { led_state = state;
    current_color=led_state ? lightcolor[led_color] : darkcolor[led_color];
    repaint(); }

  /**
   * toggles the state of the led from Off to On an vice versa.
   * The widget repaints itself immediately.
   * @short toggles led on->off / off->on
   */
  void toggleState() { led_state = (State)!led_state;
    current_color=led_state ? lightcolor[led_color] : darkcolor[led_color];
    repaint(); }

  /**
   * Sets the color of the widget.
   * The widget doesn't calls the repaint method, so it will
   * be updated with the next repainting call.
   *
   * see also: enum Color
   *
   * @param color new Color of the led
   * @short sets the led color
   */
  void setColor( KLed::Color color) { led_color = color;
    current_color=led_state ? lightcolor[led_color] : darkcolor[led_color]; }

  /**
   * Sets the look of the widget.
   * the look may be flat, round or sunken.
   * The widget doesn't calls the repaint method, so it will
   * be updated with the next repainting call.
   *
   * see also: enum Look
   *
   * @param look new look of the led
   * @short sets led look
   */
  void setLook( Look look ) { led_look = look; }

public slots:

  /**
   * toggles the state of the led from Off to On an vice versa.
   * The widget repaints itself immediately.
   */
  void toggle() { toggleState(); };

  /**
   * sets the state of the widget to On.
   * The widget will be painted immediately.
   * see also: off(), toggle(), toggleState(), setState()
   */  
  void on() { setState(On); };

  /**
   * sets the state of the widget to Off.
   * The widget will be painted immediately.
   * see also: on(), toggle(), toggleState(), setState()
   */
  void off() { setState(Off); };

protected:
  void paintEvent(QPaintEvent *);

private:
  void paintflat();
  void paintround();
  void paintsunken();

  static const QRgb lightcolor[], darkcolor[];
  State led_state;
  Color led_color;
  Look  led_look;
  QRgb current_color;
};


#endif
