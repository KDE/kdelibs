/***************************************************************************
                          kplotaxis.h - An axis for the plot widget
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

#ifndef KPLOTAXIS_H
#define KPLOTAXIS_H

#include <QString>
#include <QList>

#include <libkdeedu_plot_export.h>

/**
 * @short Axis for KPlotWidget
 *
 * Contains all data for drawing an axis including format specification axis labels.
 *
 * @author Andreas Nicolai
 * @version 1.0
 */
class KDEEDUPLOT_EXPORT KPlotAxis {
public:

	/**
	 * Constructor, constructs an axis with the label @p label.
	 */
        explicit KPlotAxis( const QString& label = QString() );

	/**
	 * Destructor.
	 */
        ~KPlotAxis();

	/**
	 * @return whether the axis is visible or not
	 */
        bool isVisible() const;

	/**
	 * Sets the "visible" property of the axis.
	 */
        void setVisible( bool visible );

	/**
	 * @return whether tick labels will be drawn for this axis
	 */
        bool areTickLabelsShown() const;

	/**
	 * Determine whether tick labels will be drawn for this axis.
	 */
        void setTickLabelsShown( bool b );

	/**
	 * Sets the axis label.
	 * Set the label to an empty string to omit the axis label.
	 * @param label a string describing the data plotted on the axis.
	 */
        void setLabel( const QString& label );

	/**
	 * @return the axis label
	 */
        QString label() const;

	/**
	 * @return the ticklabel string for the given value, rendered according
	 * to the current format specification.
	 * @param the value to be rendered as a tick label.
	 * @sa setTickLabelFormat()
	 */
	QString tickLabel( double value ) const;

	/**
	 * Set the display format for converting the double value of the 
	 * tick's position to the QString for the tick label.
	 *
	 * Normally, the format character is one of 'e', 'E', 'f', 'g', or 'G'
	 * (see the documentation for QString::arg(double) for details).
	 *
	 * In addition, it is possible to set the format character to 't';
	 * in this case the tickmark value is interpreted as a time in hours,
	 * and the ticklabel string will be in "hh:mm" clock format.
	 * Note that when the format character is 't', the fieldWidth and prec
	 * values are ignored.
	 *
	 * @param format the format specification character 
	 * @param fieldWidth the number of characters in the output string.
	 * If set to 0, the string will be as wide as it needs to be to fully 
	 * render the value.
	 * @param precision the number of characters following the decimal point.
	 */
        void setTickLabelFormat( char format = 'g', int fieldWidth = 0, int precision = -1 );

	/**
	 * @return the field width of the tick labels
	 */
        int tickLabelWidth() const;

	/**
	 * @return the number format of the tick labels
	 */
        char tickLabelFormat() const;

	/**
	 * @return the number precision of the tick labels
	 */
        int tickLabelPrecision() const;

	/**
	 * Determine the positions of major and minor tickmarks for this axis.
	 * @param x0 the minimum data coordinate of the axis.
	 * @param length the range covered by the axis, in data units.
	 * @sa majorTickMarks()
	 * @sa minorTickMarks()
	 */
	void setTickMarks( double x0, double length );

        /**
         * @return the list with the major tickmarks
         */
        QList< double > majorTickMarks() const;

        /**
         * @return the list with the minor tickmarks
         */
        QList< double > minorTickMarks() const;

private:
        class Private;
        Private * const d;

        Q_DISABLE_COPY( KPlotAxis )
};

#endif // KPLOTAXIS_H
