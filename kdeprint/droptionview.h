/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#ifndef DROPTIONVIEW_H
#define DROPTIONVIEW_H

#include <qwidget.h>
#include <qgroupbox.h>
#include <qstringlist.h>

class QLineEdit;
class QSlider;
class QLabel;
class KListBox;
class QListBoxItem;
class QVButtonGroup;
class QWidgetStack;
class QListViewItem;
class DrBase;
class DriverItem;

class BaseView : public QWidget
{
	Q_OBJECT
public:
	BaseView(QWidget *parent = 0, const char *name = 0);
	virtual void setOption(DrBase*);
	virtual void setValue(const QString&);

signals:
	void valueChanged(const QString&);

protected:
	bool	blockSS;
};

class NumericView : public BaseView
{
	Q_OBJECT
public:
	NumericView(QWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

protected slots:
	void slotSliderChanged(int);
	void slotEditChanged(const QString&);

private:
	QLineEdit	*m_edit;
	QSlider		*m_slider;
	QLabel		*m_minval, *m_maxval;
	bool		m_integer;
};

class StringView : public BaseView
{
public:
	StringView(QWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

private:
	QLineEdit	*m_edit;
};

class ListView : public BaseView
{
	Q_OBJECT
public:
	ListView(QWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

protected slots:
	void slotSelectionChanged();

private:
	KListBox	*m_list;
	QStringList	m_choices;
};

class BooleanView : public BaseView
{
	Q_OBJECT
public:
	BooleanView(QWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

protected slots:
	void slotSelected(int);

private:
	QVButtonGroup	*m_group;
	QStringList	m_choices;
};

class DrOptionView : public QGroupBox
{
	Q_OBJECT
public:
	DrOptionView(QWidget *parent = 0, const char *name = 0);
	QSize sizeHint() const;
	void setAllowFixed(bool on) 	{ m_allowfixed = on; }

signals:
	void changed();

public slots:
	void slotValueChanged(const QString&);
	void slotItemSelected(QListViewItem*);

private:
	QWidgetStack	*m_stack;
	DriverItem	*m_item;
	bool		m_block;
	bool		m_allowfixed;
};

#endif
