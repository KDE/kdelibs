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

#include "driver.h"
#include "driveritem.h"

#include <qfile.h>
#include <qstringlist.h>
#include <stdlib.h>
#include <math.h>

/******************
 * DrBase members *
 ******************/

DrBase::DrBase()
: m_type(DrBase::Base), m_conflict(false)
{
}

DrBase::~DrBase()
{
}

QString DrBase::valueText()
{
	return QString::null;
}

QString DrBase::prettyText()
{
	return valueText();
}

void DrBase::setValueText(const QString&)
{
}

DriverItem* DrBase::createItem(DriverItem *parent)
{
	return new DriverItem(parent, this);
}

void DrBase::setOptions(const QMap<QString,QString>& opts)
{
	if (opts.contains(name())) setValueText(opts[name()]);
}

void DrBase::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	QString	val = valueText();
	if (incldef || get("default") != val)
		opts[name()] = val;
}

/******************
 * DrMain members *
 ******************/

DrMain::DrMain()
: DrGroup()
{
	m_type = DrBase::Main;
	m_constraints.setAutoDelete(true);
	m_pagesizes.setAutoDelete(true);
}

DrMain::~DrMain()
{
	// remove a possible temporary file
	if (has("temporary"))
		QFile::remove(get("temporary"));
}

DriverItem* DrMain::createTreeView(QListView *parent)
{
	DriverItem	*root = new DriverItem(parent, this);
	createTree(root);
	return root;
}

int DrMain::checkConstraints()
{
	int 	result(0);
	clearConflict();
	QPtrListIterator<DrConstraint>	it(m_constraints);
	for (;it.current();++it)
		if (it.current()->check(this))
			result++;
	return result;
}

void DrMain::addPageSize(DrPageSize *ps)
{
	m_pagesizes.insert(ps->name(),ps);
}

void DrMain::removeOptionGlobally(const QString& name)
{
	DrGroup	*grp(0);
	DrBase	*opt = findOption(name, &grp);

	if (opt && grp)
	{
		grp->removeOption(name);
		if (grp->isEmpty())
			removeGroup(grp);
	}
}

void DrMain::removeGroupGlobally(DrGroup *grp)
{
	DrGroup	*parent(0);
	if (findGroup(grp, &parent) && parent)
	{
		parent->removeGroup(grp);
		if (parent->isEmpty() && parent != this)
			removeGroupGlobally(parent);
	}
}

/*******************
 * DrGroup members *
 *******************/

DrGroup::DrGroup()
: DrBase()
{
	m_type = DrBase::Group;

	m_subgroups.setAutoDelete(true);
	m_options.setAutoDelete(true);
}

DrGroup::~DrGroup()
{
}

DriverItem* DrGroup::createItem(DriverItem *parent)
{
	DriverItem	*item = DrBase::createItem(parent);
	createTree(item);
	return item;
}

void DrGroup::createTree(DriverItem *parent)
{
	QPtrListIterator<DrGroup>	lit(m_subgroups);
	for (lit.toLast();lit.current();--lit)
		lit.current()->createItem(parent);

	QDictIterator<DrBase>	dit(m_options);
	for (;dit.current();++dit)
		dit.current()->createItem(parent);
}

DrBase* DrGroup::findOption(const QString& name, DrGroup **parentGroup)
{
	DrBase	*opt = m_options.find(name);
	if (!opt)
	{
		QPtrListIterator<DrGroup>	it(m_subgroups);
		for (;it.current() && !opt; ++it)
			opt = it.current()->findOption(name, parentGroup);
	}
	else if (parentGroup)
		*parentGroup = this;
	return opt;
}

DrGroup* DrGroup::findGroup(DrGroup *grp, DrGroup ** parentGroup)
{
	DrGroup	*group = (m_subgroups.findRef(grp) == -1 ? 0 : grp);
	if (!group)
	{
		QPtrListIterator<DrGroup>	it(m_subgroups);
		for (;it.current() && !group; ++it)
			group = it.current()->findGroup(grp, parentGroup);
	}
	else if (parentGroup)
		*parentGroup = this;
	return group;
}

void DrGroup::clearConflict()
{
	QDictIterator<DrBase>	dit(m_options);
	for (;dit.current();++dit)
		dit.current()->setConflict(false);

	QPtrListIterator<DrGroup>	lit(m_subgroups);
	for (;lit.current();++lit)
		lit.current()->clearConflict();
}

void DrGroup::setOptions(const QMap<QString,QString>& opts)
{
	QDictIterator<DrBase>	dit(m_options);
	for (;dit.current();++dit)
		dit.current()->setOptions(opts);

	QPtrListIterator<DrGroup>	lit(m_subgroups);
	for (;lit.current();++lit)
		lit.current()->setOptions(opts);
}

void DrGroup::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	QDictIterator<DrBase>	dit(m_options);
	for (;dit.current();++dit)
		dit.current()->getOptions(opts,incldef);

	QPtrListIterator<DrGroup>	lit(m_subgroups);
	for (;lit.current();++lit)
		lit.current()->getOptions(opts,incldef);
}

/**************************
 * DrStringOption members *
 **************************/

DrStringOption::DrStringOption()
: DrBase()
{
	m_type = DrBase::String;
}

DrStringOption::~DrStringOption()
{
}

QString DrStringOption::valueText()
{
	return m_value;
}

void DrStringOption::setValueText(const QString& s)
{
	m_value = s;
}

/***************************
 * DrIntegerOption members *
 ***************************/

DrIntegerOption::DrIntegerOption()
: DrBase()
{
	m_type = DrBase::Integer;
	m_value = 0;
	set("minval","0");
	set("maxval","10");
}

DrIntegerOption::~DrIntegerOption()
{
}

QString DrIntegerOption::valueText()
{
	QString	s = QString::number(m_value);
	return s;
}

void DrIntegerOption::setValueText(const QString& s)
{
	m_value = s.toInt();
}

QString DrIntegerOption::fixedVal()
{
	QStringList	vals = QStringList::split("|", get("fixedvals"), false);
	if (vals.count() == 0)
		return valueText();
	int	d(0);
	QString	val;
	for (QStringList::Iterator it=vals.begin(); it!=vals.end(); ++it)
	{
		int	thisVal = (*it).toInt();
		if (val.isEmpty() || abs(thisVal - m_value) < d)
		{
			d = abs(thisVal - m_value);
			val = *it;
		}
	}
	if (val.isEmpty())
		return valueText();
	else
		return val;
}

/*************************
 * DrFloatOption members *
 *************************/

DrFloatOption::DrFloatOption()
: DrBase()
{
	m_type = DrBase::Float;
	m_value = 0.0;
	set("minval","0.0");
	set("maxval","1.0");
}

DrFloatOption::~DrFloatOption()
{
}

QString DrFloatOption::valueText()
{
	QString	s = QString::number(m_value,'f',3);
	return s;
}

void DrFloatOption::setValueText(const QString& s)
{
	m_value = s.toFloat();
}

QString DrFloatOption::fixedVal()
{
	QStringList	vals = QStringList::split("|", get("fixedvals"), false);
	if (vals.count() == 0)
		return valueText();
	float	d(0);
	QString	val;
	for (QStringList::Iterator it=vals.begin(); it!=vals.end(); ++it)
	{
		float	thisVal = (*it).toFloat();
		if (val.isEmpty() || fabs(thisVal - m_value) < d)
		{
			d = fabs(thisVal - m_value);
			val = *it;
		}
	}
	if (val.isEmpty())
		return valueText();
	else
		return val;
}

/************************
 * DrListOption members *
 ************************/

DrListOption::DrListOption()
: DrBase()
{
	m_type = DrBase::List;

	m_choices.setAutoDelete(true);
	m_current = 0;
}

DrListOption::~DrListOption()
{
}

QString DrListOption::valueText()
{
	QString	s = (m_current ? m_current->name() : QString::null);
	return s;
}

QString DrListOption::prettyText()
{
	if (m_current)
		return m_current->get("text");
	else
		return QString::null;
}

void DrListOption::setValueText(const QString& s)
{
	m_current = findChoice(s);
}

DrBase* DrListOption::findChoice(const QString& txt)
{
	QPtrListIterator<DrBase>	it(m_choices);
	for (;it.current();++it)
		if (it.current()->name() == txt)
			return it.current();
	return NULL;
}

/************************
 * DrConstraint members *
 ************************/

DrConstraint::DrConstraint(const QString& o1, const QString& o2, const QString& c1, const QString& c2)
: m_opt1(o1), m_opt2(o2), m_choice1(c1), m_choice2(c2), m_option1(0), m_option2(0)
{
}

bool DrConstraint::check(DrMain *driver)
{
	if (!m_option1) m_option1 = (DrListOption*)driver->findOption(m_opt1);
	if (!m_option2) m_option2 = (DrListOption*)driver->findOption(m_opt2);
	if (m_option1 && m_option2 && m_option1->currentChoice() && m_option2->currentChoice())
	{
		bool	f1(false), f2(false);
		QString	c1(m_option1->currentChoice()->name()), c2(m_option2->currentChoice()->name());
		// check choices
		if (m_choice1.isEmpty())
			f1 = (c1 != "None" && c1 != "Off" && c1 != "False");
		else
			f1 = (c1 == m_choice1);
		if (m_choice2.isEmpty())
			f2 = (c2 != "None" && c2 != "Off" && c2 != "False");
		else
			f2 = (c2 == m_choice2);
		// tag options
		QString	s((f1 && f2 ? "1" : "0"));
		if (!m_option1->conflict()) m_option1->setConflict(f1 && f2);
		if (!m_option2->conflict()) m_option2->setConflict(f1 && f2);
		// return value
		return (f1 && f2);
	}
	return false;
}

/**********************
 * DrPageSize members *
 **********************/

DrPageSize::DrPageSize(const QString& s,int w, int h, int ml, int mb, int mr, int mt)
: m_name(s), m_pagesize(w,h), m_pagerect(ml,mb,mr-ml+1,mt-mb+1)
{
}
