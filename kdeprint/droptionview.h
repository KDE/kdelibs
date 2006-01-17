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

#ifndef DROPTIONVIEW_H
#define DROPTIONVIEW_H

#include <qwidget.h>
#include <q3groupbox.h>
#include <qstringlist.h>

class QLineEdit;
class QSlider;
class QLabel;
class KListBox;
class Q3ListBoxItem;
class Q3VButtonGroup;
class QStackedWidget;
class Q3ListViewItem;
class DrBase;
class DriverItem;

class OptionBaseView : public QWidget
{
	Q_OBJECT
public:
	OptionBaseView(QWidget *parent = 0);
	virtual void setOption(DrBase*);
	virtual void setValue(const QString&);

Q_SIGNALS:
	void valueChanged(const QString&);

protected:
	bool	blockSS;
};

class OptionNumericView : public OptionBaseView
{
	Q_OBJECT
public:
	OptionNumericView(QWidget *parent = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

protected Q_SLOTS:
	void slotSliderChanged(int);
	void slotEditChanged(const QString&);

private:
	QLineEdit	*m_edit;
	QSlider		*m_slider;
	QLabel		*m_minval, *m_maxval;
	bool		m_integer;
};

class OptionStringView : public OptionBaseView
{
public:
	OptionStringView(QWidget *parent = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

private:
	QLineEdit	*m_edit;
};

class OptionListView : public OptionBaseView
{
	Q_OBJECT
public:
	OptionListView(QWidget *parent = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

protected Q_SLOTS:
	void slotSelectionChanged();

private:
	KListBox	*m_list;
	QStringList	m_choices;
};

class OptionBooleanView : public OptionBaseView
{
	Q_OBJECT
public:
	OptionBooleanView(QWidget *parent = 0);
	void setOption(DrBase *opt);
	void setValue(const QString& val);

protected Q_SLOTS:
	void slotSelected(int);

private:
	Q3VButtonGroup	*m_group;
	QStringList	m_choices;
};

class DrOptionView : public Q3GroupBox
{
	Q_OBJECT
public:
	DrOptionView(QWidget *parent = 0, const char *name = 0);
	void setAllowFixed(bool on) 	{ m_allowfixed = on; }

Q_SIGNALS:
	void changed();

public Q_SLOTS:
	void slotValueChanged(const QString&);
	void slotItemSelected(Q3ListViewItem*);

private:
	QStackedWidget	*m_stack;
	DriverItem	*m_item;
	bool		m_block;
	bool		m_allowfixed;
};

#endif
