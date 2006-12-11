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

#ifndef KPLOTWIDGET_H
#define KPLOTWIDGET_H

#include <QFrame>
#include <QHash>
#include <QList>

#include <libkdeedu_plot_export.h>

class QPixmap;
class KPlotAxis;
class KPlotObject;
class KPlotPoint;

/**
 *@class KPlotWidget
 *
 *@short Generic data plotting widget.
 *
 *Widget for drawing plots. The basic idea behind KPlotWidget is that 
 *you don't have to worry about any transformation from your data's 
 *natural units to screen pixel coordinates; this is handled internally
 *by the widget.  
 *
 *Data to be plotted are represented by one or more instances of 
 *KPlotObject.  KPlotObject contains a list of QPointFs to be plotted 
 *(again, in the data's natural units), as well as information about how 
 *the data are to be rendered in the plot (i.e., as separate points or 
 *connected by lines?  With what color and point style? etc).  See 
 *KPlotObject for more information.
 *
 *KPlotWidget automatically adds axis labels with tickmarks and tick 
 *labels.  These are encapsulated in the KPlotAxis class.  All you have 
 *to do is set the limits of the plotting area in data units, and 
 *KPlotWidget wil figure out the optimal positions and labels for the 
 *tickmarks on the axes.
 *
 *Example of usage:
 *
 * @code
 *  KPlotWidget *kpw = new KPlotWidget( this, 0.0, 1.0, 0.0, 1.0 );
 *  KPlotObject *kpo = new KPlotObject( Qt::red, KPlotObject::LINES );
 *
 *  //Add points to kpo:
 *  for ( float x=0.0; x<=1.0; x+=0.1 )
 *    kpo->addPoint( x, x*x );
 *    
 *  kpw->addObject( kpo );
 *  update();
 * @endcode
 *
 *@note KPlotWidget will take care of the objects added to it, so when
 *clearing the objects list (eg with clearObjectList()) any previous 
 *reference to a KPlotObject already added to a KPlotWidget will be invalid.
 *
 *@author Jason Harris
 *@version 1.1
 */
class KDEEDUPLOT_EXPORT KPlotWidget : public QFrame {
	Q_OBJECT
	Q_PROPERTY(int leftPadding READ leftPadding)
	Q_PROPERTY(int rightPadding READ rightPadding)
	Q_PROPERTY(int topPadding READ topPadding)
	Q_PROPERTY(int bottomPadding READ bottomPadding)
	Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
	Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
	Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor)
	Q_PROPERTY(bool grid READ isGridShown WRITE setShowGrid)
	Q_PROPERTY(bool objectToolTip READ areObjectToolTipsShown WRITE setShowObjectToolTips)
public:
	/**
	 * @short Constructor. Sets the primary x and y limits in data units.
	 * @param parent the parent widget
	 * @param x1 the minimum X value in data units
	 * @param x2 the maximum X value in data units
	 * @param y1 the minimum Y value in data units
	 * @param y2 the maximum Y value in data units
	 */
	KPlotWidget( QWidget *parent=0, double x1=0.0, double x2=1.0, double y1=0.0, double y2=1.0 );

	/**
	 * Destructor.
	 */
	virtual ~KPlotWidget();

	/**
	 *@enum Axis The kinds of axes we have
	 */
	enum Axis { LeftAxis = 0, BottomAxis, RightAxis, TopAxis };

	/**
	 *@return suggested size for widget
	 *@note Currently just returns QSize(150,150)
	 */
	virtual QSize minimumSizeHint() const;

	/**
	 * Reset the data limits.
	 * @param x1 the minimum X value in data units
	 * @param x2 the maximum X value in data units
	 * @param y1 the minimum Y value in data units
	 * @param y2 the maximum Y value in data units
	 */
	virtual void setLimits( double x1, double x2, double y1, double y2 );

	/**
	 * Reset the secondary data limits, which control the top and right axes.
	 * Note that all data points are plotted using the coordinates defined by 
	 * the primary data limits, so this function is only useful for plotting 
	 * alternate axes along the top and right edges.
	 * @param x1 the minimum X value in secondary data units
	 * @param x2 the maximum X value in secondary data units
	 * @param y1 the minimum Y value in secondary data units
	 * @param y2 the maximum Y value in secondary data units
	 * @sa setLimits()
	 */
	virtual void setSecondaryLimits( double x1, double x2, double y1, double y2 );

	/**
	 * Unset the secondary limits, so the top and right axes 
	 * show the same tickmarks as the bottom and left axes (no tickmark
	 * labels will be drawn for the top and right axes in this case)
	 */
	virtual void clearSecondaryLimits();

	/**
	 * @return the minimum X value in data units
	 */
	virtual double x() const { return DataRect.x(); }

	/**
	 * @return the maximum X value in data units
	 */
	virtual double x2() const { return DataRect.x() + DataRect.width(); }

	/**
	 * @return the minimum Y value in data units
	 */
	virtual double y() const { return DataRect.y(); }

	/**
	 * @return the maximum Y value in data units
	 */
	virtual double y2() const { return DataRect.y() + DataRect.height(); }

	/**
	 * @return the width in data units
	 */
	virtual double dataWidth() const { return DataRect.width(); }

	/**
	 * @return the height in data units
	 */
	virtual double dataHeight() const { return DataRect.height(); }

	const QRectF& secondaryDataRect() const { return SecondDataRect; }

	/**
	 * Add an item to the list of KPlotObjects to be plotted.
	 * @param o pointer to the KPlotObject to be added
	 */
	void addObject( KPlotObject *o );

	/**
	 * Remove and delete all items from the list of KPlotObjects
	 */
	void clearObjectList();

	/**
	 * Reset the PlotMask so that all regions are empty
	 */
	void resetPlotMask();

	/**
	 * Clear the object list, reset the data limits, and remove axis labels
	 */
	void resetPlot();

	/**
	 * Replace an item in the KPlotObject list.
	 * @param i the index of th item to be replaced
	 * @param o pointer to the replacement KPlotObject
	 */
	void replaceObject( int i, KPlotObject *o );

	/**
	 * @return the number of KPlotObjects in the list
	 */
	int objectCount() const { return ObjectList.size(); }

	/**
	 * @return a pointer to a specific KPlotObject in the list
	 * @param i the index of the desired KPlotObject
	 */
	KPlotObject *object( int i );

	/**
	 * @return the background color of the plot
	 */
	QColor backgroundColor() const { return cBackground; }

	/**
	 * @return the foreground color, used for the axes, tickmarks
	 * and associated labels.
	 */
	QColor foregroundColor() const { return cForeground; }

	/**
	 * @return the grid color
	 */
	QColor gridColor() const { return cGrid; }

	/**
	 * Set the background color
	 * @param bg the new background color
	 */
	void setBackgroundColor( const QColor &bg );

	/**
	 * Set the foreground color
	 * @param fg the new foreground color
	 */
	void setForegroundColor( const QColor &fg ) { cForeground = fg; }

	/**
	 * Set the grid color
	 * @param gc the new grid color
	 */
	void setGridColor( const QColor &gc ) { cGrid = gc; }

	/**
	 * @return whether the grid lines are shown
	 */
	bool isGridShown() const { return ShowGrid; }

	/**
	 * @return whether the tooltip for the point objects are shown
	 */
	bool areObjectToolTipsShown() const { return ShowObjectToolTips; }

	inline void setAntialias( bool b ) { UseAntialias = b; }

	/**
	 * @return the number of pixels to the left of the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int leftPadding();
	/**
	 * @return the number of pixels to the right of the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int rightPadding();
	/**
	 * @return the number of pixels above the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int topPadding();
	/**
	 * @return the number of pixels below the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
	virtual int bottomPadding();

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
	void setDefaultPaddings() { LeftPadding = -1; RightPadding = -1; TopPadding = -1; BottomPadding = -1; }

	/**
	 * Map a coordinate @p p from the data rect to the physical pixel rect.
	 * Used mainly when drawing.
	 * @return the coordinate in the pixel coordinate system
	 */
	QPointF toScreen( const QPointF& p ) const;

	/**
	 * Indicate that object labels should not occupy the given 
	 * rectangle in the plot.  The rectangle is in pixel coordinates.
	 *
	 * @note You should not normally call this function directly.
	 * It is called by KPlotObject when points, bars and labels are drawn.
	 * @param r the rectangle defining the region in the plot that 
	 * text labels should avoid (in pixel coordinates)
	 * @param value Allows you to determine how strongly the rectangle 
	 * should be avoided.  Larger values are avoided more strongly.
	 */
	void maskRect( const QRectF &r, float value=1.0 );

	/**
	 * Indicate that object labels should not be placed over the line 
   * joining the two given points (in pixel coordinates).
	 *
	 * @note You should not normally call this function directly.
	 * It is called by KPlotObject when lines are drawn in the plot.
	 * @param p1 the starting point for the line
	 * @param p2 the ending point for the line
	 * @param value Allows you to determine how strongly the line
	 * should be avoided.  Larger values are avoided more strongly.
	 */
	void maskAlongLine( const QPointF &p1, const QPointF &p2, float value=1.0 );

	/**
	 * Place an object label optimally in the plot.  This function will
	 * attempt to place the label as close as it can to the point to which 
	 * the label belongs, while avoiding overlap with regions of the plot 
	 * that have been masked. 
	 *
	 * @note You should not normally call this function directly.
	 * It is called internally in KPlotObject::draw().
	 *
	 * @param painter Pointer to the painter on which to draw the label
	 * @param pp pointer to the KPlotPoint whose label is to be drawn.
	 */
	void placeLabel( QPainter *painter, KPlotPoint *pp );

	/**
	 * Retrieve the pointer to the axis of type @p a.
	 * @sa Axis
	 * @return a pointer to the axis @p a , or 0 if not found
	 */
	KPlotAxis* axis( Axis a );

	inline QRect& pixRect() { return PixRect; }

public slots:
	/**
	 * Toggle whether grid lines are drawn at major tickmarks.
	 * @param show if true, grid lines will be drawn.
	 */
	void setShowGrid( bool show );

	/**
	 * Toggle whether the tooltip for point objects are shown.
	 * @param show if true, the tooltips will be shown.
	 */
	void setShowObjectToolTips( bool show );

protected:
	/**
	 * Generic event handler.
	 */
	virtual bool event( QEvent* );

	/**
	 * The paint event handler, executed when update() or repaint() is called.
	 */
	virtual void paintEvent( QPaintEvent* );

	/**
	 * The resize event handler, called when the widget is resized.
	 */
	virtual void resizeEvent( QResizeEvent* );

	/**
	 * Draws the plot axes and axis labels.
	 * @internal Internal use only; one should simply call update()
	 * to draw the widget with axes and all objects.
	 * @param p pointer to the painter on which we are drawing
	 */
	virtual void drawAxes( QPainter *p );

	/**
	 * Synchronize the PixRect with the current widget size and 
	 * padding settings
	 */
	void setPixRect();

	/**
	 * @return a list of points in the plot which are within 4 pixels
	 * of the screen position given as an argument.
	 * @param p The screen position from which to check for plot points.
	 */
	QList<KPlotPoint*> pointsUnderPoint( const QPoint& p ) const;

	/**
	 * @return a value indicating how well the given rectangle is 
	 * avoiding masked regions in the plot.  A higher returned value 
	 * indicates that the rectangle is intersecting a larger portion 
	 * of the masked region, or a portion of the masked region which 
	 * is weighted higher.
	 * @param r The rectangle to be tested
	 */
	float rectCost( const QRectF &r );

	/**
	 * Limits of the plot area in pixel units
	 */
	QRect PixRect;
	/**
	 * Limits of the plot area in data units
	 */
	QRectF DataRect, SecondDataRect;
	/**
	 * List of KPlotObjects
	 */
	QList<KPlotObject*> ObjectList;

	/**
	 * Hashmap with the axes we have
	 */
	QHash<Axis, KPlotAxis*> mAxes;

	//Colors
	QColor cBackground, cForeground, cGrid;
	//draw options
	bool ShowGrid, ShowObjectToolTips, UseAntialias;
	//padding
	int LeftPadding, RightPadding, TopPadding, BottomPadding;

	//Grid of bools to mask "used" regions of the plot
	float PlotMask[100][100];
	double px[100], py[100];
};

#endif
