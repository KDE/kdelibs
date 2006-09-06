/***************************************************************************
                          kplotaxis.cpp - An axis for the plot widget
                             -------------------
    begin                : 16 June 2005
    copyright            : (C) 2005 by Andreas Nicolai
    email                : Andreas.Nicolai@gmx.net
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

#include "kplotaxis.h"

KPlotAxis::KPlotAxis() : m_visible(true), m_showTickLabels(false), m_label(QString()),
	m_labelFieldWidth(0), m_labelFmt('g'), m_labelPrec(-1)
{
}

KPlotAxis::KPlotAxis(const QString& label) : m_visible(true), m_label(label), 
	m_labelFieldWidth(0), m_labelFmt('g'), m_labelPrec(-1)
{
}

void KPlotAxis::setTickMarks( double x0, double length ) {
	m_MajorTickMarks.clear();
	m_MinorTickMarks.clear();

	//s is the power-of-ten factor of length:
	//length = t * s; s = 10^(pwr).  e.g., length=350.0 then t=3.5, s = 100.0; pwr = 2.0
	double pwr = 0.0;
	modf( log10( length ), &pwr );
	double s = pow( 10.0, pwr );
	double t = length / s;

	double TickDistance = 0.0; //The distance between major tickmarks
	int NumMajorTicks = 0; //will be between 3 and 5
	int NumMinorTicks = 0; //The number of minor ticks between major ticks (will be 4 or 5)

	//adjust s and t such that t is between 3 and 5:
	if ( t < 3.0 ) {
		t *= 10.0;
		s /= 10.0;
		// t is now between 3 and 30
	}

	if ( t < 6.0 ) { //accept current values
		TickDistance = s;
		NumMajorTicks = int( t );
		NumMinorTicks = 5;
	} else if ( t < 10.0 ) { // adjust by a factor of 2
		TickDistance = s * 2.0;
		NumMajorTicks = int( t / 2.0 );
		NumMinorTicks = 4;
	} else if ( t < 20.0 ) { //adjust by a factor of 4
		TickDistance = s * 4.0;
		NumMajorTicks = int( t / 4.0 );
		NumMinorTicks = 4;
	} else { //adjust by a factor of 5
		TickDistance = s * 5.0;
		NumMajorTicks = int( t / 5.0 );
		NumMinorTicks = 5;
	}

	//We have determined the number of tickmarks and their separation
	//Now we determine their positions in the Data space.

	//Tick0 is the position of a "virtual" tickmark; the first major tickmark 
	//position beyond the "minimum" edge of the data range.
	double Tick0 = x0 - fmod( x0, TickDistance );
	if ( x0 < 0.0 ) {
		Tick0 -= TickDistance;
		NumMajorTicks++;
	}

	for ( int i=0; i<NumMajorTicks+1; i++ ) {
		double xmaj = Tick0 + i*TickDistance;
		if ( xmaj >= x0 && xmaj <= x0 + length ) {
			m_MajorTickMarks.append( xmaj );
		}

		for ( int j=1; j<NumMinorTicks; j++ ) {
			double xmin = xmaj + TickDistance*j/NumMinorTicks;
			if ( xmin >= x0 && xmin <= x0 + length ) 
				m_MinorTickMarks.append( xmin );
		}
	}
}

QString KPlotAxis::tickLabel( double val ) const {
	if ( tickLabelFmt() == 't' ) {
		while ( val <   0.0 ) val += 24.0;
		while ( val >= 24.0 ) val -= 24.0;

		int h = int(val);
		int m = int( 60.*(val - h) );
		return QString( "%1:%2" ).arg( h, 2, 10, QLatin1Char('0') ).arg( m, 2, 10, QLatin1Char('0') );
	}

	return QString( "%1" ).arg( val, tickLabelWidth(), tickLabelFmt(), tickLabelPrec() );
}
