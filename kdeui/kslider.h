//-----------------------------------------------------------------------------
// Slider control
//
// V1.0 Copyright (C) Martin R. Jones 1997
// V1.1 and following: Now maintained by Christian Esken <chris@magicon.prima.ruhr.de>

#ifndef __KSLIDER_H__
#define __KSLIDER_H__

#include <qrangect.h>
#include <qwidget.h>


/// Control for selecting a value from using a sliding pointer.
/**
KSlider is a control for selecting a value from using a sliding pointer.
It can be setup similarly to a scrollbar
Calling setSteps( line, page ) sets up the ruler marks along the slider.
e.g.

KSlider slider( KSlider::Horizontal, parent );

slider.setRange( 0, 20 );
slider.setSteps( 5, 10 );

will create a horizontal slider with a range of 0 to 20 and small ruler
marks spaced 5 apart and long ruler marks placed 10 apart.

TODO

Currently clicking any where in the control moves the slider to that position.
This should be changed so that the slider may be grabbed and moved, or an
area on the left or right of the control clicked to move the slider in
lineStep() increments.
*/
class KSlider : public QWidget, public QRangeControl
{
  Q_OBJECT
    public:
  enum Orientation { Horizontal, Vertical };

  KSlider( Orientation o, QWidget *parent = NULL, const char *name = NULL );
  KSlider( int _minValue, int _maxValue, int _lineStep, int _pageStep,
	   Orientation o, QWidget *parent = NULL, const char *name = NULL );

  void setOrientation( Orientation o )
    {	_orientation = o; }
  Orientation orientation() const
    {	return _orientation; }

  virtual QSize sizeHint() const;

signals:
  void valueChanged( int value );

protected:
  virtual void valueChange();
  virtual void drawArrow( QPainter &painter, bool show, const QPoint &pos );

private:
  QPoint calcArrowPos( int val );
  void moveArrow( const QPoint &pos );

  virtual void paintEvent( QPaintEvent * );
  virtual void mousePressEvent( QMouseEvent *e );
  virtual void mouseMoveEvent( QMouseEvent *e );

protected:
  Orientation _orientation;

  virtual void rangeChange();

private:
  int checkWidth();
};

//-----------------------------------------------------------------------------

#endif

