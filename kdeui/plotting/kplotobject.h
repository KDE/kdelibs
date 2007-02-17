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
 * The bar-width is only used for plots of type KPlotObject::Bars,
 * and it allows the width of each bar to be set manually.  If 
 * bar-widths are omitted, then the widths will be set automatically, 
 * based on the halfway-mark between adjacent points.
 */
class KDEEDUPLOT_EXPORT KPlotPoint {
 public:
        /**
         * Default constructor.
         */
        explicit KPlotPoint();
	/**
	 * Constructor.  Sets the KPlotPoint according to the given arguments
	 * @param x the X-position for the point, in Data units
	 * @param y the Y-position for the point, in Data units
	 * @param label the label string for the point.  If the string 
	 * is defined, the point will be labeled in the plot.
	 * @param width the BarWidth to use for this point (only used for 
	 * plots of type KPlotObject::Bars)
	 */
        KPlotPoint( double x, double y, const QString &label = QString(), double width = 0.0 );
	/**
	 * Constructor.  Sets the KPlotPoint according to the given arguments
	 * @param p the position for the point, in Data units
	 * @param label the label string for the point.  If the string 
	 * is defined, the point will be labeled in the plot.
	 * @param width the BarWidth to use for this point (only used for 
	 * plots of type KPlotObject::Bars)
	 */
        explicit KPlotPoint( const QPointF &p, const QString &label = QString(), double width = 0.0 );
	/**
	 * Destructor
	 */
	~KPlotPoint();

	/**
	 * @return the position of the point, in data units
	 */
        QPointF position() const;
	/**
	 * Set the position of the point, in data units
	 * @param pos the new position for the point.
	 */
        void setPosition( const QPointF &pos );
	/**
	 * @return the X-position of the point, in data units
	 */
        double x() const;
	/**
	 * Set the X-position of the point, in Data units
	 */
        void setX( double x );
	/**
	 * @return the Y-position of the point, in data units
	 */
        double y() const;
	/**
	 * Set the Y-position of the point, in Data units
	 */
        void setY( double y );

	/**
	 * @return the label for the point
	 */
        QString label() const;
	/**
	 * Set the label for the point
	 */
        void setLabel( const QString &label );

	/**
	 * @return the bar-width for the point
	 */
        double barWidth() const;
	/**
	 * Set the bar-width for the point
	 */
        void setBarWidth( double w );

 private:
        class Private;
        Private * const d;

        Q_DISABLE_COPY( KPlotPoint )
};

/**
 * @class KPlotObject
 * @short Encapsulates an object to be plotted in a KPlotWidget.
 *
 * Think of a KPlotObject as a set of data displayed as a group in the plot.
 * Each KPlotObject consists of a list of KPlotPoints, a "type" controlling 
 * how the data points are displayed (some combination of Points, Lines, or
 * Bars), a color, and a size. There is also a parameter which controls the
 * shape of the points used to display the KPlotObject.
 *
 * @note KPlotObject will take care of the points added to it, so when clearing
 * the points list (eg with clearPoints()) any previous reference to a KPlotPoint
 * already added to a KPlotObject will be invalid.
 *
 * @author Jason Harris
 * @version 1.1
 */
class KDEEDUPLOT_EXPORT KPlotObject{
public:
        /**
         * The type classification of the KPlotObject.
         *
         * These are bitmask values that can be OR'd together, so that a set
         * of points can be represented in the plot in multiple ways.
         *
         * @note points should be added in order of increasing x-coordinate
         * when using Bars.
         */
        enum PlotType
        {
            UnknownType = 0,
            Points = 1,       ///< each KPlotPoint is represented with a drawn point
            Lines = 2,        ///< each KPlotPoint is connected with a line
            Bars = 4          ///< each KPlotPoint is shown as a vertical bar
        };

        /**
         * The possible kind of points.
         */
        enum PointStyle
        {
            NoPoints = 0,
            Circle = 1,
            Letter = 2,
            Triangle = 3,
            Square = 4,
            Pentagon = 5,
            Hexagon = 6,
            Asterisk = 7,
            Star = 8,
            UnknwonPoint
        };

        /**
         * Constructor.
         * @param color The color for plotting this object. By default this sets
         * the color for Points, Lines and Bars, but there are functions to
         * override any of these.
         * @param otype the PlotType for this object
         * @param size the size to use for the points, in pixels
         * @param ps The PointStyle describing the shape for the points
         */
        explicit KPlotObject( const QColor &color = Qt::white, PlotType otype = Points, double size = 2, PointStyle ps = Circle );

	/**
	 * Destructor.
	 */
	~KPlotObject();

	/**
	 * @return the label of point i
	 * @param i the index of the point
	 */
        QString label( int i ) const;

	/**
	 * Set the label text for point i
	 * @param i the index of the point
	 * @param n the new name
	 */
        void setLabel( int i, const QString &n );

	/**
	 * @return true if points will be drawn for this object
	 */
        bool showPoints() const;
	/**
	 * @return true if lines will be drawn for this object
	 */
        bool showLines() const;
	/**
	 * @return true if bars will be drawn for this object
	 */
        bool showBars() const;

	/**
	 * Set whether points will be drawn for this object
	 * @param b if true, points will be drawn
	 */
        void setShowPoints( bool b );

	/**
	 * Set whether lines will be drawn for this object
	 * @param b if true, lines will be drawn
	 */
        void setShowLines( bool b );

	/**
	 * Set whether bars will be drawn for this object
	 * @param b if true, bars will be drawn
	 */
        void setShowBars( bool b );

        /**
         * @return the size of the object
         */
        double size() const;

        /**
         * Set the new size for the object
         * @param s the new size
         */
        void setSize( double s );

        /**
         * @return the style used for drawing the points of the object
         */
        PointStyle pointStyle() const;

        /**
         * Set a new style for drawing the points
         * @param p the new style
         */
        void setPointStyle( PointStyle p );

	/**
	 * @return the default pen for this Object.
	 * If no other pens are set, this pen will be used for 
	 * points, lines, bars and labels (this pen is always used for points).
	 */
        const QPen& pen() const;
	/**
	 * Set the default pen for this object
	 * @p The pen to use
	 */
        void setPen( const QPen &p );
	
	/**
	 * @return the pen to use for drawing lines for this Object.
	 */
        const QPen& linePen() const;
	/**
	 * Set the pen to use for drawing lines for this object
	 * @p The pen to use
	 */
        void setLinePen( const QPen &p );
	
	/**
	 * @return the pen to use for drawing bars for this Object.
	 */
        const QPen& barPen() const;
	/**
	 * Set the pen to use for drawing bars for this object
	 * @p The pen to use
	 */
        void setBarPen( const QPen &p );
	
	/**
	 * @return the pen to use for drawing labels for this Object.
	 */
        const QPen& labelPen() const;
	/**
	 * Set the pen to use for labels for this object
	 * @p The pen to use
	 */
        void setLabelPen( const QPen &p );
	
	/**
	 * @return the default Brush to use for this Object.
	 */
        const QBrush brush() const;
	/**
	 * Set the default brush for this object
	 * @b The brush to use
	 */
        void setBrush( const QBrush &b );

	/**
	 * @return the brush to use for filling bars for this Object.
	 */
        const QBrush barBrush() const;
	/**
	 * Set the brush to use for drawing bars for this object
	 * @b The brush to use
	 */
        void setBarBrush( const QBrush &b );

	/**
	 * @return the list of KPlotPoints that make up this object
	 */
        QList< KPlotPoint* > points() const;

	/**
	 * Add a point to the object's list.
	 * @param p the QPointF to add.
	 * @param label the optional text label
	 * @param barWidth the width of the bar, if this object is drawn as bars
	 */
        void addPoint( const QPointF &p, const QString &label = QString(), double barWidth = 0.0 );

	/**
	 * Add a point to the object's list.
	 * @overload
	 * @param p pointer to the KPlotPoint to add.
	 */
        void addPoint( KPlotPoint *p );

	/**
	 * Add a point to the object's list.
	 * @overload
	 * @param x the X-coordinate of the point to add.
	 * @param y the Y-coordinate of the point to add.
	 * @param label the optional text label
	 * @param barWidth the width of the bar, if this object is drawn as bars
	 */
        void addPoint( double x, double y, const QString &label = QString(), double barWidth = 0.0 );

	/**
	 * Remove the QPointF at position index from the list of points
	 * @param index the index of the point to be removed.
	 */
	void removePoint( int index );

        /**
         * Remove and destroy the points of this object
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
        class Private;
        Private * const d;

        Q_DISABLE_COPY( KPlotObject );
};

#endif
