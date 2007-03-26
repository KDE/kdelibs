/***************************************************************************
                          testplot_widget.h  -  description
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

#ifndef TESTPLOT_WIDGET_H
#define TESTPLOT_WIDGET_H

#include <kmainwindow.h>

class QComboBox;
class QVBoxLayout;
class KPlotWidget;
class KPlotObject;

class TestPlot : public KMainWindow {
 Q_OBJECT

 public:
	TestPlot( QWidget *parent=0 );
	~TestPlot() {}

 public slots:
	void slotSelectPlot( int index );

 private:

	QVBoxLayout *vlay;
	QComboBox *PlotSelector;
	KPlotWidget *plot;
	KPlotObject *po1, *po2;
};

#endif
