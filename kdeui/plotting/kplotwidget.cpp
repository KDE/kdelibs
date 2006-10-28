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

//Different architectures seem to draw the tickmarks differently.
#define TICKOFFSET 2
#if defined DARWIN
#define TICKOFFSET 0
#endif

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
}

QPointF KPlotWidget::toScreen( const QPointF& p ) const {
	float px = PixRect.left() + PixRect.width()*( p.x() -  DataRect.x() )/DataRect.width();
	float py = PixRect.top() + PixRect.height()*( DataRect.y() + DataRect.height() - p.y() )/DataRect.height();
	return QPointF( px, py );
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

	foreach( KPlotObject *po, ObjectList ) 
		po->draw( &p, this );

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

	/*** TopAxis ***/
	a = axis(TopAxis);
	if (a->isVisible()) {
		//Draw axis line
		p->drawLine( 0, 0, PixRect.width(), 0 );

		// Draw major tickmarks
		foreach( double xx, a->majorTickMarks() ) {
			double px = PixRect.width() * (xx - x()) / dataWidth();
			if ( px > 0 && px < PixRect.width() ) {
				p->drawLine( QPointF( px, TICKOFFSET ), QPointF( px, double(BIGTICKSIZE + TICKOFFSET)) );

				//Draw ticklabel
				if ( a->showTickLabels() ) {
					QRect r( int(px) - BIGTICKSIZE, -1*BIGTICKSIZE, 2*BIGTICKSIZE, BIGTICKSIZE );
					p->drawText( r, Qt::AlignCenter | Qt::TextDontClip, a->tickLabel( xx ) );
				}
			}
		}

		// Draw minor tickmarks
		foreach ( double xx, a->minorTickMarks() ) {
			double px = PixRect.width() * (xx - x()) / dataWidth();
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
			double py = PixRect.height() * ( 1.0 - (yy - y()) / dataHeight() );
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
			double py = PixRect.height() * ( 1.0 - (yy - y()) / dataHeight() );
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
