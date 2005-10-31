/***************************************************************************
                          kplotaxis.cpp - An axis for the plot widget
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

#include "kplotaxis.h"

KPlotAxis::KPlotAxis() : m_visible(true), m_labelFieldWidth(0), m_labelFmt('g'),
	m_labelPrec(2)
{
}

KPlotAxis::KPlotAxis(const QString& label) : m_visible(true), m_label(label), 
	m_labelFieldWidth(0), m_labelFmt('g'), m_labelPrec(2)
{
}

