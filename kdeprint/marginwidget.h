/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <goffioul@imec.be>
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

#ifndef MARGINWIDGET_H
#define MARGINWIDGET_H

#include <qwidget.h>
#include <qvaluevector.h>

class MarginValueWidget;
class MarginPreview;
class QCheckBox;
class QComboBox;

class MarginWidget : public QWidget
{
	Q_OBJECT
public:
	MarginWidget(QWidget *parent = 0, const char *name = 0, bool allowMetricUnit = true);
	~MarginWidget();

	void setSymetricMargins(bool on);
	void setPageSize(int w, int h);
	void setOrientation(int orient);
	int top() const;
	void setTop(int val);
	int bottom() const;
	void setBottom(int val);
	int left() const;
	void setLeft(int val);
	int right() const;
	void setRight(int val);
	void setResolution(int dpi);
	void setDefaultMargins(int t, int b, int l, int r);
	void setCustomEnabled(bool on);
	bool isCustomEnabled() const;

protected slots:
	void slotMarginValueChanged();
	void slotMarginPreviewChanged(int, int);
	void slotCustomMarginsToggled(bool);

protected:
	void resetDefault();

private:
	MarginValueWidget	*m_top, *m_bottom, *m_left, *m_right;
	MarginPreview		*m_preview;
	QComboBox		*m_units;
	QCheckBox		*m_custom;
	bool			m_symetric, m_block;
	QValueVector<int>	m_default;
	QSize			m_pagesize;
	bool			m_landscape;
};

#endif
