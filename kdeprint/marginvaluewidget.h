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

#ifndef MARGINVALUEWIDGET_H
#define MARGINVALUEWIDGET_H

#include <knuminput.h>

class MarginValueWidget : public KDoubleNumInput
{
	Q_OBJECT
public:
	enum Mode { Pixels = 0, IN, CM, MM };
	MarginValueWidget(KNumInput *below, double value = 18.0, QWidget *parent = 0, const char *name = 0);

	int margin();
	int resolution() const;
	void setResolution(int dpi);

public slots:
	void setMode(int);
	void setMargin(int);

signals:
	void marginChanged(int);

protected slots:
	void slotValueChanged(double);

protected:
	int toPixel(double value, int mode);
	double toValue(int pix, int mode);

private:
	int		m_mode;
	double	m_dpi;
	bool	m_block;
};

#endif
