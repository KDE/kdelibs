/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "marginvaluewidget.h"

#include <math.h>

MarginValueWidget::MarginValueWidget(KNumInput *below, double value, QWidget *parent, const char *name)
: KDoubleNumInput(below, value, parent, name)
{
	m_mode = Pixels;
	m_block = false;
	setPrecision(0);
	m_dpi = 72.0;
	setMode(m_mode);
	setRange(0, 999, 1, false);
	connect(this, SIGNAL(valueChanged(double)), SLOT(slotValueChanged(double)));
}

int MarginValueWidget::margin()
{
	return toPixel(value(), m_mode);
}

void MarginValueWidget::setMargin(int m)
{
	double	v = toValue(m, m_mode);
	setValue(v);
}

int MarginValueWidget::toPixel(double value, int mode)
{
	switch (mode)
	{
		default:
		case Pixels: return (int)value;
		case IN: return (int)(rint(value * m_dpi));
		case CM: return (int)(rint(value * m_dpi / 2.54));
		case MM: return (int)(rint(value * m_dpi / 25.4));
	}
}

double MarginValueWidget::toValue(int pix, int mode)
{
	switch (mode)
	{
		default:
		case Pixels:
			return (double)pix;
		case IN:
			return (double(pix) / m_dpi);
		case CM:
			return (double(pix) * 2.54 / m_dpi);
		case MM:
			return ( double( pix ) * 25.4 / m_dpi );
	}
}

void MarginValueWidget::slotValueChanged(double)
{
	if (!m_block)
		emit marginChanged(margin());
}

void MarginValueWidget::setMode(int m)
{
	if (m != m_mode)
	{
		m_block = true;
		int	p = margin();
		m_mode = m;
		double v = toValue(p, m);
		if (m == Pixels)
		{
			setPrecision(0);
			setRange(0.0, 999.0, 1.0, false);
		}
		else
		{
			setPrecision(3);
			setRange(0.0, 999.0, 0.01, false);
		}
		setValue(v);
		m_block = false;
	}
}

void MarginValueWidget::setResolution(int dpi)
{
	int	value = margin();
	m_dpi = dpi;
	m_block = true;
	setMargin(value);
	m_block = false;
}

int MarginValueWidget::resolution() const
{
	return (int)(m_dpi+0.5);
}

#include "marginvaluewidget.moc"
