/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//-----------------------------------------------------------------------------
// Selector widgets for KDE Color Selector, but probably useful for other
// stuff also.

#ifndef __KSELECT_H__
#define __KSELECT_H__

#include <qwidget.h>
#include <qrangecontrol.h>
#include <qpixmap.h>

/**
 * KXYSelector is the base class for other widgets which
 * provides the ability to choose from a two-dimensional
 * range of values. The currently chosen value is indicated
 * by a cross. An example is the @ref KHSSelector which
 * allows to choose from a range of colors, and which is
 * used in KColorDialog.
 *
 * A custom drawing routine for the widget surface has
 * to be provided by the subclass.
 */
class KXYSelector : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructs a two-dimensional selector widget which
   * has a value range of [0..100] in both directions.
   */
  KXYSelector( QWidget *parent=0, const char *name=0 );
  /**
   * Destructs the widget.
   */
  ~KXYSelector();

  /**
   * Sets the current values in horizontal and
   * vertical direction.
   */
  void setValues( int xPos, int yPos );
  /**
   * Sets the range of possible values.
   */
  void setRange( int minX, int minY, int maxX, int maxY );

  /**
   * @return the current value in horizontal direction.
   */
  int xValue() const {	return xPos; }
  /**
   * @return the current value in vertical direction.
   */
  int yValue() const {	return yPos; }

  /**
   * @return the rectangle on which subclasses should draw.
   */
  QRect contentsRect() const;

signals:
  /**
   * This signal is emitted whenever the user chooses a value,
   * e.g. by clicking with the mouse on the widget.
   */
  void valueChanged( int x, int y );

protected:
  /**
   * Override this function to draw the contents of the widget.
   * The default implementation does nothing.
   *
   * Draw within @ref contentsRect() only.
   */
  virtual void drawContents( QPainter * );
  /**
   * Override this function to draw the cursor which
   * indicates the currently selected value pair.
   */
  virtual void drawCursor( QPainter *p, int xp, int yp );
  /**
   * @reimplemented
   */
  virtual void paintEvent( QPaintEvent *e );
  /**
   * @reimplemented
   */
  virtual void mousePressEvent( QMouseEvent *e );
  /**
   * @reimplemented
   */
  virtual void mouseMoveEvent( QMouseEvent *e );
  /**
   * @reimplemented
   */
  virtual void wheelEvent( QWheelEvent * );
  /**
   * Converts a pixel position to its corresponding values.
   */
  void valuesFromPosition( int x, int y, int& xVal, int& yVal ); 

private:
  void setPosition( int xp, int yp );
  int px;
  int py;
  int xPos;
  int yPos;
  int minX;
  int maxX;
  int minY;
  int maxY;
  QPixmap store;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KXYSelectorPrivate;
  KXYSelectorPrivate *d;
};


/**
 * KSelector is the base class for other widgets which
 * provides the ability to choose from a one-dimensional
 * range of values. An example is the @ref KGradientSelector
 * which allows to choose from a range of colors.
 *
 * A custom drawing routine for the widget surface has
 * to be provided by the subclass.
 */
class KSelector : public QWidget, public QRangeControl
{
  Q_OBJECT
  Q_PROPERTY( int value READ value WRITE setValue )
  Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
  Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
public:

  /**
   * Constructs a horizontal one-dimensional selection widget.
   */
  KSelector( QWidget *parent=0, const char *name=0 );
  /**
   * Constructs a one-dimensional selection widget with
   * a given orientation.
   */
  KSelector( Orientation o, QWidget *parent = 0L, const char *name = 0L );
  /*
   * Destructs the widget.
   */
  ~KSelector();

  /**
   * @return the orientation of the widget.
   */
  Orientation orientation() const
  {	return _orientation; }

  /**
   * @return the rectangle on which subclasses should draw.
   */
  QRect contentsRect() const;

  /**
   * Sets the indent option of the widget to i.
   * This determines whether a shaded frame is drawn.
   */
  void setIndent( bool i )
  {	_indent = i; }
  /**
   * @return whether the indent option is set.
   */
  bool indent() const
  {	return _indent; }

  /**
   * Sets the value.
   */
  void setValue(int value)
  { QRangeControl::setValue(value); }

  /**
   * @returns the value.
   */
  int value() const
  { return QRangeControl::value(); }

  /**
   * Sets the min value.
   */
  void setMinValue(int value)
  { QRangeControl::setMinValue(value); }

  /**
   * @return the min value.
   */
  int minValue() const
  { return QRangeControl::minValue(); }

  /**
   * Sets the max value.
   */
  void setMaxValue(int value)
  { QRangeControl::setMaxValue(value); }

  /**
   * @return the max value.
   */
  int maxValue() const
  { return QRangeControl::maxValue(); }

signals:
  /**
   * This signal is emitted whenever the user chooses a value,
   * e.g. by clicking with the mouse on the widget.
   */
  void valueChanged( int value );

protected:
  /**
   * Override this function to draw the contents of the control.
   * The default implementation does nothing.
   *
   * Draw only within contentsRect().
   */
  virtual void drawContents( QPainter * );
  /**
   * Override this function to draw the cursor which
   * indicates the current value. This function is
   * always called twice, once with argument show=false
   * to clear the old cursor, once with argument show=true
   * to draw the new one.
   */
  virtual void drawArrow( QPainter *painter, bool show, const QPoint &pos );

  /**
   * @reimplemented
   */
  virtual void valueChange();
  /**
   * @reimplemented
   */
  virtual void paintEvent( QPaintEvent * );
  /**
   * @reimplemented
   */
  virtual void mousePressEvent( QMouseEvent *e );
  /**
   * @reimplemented
   */
  virtual void mouseMoveEvent( QMouseEvent *e );
  /**
   * @reimplemented
   */
  virtual void wheelEvent( QWheelEvent * );

private:
  QPoint calcArrowPos( int val );
  void moveArrow( const QPoint &pos );

  Orientation _orientation;
  bool _indent;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KSelectorPrivate;
  KSelectorPrivate *d;
};


/**
 * The KGradientSelector widget allows the user to choose
 * from a one-dimensional range of colors which is given as a
 * gradient between two colors provided by the programmer.
 */
class KGradientSelector : public KSelector
{
  Q_OBJECT

  Q_PROPERTY( QColor firstColor READ firstColor WRITE setFirstColor )
  Q_PROPERTY( QColor secondColor READ secondColor WRITE setSecondColor )
  Q_PROPERTY( QString firstText READ firstText WRITE setFirstText )
  Q_PROPERTY( QString secondText READ secondText WRITE setSecondText )

public:
  /**
   * Constructs a horizontal color selector which
   * contains a gradient between white and black.
   */
  KGradientSelector( QWidget *parent=0, const char *name=0 );
  /**
   * Constructs a colors selector with orientation o which
   * contains a gradient between white and black.
   */
  KGradientSelector( Orientation o, QWidget *parent=0, const char *name=0 );
  /**
   * Destructs the widget.
   */
  ~KGradientSelector();
  /**
   * Sets the two colors which span the gradient.
   */
  void setColors( const QColor &col1, const QColor &col2 )
  {	color1 = col1; color2 = col2; update();}
  void setText( const QString &t1, const QString &t2 )
  {	text1 = t1; text2 = t2; update(); }

  /**
   * Set each color on its own.
   */
  void setFirstColor( const QColor &col )
  { color1 = col; update(); }
  void setSecondColor( const QColor &col )
  { color2 = col; update(); }

  /**
   * Set each description on its own
   */
  void setFirstText( const QString &t )
  { text1 = t; update(); }
  void setSecondText( const QString &t )
  { text2 = t; update(); }

  const QColor firstColor() const
  { return color1; }
  const QColor secondColor() const
  { return color2; }

  const QString firstText() const
  { return text1; }
  const QString secondText() const
  { return text2; }

protected:
  /**
   * @reimplemented
   */
  virtual void drawContents( QPainter * );

  /**
   * @reimplemented
   */
  virtual QSize minimumSize() const
  { return sizeHint(); }

private:
  void init();
  QColor color1;
  QColor color2;
  QString text1;
  QString text2;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KGradientSelectorPrivate;
  KGradientSelectorPrivate *d;
};


#endif		// __KSELECT_H__

