/***************************************************************************
                          testplot_widget.cpp  -  description
                             -------------------
    begin                : Thu Oct 26 2006
    copyright            : (C) 2006 by Jason Harris
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
#include <QtGui/QComboBox>
#include <QtGui/QBoxLayout>
#include <QtGui/QPen>
#include <kdebug.h>

#include "kplotwidget.h"
#include "kplotobject.h"
#include "kplotaxis.h"
#include "testplot_widget.h"

TestPlot::TestPlot( QWidget *p ) : KMainWindow( p ), po1(0), po2(0) {
	QWidget *w = new QWidget(this);
	vlay = new QVBoxLayout(w);

	PlotSelector = new QComboBox( w );
	PlotSelector->addItem( "Points plot" );
	PlotSelector->addItem( "Lines plot" );
	PlotSelector->addItem( "Bars plot" );
	PlotSelector->addItem( "Points plot with labels" );
	PlotSelector->addItem( "Points, lines and bars" );
	PlotSelector->addItem( "Points, lines and bars with labels" );

	plot = new KPlotWidget( w );
	plot->setMinimumSize( 400,400 );
	plot->setAntialiasing( true );
	vlay->addWidget( PlotSelector );
	vlay->addWidget( plot );

	setCentralWidget( w );

	connect( PlotSelector, SIGNAL( activated( int ) ), this, SLOT( slotSelectPlot( int ) ) );

	slotSelectPlot( PlotSelector->currentIndex() );
}

void TestPlot::slotSelectPlot( int n ) {
	plot->resetPlot();

	switch ( n ) {
	case 0: //Points plot
		{
			plot->setLimits( -6.0, 11.0, -10.0, 110.0 );
			
			po1 = new KPlotObject( Qt::white, KPlotObject::Points, 4, KPlotObject::Asterisk );
			po2 = new KPlotObject( Qt::green, KPlotObject::Points, 4, KPlotObject::Triangle );

			for ( float x=-5.0; x<=10.0; x+=1.0 ) {
				po1->addPoint( x, x*x );
				po2->addPoint( x, 50.0 - 5.0*x );
			}
			
			plot->addPlotObject( po1 );
			plot->addPlotObject( po2 );
			
			plot->update();
			break;
		}

	case 1: //Lines plot
		{
			plot->setLimits( -0.1, 6.38, -1.1, 1.1 );
			plot->setSecondaryLimits( -5.73, 365.55, -1.1, 1.1 );
			plot->axis(KPlotWidget::TopAxis)->setTickLabelsShown( true );
			plot->axis(KPlotWidget::BottomAxis)->setLabel("Angle [radians]");
			plot->axis(KPlotWidget::TopAxis)->setLabel("Angle [degrees]");

			po1 = new KPlotObject( Qt::red,  KPlotObject::Lines, 2 );
			po2 = new KPlotObject( Qt::cyan, KPlotObject::Lines, 2 );

			for ( float t=0.0; t<=6.28; t+=0.04 ) {
				po1->addPoint( t, sin(t) );
				po2->addPoint( t, cos(t) );
			}

			plot->addPlotObject( po1 );
			plot->addPlotObject( po2 );

			plot->update();
			break;
		}

	case 2: //Bars plot
		{
			plot->setLimits( -7.0, 7.0, -5.0, 105.0 );

			po1 = new KPlotObject( Qt::white, KPlotObject::Bars, 2 );
			po1->setBarBrush( QBrush(Qt::green, Qt::Dense4Pattern) );

			for ( float x=-6.5; x<=6.5; x+=0.5 ) {
				po1->addPoint( x, 100*exp( -0.5*x*x ), "", 0.5 );
			}

			plot->addPlotObject( po1 );

			plot->update();
			break;
		}

	case 3: //Points plot with labels
		{
			plot->setLimits( -1.1, 1.1, -1.1, 1.1 );

			po1 = new KPlotObject( Qt::yellow, KPlotObject::Points, 10, KPlotObject::Star );
			po1->setLabelPen( QPen(Qt::green) );

			po1->addPoint(  0.0,  0.8, "North" );
			po1->addPoint(  0.57,  0.57, "Northeast" );
			po1->addPoint(  0.8,  0.0, "East" );
			po1->addPoint(  0.57, -0.57, "Southeast" );
			po1->addPoint(  0.0, -0.8, "South" );
			po1->addPoint( -0.57, -0.57, "Southwest" );
			po1->addPoint( -0.8,  0.0, "West" );
			po1->addPoint( -0.57,  0.57, "Northwest" );

			plot->addPlotObject( po1 );

			plot->update();
			break;
		}

	case 4: //Points, Lines and Bars plot
		{
			plot->setLimits( -2.1, 2.1, -0.1, 4.1 );

			po1 = new KPlotObject( Qt::white, KPlotObject::Points, 10, KPlotObject::Pentagon );

			po1->setShowLines( true );
			po1->setShowBars( true );
			po1->setLabelPen( QPen( QColor( "#AA8800" ) ) );
			po1->setLinePen( QPen( Qt::red, 3.0, Qt::DashDotLine ) );
			po1->setBarBrush( QBrush( Qt::blue, Qt::BDiagPattern ) );

			po1->addPoint( -1.75, 0.5  );
			po1->addPoint( -1.25, 1.0  );
			po1->addPoint( -0.75, 1.25 );
			po1->addPoint( -0.25, 1.5  );
			po1->addPoint(  0.25, 2.5  );
			po1->addPoint(  0.75, 3.0  );
			po1->addPoint(  1.25, 1.5  );
			po1->addPoint(  1.75, 1.75 );

			plot->addPlotObject( po1 );

			update();
			break;
		}

	case 5: //Points, Lines and Bars plot with labels
		{
			plot->setLimits( -2.1, 2.1, -0.1, 4.1 );

			po1 = new KPlotObject( Qt::white, KPlotObject::Points, 10, KPlotObject::Pentagon );

			po1->setShowLines( true );
			po1->setShowBars( true );
			po1->setLabelPen( QPen( QColor( "#AA8800" ) ) );
			po1->setLinePen( QPen( Qt::red, 3.0, Qt::DashDotLine ) );
			po1->setBarBrush( QBrush( Qt::blue, Qt::BDiagPattern ) );

			po1->addPoint( -1.75, 0.5, "A" );
			po1->addPoint( -1.25, 1.0, "B" );
			po1->addPoint( -0.75, 1.25, "C" );
			po1->addPoint( -0.25, 1.5, "D" );
			po1->addPoint(  0.25, 2.5, "E" );
			po1->addPoint(  0.75, 3.0, "F" );
			po1->addPoint(  1.25, 1.5, "G" );
			po1->addPoint(  1.75, 1.75, "H" );

			plot->addPlotObject( po1 );

			update();
			break;
		}
	}
}

#include "testplot_widget.moc"
