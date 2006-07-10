#include "kxyselector.h"
#include <kimageeffect.h>
#include <kstaticdeleter.h>
#include <QStyle>
#include <QPainter>
#include <QStyleOptionFrame>
#include <QMouseEvent>

//-----------------------------------------------------------------------------
/*
 * 2D value selector.
 * The contents of the selector are drawn by derived class.
 */

KXYSelector::KXYSelector( QWidget *parent )
  : QWidget( parent )
{
  xPos = 0;
  yPos = 0;
  minX = 0;
  minY = 0;
  maxX = 100;
  maxY = 100;
}


KXYSelector::~KXYSelector()
{}


void KXYSelector::setRange( int _minX, int _minY, int _maxX, int _maxY )
{
  int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  px = w;
  py = w;
  minX = _minX;
  minY = _minY;
  maxX = _maxX;
  maxY = _maxY;
}

void KXYSelector::setXValue( int _xPos )
{
  setValues(_xPos, yPos);
}

void KXYSelector::setYValue( int _yPos )
{
  setValues(xPos, _yPos);
}

void KXYSelector::setValues( int _xPos, int _yPos )
{
  int w = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  if (w < 5) w = 5;

  xPos = _xPos;
  yPos = _yPos;

  if ( xPos > maxX )
    xPos = maxX;
  else if ( xPos < minX )
    xPos = minX;

  if ( yPos > maxY )
    yPos = maxY;
  else if ( yPos < minY )
    yPos = minY;

  int xp = w + (width() - 2 * w) * xPos / (maxX - minX);
  int yp = height() - w - (height() - 2 * w) * yPos / (maxY - minY);

  setPosition( xp, yp );
}

QRect KXYSelector::contentsRect() const
{
  int w = qMax( style()->pixelMetric(QStyle::PM_DefaultFrameWidth), 5 );
  return rect().adjusted(w, w, -w, -w);
}

QSize KXYSelector::minimumSizeHint() const
{
  int w = qMax( style()->pixelMetric(QStyle::PM_DefaultFrameWidth), 5 );
  return QSize( 2 * w, 2 * w );
}

void KXYSelector::paintEvent( QPaintEvent * /* ev */ )
{
  QStyleOptionFrame opt;
  opt.initFrom(this);

  int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  if ( w < 5 ) {
    w = 5 - w;
  }
  opt.rect.adjust( w, w, -w, -w );
  opt.state = QStyle::State_Sunken;

  QPainter painter;
  painter.begin( this );

  style()->drawPrimitive( QStyle::PE_Frame, &opt, &painter, this );

  drawContents( &painter );
  drawCursor( &painter, px, py );

  painter.end();
}

void KXYSelector::mousePressEvent( QMouseEvent *e )
{
  mouseMoveEvent( e );
}

void KXYSelector::mouseMoveEvent( QMouseEvent *e )
{
  int xVal, yVal;
  int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  valuesFromPosition( e->pos().x() - w, e->pos().y() - w, xVal, yVal );
  setValues( xVal, yVal );

  emit valueChanged( xPos, yPos );
}

void KXYSelector::wheelEvent( QWheelEvent *e )
{
  if ( e->orientation() == Qt::Horizontal )
    setValues( xValue() + e->delta()/120, yValue() );
  else
    setValues( xValue(), yValue() + e->delta()/120 );

  emit valueChanged( xPos, yPos );
}

void KXYSelector::valuesFromPosition( int x, int y, int &xVal, int &yVal ) const
{
  int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  if ( w < 5 ) w = 5;
  xVal = ( ( maxX - minX ) * ( x - w ) ) / ( width() - 2 * w );
  yVal = maxY - ( ( ( maxY - minY ) * (y - w) ) / ( height() - 2 * w ) );

  if ( xVal > maxX )
    xVal = maxX;
  else if ( xVal < minX )
    xVal = minX;

  if ( yVal > maxY )
    yVal = maxY;
  else if ( yVal < minY )
    yVal = minY;
}

void KXYSelector::setPosition( int xp, int yp )
{
  int w = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  if (w < 5) w = 5;
  if ( xp < w )
    xp = w;
  else if ( xp > width() - w )
    xp = width() - w;

  if ( yp < w )
    yp = w;
  else if ( yp > height() - w )
    yp = height() - w;

  px = xp;
  py = yp;

  update();
}

void KXYSelector::drawContents( QPainter * )
{}


void KXYSelector::drawCursor( QPainter *p, int xp, int yp )
{
  p->setPen( QPen( Qt::white ) );

  p->drawLine( xp - 6, yp - 6, xp - 2, yp - 2 );
  p->drawLine( xp - 6, yp + 6, xp - 2, yp + 2 );
  p->drawLine( xp + 6, yp - 6, xp + 2, yp - 2 );
  p->drawLine( xp + 6, yp + 6, xp + 2, yp + 2 );
}

static QVector<QColor> *s_standardPalette = 0;
static KStaticDeleter<QVector<QColor> > spd;

// Shared with KColorValueSelector
KDEUI_EXPORT QVector<QColor> kdeui_standardPalette()
{
    if ( !s_standardPalette ) {
        spd.setObject(s_standardPalette, new QVector<QColor>);

        int i = 0;
#define STANDARD_PAL_SIZE 17
        s_standardPalette->resize( STANDARD_PAL_SIZE );

        (*s_standardPalette)[i++] = Qt::red;
        (*s_standardPalette)[i++] = Qt::green;
        (*s_standardPalette)[i++] = Qt::blue;
        (*s_standardPalette)[i++] = Qt::cyan;
        (*s_standardPalette)[i++] = Qt::magenta;
        (*s_standardPalette)[i++] = Qt::yellow;
        (*s_standardPalette)[i++] = Qt::darkRed;
        (*s_standardPalette)[i++] = Qt::darkGreen;
        (*s_standardPalette)[i++] = Qt::darkBlue;
        (*s_standardPalette)[i++] = Qt::darkCyan;
        (*s_standardPalette)[i++] = Qt::darkMagenta;
        (*s_standardPalette)[i++] = Qt::darkYellow;
        (*s_standardPalette)[i++] = Qt::white;
        (*s_standardPalette)[i++] = Qt::lightGray;
        (*s_standardPalette)[i++] = Qt::gray;
        (*s_standardPalette)[i++] = Qt::darkGray;
        (*s_standardPalette)[i++] = Qt::black;
    }
    return *s_standardPalette;
}

KHueSaturationSelector::KHueSaturationSelector( QWidget *parent )
	: KXYSelector( parent )
{
	setRange( 0, 0, 359, 255 );
}

void KHueSaturationSelector::updateContents()
{
	drawPalette(&pixmap);
}

void KHueSaturationSelector::resizeEvent( QResizeEvent * )
{
	updateContents();
}

void KHueSaturationSelector::drawContents( QPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KHueSaturationSelector::drawPalette( QPixmap *pixmap )
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	QImage image( QSize(xSize, ySize), QImage::Format_RGB32 );
	QColor col;
	int h, s;
	uint *p;

	for ( s = ySize-1; s >= 0; s-- )
	{
		p = (uint *) image.scanLine( ySize - s - 1 );
		for( h = 0; h < xSize; h++ )
		{
			col.setHsv( 359*h/(xSize-1), 255*s/((ySize == 1) ? 1 : ySize-1), 192 );
			*p = col.rgb();
			p++;
		}
	}

	if ( pixmap->depth() <= 8 )
	{
                const QVector<QColor> standardPalette = kdeui_standardPalette();
		KImageEffect::dither( image, standardPalette.data(), standardPalette.size() );
	}
	*pixmap=QPixmap::fromImage( image );
}

#include "kxyselector.moc"
