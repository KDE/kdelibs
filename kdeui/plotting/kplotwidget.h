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
#include "kplotobject.h"
#include "kplotaxis.h"

#define BIGTICKSIZE 10
#define SMALLTICKSIZE 4
#define XPADDING 20
#define YPADDING 20

class QColor;
class QPixmap;

/**@class KPlotWidget
	*@short Genric data plotting widget.
	*@author Jason Harris
	*@version 1.1
	*Widget for drawing plots.  Includes adjustable axes (KPlotAxis) with
 	*tickmarks and labels and a list of KPlotObjects to be drawn.
	*/

class KDE_EXPORT KPlotWidget : public QWidget {
	Q_OBJECT
public:
	/**@short Constructor.  Sets the primary x and y limits in data units.
		*@param x1 the minimum X value in data units
		*@param x2 the maximum X value in data units
		*@param y1 the minimum Y value in data units
		*@param y2 the maximum Y value in data units
		*@param parent the parent widget
		*@param name name label for the KPlotWidget
		*/
	KPlotWidget( double x1=0.0, double x2=1.0, double y1=0.0, double y2=1.0, QWidget *parent=0, const char* name=0 );

	/**Destructor (empty)
		*/
	virtual ~KPlotWidget();

	/**@short Determine the placement of major and minor tickmarks,
		*based on the current Limit settings
		*/
	virtual void updateTickmarks();

	/**@short Reset the data limits.
		*@param x1 the minimum X value in data units
		*@param x2 the maximum X value in data units
		*@param y1 the minimum Y value in data units
		*@param y2 the maximum Y value in data units
		*/
	virtual void setLimits( double x1, double x2, double y1, double y2 );

	/**@return the minimum X value in data units*/
	virtual double x() const { return DataRect.x(); }

	/**@return the maximum X value in data units*/
	virtual double x2() const { return DataRect.x2(); }

	/**@return the minimum Y value in data units*/
	virtual double y() const { return DataRect.y(); }

	/**@return the maximum Y value in data units*/
	virtual double y2() const { return DataRect.y2(); }

	/**@return the width in data units*/
	virtual double dataWidth() const { return DataRect.width(); }

	/**@return the height in data units*/
	virtual double dataHeight() const { return DataRect.height(); }

	/**@short Add an item to the list of KPlotObjects to be plotted.
		*@param o pointer to the KPlotObject to be added
		*/
	virtual void addObject( KPlotObject *o ) { ObjectList.append( o ); }

	/**@short Remove all items from the list of KPlotObjects
		*/
	virtual void clearObjectList() { ObjectList.clear(); update(); }

	/**@short replace an item in the KPlotObject list.
		*@param i the index of th item to be replaced
		*@param o pointer to the replacement KPlotObject
		*/
	virtual void replaceObject( int i, KPlotObject *o ) { ObjectList.replace( i, o ); }

	/**@return the number of KPlotObjects in the list
		*/
	virtual int objectCount() const { return ObjectList.count(); }

	/**@return a pointer to a specific KPlotObject in the list
	 *@param i the index of the desired KPlotObject
	 */
	virtual KPlotObject *object( int i ) { return ObjectList.at(i); }

	/**@return the background color */
	virtual QColor bgColor() const { return cBackground; }

	/**@return the foreground color */
	virtual QColor fgColor() const { return cForeground; }

	/**@return the grid color */
	virtual QColor gridColor() const { return cGrid; }

	/**@short set the background color
		*@param bg the new background color
		*/
	virtual void setBGColor( const QColor &bg ) { cBackground = bg; setBackgroundColor( bg ); }

	/**@short set the foreground color
		*@param fg the new foreground color
		*/
	virtual void setFGColor( const QColor &fg ) { cForeground = fg; }

	/**@short set the grid color
		*@param gc the new grid color
		*/
	virtual void setGridColor( const QColor &gc ) { cGrid = gc; }

	/**@short toggle whether plot axes are drawn.
		*@param show if true, axes will be drawn.
		*The axes are just a box outline around the plot.
		*/
	virtual void setShowAxes( bool show ) { BottomAxis.setVisible(show); LeftAxis.setVisible(show); }
	/**@short toggle whether tick marks are drawn along the axes.
		*@param show if true, tick marks will be drawn.
		*/
	virtual void setShowTickMarks( bool show ) { ShowTickMarks = show; }
	/**@short toggle whether tick labels are drawn at major tickmarks.
		*@param show if true, tick labels will be drawn.
		*/
	virtual void setShowTickLabels( bool show ) { ShowTickLabels = show; }
	/**@short toggle whether grid lines are drawn at major tickmarks.
		*@param show if true, grid lines will be drawn.
		*/
	virtual void setShowGrid( bool show ) { ShowGrid = show; }

	/**@short (Deprecated) Sets the X-axis label
		*@param xlabel a short string describing the data plotted on the x-axis.
		*Set the label to an empty string to omit the axis label.
        *This function is deprecated, set the label property in the BottomAxis directly.
		*/
	virtual void setXAxisLabel( QString xlabel ) { BottomAxis.setLabel(xlabel); }
	/**@short (Deprecated) Sets the Y-axis label
		*@param ylabel a short string describing the data plotted on the y-axis.
		*Set the label to an empty string to omit the axis label.
        *This function is deprecated, set the label property in the LeftAxis directly.
		*/
	virtual void setYAxisLabel( QString ylabel ) { LeftAxis.setLabel(ylabel); }

	/**@returns the number of pixels to the left of the plot area.  Padding values
		*are set to -1 by default; if unchanged, this function will try to guess
		*a good value, based on whether ticklabels and/or axis labels are to be drawn.
		*/
	virtual int leftPadding()   const;
	/**@returns the number of pixels to the right of the plot area.
		*Padding values are set to -1 by default; if unchanged, this function will try to guess
		*a good value, based on whether ticklabels and/or axis labels are to be drawn.
		*/
	virtual int rightPadding()  const;
	/**@returns the number of pixels above the plot area.
		*Padding values are set to -1 by default; if unchanged, this function will try to guess
		*a good value, based on whether ticklabels and/or axis labels are to be drawn.
		*/
	virtual int topPadding()    const;
	/**@returns the number of pixels below the plot area.
		*Padding values are set to -1 by default; if unchanged, this function will try to guess
		*a good value, based on whether ticklabels and/or axis labels are to be drawn.
		*/
	virtual int bottomPadding() const;

	/**@short set the number of pixels to the left of the plot area.
		*Set this to -1 to revert to automatic determination of padding values.
		*/
	virtual void setLeftPadding( int pad )   { LeftPadding = pad; }
	/**@short set the number of pixels to the right of the plot area.
		*Set this to -1 to revert to automatic determination of padding values.
		*/
	virtual void setRightPadding( int pad )  { RightPadding = pad; }
	/**@short set the number of pixels above the plot area.
		*Set this to -1 to revert to automatic determination of padding values.
		*/
	virtual void setTopPadding( int pad )    { TopPadding = pad; }
	/**@short set the number of pixels below the plot area.
		*Set this to -1 to revert to automatic determination of padding values.
		*/
	virtual void setBottomPadding( int pad ) { BottomPadding = pad; }

	/**@short revert all four padding values to be automatically determined. */
	virtual void setDefaultPadding() { LeftPadding = -1; RightPadding = -1; TopPadding = -1; BottomPadding = -1; }

	/**@short The bottom X axis. */
	KPlotAxis 	BottomAxis;
	/**@short The left Y axis. */
	KPlotAxis 	LeftAxis;

protected:
	/**@short the paint event handler, executed when update() or repaint() is called.
		*/
	virtual void paintEvent( QPaintEvent* /* e */ );

	/**@short the resize event handler, called when the widget is resized.
		*/
	virtual void resizeEvent( QResizeEvent* /* e */ );

	/**@short draws all of the objects onto the widget.  Internal use only; one should simply call update()
		*to draw the widget with axes and all objects.
		*@param p pointer to the painter on which we are drawing
		*/
	virtual void drawObjects( QPainter *p );

	/**@short draws the plot axes and axis labels.  Internal use only; one should simply call update()
		*to draw the widget with axes and all objects.
		*@param p pointer to the painter on which we are drawing
		*/
	virtual void drawBox( QPainter *p );

	/**@short modulus function for double variables.
		*For example, dmod( 17.0, 7.0 ) returns 3.0
		*@return the remainder after dividing b into a.
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
	//List of KPlotObjects
	QPtrList<KPlotObject> ObjectList;

	//Colors
	QColor cBackground, cForeground, cGrid;
	//draw options
	bool ShowTickMarks, ShowTickLabels, ShowGrid;
	//padding
	int LeftPadding, RightPadding, TopPadding, BottomPadding;

	QPixmap *buffer;
};

#endif
