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

#ifndef _KLED_H_
#define _KLED_H_

#include <qwidget.h>

class QColor;
/** 
 * Displays a round  or rectangular light emitting diode.
 *
 * It is configurable to five colors, the two on/off states and three
 * styles (or "looks");
 *
 * It may display itself in a performant flat view, a round view with
 * light spot or a round view sunken in the screen.
 *
 * @short An LED widget.
 * @author Joerg Habenicht, Richard J. Moore (rich@kde.org) 1998, 1999
 */
class KLed : public QWidget
{
    Q_OBJECT
    Q_ENUMS( State Shape Look )
    Q_PROPERTY( State state READ state WRITE setState )
    Q_PROPERTY( Shape shape READ shape WRITE setShape )
    Q_PROPERTY( Look look READ look WRITE setLook )
    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_PROPERTY( int darkFactor READ darkFactor WRITE setDarkFactor )

public:

  /**
   * Status of the light is on/off.
   * @short LED on/off.
   */
  enum State { Off, On };
  
  /** 
   * Shades of the lamp.
   * @short LED shape
   */ 
  enum Shape { Rectangular, Circular };
  
  /**
   * Displays a flat, round or sunken LED.
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
   *  @li flat Approximately 0.7 msec per paint 
   *  @li round Approximately 2.9 msec per paint
   *  @li sunken Approximately 3.3 msec per paint
   *
   * The widget will be updated on the next repaining event.
   *
   * @short LED look.
   */
  enum Look  { Flat, Raised, Sunken };

  /**
   * Constructs a green, round LED widget which will initially
   * be turned on.
   */
  KLed(QWidget *parent=0, const char *name=0);
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
   * Destructor
   * @short Destructor
   */
  ~KLed();

  /**
   * Returns the current state of the widget (on/off).
   *
   * @see State
   * @short Returns LED state.
   */
  State state() const;

  Shape shape() const;
   
  /**
   * Returns the color of the widget
   *
   * @see Color
   * @short Returns LED color.
   */
  QColor color() const;

  /**
   * Returns the look of the widget.
   *
   * @see Look
   * @short Returns LED look.
   */
  Look look() const;

  /**
   * Returns the factor to darken the LED.
   *
   * @see setDarkFactor()
   * @short Returns dark factor
   */
  int darkFactor() const;

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
  
  /**
   * Set the shape of the LED to @p s.
   */
  void setShape(Shape s);
  /**
   * Toggle the state of the LED from Off to On and vice versa.
   *
   * The widget will be repainted when returning to the main
   * event loop.
   * @short Toggles LED on->off / off->on.
   * @deprecated, use #toggle() instead.
   */
  void toggleState();

  /**
   * Set the color of the widget.
   * The Color is shown with the KLed::On state.
   * The KLed::Off state is shown with QColor.dark() method
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
   * Sets the factor to darken the LED in OFF state.
   * Same as @ref QColor::dark().
   * "darkfactor should be greater than 100, else the LED gets lighter
   * in OFF state.
   * Defaults to 300.
   *
   * @see QColor
   *
   * @param darkfactor sets the factor to darken the LED.
   * @short sets the factor to darken the LED.
   */
  void setDarkFactor(int darkfactor);

  /**
   * Sets the color of the widget.
   * The Color is shown with the KLed::On state.
   * darkcolor is explicidly used for the off state of the LED.
   * Normally you don't have to use this method, the setColor(const QColor& color) is sufficient for the task.
   *
   * The widget calls the update() method, so it will
   * be updated when entering the main event loop.
   *
   * @see Color setColor()
   *
   * @param color New color of the LED used for on state.
   * @param darkcolor Dark color of the LED used for off state.
   * @short Sets the light and dark LED color.
   *
  void setColor(const QColor& color, const QColor& darkcolor);
  */

  /**
   * Sets the look of the widget.
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

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;

public slots:
    
  /**
   * Toggles the state of the led from Off to On or vice versa.
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
  /**
   * Paints a circular, flat LED.
   */
  virtual void paintFlat();
  /**
   * Paints a circular, raised LED.
   */
  virtual void paintRound();
  /**
   * Paints a circular, sunken LED.
   */
  virtual void paintSunken();
  /**
   * Paints a rectangular, flat LED.
   */
  virtual void paintRect();
  /**
   * Paints a rectangular LED, either raised or 
   * sunken, depending on its argument.
   */
  virtual void paintRectFrame(bool raised);
  /**
   * @reimplemented
   */
  void paintEvent( QPaintEvent * );
  
private:
  State led_state;
  QColor led_color;
  Look  led_look;
  Shape led_shape;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KLedPrivate;
  KLedPrivate *d;
};

#endif
