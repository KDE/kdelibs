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

#include <QList>
#include <QHash>
#include <QWidget>

#include <kdemacros.h>

class QPixmap;
class KPlotAxis;
class KPlotObject;

/**
 * @class KPlotWidget
 *
 * @short Generic data plotting widget.
 *
 * Widget for drawing plots. Includes adjustable axes (KPlotAxis) with
 * tickmarks and labels and a list of KPlotObjects to be drawn.
 *
 * @note KPlotWidget will take care of the objects added to it, so when
 * clearing the objects list (eg with clearObjectList()) any previous reference
 * to a KPlotObject already added to a KPlotWidget will be invalid
 *
 * @author Jason Harris
 *
 * @version 1.1
 */
class KDE_EXPORT KPlotWidget : public QWidget {
	Q_OBJECT
	Q_PROPERTY(int leftPadding READ leftPadding)
	Q_PROPERTY(int rightPadding READ rightPadding)
	Q_PROPERTY(int topPadding READ topPadding)
	Q_PROPERTY(int bottomPadding READ bottomPadding)
	Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
	Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor)
	Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor)
	Q_PROPERTY(bool grid READ isGridShown WRITE setShowGrid)
	Q_PROPERTY(bool tickMarks READ areTickMarksShown WRITE setShowTickMarks)
	Q_PROPERTY(bool tickLabels READ areTickLabelsShown WRITE setShowTickLabels)
	Q_PROPERTY(bool objectToolTip READ areObjectToolTipsShown WRITE setShowObjectToolTips)
public:
	/**
	 * @short Constructor. Sets the primary x and y limits in data units.
	 * @param x1 the minimum X value in data units
	 * @param x2 the maximum X value in data units
	 * @param y1 the minimum Y value in data units
	 * @param y2 the maximum Y value in data units
	 * @param parent the parent widget
	 */
	KPlotWidget( double x1=0.0, double x2=1.0, double y1=0.0, double y2=1.0, QWidget *parent=0 );

	/**
	 * Destructor.
	 */
	virtual ~KPlotWidget();

	/**
	 * The kinds of axes we have
	 */
	enum Axis
	{
		LeftAxis = 0,
		BottomAxis
	};

	virtual QSize minimumSizeHint() const;

	/**
	 * Determine the placement of major and minor tickmarks, based on the
	 * current Limit settings
	 */
	virtual void updateTickmarks();

	/**
	 * Reset the data limits.
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
	 * Replace an item in the KPlotObject list.
	 * @param i the index of th item to be replaced
	 * @param o pointer to the replacement KPlotObject
	 */
	void replaceObject( int i, KPlotObject *o );

	/**
	 * @return the number of KPlotObjects in the list
	 */
	int objectCount() const { return ObjectList.count(); }

	/**
	 * @return a pointer to a specific KPlotObject in the list
	 * @param i the index of the desired KPlotObject
	 */
	KPlotObject *object( int i );

	/**
	 * @return the background color
	 */
	QColor backgroundColor() const { return cBackground; }

	/**
	 * @return the foreground color
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
	 * Toggle whether plot axes are drawn.
	 * @param show if true, axes will be drawn.
	 * The axes are just a box outline around the plot.
	 */
	void setShowAxes( bool show );

	/**
	 * @return whether the tick marks are shown
	 */
	bool areTickMarksShown() const { return ShowTickMarks; }

	/**
	 * @return whether the tick labels are shown
	 */
	bool areTickLabelsShown() const { return ShowTickLabels; }

	/**
	 * @return whether the grid lines are shown
	 */
	bool isGridShown() const { return ShowGrid; }

	/**
	 * @return whether the tooltip for the point objects are shown
	 */
	bool areObjectToolTipsShown() const { return ShowObjectToolTips; }

	/**
	 * Sets the X-axis label.
	 * Set the label to an empty string to omit the axis label.
	 *
	 * @deprecated set the label property in the BottomAxis directly
	 * @param xlabel a short string describing the data plotted on the x-axis.
	 */
	KDE_DEPRECATED void setXAxisLabel( const QString& xlabel );
	/**
	 * Sets the Y-axis label
	 * Set the label to an empty string to omit the axis label.
	 *
	 * @deprecated set the label property in the LeftAxis directly
	 * @param ylabel a short string describing the data plotted on the y-axis.
	 */
	KDE_DEPRECATED void setYAxisLabel( const QString& ylabel );

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
	void setDefaultPaddings() { LeftPadding = -1; RightPadding = -1; TopPadding = -1; BottomPadding = -1; }

	QPoint mapToPoint( const QPointF& p ) const {
		int px = PixRect.left() + int( PixRect.width()*( p.x() -  DataRect.x() )/DataRect.width() );
		int py = PixRect.top() + int( PixRect.height()*( DataRect.y() + DataRect.height() - p.y() )/DataRect.height() );
		return QPoint( px, py );
	}

	/**
	 * Retrieve the pointer to the axis of type @p a.
	 * @sa Axis
	 * @return a pointer to the axis @p a , or 0 if not found
	 */
	KPlotAxis* axis( Axis a );

public slots:
	/**
	 * Toggle whether tick marks are drawn along the axes.
	 * @param show if true, tick marks will be drawn.
	 */
	void setShowTickMarks( bool show );

	/**
	 * Toggle whether tick labels are drawn at major tickmarks.
	 * @param show if true, tick labels will be drawn.
	 */
	void setShowTickLabels( bool show );

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
	 * @deprecated use fmod (already defined in \<math.h\>)
	 * @return the remainder after dividing @p b into @p a.
	 */
	KDE_DEPRECATED double dmod( double a, double b );

	virtual void recalcPixRect();

	QList<KPlotObject*> pointsUnderPoint( const QPoint& p ) const;

	//The distance between major tickmarks in data units
	double dXtick, dYtick;
	//The number of major and minor tickmarks to be plotted in X and Y
	int nmajX, nminX, nmajY, nminY;

	/**
	 * Limits of the plot area in pixel units
	 */
	QRect PixRect;
	/**
	 * Limits of the plot area in data units
	 */
	QRectF DataRect;
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
	bool ShowTickMarks, ShowTickLabels, ShowGrid, ShowObjectToolTips;
	//padding
	int LeftPadding, RightPadding, TopPadding, BottomPadding;

	QPixmap *buffer;
};

#endif
