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

#include "marginwidget.h"

#include <math.h>

MarginWidget::MarginWidget(KNumInput *below, double value, QWidget *parent, const char *name)
: KDoubleNumInput(below, value, parent, name)
{
	m_mode = Pixels;
	setFormat("%.0f");
	m_dpi = 72.0;
	setMode(m_mode);
	setRange(0, 999, 1, false);
	connect(this, SIGNAL(valueChanged(double)), SLOT(slotValueChanged(double)));
}

int MarginWidget::margin()
{
	return toPixel(value(), m_mode);
}

void MarginWidget::setMargin(int m)
{
	double	v = toValue(m, m_mode);
	setValue(v);
}

int MarginWidget::toPixel(double value, int mode)
{
	switch (mode)
	{
		default:
		case Pixels: return (int)value;
		case IN: return (int)(rint(value * m_dpi));
		case CM: return (int)(rint(value * m_dpi / 2.54));
	}
}

double MarginWidget::toValue(int pix, int mode)
{
	switch (mode)
	{
		default:
		case Pixels: return (double)pix;
		case IN: return (double(pix) / m_dpi);
		case CM: return (double(pix) * 2.54 / m_dpi);
	}
}

void MarginWidget::slotValueChanged(double value)
{
	emit marginChanged(margin());
}

void MarginWidget::setMode(int m)
{
	if (m != m_mode)
	{
		int	p = margin();
		m_mode = m;
		double v = toValue(p, m);
		if (m == Pixels)
		{
			setFormat("%.0f");
			setRange(0.0, 999.0, 1.0, false);
		}
		else
		{
			setFormat("%.3f");
			setRange(0.0, 999.0, 0.01, false);
		}
		setValue(v);
	}
}

#include "marginwidget.moc"
