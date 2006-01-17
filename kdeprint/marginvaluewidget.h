/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef MARGINVALUEWIDGET_H
#define MARGINVALUEWIDGET_H

#include <knuminput.h>

class MarginValueWidget : public KDoubleNumInput
{
	Q_OBJECT
public:
	enum Mode { Pixels = 0, IN, CM, MM };
	MarginValueWidget(KNumInput *below, double value = 18.0, QWidget *parent = 0, const char *name = 0);

	float margin();
	int resolution() const;
	void setResolution(int dpi);

public Q_SLOTS:
	void setMode(int);
	void setMargin(float);

Q_SIGNALS:
	void marginChanged(float);

protected Q_SLOTS:
	void slotValueChanged(double);

protected:
	float toPixel(double value, int mode);
	double toValue(float pix, int mode);

private:
	int		m_mode;
	double	m_dpi;
	bool	m_block;
	float m_margin;
};

#endif
