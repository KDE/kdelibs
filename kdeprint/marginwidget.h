/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <kdeprint@swing.be>
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
	void setPageSize(float w, float h);
	void setOrientation(int orient);
	float top() const;
	void setTop(float val);
	float bottom() const;
	void setBottom(float val);
	float left() const;
	void setLeft(float val);
	float right() const;
	void setRight(float val);
	void setResolution(int dpi);
	void setDefaultMargins(float t, float b, float l, float r);
	void setCustomEnabled(bool on);
	bool isCustomEnabled() const;

protected slots:
	void slotMarginValueChanged();
	void slotMarginPreviewChanged(int, float);
	void slotCustomMarginsToggled(bool);

protected:
	void resetDefault();

private:
	MarginValueWidget	*m_top, *m_bottom, *m_left, *m_right;
	MarginPreview		*m_preview;
	QComboBox		*m_units;
	QCheckBox		*m_custom;
	bool			m_symetric, m_block;
	QValueVector<float>	m_default;
	QValueVector<float>	m_pagesize;
	bool			m_landscape;
};

#endif
