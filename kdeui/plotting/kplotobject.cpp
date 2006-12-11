/***************************************************************************
                          kplotobject.cpp - A list of points to be plotted
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

#include <QtAlgorithms>
#include <QPainter>

#include <kdebug.h>

#include "kplotobject.h"
#include "kplotwidget.h"

KPlotPoint::KPlotPoint()
 : X(0), Y(0), Label(QString()), BarWidth(0.0)
{
}

KPlotPoint::KPlotPoint( double x, double y, const QString &label, double barWidth ) 
	: X( x ), Y( y ), Label( label ), BarWidth( barWidth )
{
}

KPlotPoint::KPlotPoint( const QPointF &p, const QString &label, double barWidth )
	:	X( p.x() ), Y( p.y() ), Label( label ), BarWidth( barWidth )
{
}

KPlotPoint::~KPlotPoint() 
{
}

KPlotObject::KPlotObject() {
	KPlotObject( Qt::white, POINTS );
}

KPlotObject::KPlotObject( const QColor &c, PlotType t, double size, PStyle ps ) {
	//By default, all pens and brushes are set to the given color
	setBrush( c );
	setBarBrush( c );
	setPen( QPen( brush(), 1 ) );
	setLinePen( pen() );
	setBarPen( pen() );
	setLabelPen( pen() );

	Type = t;
	setSize( size );
	setPointStyle( ps );
}

KPlotObject::~KPlotObject()
{
	qDeleteAll( pList );
	pList.clear();
}

void KPlotObject::removePoint( int index ) {
	if ( ( index < 0 ) || ( index >= pList.count() ) ) {
		kWarning() << "KPlotObject::removePoint(): index " << index << " out of range!" << endl;
		return;
	}

	pList.removeAt( index );
}

void KPlotObject::clearPoints() {
	qDeleteAll( pList );
	pList.clear();
}

void KPlotObject::draw( QPainter *painter, KPlotWidget *pw ) {
	//Order of drawing determines z-distance: Bars in the back, then lines, 
	//then points, then labels.

	if ( showBars() ) {
		painter->setPen( barPen() );
		painter->setBrush( barBrush() );

		for ( int i=0; i<pList.size(); ++i ) {
			double w = 0;
			if ( pList[i]->barWidth() == 0.0 ) {
				if ( i<pList.size()-1 ) 
					w = pList[i+1]->x() - pList[i]->x();
				//For the last bin, we'll just keep the previous width

			} else {
				w = pList[i]->barWidth();
			}

			QPointF pp = pList[i]->position();
			QPointF p1( pp.x() - 0.5*w, 0.0 );
			QPointF p2( pp.x() + 0.5*w, pp.y() );
			QPointF sp1 = pw->toScreen( p1 );
			QPointF sp2 = pw->toScreen( p2 );

			QRectF barRect = QRectF( sp1.x(), sp1.y(), sp2.x()-sp1.x(), sp2.y()-sp1.y() ).normalized();
			painter->drawRect( barRect );
			pw->maskRect( barRect, 0.25 );
		}
	}
	
	//Draw lines:
	if ( showLines() ) {
		painter->setPen( linePen() );

		QPointF Previous = QPointF();  //Initialize to null

		foreach ( KPlotPoint *pp, pList ) {
			//q is the position of the point in screen pixel coordinates
			QPointF q = pw->toScreen( pp->position() );

			if ( ! Previous.isNull() ) {
				painter->drawLine( Previous, q );
				pw->maskAlongLine( Previous, q );
			}
			
			Previous = q;
		}
	}

	//Draw points:
	if ( showPoints() ) {

		foreach( KPlotPoint *pp, pList ) {
			//q is the position of the point in screen pixel coordinates
			QPointF q = pw->toScreen( pp->position() );
			if ( pw->pixRect().contains( q.toPoint(), false ) ) {
				double x1 = q.x() - size();
				double y1 = q.y() - size();
				QRectF qr = QRectF( x1, y1, 2*size(), 2*size() );
	
				//Mask out this rect in the plot for label avoidance
				pw->maskRect( qr, 2.0 );
	
				painter->setPen( pen() );
				painter->setBrush( brush() );
	
				switch ( pointStyle() ) {
				case CIRCLE:
					painter->drawEllipse( qr );
					break;
	
				case LETTER:
					painter->drawText( qr, Qt::AlignCenter, pp->label().left(1) );
					break;
	
				case TRIANGLE:
					{
						QPolygonF tri;
						tri << QPointF( q.x() - size(), q.y() + size() ) 
								<< QPointF( q.x(), q.y() - size() ) 
								<< QPointF( q.x() + size(), q.y() + size() );
						painter->drawPolygon( tri );
						break;
					}
	
				case SQUARE:
					painter->drawRect( qr );
					break;
	
				case PENTAGON:
					{
						QPolygonF pent;
						pent << QPointF( q.x(), q.y() - size() ) 
								<< QPointF( q.x() + size(), q.y() - 0.309*size() )
								<< QPointF( q.x() + 0.588*size(), q.y() + size() )
								<< QPointF( q.x() - 0.588*size(), q.y() + size() )
								<< QPointF( q.x() - size(), q.y() - 0.309*size() );
						painter->drawPolygon( pent );
						break;
					}
	
				case HEXAGON:
					{
						QPolygonF hex;
						hex << QPointF( q.x(), q.y() + size() ) 
								<< QPointF( q.x() + size(), q.y() + 0.5*size() )
								<< QPointF( q.x() + size(), q.y() - 0.5*size() )
								<< QPointF( q.x(), q.y() - size() )
								<< QPointF( q.x() - size(), q.y() + 0.5*size() )
								<< QPointF( q.x() - size(), q.y() - 0.5*size() );
						painter->drawPolygon( hex );
						break;
					}
	
				case ASTERISK:
					painter->drawLine( q, QPointF( q.x(), q.y() + size() ) );
					painter->drawLine( q, QPointF( q.x() + size(), q.y() + 0.5*size() ) );
					painter->drawLine( q, QPointF( q.x() + size(), q.y() - 0.5*size() ) );
					painter->drawLine( q, QPointF( q.x(), q.y() - size() ) );
					painter->drawLine( q, QPointF( q.x() - size(), q.y() + 0.5*size() ) );
					painter->drawLine( q, QPointF( q.x() - size(), q.y() - 0.5*size() ) );
					break;
	
				case STAR:
					{
						QPolygonF star;
						star << QPointF( q.x(), q.y() - size() ) 
								<< QPointF( q.x() + 0.2245*size(), q.y() - 0.309*size() )
								<< QPointF( q.x() + size(), q.y() - 0.309*size() )
								<< QPointF( q.x() + 0.363*size(), q.y() + 0.118*size() )
								<< QPointF( q.x() + 0.588*size(), q.y() + size() )
								<< QPointF( q.x(), q.y() + 0.382*size() )
								<< QPointF( q.x() - 0.588*size(), q.y() + size() )
								<< QPointF( q.x() - 0.363*size(), q.y() + 0.118*size() )
								<< QPointF( q.x() - size(), q.y() - 0.309*size() )
								<< QPointF( q.x() - 0.2245*size(), q.y() - 0.309*size() );
						painter->drawPolygon( star );
						break;
					}
	
				default:
					break;
				}
			}
		}
	}

	//Draw labels
	painter->setPen( labelPen() );

	foreach ( KPlotPoint *pp, pList ) {
		QPoint q = pw->toScreen( pp->position() ).toPoint();
		if ( pw->pixRect().contains(q, false) && ! pp->label().isEmpty() ) {
			pw->placeLabel( painter, pp );
		}
	}

}
