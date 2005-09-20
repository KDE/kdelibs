/***************************************************************************
                          kplotwidget.h - A generic data-plotting widget
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

#ifndef _KPLOTWIDGET_H_
#define _KPLOTWIDGET_H_

#include <qwidget.h>
#include <qlist.h>
#include "kplotobject.h"
#include "kplotaxis.h"

#define BIGTICKSIZE 10
#define SMALLTICKSIZE 4
#define XPADDING 20
#define YPADDING 20

class QColor;
class QPixmap;

/**
 * @class KPlotWidget
 *
 * @short Genric data plotting widget.
 *
 * Widget for drawing plots. Includes adjustable axes (KPlotAxis) with
 * tickmarks and labels and a list of KPlotObjects to be drawn.
 *
 * @author Jason Harris
 *
 * @version 1.1
 */

class KDE_EXPORT KPlotWidget : public QWidget {
	Q_OBJECT
public:
	/**
	 * @short Constructor. Sets the primary x and y limits in data units.
	 * @param x1 the minimum X value in data units
	 * @param x2 the maximum X value in data units
	 * @param y1 the minimum Y value in data units
	 * @param y2 the maximum Y value in data units
	 * @param parent the parent widget
	 * @param name name label for the KPlotWidget
	 */
	KPlotWidget( double x1=0.0, double x2=1.0, double y1=0.0, double y2=1.0, QWidget *parent=0 );

	/**
	 * Destructor (empty)
	 */
	virtual ~KPlotWidget();

	/**
	 * @short Determine the placement of major and minor tickmarks,
	 * Based on the current Limit settings
	 */
	virtual void updateTickmarks();

	/**
	 * @short Reset the data limits.
	 * @param x1 the minimum X value in data units
	 * @param x2 the maximum X value in data units
	 * @param y1 the minimum Y value in data units
	 * @param y2 the maximum Y value in data units
	 */
	virtual void setLimits( double x1, double x2, double y1, double y2 );

	/**
	 * @return the minimum X value in data units
	 */
	virtual double x() const { return DataRect.x(); }

	/**
	 * @return the maximum X value in data units
	 */
	virtual double x2() const { return DataRect.x2(); }

	/**
	 * @return the minimum Y value in data units
	 */
	virtual double y() const { return DataRect.y(); }

	/**
	 * @return the maximum Y value in data units
	 */
	virtual double y2() const { return DataRect.y2(); }

	/**
	 * @return the width in data units
	 */
	virtual double dataWidth() const { return DataRect.width(); }

	/**
	 * @return the height in data units
	 */
	virtual double dataHeight() const { return DataRect.height(); }

	/**
	 * Add an item to the list of KPlotObjects to be plotted.
	 * @param o pointer to the KPlotObject to be added
	 */
	virtual void addObject( KPlotObject *o ) { ObjectList.append( o ); }

	/**
	 * Remove all items from the list of KPlotObjects
	 */
	virtual void clearObjectList() { ObjectList.clear(); update(); }

	/**
	 * Replace an item in the KPlotObject list.
	 * @param i the index of th item to be replaced
	 * @param o pointer to the replacement KPlotObject
	 */
	virtual void replaceObject( int i, KPlotObject *o ) { ObjectList.replace( i, o ); }

	/**
	 * @return the number of KPlotObjects in the list
	 */
	virtual int objectCount() const { return ObjectList.count(); }

	/**
	 * @return a pointer to a specific KPlotObject in the list
	 * @param i the index of the desired KPlotObject
	 */
	virtual KPlotObject *object( int i ) { return ObjectList.at(i); }

	/**
	 * @return the background color
	 */
	virtual QColor bgColor() const { return cBackground; }

	/**
	 * @return the foreground color
	 */
	virtual QColor fgColor() const { return cForeground; }

	/**
	 * @return the grid color
	 */
	virtual QColor gridColor() const { return cGrid; }

	/**
	 * Set the background color
	 * @param bg the new background color
	 */
	virtual void setBGColor( const QColor &bg ) { cBackground = bg; setBackgroundColor( bg ); }

	/**
	 * Set the foreground color
	 * @param fg the new foreground color
	 */
	virtual void setFGColor( const QColor &fg ) { cForeground = fg; }

	/**
	 * Set the grid color
	 * @param gc the new grid color
	 */
	virtual void setGridColor( const QColor &gc ) { cGrid = gc; }

	/**
	 * Toggle whether plot axes are drawn.
	 * @param show if true, axes will be drawn.
	 * The axes are just a box outline around the plot.
	 */
	virtual void setShowAxes( bool show ) { BottomAxis.setVisible(show); LeftAxis.setVisible(show); }
	/**
	 * Toggle whether tick marks are drawn along the axes.
	 * @param show if true, tick marks will be drawn.
	 */
	virtual void setShowTickMarks( bool show ) { ShowTickMarks = show; }
	/**
	 * Toggle whether tick labels are drawn at major tickmarks.
	 * @param show if true, tick labels will be drawn.
	 */
	virtual void setShowTickLabels( bool show ) { ShowTickLabels = show; }
	/**
	 * Toggle whether grid lines are drawn at major tickmarks.
	 * @param show if true, grid lines will be drawn.
	 */
	virtual void setShowGrid( bool show ) { ShowGrid = show; }

	/**
	 * Sets the X-axis label.
	 * Set the label to an empty string to omit the axis label.
	 *
	 * This function is deprecated, set the label property in the BottomAxis directly.
	 * @param xlabel a short string describing the data plotted on the x-axis.
	 */
	virtual void setXAxisLabel( const QString& xlabel ) { BottomAxis.setLabel(xlabel); }
	/**
	 * Sets the Y-axis label
	 * Set the label to an empty string to omit the axis label.
	 *
	 * This function is deprecated, set the label property in the LeftAxis directly.
	 * @param ylabel a short string describing the data plotted on the y-axis.
	 */
	virtual void setYAxisLabel( const QString& ylabel ) { LeftAxis.setLabel(ylabel); }

	/**
	 * @returns the number of pixels to the left of the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int leftPadding() const;
	/**
	 * @returns the number of pixels to the right of the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int rightPadding() const;
	/**
	 * @returns the number of pixels above the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int topPadding() const;
	/**
	 * @returns the number of pixels below the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int bottomPadding() const;

	/**
	 * Set the number of pixels to the left of the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
	virtual void setLeftPadding( int pad )   { LeftPadding = pad; }
	/**
	 * Set the number of pixels to the right of the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
	virtual void setRightPadding( int pad )  { RightPadding = pad; }
	/**
	 * Set the number of pixels above the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
	virtual void setTopPadding( int pad )    { TopPadding = pad; }
	/**
	 * Set the number of pixels below the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
	virtual void setBottomPadding( int pad ) { BottomPadding = pad; }

	/**
	 * Revert all four padding values to be automatically determined.
	 */
	virtual void setDefaultPadding() { LeftPadding = -1; RightPadding = -1; TopPadding = -1; BottomPadding = -1; }

	/**
	 * The bottom X axis.
	 */
	KPlotAxis BottomAxis;
	/**
	 * The left Y axis.
	 */
	KPlotAxis LeftAxis;

protected:
	/**
	 * The paint event handler, executed when update() or repaint() is called.
	 */
	virtual void paintEvent( QPaintEvent* );

	/**
	 * The resize event handler, called when the widget is resized.
	 */
	virtual void resizeEvent( QResizeEvent* );

	/**
	 * Draws all of the objects onto the widget.
	 * @internal Internal use only; one should simply call update()
	 * to draw the widget with axes and all objects.
	 * @param p pointer to the painter on which we are drawing
	 */
	virtual void drawObjects( QPainter *p );

	/**
	 * Draws the plot axes and axis labels.
	 * @internal Internal use only; one should simply call update()
	 * to draw the widget with axes and all objects.
	 * @param p pointer to the painter on which we are drawing
	 */
	virtual void drawBox( QPainter *p );

	/**
	 * Modulus function for double variables.
	 * For example:
	 * @code
	 * double m = dmod( 17.0, 7.0 ); // m == 3.0
	 * @endcode
	 * @return the remainder after dividing @p b into @p a.
	 */
	double dmod( double a, double b );

	//The distance between major tickmarks in data units
	double dXtick, dYtick;
	//The number of major and minor tickmarks to be plotted in X and Y
	int nmajX, nminX, nmajY, nminY;

	//Limits of the plot area in pixel units
	QRect PixRect;
	//Limits of the plot area in data units
	DRect DataRect;
	/**
	 * List of KPlotObjects
	 */
	QList<KPlotObject*> ObjectList;

	//Colors
	QColor cBackground, cForeground, cGrid;
	//draw options
	bool ShowTickMarks, ShowTickLabels, ShowGrid;
	//padding
	int LeftPadding, RightPadding, TopPadding, BottomPadding;

	QPixmap *buffer;
};

#endif
