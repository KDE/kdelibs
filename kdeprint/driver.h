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

#ifndef DRIVER_H
#define DRIVER_H

#include <qstring.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qmap.h>
#include <qrect.h>
#include <qsize.h>

class DriverItem;
class QListView;

/***********************
 * Forward definitions *
 ***********************/

class DrBase;
class DrMain;
class DrGroup;
class DrConstraint;
class DrPageSize;

/*************************************
 * Base class for all driver objects *
 *************************************/

class DrBase
{
public:
	enum Type { Base = 0, Main, Group, String, Integer, Float, List, Boolean };

	DrBase();
	virtual ~DrBase();

	Type type() const 					{ return m_type; }
	bool isOption() const 					{ return (m_type > DrBase::Group); }

	const QString& get(const QString& key) const 		{ return m_map[key]; }
	void set(const QString& key, const QString& val)	{ m_map[key] = val; }
	bool has(const QString& key) const 			{ return m_map.contains(key); }
	const QString& name() const				{ return m_name; }
	void setName(const QString& s)				{ m_name = s; }
	bool conflict() const 					{ return m_conflict; }
	void setConflict(bool on)				{ m_conflict = on; }

	virtual QString valueText();
	virtual QString prettyText();
	virtual void setValueText(const QString&);
	virtual DriverItem* createItem(DriverItem *parent);
	virtual void setOptions(const QMap<QString,QString>& opts);
	virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected:
	QMap<QString,QString>	m_map;
	QString			m_name;		// used as a search key, better to have defined directly
	Type			m_type;
	bool			m_conflict;
};

/**********************
 * Option group class *
 **********************/

class DrGroup : public DrBase
{
public:
	DrGroup();
	~DrGroup();

	void addOption(DrBase *opt)	{ if (!opt->name().isEmpty()) m_options.insert(opt->name(),opt); }
	void addGroup(DrGroup *grp)	{ m_subgroups.append(grp); }
	void clearConflict();
	void removeOption(const QString& name)	{ m_options.remove(name); }
	void removeGroup(DrGroup *grp)	{ m_subgroups.removeRef(grp); }
	bool isEmpty()	{ return (m_options.count()+m_subgroups.count() == 0); }

	virtual DriverItem* createItem(DriverItem *parent);
	DrBase* findOption(const QString& name, DrGroup **parentGroup = 0);
	DrGroup* findGroup(DrGroup *grp, DrGroup **parentGroup = 0);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected:
	void createTree(DriverItem *parent);

protected:
	QPtrList<DrGroup>	m_subgroups;
	QDict<DrBase>	m_options;
};

/*********************
 * Main driver class *
 *********************/

class DrMain : public DrGroup
{
public:
	DrMain();
	~DrMain();

	DriverItem* createTreeView(QListView *parent);
	void addConstraint(DrConstraint *c)		{ m_constraints.append(c); }
	int checkConstraints();
	DrPageSize* findPageSize(const QString& name)	{ return m_pagesizes.find(name); }
	void addPageSize(DrPageSize *sz);
	void removeOptionGlobally(const QString& name);
	void removeGroupGlobally(DrGroup *grp);

protected:
	QPtrList<DrConstraint>	m_constraints;
	QDict<DrPageSize>	m_pagesizes;
};

/***********************
 * String option class *
 ***********************/

class DrStringOption : public DrBase
{
public:
	DrStringOption();
	~DrStringOption();

	virtual QString valueText();
	virtual void setValueText(const QString& s);

protected:
	QString	m_value;
};

/**********************************
 * Integer numerical option class *
 **********************************/

class DrIntegerOption : public DrBase
{
public:
	DrIntegerOption();
	~DrIntegerOption();

	virtual QString valueText();
	virtual void setValueText(const QString& s);
	QString fixedVal();

protected:
	int	m_value;
};

/********************************
 * Float numerical option class *
 ********************************/

class DrFloatOption : public DrBase
{
public:
	DrFloatOption();
	~DrFloatOption();

	virtual QString valueText();
	virtual void setValueText(const QString& s);
	QString fixedVal();

protected:
	float	m_value;
};

/***********************
 * Single choice class *
 ***********************/

class DrListOption : public DrBase
{
public:
	DrListOption();
	~DrListOption();

	void addChoice(DrBase *ch)	{ m_choices.append(ch); }
	QPtrList<DrBase>* choices()	{ return &m_choices; }
	DrBase* currentChoice() const 	{ return m_current; }
	DrBase* findChoice(const QString& txt);

	virtual QString valueText();
	virtual QString prettyText();
	virtual void setValueText(const QString& s);

protected:
	QPtrList<DrBase>	m_choices;
	DrBase		*m_current;
};

// just an overloaded class, with different type.
class DrBooleanOption : public DrListOption
{
public:
	DrBooleanOption() : DrListOption() { m_type = DrBase::Boolean; }
	~DrBooleanOption() {}
};

/********************
 * Constraint class *
 ********************/

class DrConstraint
{
public:
	DrConstraint(const QString& o1, const QString& o2, const QString& c1 = QString::null, const QString& c2 = QString::null);
	bool check(DrMain*);

protected:
	QString		m_opt1, m_opt2;
	QString		m_choice1, m_choice2;
	DrListOption	*m_option1, *m_option2;
};

/*******************
 * Page Size class *
 *******************/

class DrPageSize
{
public:
	DrPageSize(const QString& s, int w, int h, int ml, int mb, int mr, int mt);
	QRect pageRect() const 	{ return m_pagerect; }
	QSize pageSize() const 	{ return m_pagesize; }
	QSize margins() const 	{ return QSize(m_pagerect.left(),m_pagerect.top()); }
	QString name() const 	{ return m_name; }

protected:
	QString	m_name;
	QSize	m_pagesize;
	QRect	m_pagerect;
};

#endif
