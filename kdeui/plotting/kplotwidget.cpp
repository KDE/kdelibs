/***************************************************************************
                          kplotwidget.cpp - A widget for plotting in KStars
                             -------------------
    begin                : Sun 18 May 2003
    copyright            : (C) 2003 by Jason Harris
    email                : kstars@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h> //for log10(), pow(), modf()
#include <kdebug.h>
#include <qevent.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpolygon.h>

#include "kplotwidget.h"
#include "kplotwidget.moc"

#include "kplotobject.h"

KPlotWidget::KPlotWidget( double x1, double x2, double y1, double y2, QWidget *parent )
 : QFrame( parent ),
   dXtick(0.0), dYtick(0.0),
   nmajX(0), nminX(0), nmajY(0), nminY(0),
   ShowTickMarks( true ), ShowTickLabels( true ), ShowGrid( false )
{
	setBackgroundMode( Qt::NoBackground );

	//set DataRect
	setLimits( x1, x2, y1, y2 );
	setDefaultPadding();

	//Set PixRect (starts at (0,0) because we will translate by leftPadding(), topPadding() )
	PixRect = QRect( 0, 0, width() - leftPadding() - rightPadding(),
	                       height() - topPadding() - bottomPadding() );

	buffer = new QPixmap();

	//default colors:
	setBGColor( Qt::black );
	setFGColor( Qt::white );
	setGridColor( Qt::gray );
}

KPlotWidget::~KPlotWidget()
{
	delete (buffer);
	ObjectList.clear();
}

void KPlotWidget::setLimits( double x1, double x2, double y1, double y2 ) {
	double XA1, XA2, YA1, YA2;
	if (x2<x1) { XA1=x2; XA2=x1; }
	else { XA1=x1; XA2=x2; }
	if ( y2<y1) { YA1=y2; YA2=y1; }
	else { YA1=y1; YA2=y2; }

	DataRect = QRectF( XA1, YA1, XA2-XA1, YA2-YA1 );
	updateTickmarks();
}

void KPlotWidget::updateTickmarks() {
	// Determine the number and spacing of tickmarks for the current plot limits.
	if ( dataWidth() == 0.0 ) {
		kdWarning() << "X range invalid! " << x() << " to " << x2() << endl;
		DataRect.setWidth( 1.0 );
		return;
	}
	if ( dataHeight() == 0.0 ) {
		kdWarning() << "Y range invalid! " << y() << " to " << y2() << endl;
		DataRect.setHeight( 1.0 );
		return;
	}

	int nmajor(0), nminor(0);
	double z(0.0), z2(0.0);
	double Range(0.0), s(0.0), t(0.0), pwr(0.0), dTick(0.0);

	//loop over X and Y axes...the z variables substitute for either X or Y
	for ( unsigned int iaxis=0; iaxis<2; ++iaxis ) {
		if ( iaxis == 1 ) {
			z = x(); z2 = x2();
		} else {
			z = y(); z2 = y2();
		}

		//determine size of region to be drawn, in draw units
		Range = z2 - z;

		//s is the power-of-ten factor of Range:
		//Range = t * s; s = 10^(pwr).  e.g., Range=350.0 then t=3.5, s = 100.0; pwr = 2.0
		modf( log10(Range), &pwr );
		s = pow( 10.0, pwr );
		t = Range/s;

		//adjust s and t such that t is between 3 and 5:
		if ( t < 3.0 ) { t *= 10.0; s /= 10.0; } //t now btwn 3 and 30
		if ( t < 6.0 ) { //accept current values
			dTick = s;
			nmajor = int(t);
			nminor = 5;
		} else if ( t < 10.0 ) { //factor of 2
			dTick = s*2.0;
			nmajor = int(t/2.0);
			nminor = 4;
		} else if ( t < 20.0 ) { //factor of 4
			dTick = s*4.0;
			nmajor = int(t/4.0);
			nminor = 4;
		} else { //factor of 5
			dTick = s*5.0;
			nmajor = int(t/5.0);
			nminor = 5;
		}

		if ( iaxis==1 ) { //X axis
			nmajX = nmajor;
			nminX = nminor;
			dXtick = dTick;
		} else { //Y axis
			nmajY = nmajor;
			nminY = nminor;
			dYtick = dTick;
		}
	} //end for iaxis
}

void KPlotWidget::resizeEvent( QResizeEvent* /* e */ ) {
	int newWidth = width() - leftPadding() - rightPadding();
	int newHeight = height() - topPadding() - bottomPadding();
	PixRect = QRect( 0, 0, newWidth, newHeight );

	buffer->resize( width(), height() );
}

void KPlotWidget::paintEvent( QPaintEvent* /* e */ ) {
	QPainter p;

	p.begin( buffer );
	p.fillRect( 0, 0, width(), height(), bgColor() );
	p.translate( leftPadding(), topPadding() );

	drawObjects( &p );
	drawBox( &p );

	p.end();
	bitBlt( this, 0, 0, buffer );
}

void KPlotWidget::drawObjects( QPainter *p ) {
	for ( QList<KPlotObject*>::ConstIterator it = ObjectList.begin(); it != ObjectList.constEnd(); ++it ) {
		KPlotObject *po = ( *it );
		if ( po->points()->count() ) {
			//draw the plot object
			p->setPen( po->color() );

			switch ( po->type() ) {
				case KPlotObject::POINTS :
				{
					p->setBrush( po->color() );

					for ( QList<QPointF*>::ConstIterator dpit = po->points()->begin(); dpit != po->points()->constEnd(); ++dpit )
					{
						QPoint q = mapToPoint( **dpit );
						int x1 = q.x() - po->size()/2;
						int y1 = q.y() - po->size()/2;

						switch( po->param() ) {
							case KPlotObject::CIRCLE : p->drawEllipse( x1, y1, po->size(), po->size() ); break;
							case KPlotObject::SQUARE : p->drawRect( x1, y1, po->size(), po->size() ); break;
							case KPlotObject::LETTER : p->drawText( q, po->name().left(1) ); break;
							default: p->drawPoint( q );
						}
					}

					p->setBrush( Qt::NoBrush );
					break;
				}

				case KPlotObject::CURVE :
				{
					p->setPen( QPen( po->color(), po->size(), (Qt::PenStyle)po->param() ) );
					QPolygon poly;
					for ( QList<QPointF*>::ConstIterator dpit = po->points()->begin(); dpit != po->points()->constEnd(); ++dpit )
						poly << mapToPoint( **dpit );
					p->drawPolyline( poly );
					break;
				}

				case KPlotObject::LABEL : //draw label centered at point in x, and slightly below point in y.
				{
					QPoint q = mapToPoint( *(po->points()->first()) );
					p->drawText( q.x()-20, q.y()+6, 40, 10, Qt::AlignCenter | Qt::TextDontClip, po->name() );
					break;
				}

				case KPlotObject::POLYGON :
				{
					p->setPen( QPen( po->color(), po->size(), (Qt::PenStyle)po->param() ) );
					p->setBrush( po->color() );

					QPolygon a( po->count() );

					unsigned int i=0;
					for ( QList<QPointF*>::ConstIterator dpit = po->points()->begin(); dpit != po->points()->constEnd(); ++dpit )
						a.setPoint( i++, mapToPoint( **dpit ) );

					p->drawPolygon( a );
					break;
				}

				case KPlotObject::UNKNOWN_TYPE :
				default:
					kdDebug() << "Unknown object type: " << po->type() << endl;
			}
		}
	}
}

double KPlotWidget::dmod( double a, double b ) { return ( b * ( ( a / b ) - int( a / b ) ) ); }

void KPlotWidget::drawBox( QPainter *p ) {
	//First, fill in padding region with bgColor() to mask out-of-bounds plot data
	p->setPen( bgColor() );
	p->setBrush( bgColor() );

	//left padding ( don't forget: we have translated by XPADDING, YPADDING )
	p->drawRect( -leftPadding(), -topPadding(), leftPadding(), height() );

	//right padding
	p->drawRect( PixRect.width(), -topPadding(), rightPadding(), height() );

	//top padding
	p->drawRect( 0, -topPadding(), PixRect.width(), topPadding() );

	//bottom padding
	p->drawRect( 0, PixRect.height(), PixRect.width(), bottomPadding() );

	if ( ShowGrid ) {
		//Grid lines are placed at locations of primary axes' major tickmarks
		p->setPen( gridColor() );

		//vertical grid lines
		double x0 = x() - dmod( x(), dXtick ); //zeropoint; x(i) is this plus i*dXtick1
		for ( int ix = 0; ix <= nmajX+1; ix++ ) {
			int px = int( PixRect.width() * ( (x0 + ix*dXtick - x())/dataWidth() ) );
			p->drawLine( px, 0, px, PixRect.height() );
		}

		//horizontal grid lines
		double y0 = y() - dmod( y(), dYtick ); //zeropoint; y(i) is this plus i*mX
		for ( int iy = 0; iy <= nmajY+1; iy++ ) {
			int py = int( PixRect.height() * ( (y0 + iy*dYtick - y())/dataHeight() ) );
			p->drawLine( 0, py, PixRect.width(), py );
		}
	}

	p->setPen( fgColor() );
	p->setBrush( Qt::NoBrush );

	if (BottomAxis.isVisible() || LeftAxis.isVisible())  p->drawRect( PixRect ); //box outline

	if ( ShowTickMarks ) {
		//spacing between minor tickmarks (in data units)
		double dminX = dXtick/nminX;
		double dminY = dYtick/nminY;

		//set small font for tick labels
		QFont f = p->font();
		int s = f.pointSize();
		f.setPointSize( s - 2 );
		p->setFont( f );

		//--- Draw bottom X Axis ---//
		if (BottomAxis.isVisible()) {
			// Draw X tickmarks
			double x0 = x() - dmod( x(), dXtick ); //zeropoint; tickmark i is this plus i*dXtick (in data units)
			if ( x() < 0.0 ) x0 -= dXtick;
		
			for ( int ix = 0; ix <= nmajX+1; ix++ ) {
				//position of tickmark i (in screen units)
				int px = int( PixRect.width() * ( (x0 + ix*dXtick - x() )/dataWidth() ) );
		
				if ( px > 0 && px < PixRect.width() ) {
					p->drawLine( px, PixRect.height() - 2, px, PixRect.height() - BIGTICKSIZE - 2 );
					p->drawLine( px, 0, px, BIGTICKSIZE );
				}
		
				//tick label
				if ( ShowTickLabels ) {
					double lab = x0 + ix*dXtick;
					if ( fabs(lab)/dXtick < 0.00001 ) lab = 0.0; //fix occassional roundoff error with "0.0" label
		
					QString str = QString( "%1" ).arg( lab, BottomAxis.labelFieldWidth(), BottomAxis.labelFmt(), BottomAxis.labelPrec() );
					if ( px > 0 && px < PixRect.width() ) {
						QRect r( px - BIGTICKSIZE, PixRect.height()+BIGTICKSIZE, 2*BIGTICKSIZE, BIGTICKSIZE );
						p->drawText( r, Qt::AlignCenter | Qt::TextDontClip, str );
					}
				}
		
				//draw minor ticks
				for ( int j=0; j < nminX; j++ ) {
					//position of minor tickmark j (in screen units)
					int pmin = int( px + PixRect.width()*j*dminX/dataWidth() );
		
					if ( pmin > 0 && pmin < PixRect.width() ) {
						p->drawLine( pmin, PixRect.height() - 2, pmin, PixRect.height() - SMALLTICKSIZE - 2 );
						p->drawLine( pmin, 0, pmin, SMALLTICKSIZE );
					}
				}
			} // end draw X tickmarks

			// Draw X Axis Label
			if ( ! BottomAxis.label().isEmpty() ) {
				QRect r( 0, PixRect.height() + 2*YPADDING, PixRect.width(), YPADDING );
				p->drawText( r, Qt::AlignCenter, BottomAxis.label() );
			}

		}

		//--- Draw left Y Axis ---//
		if (LeftAxis.isVisible()) {
			// Draw Y tickmarks
			double y0 = y() - dmod( y(), dYtick ); //zeropoint; tickmark i is this plus i*dYtick1 (in data units)
			if ( y() < 0.0 ) y0 -= dYtick;
	
			for ( int iy = 0; iy <= nmajY+1; iy++ ) {
				//position of tickmark i (in screen units)
				int py = PixRect.height() - int( PixRect.height() * ( (y0 + iy*dYtick - y())/dataHeight() ) );
				if ( py > 0 && py < PixRect.height() ) {
					p->drawLine( 0, py, BIGTICKSIZE, py );
					p->drawLine( PixRect.width()-2, py, PixRect.width()-BIGTICKSIZE-2, py );
				}
	
				//tick label
				if ( ShowTickLabels ) {
					double lab = y0 + iy*dYtick;
					if ( fabs(lab)/dYtick < 0.00001 ) lab = 0.0; //fix occassional roundoff error with "0.0" label
	
					QString str = QString( "%1" ).arg( lab, LeftAxis.labelFieldWidth(), LeftAxis.labelFmt(), LeftAxis.labelPrec() );
					if ( py > 0 && py < PixRect.height() ) {
						QRect r( -2*BIGTICKSIZE, py-SMALLTICKSIZE, 2*BIGTICKSIZE, 2*SMALLTICKSIZE );
						p->drawText( r, Qt::AlignCenter | Qt::TextDontClip, str );
					}
				}
	
				//minor ticks
				for ( int j=0; j < nminY; j++ ) {
					//position of minor tickmark j (in screen units)
					int pmin = int( py - PixRect.height()*j*dminY/dataHeight() );
					if ( pmin > 0 && pmin < PixRect.height() ) {
						p->drawLine( 0, pmin, SMALLTICKSIZE, pmin );
						p->drawLine( PixRect.width()-2, pmin, PixRect.width()-SMALLTICKSIZE-2, pmin );
					}
				}
			} // end draw Y tickmarks

			//Draw Y Axis Label.  We need to draw the text sideways.
			if ( ! LeftAxis.label().isEmpty() ) {
				//store current painter translation/rotation state
				p->save();
		
				//translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
				p->translate( -3*XPADDING, PixRect.height() );
				p->rotate( -90.0 );
		
				QRect r( 0, 0, PixRect.height(), XPADDING );
				p->drawText( r, Qt::AlignCenter, LeftAxis.label() ); //draw the label, now that we are sideways
		
				p->restore();  //restore translation/rotation state
			}
		}

	} //end if ( ShowTickMarks )


}

int KPlotWidget::leftPadding() const {
	if ( LeftPadding >= 0 ) return LeftPadding;
	if ( ! LeftAxis.label().isEmpty() && ShowTickLabels ) return 3*XPADDING;
	if ( ! LeftAxis.label().isEmpty() || ShowTickLabels ) return 2*XPADDING;
	return XPADDING;
}

int KPlotWidget::rightPadding() const {
	if ( RightPadding >= 0 ) return RightPadding;
	return XPADDING;
}

int KPlotWidget::topPadding() const {
	if ( TopPadding >= 0 ) return TopPadding;
	return YPADDING;
}

int KPlotWidget::bottomPadding() const {
	if ( BottomPadding >= 0 ) return BottomPadding;
	if ( ! BottomAxis.label().isEmpty() && ShowTickLabels ) return 3*YPADDING;
	if ( ! BottomAxis.label().isEmpty() || ShowTickLabels ) return 2*YPADDING;
	return YPADDING;
}

