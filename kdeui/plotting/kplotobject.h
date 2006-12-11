/***************************************************************************
                          kplotobject.h - A list of points to be plotted
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

#ifndef KPLOTOBJECT_H
#define KPLOTOBJECT_H

#include <QColor>
#include <QPointF>
#include <QString>
#include <QPen>
#include <QBrush>

#include <libkdeedu_plot_export.h>

class QPainter;
class KPlotWidget;

/**
 * @class KPlotPoint
 * @short Encapsulates a point in the plot.
 * A KPlotPoint consists of X and Y coordinates (in Data units),
 * an optional label string, and an optional bar-width,
 * The bar-width is only used for plots of type KPlotObject::BARS, 
 * and it allows the width of each bar to be set manually.  If 
 * bar-widths are omitted, then the widths will be set automatically, 
 * based on the halfway-mark between adjacent points.
 */
class KDEEDUPLOT_EXPORT KPlotPoint {
 public:
	/** 
	 *Default constructor.
	 */
	KPlotPoint();
	/**
	 * Constructor.  Sets the KPlotPoint according to the given arguments
	 * @param x the X-position for the point, in Data units
	 * @param y the Y-position for the point, in Data units
	 * @param label the label string for the point.  If the string 
	 * is defined, the point will be labeled in the plot.
	 * @param width the BarWidth to use for this point (only used for 
	 * plots of type KPlotObject::BARS)
	 */
	KPlotPoint( double x, double y, const QString &label="", double width=0.0 );
	/**
	 * Constructor.  Sets the KPlotPoint according to the given arguments
	 * @param p the position for the point, in Data units
	 * @param label the label string for the point.  If the string 
	 * is defined, the point will be labeled in the plot.
	 * @param width the BarWidth to use for this point (only used for 
	 * plots of type KPlotObject::BARS)
	 */
	KPlotPoint( const QPointF &p, const QString &label="", double width=0.0 );
	/**
	 * Destructor
	 */
	~KPlotPoint();

	/**
	 * @return the position of the point, in data units
	 */
	QPointF position() const { return QPointF( X, Y ); }
	/**
	 * Set the position of the point, in data units
	 * @param pos the new position for the point.
	 */
	void setPosition( const QPointF &pos ) { X = pos.x(); Y = pos.y(); }
	/**
	 * @return the X-position of the point, in data units
	 */
	double x() const { return X; }
	/**
	 * Set the X-position of the point, in Data units
	 */
	void setX( double x ) { X = x; }
	/**
	 * @return the Y-position of the point, in data units
	 */
	double y() const { return Y; }
	/**
	 * Set the Y-position of the point, in Data units
	 */
	void setY( double y ) { Y = y; }

	/**
	 * @return the label for the point
	 */
	const QString& label() const { return Label; }
	/**
	 * Set the label for the point
	 */
	void setLabel( const QString &label ) { Label = label; }

	/**
	 * @return the bar-width for the point
	 */
	double barWidth() const { return BarWidth; }
	/**
	 * Set the bar-width for the point
	 */
	void stBarWidth( double w ) { BarWidth = w; }

 private:
	double X, Y;
	QString Label;
  double BarWidth;
};

/**
 * @class KPlotObject
 * @short Encapsulates an object to be plotted in a KPlotWidget.
 *
 * Think of a KPlotObject as a set of data displayed as a group in the plot.
 * Each KPlotObject consists of a list of KPlotPoints, a "type" controlling 
 * how the data points are displayed (some combination of POINTS, LINES, or 
 * BARS), a color, and a size.  There is also a parameter which controls the 
 * shape of the points used to display the KPlotObject.
 *
 * @note KPlotObject will take care of the points added to it, so when clearing
 * the points list (eg with clearPoints()) any previous reference to a QPointF
 * already added to a KPlotObject will be invalid.
 *
 * @author Jason Harris
 * @version 1.1
 */
class KDEEDUPLOT_EXPORT KPlotObject{
public:
	/**
	 * @enum PlotType
	 * The Type classification of the KPlotObject.  The possible values are:
	 * @li POINTS: each KPlotPoint is represented with a drawn point
	 * @li LINES: each KPlotPoint is connected with a line
	 * @li BARS: each KPlotPoint is shown as a vertical bar.  Note that 
	 * points should be added in order of increasing x-coordinate when 
	 * using BARS.
	 *
	 * These are bitmask values that can be OR'd together, so that a set 
	 * of points can be represented in the plot in multiple ways.
	 */
	enum PlotType { POINTS=1, LINES=2, BARS=4, UNKNOWN_TYPE };

	/**
	 * @enum PStyle
	 * Parameter specifying the kind of points.  The possible values are:
	 * @li CIRCLE
	 * @li LETTER
	 * @li TRIANGLE
	 * @li SQUARE
	 * @li PENTAGON
	 * @li HEXAON
	 * @li ASTERISK
	 * @li STAR
	 */
	enum PStyle { NOPOINTS=0, CIRCLE=1, LETTER=2, TRIANGLE=3, SQUARE=4, PENTAGON=5, HEXAGON=6, ASTERISK=7, STAR=8, UNKNOWN_POINT };

	/**
	 * Default constructor.  Create a POINTS-type object with an 
	 * empty list of points.
	 */
	KPlotObject();

	/**
	 * Constructor. Create a KPlotObject according to the arguments.
	 * @param color The color for plotting this object.  By default this sets 
	 * the color for POINTS, LINES and BARS, but there are functins to 
	 * override any of these.
	 * @param otype the PlotType for this object
	 * @param size the size to use for the drawn points, in pixels
	 * @param ps The PStyle describing the shape for the drawn points
	 */
	KPlotObject( const QColor &color, PlotType otype, double size=2, PStyle ps=CIRCLE );

	/**
	 * Destructor.
	 */
	~KPlotObject();

	/**
	 * @return the label of point i
	 * @param i the index of the point
	 */
	inline QString label( int i ) const { if (pList.size() > i) return pList.at(i)->label(); }

	/**
	 * Set the label text for point i
	 * @param i the index of the point
	 * @param n the new name
	 */
	inline void setLabel( int i, const QString &n ) { if (pList.size() > i) pList.at(i)->setLabel(n); }

	/**
	 * @return true if points will be drawn for this object
	 */
	bool showPoints() const { return Type & KPlotObject::POINTS; }
	/**
	 * @return true if lines will be drawn for this object
	 */
	bool showLines() const { return Type & KPlotObject::LINES; }
	/**
	 * @return true if bars will be drawn for this object
	 */
	bool showBars() const { return Type & KPlotObject::BARS; }

	/**
	 * Set whether points will be drawn for this object
	 * @param b if true, points will be drawn
	 */
	void setShowPoints( bool b ) { 
		if ( b ) { Type = Type | KPlotObject::POINTS; }
		else     { Type = Type & ~KPlotObject::POINTS; }
	}

	/**
	 * Set whether lines will be drawn for this object
	 * @param b if true, lines will be drawn
	 */
	void setShowLines( bool b ) { 
		if ( b ) { Type = Type | KPlotObject::LINES; }
		else     { Type = Type & ~KPlotObject::LINES; }
	}

	/**
	 * Set whether bars will be drawn for this object
	 * @param b if true, bars will be drawn
	 */
	void setShowBars( bool b ) { 
		if ( b ) { Type = Type | KPlotObject::BARS; }
		else     { Type = Type & ~KPlotObject::BARS; }
	}

	/**
	 * @return the KPlotObject's Size
	*/
	double size() const { return Size; }

	/**
	 * Set the KPlotObject's Size
	 * @param s the new size
	 */
	void setSize( double s ) { Size = s; }

	/**
	 * @return the KPlotObject's PointStyle value
	 */
	unsigned int pointStyle() const { return PointStyle; }

	/**
	 * Set the KPlotObject's type-specific Parameter
	 * @param p the new parameter
	 */
	void setPointStyle( PStyle p ) { PointStyle = p; }

	/**
	 * @return the default pen for this Object.
	 * If no other pens are set, this pen will be used for 
	 * points, lines, bars and labels (this pen is always used for points).
	 */
	const QPen& pen() const { return Pen; }
	/**
	 * Set the default pen for this object
	 * @p The pen to use
	 */
	void setPen( const QPen &p ) { Pen = p; }
	
	/**
	 * @return the pen to use for drawing lines for this Object.
	 */
	const QPen& linePen() const { return LinePen; }
	/**
	 * Set the pen to use for drawing lines for this object
	 * @p The pen to use
	 */
	void setLinePen( const QPen &p ) { LinePen = p; }
	
	/**
	 * @return the pen to use for drawing bars for this Object.
	 */
	const QPen& barPen() const { return BarPen; }
	/**
	 * Set the pen to use for drawing bars for this object
	 * @p The pen to use
	 */
	void setBarPen( const QPen &p ) { BarPen = p; }
	
	/**
	 * @return the pen to use for drawing labels for this Object.
	 */
	const QPen& labelPen() const { return LabelPen; }
	/**
	 * Set the pen to use for labels for this object
	 * @p The pen to use
	 */
	void setLabelPen( const QPen &p ) { LabelPen = p; }
	
	/**
	 * @return the default Brush to use for this Object.
	 */
	const QBrush brush() const { return Brush; }
	/**
	 * Set the default brush for this object
	 * @b The brush to use
	 */
	void setBrush( const QBrush &b ) { Brush = b; }

	/**
	 * @return the brush to use for filling bars for this Object.
	 */
	const QBrush barBrush() const { return BarBrush; }
	/**
	 * Set the brush to use for drawing bars for this object
	 * @b The brush to use
	 */
	void setBarBrush( const QBrush &b ) { BarBrush = b; }

	/**
	 * @return the list of KPlotPoints that make up this object
	 */
	QList<KPlotPoint*> points() { return pList; }

	/**
	 * Add a point to the object's list.
	 * @param p the QPointF to add.
	 * @param label the optional text label
	 * @param barWidth the width of the bar, if this object is drawn as bars
	 */
	void addPoint( const QPointF &p, const QString &label="", double barWidth=0.0 ) { 
		addPoint( new KPlotPoint( p.x(), p.y(), label ) ); 
	}

	/**
	 * Add a point to the object's list.
	 * @overload
	 * @param p pointer to the KPlotPoint to add.
	 */
	void addPoint( KPlotPoint *p ) {
		pList.append( p );
	}

	/**
	 * Add a point to the object's list.
	 * @overload
	 * @param x the X-coordinate of the point to add.
	 * @param y the Y-coordinate of the point to add.
	 * @param label the optional text label
	 * @param barWidth the width of the bar, if this object is drawn as bars
	 */
	void addPoint( double x, double y, const QString &label="", double barWidth=0.0 ) { 
		addPoint( new KPlotPoint( x, y, label ) ); 
	}

	/**
	 * Remove the QPointF at position index from the list of points
	 * @param index the index of the point to be removed.
	 */
	void removePoint( int index );

	/**
	 * @return a pointer to the KPlotPoint at the given position in the list
	 * @param index the index of the point in the list
	 */
	KPlotPoint* point( int index ) { return pList[index]; }

	/**
	 * @return the number of QPoints currently in the list
	 */
	int count() const { return pList.count(); }

	/**
	 * Clear the Object's points list
	 */
	void clearPoints();

	/**
	 * Draw this KPlotObject on the given QPainter
	 * @param p The QPainter to draw on
	 * @param pw the KPlotWidget to draw on (this is needed 
	 * for the KPlotWidget::toScreen() function)
	 */
	void draw( QPainter *p, KPlotWidget *pw );

private:
	QList<KPlotPoint*> pList;
	int Type;
	PStyle PointStyle;
	double Size;
	QPen Pen, LinePen, BarPen, LabelPen;
	QBrush Brush, BarBrush;
};

#endif
