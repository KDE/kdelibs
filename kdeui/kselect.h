//-----------------------------------------------------------------------------
// Selector widgets for KDE Color Selector, but probably useful for other
// stuff also.
//
// Copyright (C) Martin R. Jones 1996
//

#ifndef __KSELECT_H__
#define __KSELECT_H__

#include <qdialog.h>
#include <qframe.h>
#include <qrangect.h>
#include <qlined.h>
#include <qpixmap.h>

/// 2D value selector.
/** 2D value selector.
  The contents of the selector are drawn by derived class.
  */
class KXYSelector : public QWidget
{
  Q_OBJECT
public:
  KXYSelector( QWidget *parent = NULL, const char *name = NULL );

  void setValues( int _xPos, int _yPos );
  void setRange( int _minX, int _minY, int _maxX, int _maxY );

  int xValue()	{	return xPos; }
  int yValue()	{	return yPos; }

  QRect contentsRect();

  signals:
  void valueChanged( int _x, int _y );

protected:
  virtual void paintEvent( QPaintEvent *e );

  /// Draw contents
  /** Override this function to draw the contents of the widget.
	Draw within contentsRect() only
	*/
  virtual void drawContents( QPainter * ) {}

  virtual void mousePressEvent( QMouseEvent *e );
  virtual void mouseMoveEvent( QMouseEvent *e );

private:
  void setPosition( int xp, int yp );
  void drawCursor( QPainter &painter, int xp, int yp );

protected:
  int px;
  int py;
  int xPos;
  int yPos;
  int minX;
  int maxX;
  int minY;
  int maxY;
  QPixmap store;
};

/// 1D value selector
/** 1D value selector with contents drawn by derived class.
  See KColorDialog for example.
 */
class KSelector : public QWidget, public QRangeControl
{
  Q_OBJECT
public:
  enum Orientation { Horizontal, Vertical };

  KSelector( Orientation o, QWidget *parent = NULL, const char *name = NULL );

  Orientation orientation() const
  {	return _orientation; }

  QRect contentsRect();

  void setIndent( bool i )
  {	_indent = i; }
  bool indent() const
  {	return _indent; }

  signals:
  void valueChanged( int value );

protected:
  /**
	Override this function to draw the contents of the control.
	Draw only within contentsRect().
	*/
  virtual void drawContents( QPainter * ) {}

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
  bool _indent;
};

//-----------------------------------------------------------------------------

class KGradientSelector : public KSelector
{
  Q_OBJECT
public:
  KGradientSelector( Orientation o, QWidget *parent = NULL,
					 const char *name = NULL );

  void setColors( const QColor &col1, const QColor &col2 )
  {	color1 = col1; color2 = col2; }
  void setText( const char *t1, const char *t2 )
  {	text1 = t1; text2 = t2; }

protected:
  virtual void drawContents( QPainter * );

protected:
  QColor color1, color2;
  QString text1, text2;
};

#endif		// __KSELECT_H__

