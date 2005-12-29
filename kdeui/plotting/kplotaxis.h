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

#include <kdemacros.h>

/**
 * @short Axis for KPlotWidget
 *
 * Contains all data for drawing an axis including format specification axis labels.
 *
 * @author Andreas Nicolai
 * @version 1.0
 */
class KDE_EXPORT KPlotAxis {
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
	bool isVisible() const { return m_visible; }

	/**
	 * Sets the "visible" property of the axis.
	 */
	void setVisible(bool visible) { m_visible = visible; }

	/**
	 * Shows the axis (axis will be shown at next update of plot widget).
	 */
	void show() { m_visible = true; }

	/**
	 * Hides the axis (axis will be hidden at next update of plot widget).
	 */
	void hide() { m_visible = false; }

	/**
	 * Sets the axis label.
	 * Set the label to an empty string to omit the axis label.
	 * @param label a string describing the data plotted on the axis.
	 */
	void setLabel( const QString& label ) { m_label = label; }

	/**
	 * @return the axis label
	 */
	QString label() const { return m_label; }

	/**
	 * Set the number format for the tick labels, see QString::arg() for
	 * description of arguments.
         */
	void setLabelFormat(int fieldWidth, char fmt = 'g', int prec=-1) {
		m_labelFieldWidth = fieldWidth; m_labelFmt = fmt; m_labelPrec = prec; }

	/**
	 * @return the field width of the tick labels
	 */
	int labelFieldWidth() const { return m_labelFieldWidth; }

	/**
	 * @return the number format of the tick labels
	 */
	char labelFmt() const { return m_labelFmt; }

	/**
	 * @return the number precision of the tick labels
	 */
	int labelPrec() const { return m_labelPrec; }

private:
	bool		m_visible;			///< Property "visible" defines if Axis is drawn or not.
	QString		m_label;			///< The label of the axis.
	int 		m_labelFieldWidth;	///< Field width for number labels, see QString::arg().
	char 		m_labelFmt;			///< Number format for number labels, see QString::arg().
	int 		m_labelPrec;		///< Number precision for number labels, see QString::arg().
};

#endif // KPLOTAXIS_H
