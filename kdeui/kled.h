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
* Display a round  or rectangular light emmitting diode.
*
* It is configurable to 5 colors, the 2 on/off states and 3 styles (or
*  "looks");
*
* It may display itself in a performant flat view, a round view with
* light spot or a round view sunken in the screen.
*
* @short A round LED widget.
* @author Joerg Habenicht, Richard J. Moore (rich@kde.org) 1998, 1999
*/

class KLed : public QWidget
{
  Q_OBJECT

    public:

  /**
   * Status of the light is on/off.
   * @short LED on/off.
   */
  enum State { Off, On, NoOfStates };
  
  /** 
   * Shades of the lamp. 
   * @short shade
   */ 
  enum Shape { NoShape, Rectangular, Circular, NoOfShapes=Circular };
  
  /**
   * Display a flat, round or sunken LED.
   *
   * Displaying the LED flat is less time and color consuming,
   * but not so nice to see.
   *
   * The sunken LED itself is (certainly) smaller than the round LED
   * because of the 3 shading circles and is
   * most time consuming. Makes sense for LED > 15x15 pixels.
   *
   * @sect Timings:
   * ( AMD K5/133, Diamond Stealth 64 PCI Graphics, widgetsize 29x29 )
   *  @li flat approx. 0.7 msec per paint 
   *  @li round approx. 2.9 msec per paint
   *  @li sunken approx. 3.3 msec per paint
   *
   * The widget will be updated on the next repaining event.
   * @short LED look.
   */
  enum Look  { NoLook, Flat, Raised, Sunken, NoOfLooks=Sunken };

  /**
   * Constructor with the ledcolor, the parent widget, and the name.
   *
   * The State will be defaulted On and the Look round.
   *
   * @param ledcolor Initial color of the LED.
   * @param parent   Will be handed over to QWidget.
   * @param name     Will be handed over to QWidget.
   * @short Constructor
   */
  KLed(const QColor &col=Qt::green, QWidget *parent=0, const char *name=0);

  /**
   * Constructor with the ledcolor, ledstate, ledlook,
   * the parent widget, and the name.
   *
   * Differs from above only in the parameters, which configure all settings.
   *
   * @param ledcolor Initial color of the LED.
   * @param state    Sets the State.
   * @param look     Sets the Look.
   * @param parent   Will be handed over to QWidget.
   * @param name     Will be handed over to QWidget.
   * @short Constructor
   */
  KLed(const QColor& col, KLed::State st, KLed::Look look, KLed::Shape shape, 
       QWidget *parent=0, const char *name=0);

  /**
   * Hand back the current state of the widget (on/off).
   *
   * @see State
   * @short Returns LED state.
   */
  State state() const;

  /**
   * Hand back the color of the widget
   *
   * @see Color
   * @short Returns LED color.
   */
  const QColor color() const;

  /**
   * Hand back the look of the widget.
   *
   * @see Look
   * @short Returns LED look.
   */
   Look look() const;

  /**
   * Sets the state of the widget to On or Off.
   *
   * The widget will be painted immediately.
   * @see on() off() toggle() toggleState()
   *
   * @param state The LED state: on or off.
   * @short Set LED state.
   */
  void setState( State state );
  
  /** Set the shape. */
  void setShape(Shape s);
  /**
   * Toggle the state of the LED from Off to On and vice versa.
   *
   * The widget will be repainted when returning to the main
   * event loop.
   * @short Toggles LED on->off / off->on.
   */
  void toggleState();

  /**
   * Set the color of the widget.
   *
   * The widget calls the update() method, so it will
   * be updated when entering the main event loop.
   *
   * @see Color
   *
   * @param color New color of the LED.
   * @short Sets the LED color.
   */
  void setColor(const QColor& color);

  /**
   * Set the look of the widget.
   *
   * The look may be flat, round or sunken.
   * The widget calls the update() method, so it will
   * be updated when entering the main event loop.
   *
   * @see Look
   *
   * @param look New look of the LED.
   * @short Sets LED look.
   */
  void setLook( Look look );

  public slots:

    /**
     * Toggle the state of the led from Off to On or vice versa.
     *
     * The widget repaints itself immediately.
     */
    void toggle();

  /**
   * Sets the state of the widget to On.
   *
   * The widget will be painted immediately.
   * @see off() toggle() toggleState() setState()
   */  
  void on();

  /**
   * Sets the state of the widget to Off.
   *
   * The widget will be painted immediately.
   * @see on() toggle() toggleState() setState()
   */
  void off();

 protected:
  void paintEvent( QPaintEvent * );
  
  void paintflat();
  void paintround();
  void paintsunken();
  void paintrect();
  // paints the LED lamp with a frame, either raised (true) or sunken(false)
  void paintrectframe(bool raised);

  State led_state;
  QColor led_color;
  Look  led_look;
  Shape led_shape;
};

#endif
