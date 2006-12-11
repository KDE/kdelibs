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

#include <math.h>
#include <kdebug.h>

#include <qevent.h>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QToolTip>
#include <QtAlgorithms>
#include <klocale.h>

#include "kplotwidget.h"
#include "kplotwidget.moc"

#include "kplotaxis.h"
#include "kplotobject.h"

#define XPADDING 20
#define YPADDING 20
#define BIGTICKSIZE 10
#define SMALLTICKSIZE 4
#define TICKOFFSET 0

KPlotWidget::KPlotWidget( QWidget *parent, double x1, double x2, double y1, double y2 )
 : QFrame( parent ), ShowGrid( false ), ShowObjectToolTips( true ), UseAntialias( false )
{
	setAttribute( Qt::WA_NoBackground, true );

	// create the axes
	mAxes[LeftAxis] = new KPlotAxis();
	mAxes[BottomAxis] = new KPlotAxis();
	mAxes[RightAxis] = new KPlotAxis();
	mAxes[TopAxis] = new KPlotAxis();

	//set DataRect
	setLimits( x1, x2, y1, y2 );
	SecondDataRect = QRect(); //default: no secondary data rect

	//By default, the left and bottom axes have tickmark labels
	axis(LeftAxis)->setShowTickLabels( true );
	axis(BottomAxis)->setShowTickLabels( true );

	setDefaultPaddings();

	//default colors:
	setBackgroundColor( Qt::black );
	setForegroundColor( Qt::white );
	setGridColor( Qt::gray );

	setMinimumSize( 150, 150 );
}

KPlotWidget::~KPlotWidget()
{
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

	if ( XA2 == XA1 ) {
		kWarning() << k_funcinfo << ": " <<
			i18n("x1 and x2 cannot be equal.  Setting x2 = x1 + 1.0") << endl;
		XA2 = XA1 + 1.0;
	}
	if ( YA2 == YA1 ) {
		kWarning() << k_funcinfo << ": " << 
			i18n("y1 and y2 cannot be equal.  Setting y2 = y1 + 1.0") << endl;
		YA2 = YA1 + 1.0;
	}
	DataRect = QRectF( XA1, YA1, XA2-XA1, YA2-YA1 );

	axis(LeftAxis)->setTickMarks( y(), dataHeight() );
	axis(BottomAxis)->setTickMarks( x(), dataWidth() );

	if ( secondaryDataRect().isNull() ) {
		axis(RightAxis)->setTickMarks( y(), dataHeight() );
		axis(TopAxis)->setTickMarks( x(), dataWidth() );
	}

	update();
}

void KPlotWidget::setSecondaryLimits( double x1, double x2, double y1, double y2 ) {
	double XA1, XA2, YA1, YA2;
	if (x2<x1) { XA1=x2; XA2=x1; }
	else { XA1=x1; XA2=x2; }
	if ( y2<y1) { YA1=y2; YA2=y1; }
	else { YA1=y1; YA2=y2; }

	if ( XA2 == XA1 ) {
		kWarning() << k_funcinfo << ": " <<
			i18n("x1 and x2 cannot be equal.  Setting x2 = x1 + 1.0") << endl;
		XA2 = XA1 + 1.0;
	}
	if ( YA2 == YA1 ) {
		kWarning() << k_funcinfo << ": " << 
			i18n("y1 and y2 cannot be equal.  Setting y2 = y1 + 1.0") << endl;
		YA2 = YA1 + 1.0;
	}
	SecondDataRect = QRectF( XA1, YA1, XA2-XA1, YA2-YA1 );

	axis(RightAxis)->setTickMarks( SecondDataRect.y(), SecondDataRect.height() );
	axis(TopAxis)->setTickMarks( SecondDataRect.x(), SecondDataRect.width() );

	update();
}

void KPlotWidget::clearSecondaryLimits() {
	SecondDataRect = QRectF();
	axis(RightAxis)->setTickMarks( DataRect.y(), DataRect.height() );
	axis(TopAxis)->setTickMarks( DataRect.x(), DataRect.width() );

	update();
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

void KPlotWidget::resetPlotMask() {
	for (int ix=0; ix<100; ++ix ) 
		for ( int iy=0; iy<100; ++iy ) 
			PlotMask[ix][iy] = 0.0;
}
		
void KPlotWidget::resetPlot() {
	clearObjectList();
	clearSecondaryLimits();
	setLimits(0.0, 1.0, 0.0, 1.0);
	axis(KPlotWidget::RightAxis)->setShowTickLabels( false );
	axis(KPlotWidget::TopAxis)->setShowTickLabels( false );
	axis(KPlotWidget::LeftAxis)->setLabel( QString() );
	axis(KPlotWidget::BottomAxis)->setLabel( QString() );
	axis(KPlotWidget::RightAxis)->setLabel( QString() );
	axis(KPlotWidget::TopAxis)->setLabel( QString() );
	resetPlotMask();
}

void KPlotWidget::replaceObject( int i, KPlotObject *o ) {
	// skip null pointers
	if ( !o ) return;
	ObjectList.replace( i, o );
	update();
}


KPlotObject *KPlotWidget::object( int i ) {
	if ( i < 0 || i >= ObjectList.count() ) {
		kWarning() << "KPlotWidget::object(): index " << i << " out of range!" << endl;
		return 0;
	}
	return ObjectList.at(i);
}

void KPlotWidget::setBackgroundColor( const QColor &bg ) {
	cBackground = bg;
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

QList<KPlotPoint*> KPlotWidget::pointsUnderPoint( const QPoint& p ) const {
	QList<KPlotPoint*> pts;
	foreach ( KPlotObject *po, ObjectList ) {
		foreach ( KPlotPoint *pp, po->points() ) {
			if ( ( p - toScreen( pp->position() ).toPoint() ).manhattanLength() <= 4 )
				pts << pp;
		}
	}

	return pts;
}


bool KPlotWidget::event( QEvent* e ) {
	if ( e->type() == QEvent::ToolTip ) {
		if ( ShowObjectToolTips )
		{
			QHelpEvent *he = static_cast<QHelpEvent*>( e );
			QList<KPlotPoint*> pts = pointsUnderPoint( he->pos() - QPoint( leftPadding(), topPadding() ) - contentsRect().topLeft() );
			if ( pts.count() > 0 ) {
				QToolTip::showText( he->globalPos(), pts.front()->label(), this );
			}
		}
		e->accept();
		return true;
	}
	else
		return QWidget::event( e );
}

void KPlotWidget::resizeEvent( QResizeEvent* /* e */ ) {
	update();
}

void KPlotWidget::setPixRect() {
	int newWidth = contentsRect().width() - leftPadding() - rightPadding();
	int newHeight = contentsRect().height() - topPadding() - bottomPadding();
	// PixRect starts at (0,0) because we will translate by leftPadding(), topPadding()
	PixRect = QRect( 0, 0, newWidth, newHeight );
	for ( int i=0; i<100; ++i ) {
		px[i] = double(i*PixRect.width())/100.0 + double(PixRect.x());
		py[i] = double(i*PixRect.height())/100.0 + double(PixRect.y());
	}
}

QPointF KPlotWidget::toScreen( const QPointF& p ) const {
	float px = PixRect.left() + PixRect.width()*( p.x() -  DataRect.x() )/DataRect.width();
	float py = PixRect.top() + PixRect.height()*( DataRect.y() + DataRect.height() - p.y() )/DataRect.height();
	return QPointF( px, py );
}

void KPlotWidget::maskRect( const QRectF& r, float value ) {
	//Loop over Mask grid points that are near the target rectangle.
	int ix1 = int( 100.0*(r.x() - PixRect.x())/PixRect.width() );
	int iy1 = int( 100.0*(r.y() - PixRect.y())/PixRect.height() );
	if ( ix1 < 0 ) ix1 = 0;
	if ( iy1 < 0 ) iy1 = 0;
	int ix2 = int( 100.0*(r.right() - PixRect.x())/PixRect.width() ) + 2;
	int iy2 = int( 100.0*(r.bottom() - PixRect.y())/PixRect.height() ) + 2;
	if ( ix1 > 99 ) ix1 = 99;
	if ( iy1 > 99 ) iy1 = 99;

	for ( int ix=ix1; ix<ix2; ++ix ) 
		for ( int iy=iy1; iy<iy2; ++iy ) 
			PlotMask[ix][iy] += value;
}

void KPlotWidget::maskAlongLine( const QPointF &p1, const QPointF &p2, float value ) {
	//Determine slope and zeropoint of line
	double m = (p2.y() - p1.y())/(p2.x() - p1.x());
	double y0 = p1.y() - m*p1.x();
 
	//Make steps along line from p1 to p2, computing the nearest 
	//gridpoint position at each point.
	double x1 = p1.x();
	double x2 = p2.x();
	if ( x1 > x2 ) {
		x1 = p2.x(); 
		x2 = p1.x();
	}
	for ( double x=x1; x<x2; x+=0.01*(x2-x1) ) {
		double y = y0 + m*x;
		int ix = int( 100.0*( x - PixRect.x() )/PixRect.width() );
		int iy = int( 100.0*( y - PixRect.y() )/PixRect.height() );

		if ( ix >= 0 && ix < 100 && iy >= 0 && iy < 100 )
  		PlotMask[ix][iy] += value;

	}
}

void KPlotWidget::placeLabel( QPainter *painter, KPlotPoint *pp ) {
	int textFlags = Qt::TextSingleLine | Qt::AlignCenter;

	float rbest = 100;
	float bestCost = 1.0e7;
	QPointF pos = toScreen( pp->position() );
	QRectF bestRect;
	int ix0 = int( 100.0*( pos.x() - PixRect.x() )/PixRect.width() );
	int iy0 = int( 100.0*( pos.y() - PixRect.y() )/PixRect.height() );

	for ( int ix=ix0-20; ix<ix0+20; ix++ ) {
		for ( int iy=iy0-20; iy<iy0+20; iy++ ) {
			if ( ( ix >= 0 && ix < 100 ) && ( iy >= 0 && iy < 100 ) ) {
				QRectF labelRect = painter->boundingRect( QRectF( px[ix], py[iy], 1, 1 ), textFlags, pp->label() );
				//Add some padding to labelRect
				labelRect.adjust( -2, -2, 2, 2 );

				float r = sqrt( (ix-ix0)*(ix-ix0) + (iy-iy0)*(iy-iy0) );
				float cost = rectCost( labelRect ) + 0.1*r;

				if ( cost < bestCost ) {
					bestRect = labelRect;
					bestCost = cost;
					rbest = r;
				}
			}
		}
	}

	if ( ! bestRect.isNull() ) {
		painter->drawText( bestRect, textFlags, pp->label() );

		//Is a line needed to connect the label to the point?
		if ( rbest > 2.0 ) {
			//Draw a rectangle around the label 
			painter->setBrush( QBrush() );
			//QPen pen = painter->pen();
			//pen.setStyle( Qt::DotLine );
			//painter->setPen( pen );
			painter->drawRoundRect( bestRect );
	
			//Now connect the label to the point with a line.
			//The line is drawn from the center of the near edge of the rectangle
			float xline = bestRect.center().x();
			if ( bestRect.left() > pos.x() )
				xline = bestRect.left();
			if ( bestRect.right() < pos.x() )
				xline = bestRect.right();
	
			float yline = bestRect.center().y();
			if ( bestRect.top() > pos.y() )
				yline = bestRect.top();
			if ( bestRect.bottom() < pos.y() )
				yline = bestRect.bottom();
	
			painter->drawLine( QPointF( xline, yline ), pos );
		}
												
		//Mask the label's rectangle so other labels won't overlap it.
		maskRect( bestRect );
	}
}

float KPlotWidget::rectCost ( const QRectF &r ) {
	int ix1= int( 100.0*( r.x() - PixRect.x() )/PixRect.width() );
	int ix2= int( 100.0*( r.right() - PixRect.x() )/PixRect.width() );
	int iy1= int( 100.0*( r.y() - PixRect.y() )/PixRect.height() );
	int iy2= int( 100.0*( r.bottom() - PixRect.y() )/PixRect.height() );
	float cost = 0.0;

	for ( int ix=ix1; ix<ix2; ++ix ) {
		for ( int iy=iy1; iy<iy2; ++iy ) {
			if ( ix >= 0 && ix < 100 && iy >= 0 && iy < 100 ) {
				cost += PlotMask[ix][iy];
			} else {
				cost += 100.;
			}
		}
	}

	return cost;
}

void KPlotWidget::paintEvent( QPaintEvent *e ) {
	// let QFrame draw its default stuff (like the frame)
	QFrame::paintEvent( e );
	QPainter p;

	p.begin( this );
	p.setRenderHint( QPainter::Antialiasing, UseAntialias );
	p.fillRect( rect(), backgroundColor() );
	p.translate( leftPadding(), topPadding() );

	setPixRect();
	p.setClipRect( PixRect );
	p.setClipping( true );

	resetPlotMask();

	foreach( KPlotObject *po, ObjectList ) 
		po->draw( &p, this );

	//DEBUG_MASK
	/*
	p.setPen( Qt::magenta );
	p.setBrush( Qt::magenta );
	for ( int ix=0; ix<100; ++ix ) {
		for ( int iy=0; iy<100; ++iy ) {
			if ( PlotMask[ix][iy] > 0.0 ) {
				double x = PixRect.x() + double(ix*PixRect.width())/100.;
				double y = PixRect.y() + double(iy*PixRect.height())/100.;

				p.drawRect( QRectF(x-1, y-1, 2, 2 ) );
			}
		}
	}
  */

	p.setClipping( false );
	drawAxes( &p );

	p.end();
}

void KPlotWidget::drawAxes( QPainter *p ) {
	if ( ShowGrid ) {
		p->setPen( gridColor() );

		//Grid lines are placed at locations of primary axes' major tickmarks
		//vertical grid lines
		foreach ( double xx, axis(BottomAxis)->majorTickMarks() ) {
			double px = PixRect.width() * (xx - x()) / dataWidth();
			p->drawLine( QPointF( px, 0.0 ), QPointF( px, double(PixRect.height()) ) );
		}
		//horizontal grid lines
		foreach( double yy, axis(LeftAxis)->majorTickMarks() ) {
			double py = PixRect.height() * (yy - y()) / dataHeight();
			p->drawLine( QPointF( 0.0, py ), QPointF( double(PixRect.width()), py ) );
		}
	}

	p->setPen( foregroundColor() );
	p->setBrush( Qt::NoBrush );

	//set small font for tick labels
	QFont f = p->font();
	int s = f.pointSize();
	f.setPointSize( s - 2 );
	p->setFont( f );

	/*** BottomAxis ***/
	KPlotAxis *a = axis(BottomAxis);
	if (a->isVisible()) {
		//Draw axis line
		p->drawLine( 0, PixRect.height(), PixRect.width(), PixRect.height() );

		// Draw major tickmarks
		foreach( double xx, a->majorTickMarks() ) {
			double px = PixRect.width() * (xx - x()) / dataWidth();
			if ( px > 0 && px < PixRect.width() ) {
				p->drawLine( QPointF( px, double(PixRect.height() - TICKOFFSET)), 
						QPointF( px, double(PixRect.height() - BIGTICKSIZE - TICKOFFSET)) );

				//Draw ticklabel
				if ( a->showTickLabels() ) {
					QRect r( int(px) - BIGTICKSIZE, PixRect.height()+BIGTICKSIZE, 2*BIGTICKSIZE, BIGTICKSIZE );
					p->drawText( r, Qt::AlignCenter | Qt::TextDontClip, a->tickLabel( xx ) );
				}
			}
		}

		// Draw minor tickmarks
		foreach ( double xx, a->minorTickMarks() ) {
			double px = PixRect.width() * (xx - x()) / dataWidth();
			if ( px > 0 && px < PixRect.width() ) {
				p->drawLine( QPointF( px, double(PixRect.height() - TICKOFFSET)), 
						QPointF( px, double(PixRect.height() - SMALLTICKSIZE -TICKOFFSET)) );
			}
		}

		// Draw BottomAxis Label
		if ( ! a->label().isEmpty() ) {
			QRect r( 0, PixRect.height() + 2*YPADDING, PixRect.width(), YPADDING );
			p->drawText( r, Qt::AlignCenter, a->label() );
		}
	}  //End of BottomAxis

	/*** LeftAxis ***/
	a = axis(LeftAxis);
	if (a->isVisible()) {
		//Draw axis line
		p->drawLine( 0, 0, 0, PixRect.height() );

		// Draw major tickmarks
		foreach( double yy, a->majorTickMarks() ) {
			double py = PixRect.height() * ( 1.0 - (yy - y()) / dataHeight() );
			if ( py > 0 && py < PixRect.height() ) {
				p->drawLine( QPointF( TICKOFFSET, py ), QPointF( double(TICKOFFSET + BIGTICKSIZE), py ) );

				//Draw ticklabel
				if ( a->showTickLabels() ) {
					QRect r( -2*BIGTICKSIZE-SMALLTICKSIZE, int(py)-SMALLTICKSIZE, 2*BIGTICKSIZE, 2*SMALLTICKSIZE );
					p->drawText( r, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip, a->tickLabel( yy ) );
				}
			}
		}

		// Draw minor tickmarks
		foreach ( double yy, a->minorTickMarks() ) {
			double py = PixRect.height() * ( 1.0 - (yy - y()) / dataHeight() );
			if ( py > 0 && py < PixRect.height() ) {
				p->drawLine( QPointF( TICKOFFSET, py ), QPointF( double(TICKOFFSET + SMALLTICKSIZE), py ) );
			}
		}

		//Draw LeftAxis Label.  We need to draw the text sideways.
		if ( ! a->label().isEmpty() ) {
			//store current painter translation/rotation state
			p->save();
	
			//translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
			p->translate( -3*XPADDING, PixRect.height() );
			p->rotate( -90.0 );
	
			QRect r( 0, 0, PixRect.height(), XPADDING );
			p->drawText( r, Qt::AlignCenter, a->label() ); //draw the label, now that we are sideways
	
			p->restore();  //restore translation/rotation state
		}
	}  //End of LeftAxis

	//Prepare for top and right axes; we may need the secondary data rect
	double x0 = x();
	double y0 = y();
	double dw = dataWidth();
	double dh = dataHeight();
	if ( secondaryDataRect().isValid() ) {
		x0 = secondaryDataRect().x();
		y0 = secondaryDataRect().y();
		dw = secondaryDataRect().width();
		dh = secondaryDataRect().height();
	}

	/*** TopAxis ***/
	a = axis(TopAxis);
	if (a->isVisible()) {
		//Draw axis line
		p->drawLine( 0, 0, PixRect.width(), 0 );

		// Draw major tickmarks
		foreach( double xx, a->majorTickMarks() ) {
			double px = PixRect.width() * (xx - x0) / dw;
			if ( px > 0 && px < PixRect.width() ) {
				p->drawLine( QPointF( px, TICKOFFSET ), QPointF( px, double(BIGTICKSIZE + TICKOFFSET)) );

				//Draw ticklabel
				if ( a->showTickLabels() ) {
					QRect r( int(px) - BIGTICKSIZE, (int)-1.5*BIGTICKSIZE, 2*BIGTICKSIZE, BIGTICKSIZE );
					p->drawText( r, Qt::AlignCenter | Qt::TextDontClip, a->tickLabel( xx ) );
				}
			}
		}

		// Draw minor tickmarks
		foreach ( double xx, a->minorTickMarks() ) {
			double px = PixRect.width() * (xx - x0) / dw;
			if ( px > 0 && px < PixRect.width() ) {
				p->drawLine( QPointF( px, TICKOFFSET ), QPointF( px, double(SMALLTICKSIZE + TICKOFFSET)) );
			}
		}

		// Draw TopAxis Label
		if ( ! a->label().isEmpty() ) {
			QRect r( 0, 0 - 3*YPADDING, PixRect.width(), YPADDING );
			p->drawText( r, Qt::AlignCenter, a->label() );
		}
	}  //End of TopAxis

	/*** RightAxis ***/
	a = axis(RightAxis);
	if (a->isVisible()) {
		//Draw axis line
		p->drawLine( PixRect.width(), 0, PixRect.width(), PixRect.height() );

		// Draw major tickmarks
		foreach( double yy, a->majorTickMarks() ) {
			double py = PixRect.height() * ( 1.0 - (yy - y0) / dh );
			if ( py > 0 && py < PixRect.height() ) {
				p->drawLine( QPointF( double(PixRect.width() - TICKOFFSET), py ), 
						QPointF( double(PixRect.width() - TICKOFFSET - BIGTICKSIZE), py ) );

				//Draw ticklabel
				if ( a->showTickLabels() ) {
					QRect r( PixRect.width() + SMALLTICKSIZE, int(py)-SMALLTICKSIZE, 2*BIGTICKSIZE, 2*SMALLTICKSIZE );
					p->drawText( r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, a->tickLabel( yy ) );
				}
			}
		}

		// Draw minor tickmarks
		foreach ( double yy, a->minorTickMarks() ) {
			double py = PixRect.height() * ( 1.0 - (yy - y0) / dh );
			if ( py > 0 && py < PixRect.height() ) {
				p->drawLine( QPointF( double(PixRect.width() - 0.0), py ), 
						QPointF( double(PixRect.width() - 0.0 - SMALLTICKSIZE), py ) );
			}
		}

		//Draw RightAxis Label.  We need to draw the text sideways.
		if ( ! a->label().isEmpty() ) {
			//store current painter translation/rotation state
			p->save();
	
			//translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
			p->translate( PixRect.width() + 2*XPADDING, PixRect.height() );
			p->rotate( -90.0 );
	
			QRect r( 0, 0, PixRect.height(), XPADDING );
			p->drawText( r, Qt::AlignCenter, a->label() ); //draw the label, now that we are sideways
	
			p->restore();  //restore translation/rotation state
		}
	}  //End of RightAxis
}

int KPlotWidget::leftPadding() {
	if ( LeftPadding >= 0 ) return LeftPadding;
	if ( axis(LeftAxis)->isVisible() && axis(LeftAxis)->showTickLabels() ) {
		if ( ! axis(LeftAxis)->label().isEmpty() ) return 3*XPADDING;
		else return 2*XPADDING;
	}
	return XPADDING;
}

int KPlotWidget::rightPadding() {
	if ( RightPadding >= 0 ) return RightPadding;
	if ( axis(RightAxis)->isVisible() && axis(RightAxis)->showTickLabels() ) {
		if ( ! axis(RightAxis)->label().isEmpty() ) return 3*XPADDING;
		else return 2*XPADDING;
	}
	return XPADDING;
}

int KPlotWidget::topPadding() {
	if ( TopPadding >= 0 ) return TopPadding;
	if ( axis(TopAxis)->isVisible() && axis(TopAxis)->showTickLabels() ) {
		if ( ! axis(TopAxis)->label().isEmpty() ) return 3*YPADDING;
		else return 2*YPADDING;
	}
	return YPADDING;
}

int KPlotWidget::bottomPadding() {
	if ( BottomPadding >= 0 ) return BottomPadding;
	if ( axis(BottomAxis)->isVisible() && axis(BottomAxis)->showTickLabels() ) {
		if ( ! axis(BottomAxis)->label().isEmpty() ) return 3*YPADDING;
		else return 2*YPADDING;
	}
	return YPADDING;
}
