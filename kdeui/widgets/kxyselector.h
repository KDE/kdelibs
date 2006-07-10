#ifndef KXYSELECTOR_H
#define KXYSELECTOR_H

#include <kdelibs_export.h>
#include <QtGui/QWidget>
#include <QPixmap>

/**
 * KXYSelector is the base class for other widgets which
 * provides the ability to choose from a two-dimensional
 * range of values. The currently chosen value is indicated
 * by a cross. An example is the KHSSelector which
 * allows to choose from a range of colors, and which is
 * used in KColorDialog.
 *
 * A custom drawing routine for the widget surface has
 * to be provided by the subclass.
 */
class KDEUI_EXPORT KXYSelector : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( int xValue READ xValue WRITE setXValue )
  Q_PROPERTY( int yValue READ yValue WRITE setYValue )

public:
  /**
   * Constructs a two-dimensional selector widget which
   * has a value range of [0..100] in both directions.
   */
  explicit KXYSelector( QWidget *parent=0 );
  /**
   * Destructs the widget.
   */
  ~KXYSelector();

  /**
   * Sets the current values in horizontal and
   * vertical direction.
   * @param xPos the horizontal value
   * @param yPos the veritcal value
   */
  void setValues( int xPos, int yPos );

  /**
   * Sets the current horizontal value
   * @param xPos the horizontal value
   */
  void setXValue( int xPos );

  /**
   * Sets the current vertical value
   * @param yPos the veritcal value
   */
  void setYValue( int yPos );

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

  /**
   * Reimplemented to give the widget a minimum size
   */
  virtual QSize minimumSizeHint() const;

Q_SIGNALS:
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
   * Draw within contentsRect() only.
   */
  virtual void drawContents( QPainter * );
  /**
   * Override this function to draw the cursor which
   * indicates the currently selected value pair.
   */
  virtual void drawCursor( QPainter *p, int xp, int yp );

  virtual void paintEvent( QPaintEvent *e );
  virtual void mousePressEvent( QMouseEvent *e );
  virtual void mouseMoveEvent( QMouseEvent *e );
  virtual void wheelEvent( QWheelEvent * );

  /**
   * Converts a pixel position to its corresponding values.
   */
  void valuesFromPosition( int x, int y, int& xVal, int& yVal ) const;

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

private:
  class KXYSelectorPrivate;
  KXYSelectorPrivate *d;
};

class KDEUI_EXPORT KHueSaturationSelector : public KXYSelector
{
    Q_OBJECT
public:
  /**
   * Constructs a hue/saturation selection widget.
   */
  KHueSaturationSelector( QWidget *parent=0);

protected:
  /**
   * Draws the contents of the widget on a pixmap,
   * which is used for buffering.
   */
  virtual void drawPalette( QPixmap *pixmap );
  virtual void resizeEvent( QResizeEvent * );

  /**
   * Reimplemented from KXYSelector. This drawing is
   * buffered in a pixmap here. As real drawing
   * routine, drawPalette() is used.
   */
  virtual void drawContents( QPainter *painter );

private:
  void updateContents();
  QPixmap pixmap;
};

#endif /* KXYSELECTOR_H */
