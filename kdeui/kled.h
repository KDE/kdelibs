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

/*************************************************************************
 * $Id$
 *************************************************************************/

#ifndef LED_H
#define LED_H

#include <qwidget.h>

class QColor;
/** 
* KLed displays a round  or rectangular light emmitting diode.
*
* It is configurable to 5 colors, the 2 on/off states and .
*
* It may display itself in a performant flat view, a round view with
* light spot or a round view sunken in the screen.
*
* @short A round led widget
* @author Joerg Habenicht, Richard J. Moore (rich@kde.org) 1998, 1999
*/

class KLed : public QWidget
{
  Q_OBJECT

    public:

  /**
   * Status of the light is on/off
   * @short led on/off
   */
  enum State { Off=0, On=1 };
  
  /** 
   * Shades of the lamp. 
   * @short shade
   */ 
  enum Shape { NoShape, Rectangular, Circular };
  
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
  KLed(const QColor &col=Qt::green, QWidget *parent=0, const char *name=0);

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
  KLed(const QColor& col, KLed::State st, KLed::Look look, KLed::Shape shape, 
       QWidget *parent=0, const char *name=0);

  /**
   * Hands back the current state of the widget (on/off)
   * see enum State
   * @short returns led state
   */
  State state() const;

  /**
   * Hands back the color of the widget
   * see enum Color
   * @short returns led color
   */
  const QColor color() const;

  /**
   * Hands back the look of the widget
   * see enum Look
   * @short returns led look
   */
   Look look() const;

  /**
   * sets the state of the widget to On or Off.
   * The widget will be painted immediately.
   * see also: on(), off(), toggle(), toggleState()
   *
   * @param state the led state on or off
   * @short set led state
   */
  void setState( State state );
  
  /** Set the shape. */
  void setShape(Shape s);
  /**
   * toggles the state of the led from Off to On and vice versa.
   * The widget will be repainted when returning to the main
   * event loop.
   * @short toggles led on->off / off->on
   */
  void toggleState();

  /**
   * Sets the color of the widget.
   * The widget calls the update method, so it will
   * be updated when entering the main event loop.
   *
   * see also: enum Color
   *
   * @param color new Color of the led
   * @short sets the led color
   */
  void setColor(const QColor& color);

  /**
   * Sets the look of the widget.
   * the look may be flat, round or sunken.
   * The widget calls the update method, so it will
   * be updated when entering the main event loop.
   *
   * see also: enum Look
   *
   * @param look new look of the led
   * @short sets led look
   */
  void setLook( Look look );

  public slots:

    /**
     * toggles the state of the led from Off to On an vice versa.
     * The widget repaints itself immediately.
     */
    void toggle();

  /**
   * sets the state of the widget to On.
   * The widget will be painted immediately.
   * see off(), toggle(), toggleState(), setState()
   */  
  void on();

  /**
   * sets the state of the widget to Off.
   * The widget will be painted immediately.
   * see also: on(), toggle(), toggleState(), setState()
   */
  void off();

 protected:
  void paintEvent( QPaintEvent * );
  
  void paintflat();
  void paintround();
  void paintsunken();
  void paintrect();

  State led_state;
  QColor current_color;
  QColor led_color;
  Look  led_look;
  Shape led_shape;
};

#endif
