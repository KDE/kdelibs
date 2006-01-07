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
#include <qpalette.h>
#include <qpixmap.h>
#include <qpolygon.h>
#include <QtAlgorithms>
#include <qtooltip.h>

#include "kplotwidget.h"
#include "kplotwidget.moc"

#include "kplotaxis.h"
#include "kplotobject.h"

#define BIGTICKSIZE 10
#define SMALLTICKSIZE 4
#define XPADDING 20
#define YPADDING 20

KPlotWidget::KPlotWidget( double x1, double x2, double y1, double y2, QWidget *parent )
 : QFrame( parent ),
   dXtick(0.0), dYtick(0.0),
   nmajX(0), nminX(0), nmajY(0), nminY(0),
   ShowTickMarks( true ), ShowTickLabels( true ), ShowGrid( false ), ShowObjectToolTips( true )
{
	setAttribute( Qt::WA_NoBackground, true );

	// creating the axes
	mAxes[LeftAxis] = new KPlotAxis();
	mAxes[BottomAxis] = new KPlotAxis();

	//set DataRect
	setLimits( x1, x2, y1, y2 );
	setDefaultPaddings();

	//Set PixRect
	recalcPixRect();

	buffer = new QPixmap();

	//default colors:
	setBackgroundColor( Qt::black );
	setForegroundColor( Qt::white );
	setGridColor( Qt::gray );

	setMinimumSize( 150, 150 );
}

KPlotWidget::~KPlotWidget()
{
	delete (buffer);
	qDeleteAll( ObjectList );
	ObjectList.clear();
	qDeleteAll( mAxes );
	mAxes.clear();
}

QSize KPlotWidget::minimumSizeHint() const
{
	return QSize( 150, 150 );
}

void KPlotWidget::setLimits( double x1, double x2, double y1, double y2 ) {
	double XA1, XA2, YA1, YA2;
	if (x2<x1) { XA1=x2; XA2=x1; }
	else { XA1=x1; XA2=x2; }
	if ( y2<y1) { YA1=y2; YA2=y1; }
	else { YA1=y1; YA2=y2; }

	DataRect = QRectF( XA1, YA1, XA2-XA1, YA2-YA1 );
	updateTickmarks();
	update();
}

void KPlotWidget::updateTickmarks() {
	// Determine the number and spacing of tickmarks for the current plot limits.
	if ( dataWidth() == 0.0 ) {
		kdWarning() << "KPlotWidget::updateTickmarks(): X range [" << x() << ", " << x2() << "] invalid!" << endl;
		DataRect.setWidth( 1.0 );
		return;
	}
	if ( dataHeight() == 0.0 ) {
		kdWarning() << "KPlotWidget::updateTickmarks(): Y range [" << y() << ", " << y2() << "] invalid!" << endl;
		DataRect.setHeight( 1.0 );
		return;
	}

	calcTickMarks( DataRect.width(), dXtick, nmajX, nminX );
	calcTickMarks( DataRect.height(), dYtick, nmajY, nminY );
}

void KPlotWidget::calcTickMarks( double length, double& dTick, int& nmajor, int& nminor ) {
	//s is the power-of-ten factor of length:
	//length = t * s; s = 10^(pwr).  e.g., length=350.0 then t=3.5, s = 100.0; pwr = 2.0
	double pwr = 0.0;
	modf( log10( length ), &pwr );
	double s = pow( 10.0, pwr );
	double t = length / s;

	//adjust s and t such that t is between 3 and 5:
	if ( t < 3.0 ) {
		t *= 10.0;
		s /= 10.0;
		// t now between 3 and 30
	}
	double _dTick = 0.0;
	int _nmajor = 0;
	int _nminor = 0;
	if ( t < 6.0 ) { //accept current values
		_dTick = s;
		_nmajor = int( t );
		_nminor = 5;
	} else if ( t < 10.0 ) { //factor of 2
		_dTick = s * 2.0;
		_nmajor = int( t / 2.0 );
		_nminor = 4;
	} else if ( t < 20.0 ) { //factor of 4
		_dTick = s * 4.0;
		_nmajor = int( t / 4.0 );
		_nminor = 4;
	} else { //factor of 5
		_dTick = s * 5.0;
		_nmajor = int( t / 5.0 );
		_nminor = 5;
	}

	nmajor = _nmajor;
	nminor = _nminor;
	dTick = _dTick;
}

void KPlotWidget::addObject( KPlotObject *o ) {
	// skip null pointers
	if ( !o ) return;
	ObjectList.append( o );
	update();
}

void KPlotWidget::clearObjectList() {
	qDeleteAll( ObjectList );
	ObjectList.clear();
	update();
}

void KPlotWidget::replaceObject( int i, KPlotObject *o ) {
	// skip null pointers
	if ( !o ) return;
	ObjectList.replace( i, o );
	update();
}


KPlotObject *KPlotWidget::object( int i ) {
	if ( i < 0 || i >= ObjectList.count() ) {
		kdWarning() << "KPlotWidget::object(): index " << i << " out of range!" << endl;
		return 0;
	}
	return ObjectList.at(i);
}

void KPlotWidget::setBackgroundColor( const QColor &bg ) {
	cBackground = bg;
	update();
}

void KPlotWidget::setShowAxes( bool show ) {
	QHash<Axis, KPlotAxis*>::iterator itEnd = mAxes.end();
	for ( QHash<Axis, KPlotAxis*>::iterator it = mAxes.begin(); it != itEnd; ++it ) {
		(*it)->setVisible(show);
	}
	update();
}


void KPlotWidget::setShowTickMarks( bool show ) {
	ShowTickMarks = show;
	update();
}

void KPlotWidget::setShowTickLabels( bool show ) {
	ShowTickLabels = show;
	recalcPixRect();
	update();
}

void KPlotWidget::setShowGrid( bool show ) {
	ShowGrid = show;
	update();
}

void KPlotWidget::setShowObjectToolTips( bool show ) {
	ShowObjectToolTips = show;
}


KPlotAxis* KPlotWidget::axis( Axis a ) {
	return mAxes.contains( a ) ? mAxes[a] : 0;
}

void KPlotWidget::recalcPixRect() {
	int newWidth = contentsRect().width() - leftPadding() - rightPadding();
	int newHeight = contentsRect().height() - topPadding() - bottomPadding();
	// PixRect starts at (0,0) because we will translate by leftPadding(), topPadding()
	PixRect = QRect( 0, 0, newWidth, newHeight );
}

QList<KPlotObject*> KPlotWidget::pointsUnderPoint( const QPoint& p ) const {
	QList<KPlotObject*> pts;
	for ( QList<KPlotObject*>::ConstIterator it = ObjectList.begin(); it != ObjectList.constEnd(); ++it ) {
		KPlotObject *po = ( *it );
		if ( ( po->count() == 0 ) || ( po->type() != KPlotObject::POINTS ) )
//		if ( ( po->count() == 0 ) || ( po->type() != KPlotObject::POINTS ) || ( po->type() != KPlotObject::POLYGON ) )
			continue;

		for ( QList<QPointF*>::ConstIterator dpit = po->points()->begin(); dpit != po->points()->constEnd(); ++dpit ) {
			if ( ( p - mapToPoint( **dpit ) ).manhattanLength() <= 4 )
				pts << po;
		}
	}
	return pts;
}


bool KPlotWidget::event( QEvent* e ) {
	if ( e->type() == QEvent::ToolTip ) {
		if ( ShowObjectToolTips )
		{
			QHelpEvent *he = static_cast<QHelpEvent*>( e );
			QList<KPlotObject*> pts = pointsUnderPoint( he->pos() - QPoint( leftPadding(), topPadding() ) - contentsRect().topLeft() );
			if ( pts.count() > 0 ) {
				QToolTip::showText( he->globalPos(), pts.front()->name(), this );
			}
		}
		e->accept();
		return true;
	}
	else
		return QWidget::event( e );
}

void KPlotWidget::resizeEvent( QResizeEvent* /* e */ ) {
	recalcPixRect();

	QPixmap *tmp = new QPixmap( contentsRect().size() );
	delete buffer;
	buffer = tmp;
	tmp = 0;
}

void KPlotWidget::paintEvent( QPaintEvent *e ) {
	// let QFrame draw its default stuff (like the frame)
	QFrame::paintEvent( e );
	QPainter p;

	p.begin( buffer );
	p.fillRect( buffer->rect(), backgroundColor() );
	p.translate( leftPadding(), topPadding() );

	p.setClipRect( PixRect );
	p.setClipping( true );
	drawObjects( &p );
	p.setClipping( false );
	drawBox( &p );

	p.end();
	p.begin( this );
	p.drawPixmap( contentsRect().topLeft(), *buffer );
	p.end();
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

					for ( QList<QPointF*>::ConstIterator dpit = po->points()->begin(); dpit != po->points()->constEnd(); ++dpit )
						a << mapToPoint( **dpit );

					p->drawPolygon( a );
					break;
				}

				case KPlotObject::UNKNOWN_TYPE :
				default:
					kdDebug() << "KPlotWidget::drawObjects(): Unknown object type: " << po->type() << endl;
			}
		}
	}
}

// DEPRECATED
double KPlotWidget::dmod( double a, double b ) { return ( b * ( ( a / b ) - int( a / b ) ) ); }

void KPlotWidget::drawBox( QPainter *p ) {
	if ( ShowGrid ) {
		//Grid lines are placed at locations of primary axes' major tickmarks
		p->setPen( gridColor() );

		//vertical grid lines
		double x0 = x() - fmod( x(), dXtick ); //zeropoint; x(i) is this plus i*dXtick1
		for ( int ix = 0; ix <= nmajX; ix++ ) {
			int px = int( PixRect.width() * ( (x0 + ix*dXtick - x())/dataWidth() ) );
			p->drawLine( px, 0, px, PixRect.height() );
		}

		//horizontal grid lines
		double y0 = y() - fmod( y(), dYtick ); //zeropoint; y(i) is this plus i*mX
		for ( int iy = 0; iy <= nmajY; iy++ ) {
			int py = PixRect.height() - int( PixRect.height() * ( (y0 + iy*dYtick - y())/dataHeight() ) );
			p->drawLine( 0, py, PixRect.width(), py );
		}
	}

	p->setPen( foregroundColor() );
	p->setBrush( Qt::NoBrush );

	if (mAxes[BottomAxis]->isVisible() || mAxes[LeftAxis]->isVisible())  p->drawRect( PixRect ); //box outline

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
		if (mAxes[BottomAxis]->isVisible()) {
			// Draw X tickmarks
			double x0 = x() - fmod( x(), dXtick ); //zeropoint; tickmark i is this plus i*dXtick (in data units)
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
		
					QString str = QString( "%1" ).arg( lab, mAxes[BottomAxis]->labelFieldWidth(), mAxes[BottomAxis]->labelFmt(), mAxes[BottomAxis]->labelPrec() );
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
			if ( ! mAxes[BottomAxis]->label().isEmpty() ) {
				QRect r( 0, PixRect.height() + 2*YPADDING, PixRect.width(), YPADDING );
				p->drawText( r, Qt::AlignCenter, mAxes[BottomAxis]->label() );
			}

		}

		//--- Draw left Y Axis ---//
		if (mAxes[LeftAxis]->isVisible()) {
			// Draw Y tickmarks
			double y0 = y() - fmod( y(), dYtick ); //zeropoint; tickmark i is this plus i*dYtick1 (in data units)
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
	
					QString str = QString( "%1" ).arg( lab, mAxes[LeftAxis]->labelFieldWidth(), mAxes[LeftAxis]->labelFmt(), mAxes[LeftAxis]->labelPrec() );
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
			if ( ! mAxes[LeftAxis]->label().isEmpty() ) {
				//store current painter translation/rotation state
				p->save();
		
				//translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
				p->translate( -3*XPADDING, PixRect.height() );
				p->rotate( -90.0 );
		
				QRect r( 0, 0, PixRect.height(), XPADDING );
				p->drawText( r, Qt::AlignCenter, mAxes[LeftAxis]->label() ); //draw the label, now that we are sideways
		
				p->restore();  //restore translation/rotation state
			}
		}

	} //end if ( ShowTickMarks )


}

// DEPRECATED
void KPlotWidget::setXAxisLabel( const QString& xlabel ) {
	mAxes[BottomAxis]->setLabel(xlabel);
}

// DEPRECATED
void KPlotWidget::setYAxisLabel( const QString& ylabel ) {
	mAxes[LeftAxis]->setLabel(ylabel);
}


int KPlotWidget::leftPadding() const {
	if ( LeftPadding >= 0 ) return LeftPadding;
	if ( ! mAxes[LeftAxis]->label().isEmpty() && ShowTickLabels ) return 3*XPADDING;
	if ( ! mAxes[LeftAxis]->label().isEmpty() || ShowTickLabels ) return 2*XPADDING;
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
	if ( ! mAxes[BottomAxis]->label().isEmpty() && ShowTickLabels ) return 3*YPADDING;
	if ( ! mAxes[BottomAxis]->label().isEmpty() || ShowTickLabels ) return 2*YPADDING;
	return YPADDING;
}

