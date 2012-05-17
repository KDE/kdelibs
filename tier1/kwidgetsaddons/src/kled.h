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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KLED_H
#define KLED_H

#include <kwidgetsaddons_export.h>

#include <QWidget>

class QColor;

/**
 * @short An LED widget.
 *
 * Displays a round or rectangular light emitting diode.
 *
 * It is configurable to arbitrary colors, the two on/off states and three
 * styles (or "looks");
 *
 * It may display itself in a performant flat view, a round view with
 * light spot or a round view sunken in the screen.
 *
 * \image html kled.png "KDE LED Widget"
 *
 * @author Joerg Habenicht, Richard J. Moore (rich@kde.org) 1998, 1999
 */
class KWIDGETSADDONS_EXPORT KLed : public QWidget
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
     * \b Timings: \n
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
     *
     * @param parent The parent widget.
     */
    explicit KLed( QWidget *parent = 0 );

    /**
     * Constructs a round LED widget with the supplied color which will
     * initially be turned on.
     *
     * @param color Initial color of the LED.
     * @param parent The parent widget.
     * @short Constructor
     */
    explicit KLed( const QColor &color, QWidget *parent = 0 );

    /**
     * Constructor with the color, state and look.
     *
     * Differs from above only in the parameters, which configure all settings.
     *
     * @param color  Initial color of the LED.
     * @param state  Sets the State.
     * @param look   Sets the Look.
     * @param shape  Sets the Shape (rectangular or circular).
     * @param parent The parent widget.
     * @short Constructor
     */
    KLed( const QColor& color, KLed::State state, KLed::Look look, KLed::Shape shape,
          QWidget *parent = 0 );

    /**
     * Destroys the LED widget.
     * @short Destructor
     */
    ~KLed();

    /**
     * Returns the current color of the widget.
     *
     * @see Color
     * @short Returns LED color.
     */
    QColor color() const;

    /**
     * Returns the current state of the widget (on/off).
     *
     * @see State
     * @short Returns LED state.
     */
    State state() const;

    /**
     * Returns the current look of the widget.
     *
     * @see Look
     * @short Returns LED look.
     */
    Look look() const;

    /**
     * Returns the current shape of the widget.
     *
     * @see Shape
     * @short Returns LED shape.
     */
    Shape shape() const;

    /**
     * Returns the factor to darken the LED.
     *
     * @see setDarkFactor()
     * @short Returns dark factor.
     */
    int darkFactor() const;

    /**
     * Set the color of the widget.
     *
     * The LED is shown with Color when in the KLed::On state
     * or with the darken Color (@see setDarkFactor) in KLed::Off
     * state.
     *
     * The widget calls the update() method, so it will
     * be updated when entering the main event loop.
     *
     * @see Color
     *
     * @param color New color of the LED.
     * @short Sets the LED color.
     */
    void setColor( const QColor& color );

    /**
     * Sets the state of the widget to On or Off.
     *
     * The widget will be painted immediately.
     * @see on() off() toggle()
     *
     * @param state The LED state: on or off.
     * @short Set LED state.
     */
    void setState( State state );

    /**
     * Sets the look of the widget.
     *
     * The look may be Flat, Raised or Sunken.
     *
     * The widget calls the update() method, so it will
     * be updated when entering the main event loop.
     *
     * @see Look
     *
     * @param look New look of the LED.
     * @short Sets LED look.
     */
    void setLook( Look look );

    /**
     * Set the shape of the LED.
     *
     * @param shape The LED shape.
     * @short Set LED shape.
     */
    void setShape( Shape shape );

    /**
     * Sets the factor to darken the LED in KLed::Off state.
     *
     * The @param darkFactor should be greater than 100, otherwise the LED
     * becomes lighter in KLed::Off state.
     *
     * Defaults to 300.
     *
     * @see QColor
     *
     * @param darkFactor Sets the factor to darken the LED.
     * @short Sets the factor to darken the LED.
     */
    void setDarkFactor( int darkFactor );

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

  public Q_SLOTS:

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
     * @see off() toggle()  setState()
     */
    void on();

    /**
     * Sets the state of the widget to Off.
     *
     * The widget will be painted immediately.
     * @see on() toggle()  setState()
     */
    void off();

  protected:
    /**
     * Returns the width of the led.
     */
    virtual int ledWidth() const;

    /**
     * Paints a circular, flat LED.
     */
    virtual void paintFlat();

    /**
     * Paints a circular, raised LED.
     */
    virtual void paintRaised();

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
    virtual void paintRectFrame( bool raised );

    void paintEvent( QPaintEvent* );
    void resizeEvent( QResizeEvent* );

    /**
     * Paint the cached antialiased pixmap corresponding to the state if any
     * @return true if the pixmap was painted, false if it hasn't been created yet
     */
    bool paintCachedPixmap();

    /**
     * @internal
     * invalidates caches after property changes and calls update()
     */
    void updateCachedPixmap();

    /**
     * @internal
     */
    void paintLed(Shape shape, Look look);

  private:
    class Private;
    Private * const d;
};

#endif
