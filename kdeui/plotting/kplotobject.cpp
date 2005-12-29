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

#include "kplotobject.h"

#include <QtAlgorithms>

#include <kdebug.h>

KPlotObject::KPlotObject() {
	KPlotObject( "", Qt::white, POINTS );
}

KPlotObject::KPlotObject( const QString &n, const QColor &c, PTYPE t, unsigned int s, unsigned int p ) {
	//We use the set functions because they may include data validation
	setName( n );
	setColor( c );
	setType( t );
	setSize( s );
	setParam( p );
}

KPlotObject::~KPlotObject()
{
	qDeleteAll( pList );
	pList.clear();
}

QPointF* KPlotObject::point( int index ) {
	if ( index < 0 || index >= pList.count() ) {
		kdWarning() << "KPlotObject::object(): index " << index << " out of range!" << endl;
		return 0;
	}
	return pList.at(index);
}

void KPlotObject::addPoint( QPointF *p ) {
	// skip null pointers
	if ( !p ) return;
	pList.append( p );
}

void KPlotObject::removePoint( int index ) {
	if ( ( index < 0 ) || ( index >= pList.count() ) ) {
		kdWarning() << "KPlotObject::removePoint(): index " << index << " out of range!" << endl;
		return;
	}

	pList.removeAt( index );
}

void KPlotObject::clearPoints() {
	qDeleteAll( pList );
	pList.clear();
}

