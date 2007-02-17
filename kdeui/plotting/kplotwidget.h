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
#include <QList>

#include <libkdeedu_plot_export.h>

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
KPlotWidget *kpw = new KPlotWidget( parent );
// setting our limits for the plot
kpw->setLimits( 1.0, 5.0, 1.0, 25.0 );

// creating a red polygon ...
KPlotObject *kpo = new KPlotObject( Qt::red, KPlotObject::LINES );
// ... adding some points to it ...
for ( float x = 1.0; x <= 5.0; x += 0.1 )
    kpo->addPoint( x, x*x );
// ... and adding the object to the plot widget
kpw->addPlotObject( kpo );
 * @endcode
 *
 *@note KPlotWidget will take care of the objects added to it, so when
 *clearing the objects list (eg with removeAllPlotObjects()) any previous 
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
	Q_PROPERTY(bool objectToolTip READ isObjectToolTipShown WRITE setObjectToolTipShown)
public:
        /**
         * Constructor.
         * @param parent the parent widget
         */
        explicit KPlotWidget( QWidget * parent = 0 );

	/**
	 * Destructor.
	 */
	virtual ~KPlotWidget();

        /**
         * The kinds of axes we have
         */
        enum Axis
        {
            LeftAxis = 0,  ///< the left axis
            BottomAxis,    ///< the bottom axis
            RightAxis,     ///< the right axis
            TopAxis        ///< the top axis
        };

	/**
	 *@return suggested size for widget
	 */
	virtual QSize minimumSizeHint() const;

	/**
	 * Set new data limits for the plot.
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
         * Return the rect in natural limits representing the shown data.
         * @warning the coordinate system of QRectF and the human one are not the same!
         * Though, the height of the rect is really the height of the data rect.
         */
        QRectF dataRect() const;

        QRectF secondaryDataRect() const;

        /**
         * Add an item to the list of KPlotObjects to be plotted.
         * @note do not use this multiple time if many objects have to be added,
           addPlotObjects() is strongly suggested in this case
         * @param object the KPlotObject to be added
         */
        void addPlotObject( KPlotObject *object );

        /**
         * Add more than one KPlotObject at one time.
         * @param objects the KPlotObject's to be added
         */
        void addPlotObjects( const QList< KPlotObject* >& objects );

        /**
         * @return the list of the current objects
         */
        QList< KPlotObject* > plotObjects() const;

	/**
	 * Remove and delete all items from the list of KPlotObjects
	 */
	void removeAllPlotObjects();

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
        void replacePlotObject( int i, KPlotObject *o );

        /**
         * Return the background color of the plot.
         *
         * The default color is black.
         */
        QColor backgroundColor() const;

        /**
         * Return the foreground color, used for axes, tickmarks and associated
         * labels.
         *
         * The default color is white.
         */
        QColor foregroundColor() const;

        /**
         * Return the grid color.
         *
         * The default color is gray.
         */
        QColor gridColor() const;

	/**
	 * Set the background color
	 * @param bg the new background color
	 */
	void setBackgroundColor( const QColor &bg );

	/**
	 * Set the foreground color
	 * @param fg the new foreground color
	 */
        void setForegroundColor( const QColor &fg );

	/**
	 * Set the grid color
	 * @param gc the new grid color
	 */
        void setGridColor( const QColor &gc );

	/**
	 * @return whether the grid lines are shown
	 */
        bool isGridShown() const;

        /**
         * Return whether the tooltip for the point objects is shown.
         *
         * It's enabled by default.
         */
        bool isObjectToolTipShown() const;

        /**
         * @return whether the antialiasing is active
         */
        bool antialiasing() const;

        /**
         * Toggle the antialiasing when drawing.
         */
        void setAntialiasing( bool b );

	/**
	 * @return the number of pixels to the left of the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
        int leftPadding() const;
	/**
	 * @return the number of pixels to the right of the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
        int rightPadding() const;
	/**
	 * @return the number of pixels above the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
        int topPadding() const;
	/**
	 * @return the number of pixels below the plot area.
	 * Padding values are set to -1 by default; if unchanged, this function will try to guess
	 * a good value, based on whether ticklabels and/or axis labels are to be drawn.
	 */
        int bottomPadding() const;

	/**
	 * Set the number of pixels to the left of the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
        void setLeftPadding( int padding );
	/**
	 * Set the number of pixels to the right of the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
        void setRightPadding( int padding );
	/**
	 * Set the number of pixels above the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
        void setTopPadding( int padding );
	/**
	 * Set the number of pixels below the plot area.
	 * Set this to -1 to revert to automatic determination of padding values.
	 */
        void setBottomPadding( int padding );

	/**
	 * Revert all four padding values to be automatically determined.
	 */
        void setDefaultPaddings();

	/**
	 * Map a coordinate @p p from the data rect to the physical pixel rect.
	 * Used mainly when drawing.
	 * @return the coordinate in the pixel coordinate system
	 */
        QPointF mapToWidget( const QPointF& p ) const;

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
         * Get the axis of the specified @p type, or 0 if no axis has been set.
         * @sa Axis
         */
        KPlotAxis* axis( Axis type );

        /**
         * Get the axis of the specified @p type, or 0 if no axis has been set.
         * @sa Axis
         */
        const KPlotAxis* axis( Axis type ) const;

        QRect pixRect() const;

public slots:
	/**
	 * Toggle whether grid lines are drawn at major tickmarks.
	 * @param show if true, grid lines will be drawn.
	 */
	void setShowGrid( bool show );

        /**
         * Toggle the display of a tooltip for point objects.
         * @param show whether show the tooltip.
         */
        void setObjectToolTipShown( bool show );

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

    private:
        class Private;
        Private * const d;

        Q_DISABLE_COPY( KPlotWidget )
};

#endif
