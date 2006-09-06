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
class EDUPLOT_EXPORT KPlotAxis {
public:

	/**
	 * Default constructor, creates a default axis.
	 */
	KPlotAxis();
	/**
	 * Constructor, constructs an axis with the label @p label.
	 */
	KPlotAxis(const QString& label);

	/**
	 * Destructor.
	 */
	~KPlotAxis() {}

	/**
	 * @return whether the axis is visible or not
	 */
	inline bool isVisible() const { return m_visible; }

	/**
	 * Sets the "visible" property of the axis.
	 */
	inline void setVisible(bool visible) { m_visible = visible; }

	/**
	 * @return whether tick labels will be drawn for this axis
	 */
	inline bool showTickLabels() const { return m_showTickLabels; }

	/**
	 * Determine whether tick labels will be drawn for this axis.
	 */
	inline void setShowTickLabels( bool b ) { m_showTickLabels = b; }

	/**
	 * Sets the axis label.
	 * Set the label to an empty string to omit the axis label.
	 * @param label a string describing the data plotted on the axis.
	 */
	inline void setLabel( const QString& label ) { m_label = label; }

	/**
	 * @return the axis label
	 */
	inline QString label() const { return m_label; }

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
	 * @param fmt the format specification character 
	 * @param fieldWidth the number of characters in the output string.
	 * If set to 0, the string will be as wide as it needs to be to fully 
	 * render the value.
	 * @param prec the number of characters following the decimal point.
	 */
	inline void setTickLabelFormat( char fmt = 'g', int fieldWidth = 0, int prec=-1) {
		m_labelFieldWidth = fieldWidth; m_labelFmt = fmt; m_labelPrec = prec; }

	/**
	 * @return the field width of the tick labels
	 */
	inline int tickLabelWidth() const { return m_labelFieldWidth; }

	/**
	 * @return the number format of the tick labels
	 */
	inline char tickLabelFmt() const { return m_labelFmt; }

	/**
	 * @return the number precision of the tick labels
	 */
	inline int tickLabelPrec() const { return m_labelPrec; }

	/**
	 * Determine the positions of major and minor tickmarks for this axis.
	 * @param x0 the minimum data coordinate of the axis.
	 * @param length the range covered by the axis, in data units.
	 * @sa majorTickMarks()
	 * @sa minorTickMarks()
	 */
	void setTickMarks( double x0, double length );

	inline QList<double>& majorTickMarks() { return m_MajorTickMarks; }
	inline QList<double>& minorTickMarks() { return m_MinorTickMarks; }

private:
	bool		m_visible;			///< Property "visible" defines if Axis is drawn or not.
	bool		m_showTickLabels;
	QString		m_label;			///< The label of the axis.
	int 		m_labelFieldWidth;	///< Field width for number labels, see QString::arg().
	char 		m_labelFmt;			///< Number format for number labels, see QString::arg().
	int 		m_labelPrec;		///< Number precision for number labels, see QString::arg().
	QList<double> m_MajorTickMarks, m_MinorTickMarks;

};

#endif // KPLOTAXIS_H
