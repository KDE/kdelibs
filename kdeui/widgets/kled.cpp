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

#include "kled.h"

#define PAINT_BENCH
#undef PAINT_BENCH

#ifdef PAINT_BENCH
#include <qdatetime.h>
#include <stdio.h>
#endif


#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QApplication>

class KLed::Private
{
  public:
    Private()
      : darkFactor( 300 ), color( Qt::green ),
        state( On ), look( Raised ), shape( Circular ),
        offMap( 0 ), onMap( 0 )
    {
      offColor = color.dark( darkFactor );
    }

    ~Private()
    {
      delete offMap;
      delete onMap;
    }

    int darkFactor;
    QColor color;
    State state;
    Look look;
    Shape shape;

    QColor offColor;
    QPixmap *offMap;
    QPixmap *onMap;
};



KLed::KLed( QWidget *parent )
  : QWidget( parent ),
    d( new Private )
{
  setColor( QColor( Qt::green ) );
}


KLed::KLed( const QColor& color, QWidget *parent )
  : QWidget( parent ),
    d( new Private )
{
  setColor( color );
}

KLed::KLed( const QColor& color, State state, Look look, Shape shape,
            QWidget *parent )
  : QWidget( parent ),
    d( new Private )
{
  d->state = state;
  d->look = look;
  d->shape = shape;

  setColor( color );
}

KLed::~KLed()
{
  delete d;
}

void KLed::paintEvent( QPaintEvent* )
{
#ifdef PAINT_BENCH
  const int rounds = 1000;
  QTime t;
  t.start();
  for ( int i = 0; i < rounds; i++ ) {
#endif
    switch( d->shape ) {
      case Rectangular:
        switch ( d->look ) {
          case Sunken:
            paintRectFrame( false );
            break;
          case Raised:
            paintRectFrame( true );
            break;
          case Flat:
            paintRect();
            break;
          default:
            qWarning( "%s: in class KLed: no KLed::Look set", qApp->argv()[0] );
        }
        break;
      case Circular:
        switch ( d->look ) {
          case Flat:
            paintFlat();
            break;
          case Raised:
            paintRaised();
            break;
          case Sunken:
            paintSunken();
            break;
          default:
            qWarning( "%s: in class KLed: no KLed::Look set", qApp->argv()[0] );
        }
        break;
      default:
        qWarning( "%s: in class KLed: no KLed::Shape set", qApp->argv()[0] );
        break;
    }
#ifdef PAINT_BENCH
  }

  int ready = t.elapsed();
  qWarning( "elapsed: %d msec. for %d rounds", ready, rounds );
#endif
}

int KLed::ledWidth() const
{
  // Initialize coordinates, width, and height of the LED
  int width = this->width();

  // Make sure the LED is round!
  if ( width > this->height() )
    width = this->height();

  // leave one pixel border
  width -= 2;

  if ( width < 0 )
    width = 0;

  return width;
}

bool KLed::paintCachedPixmap()
{
  if ( d->state == On ) {
    if ( d->onMap ) {
      QPainter paint( this );
      paint.drawPixmap( 0, 0, *d->onMap );
      return true;
    }
  } else {
    if ( d->offMap ) {
      QPainter paint( this );
      paint.drawPixmap( 0, 0, *d->offMap );
      return true;
    }
  }

  return false;
}

// paint a ROUND FLAT led lamp
void KLed::paintFlat()
{
  if ( paintCachedPixmap() )
    return;

  QPainter paint;
  QColor color;
  QBrush brush;
  QPen pen;

  int width = ledWidth();

  int scale = 3;
  QPixmap *tmpMap = 0;

  width *= scale;

  tmpMap = new QPixmap( width + 6, width + 6 );
  tmpMap->fill( palette().color( backgroundRole() ) );

  // start painting widget
  paint.begin( tmpMap );
  paint.setRenderHint(QPainter::Antialiasing);

  // Set the color of the LED according to given parameters
  color = ( d->state ) ? d->color : d->offColor;

  // Set the brush to SolidPattern, this fills the entire area
  // of the ellipse which is drawn with a thin gray "border" (pen)
  brush.setStyle( Qt::SolidPattern );
  brush.setColor( color );

  pen.setWidth( scale );
  color = palette().color( QPalette::Dark );
  pen.setColor( color ); // Set the pen accordingly

  paint.setPen( pen ); // Select pen for drawing
  paint.setBrush( brush ); // Assign the brush to the painter

  // Draws a "flat" LED with the given color:
  paint.drawEllipse( scale, scale, width - scale * 2, width - scale * 2 );

  paint.end();

  // painting done
  QPixmap *&dest = ( d->state == On ? d->onMap : d->offMap );
  QImage i = tmpMap->toImage();
  width /= 3;
  i = i.scaled( width, width, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
  delete tmpMap;

  dest = new QPixmap( QPixmap::fromImage( i ) );
  paint.begin( this );
  paint.drawPixmap( 0, 0, *dest );
  paint.end();
}

// paint a ROUND RAISED led lamp
void KLed::paintRaised()
{
  if ( paintCachedPixmap() )
    return;

  QPainter paint;
  QColor color;
  QBrush brush;
  QPen pen;

  // Initialize coordinates, width, and height of the LED
  int width = ledWidth();

  int scale = 3;
  QPixmap *tmpMap = 0;

  width *= scale;

  tmpMap = new QPixmap( width + 6, width + 6 );
  tmpMap->fill( palette().color( backgroundRole() ) );
  paint.begin( tmpMap );
  paint.setRenderHint(QPainter::Antialiasing);

  // Set the color of the LED according to given parameters
  color = ( d->state == On ? d->color : d->offColor );

  // Set the brush to SolidPattern, this fills the entire area
  // of the ellipse which is drawn first
  brush.setStyle( Qt::SolidPattern );
  brush.setColor( color );
  paint.setBrush( brush ); // Assign the brush to the painter

  // Draws a "flat" LED with the given color:
  paint.drawEllipse( scale, scale, width - scale * 2, width - scale * 2 );

  // Draw the bright light spot of the LED now, using modified "old"
  // painter routine taken from KDEUI's KLed widget:

  // Setting the new width of the pen is essential to avoid "pixelized"
  // shadow like it can be observed with the old LED code
  pen.setWidth( 2 * scale );

  // shrink the light on the LED to a size about 2/3 of the complete LED
  int pos = width / 5 + 1;
  int light_width = width;
  light_width *= 2;
  light_width /= 3;

  // Calculate the LED's "light factor":
  int light_quote = ( 130 * 2 / ( light_width ? light_width : 1 ) ) + 100;

  // Now draw the bright spot on the LED:
  while ( light_width ) {
    color = color.light( light_quote );  // make color lighter
    pen.setColor( color );  // set color as pen color
    paint.setPen( pen );  // select the pen for drawing
    paint.drawEllipse( pos, pos, light_width, light_width );  // draw the ellipse (circle)
    light_width--;

    if ( !light_width )
      break;

    paint.drawEllipse( pos, pos, light_width, light_width );
    light_width--;

    if ( !light_width )
      break;

    paint.drawEllipse( pos, pos, light_width, light_width );
    pos++;
    light_width--;
  }

  // Drawing of bright spot finished, now draw a thin gray border
  // around the LED; it looks nicer that way. We do this here to
  // avoid that the border can be erased by the bright spot of the LED

  pen.setWidth( 2 * scale + 1 );
  color = palette().color( QPalette::Dark );
  pen.setColor( color );  // Set the pen accordingly
  paint.setPen( pen );  // Select pen for drawing
  brush.setStyle( Qt::NoBrush );  // Switch off the brush
  paint.setBrush( brush );  // This avoids filling of the ellipse

  paint.drawEllipse( 2, 2, width, width );

  paint.end();

  // painting done
  QPixmap *&dest = ( d->state == On ? d->onMap : d->offMap );
  QImage i = tmpMap->toImage();
  width /= 3;
  i = i.scaled( width, width, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
  delete tmpMap;

  dest = new QPixmap( QPixmap::fromImage( i ) );
  paint.begin( this );
  paint.drawPixmap( 0, 0, *dest );
  paint.end();
}

// paint a ROUND SUNKEN led lamp
void KLed::paintSunken()
{
  if ( paintCachedPixmap() )
    return;

  QPainter paint;
  QColor color;
  QBrush brush;
  QPen pen;

  // First of all we want to know what area should be updated
  // Initialize coordinates, width, and height of the LED
  int width = ledWidth();

  int scale = 3;
  QPixmap *tmpMap = 0;

  width *= scale;

  tmpMap = new QPixmap( width, width );
  tmpMap->fill( palette().color( backgroundRole() ) );
  paint.begin( tmpMap );
  paint.setRenderHint(QPainter::Antialiasing);

  // Set the color of the LED according to given parameters
  color = ( d->state == On ) ? d->color : d->offColor;

  // Set the brush to SolidPattern, this fills the entire area
  // of the ellipse which is drawn first
  brush.setStyle( Qt::SolidPattern );
  brush.setColor( color );
  paint.setBrush( brush );  // Assign the brush to the painter

  // Draws a "flat" LED with the given color:
  paint.drawEllipse( scale, scale, width - scale * 2, width - scale * 2 );

  // Draw the bright light spot of the LED now, using modified "old"
  // painter routine taken from KDEUI's KLed widget:

  // Setting the new width of the pen is essential to avoid "pixelized"
  // shadow like it can be observed with the old LED code
  pen.setWidth( 2 * scale );

  // shrink the light on the LED to a size about 2/3 of the complete LED
  int pos = width / 5 + 1;
  int light_width = width;
  light_width *= 2;
  light_width /= 3;

  // Calculate the LED's "light factor":
  int light_quote = ( 130 * 2 / ( light_width ? light_width : 1 ) ) + 100;

  // Now draw the bright spot on the LED:
  while ( light_width ) {
    color = color.light( light_quote );                      // make color lighter
    pen.setColor( color );                                   // set color as pen color
    paint.setPen( pen );                                     // select the pen for drawing
    paint.drawEllipse( pos, pos, light_width, light_width ); // draw the ellipse (circle)
    light_width--;

    if ( !light_width )
      break;

    paint.drawEllipse( pos, pos, light_width, light_width );
    light_width--;

    if ( !light_width )
      break;

    paint.drawEllipse( pos, pos, light_width, light_width );
    pos++;
    light_width--;
  }

  // Drawing of bright spot finished, now draw a thin border
  // around the LED which resembles a shadow with light coming
  // from the upper left.

  pen.setWidth( 2 * scale + 1 ); // ### shouldn't this value be smaller for smaller LEDs?
  brush.setStyle( Qt::NoBrush );              // Switch off the brush
  paint.setBrush( brush );                        // This avoids filling of the ellipse

  // Set the initial color value to QColorGroup(palette()).light() (bright) and start
  // drawing the shadow border at 45° (45*16 = 720).

  int angle = -720;
  color = palette().color( QPalette::Light );

  for ( int arc = 120; arc < 2880; arc += 240 ) {
    pen.setColor( color );
    paint.setPen( pen );
    int w = width - pen.width() / 2 - scale + 1;
    paint.drawArc( pen.width() / 2, pen.width() / 2, w, w, angle + arc, 240 );
    paint.drawArc( pen.width() / 2, pen.width() / 2, w, w, angle - arc, 240 );
    color = color.dark( 110 ); //FIXME: this should somehow use the contrast value
  }  // end for ( angle = 720; angle < 6480; angle += 160 )

  paint.end();

  // painting done

  QPixmap *&dest = ( d->state == On ? d->onMap : d->offMap );
  QImage i = tmpMap->toImage();
  width /= 3;
  i = i.scaled( width, width, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
  delete tmpMap;

  dest = new QPixmap( QPixmap::fromImage( i ) );
  paint.begin( this );
  paint.setCompositionMode(QPainter::CompositionMode_Source);
  paint.drawPixmap( 0, 0, *dest );
  paint.end();
}

void KLed::paintRect()
{
  QPainter painter( this );
  painter.setRenderHint(QPainter::Antialiasing);
  QBrush lightBrush( d->color );
  QBrush darkBrush( d->offColor );

  QPen pen( d->color.dark( 300 ) );
  int w = width();
  int h = height();

  // -----
  switch ( d->state ) {
    case On:
      painter.setBrush( lightBrush );
      painter.drawRect( 0, 0, w, h );
      break;
    case Off:
      painter.setBrush( darkBrush );
      painter.drawRect( 0, 0, w, h );
      painter.setPen( pen );
      painter.drawLine( 0, 0, w, 0 );
      painter.drawLine( 0, h - 1, w, h - 1 );

      // Draw verticals
      int i;
      for ( i = 0; i < w; i += 4 /* dx */)
        painter.drawLine( i, 1, i, h - 1 );
      break;
    default:
      break;
  }
}

void KLed::paintRectFrame( bool raised )
{
  QPainter painter( this );
  painter.setRenderHint(QPainter::Antialiasing);
  QBrush lightBrush( d->color );
  QBrush darkBrush( d->offColor );
  int w = width();
  int h = height();

  if ( raised ) {
    painter.setPen( Qt::white );
    painter.drawLine( 0, 0, 0, h - 1 );
    painter.drawLine( 1, 0, w - 1, 0 );
    painter.setPen( Qt::black );
    painter.drawLine( 1, h - 1, w - 1, h - 1 );
    painter.drawLine( w - 1, 1, w - 1, h - 1 );
    painter.fillRect( 1, 1, w - 2, h - 2, ( d->state == On ? lightBrush : darkBrush ) );
  } else {
    painter.setPen( Qt::black );
    painter.drawRect( 0, 0, w, h );
    painter.drawRect( 0, 0, w - 1, h - 1 );
    painter.setPen( Qt::white );
    painter.drawRect( 1, 1, w - 1, h - 1 );
    painter.fillRect( 2, 2, w - 4, h - 4, ( d->state == On ? lightBrush : darkBrush ) );
  }
}

KLed::State KLed::state() const
{
  return d->state;
}

KLed::Shape KLed::shape() const
{
  return d->shape;
}

QColor KLed::color() const
{
  return d->color;
}

KLed::Look KLed::look() const
{
  return d->look;
}

void KLed::setState( State state )
{
  if ( d->state == state)
    return;

  d->state = state;
  update();
}

void KLed::setShape( Shape shape )
{
  if ( d->shape == shape )
    return;

  d->shape = shape;
  update();
}

void KLed::setColor( const QColor &color )
{
  if ( d->color == color )
    return;

  if ( d->onMap ) {
    delete d->onMap;
    d->onMap = 0;
  }

  if ( d->offMap ) {
    delete d->offMap;
    d->offMap = 0;
  }

  d->color = color;
  d->offColor = color.dark( d->darkFactor );

  update();
}

void KLed::setDarkFactor( int darkFactor )
{
  if ( d->darkFactor == darkFactor )
    return;

  d->darkFactor = darkFactor;
  d->offColor = d->color.dark( darkFactor );
  update();
}

int KLed::darkFactor() const
{
  return d->darkFactor;
}

void KLed::setLook( Look look )
{
  if ( d->look == look)
    return;

  d->look = look;
  update();
}

void KLed::toggle()
{
  d->state = (d->state == On ? Off : On);
  update();
}

void KLed::on()
{
  setState( On );
}

void KLed::off()
{
  setState( Off );
}

QSize KLed::sizeHint() const
{
  return QSize( 16, 16 );
}

QSize KLed::minimumSizeHint() const
{
  return QSize( 16, 16 );
}

#include "kled.moc"
