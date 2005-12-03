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

#include "driver.h"
#include "driveritem.h"

#include <qfile.h>
#include <qstringlist.h>
#include <kdebug.h>
#include <klocale.h>
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

DriverItem* DrBase::createItem(DriverItem *parent, DriverItem *after)
{
	return new DriverItem(parent, after, this);
}

void DrBase::setOptions(const QMap<QString,QString>& opts)
{
	if (opts.contains(name())) setValueText(opts[name()]);
}

void DrBase::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	QString	val = valueText();
	if ( incldef || get( "persistent" ) == "1" || get("default") != val )
		opts[name()] = val;
}

DrBase* DrBase::clone()
{
	DrBase	*opt(0);
	switch (type())
	{
		case Main: opt = new DrMain; break;
		case Group: opt = new DrGroup; break;
		case String: opt = new DrStringOption; break;
		case Integer: opt = new DrIntegerOption; break;
		case Float: opt = new DrFloatOption; break;
		case List: opt = new DrListOption; break;
		case Boolean: opt = new DrBooleanOption; break;
		default: opt = new DrBase; break;
	}
	opt->m_map = m_map;
	opt->m_name = m_name;
	opt->m_conflict = m_conflict;
	opt->setValueText(valueText());

	return opt;
}

/******************
 * DrMain members *
 ******************/

DrMain::DrMain()
: DrGroup()
{
        m_type = DrBase::Main;
}

DrMain::~DrMain()
{
	qDeleteAll(m_constraints);
	qDeleteAll(m_pagesizes);
    
	// remove a possible temporary file
	if (has("temporary"))
		QFile::remove(get("temporary"));
}

DriverItem* DrMain::createTreeView(Q3ListView *parent)
{
	DriverItem	*root = new DriverItem(parent, this);
	createTree(root);
	return root;
}

int DrMain::checkConstraints()
{
	int 	result(0);
	clearConflict();
	foreach (DrConstraint* constraint, m_constraints)
		if (constraint->check(this))
			result++;
	return result;
}

void DrMain::addPageSize(DrPageSize *ps)
{
	m_pagesizes.insert(ps->pageName(),ps);
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

QMap<QString, DrBase*> DrMain::flatten()
{
	QMap<QString, DrBase*>	optmap;
	int	index(0);
	flattenGroup(optmap, index);
	return optmap;
}

DrMain* DrMain::cloneDriver()
{
	DrMain	*driver = static_cast<DrMain*>(clone());

	foreach (DrConstraint* constraint, m_constraints)
		driver->addConstraint(new DrConstraint(*constraint));

	foreach (DrPageSize* pagesize, m_pagesizes)
		driver->addPageSize(new DrPageSize(*pagesize));

	return driver;
}

/*******************
 * DrGroup members *
 *******************/

DrGroup::DrGroup()
: DrBase()
{
	m_type = DrBase::Group;
}

DrGroup::~DrGroup()
{
	qDeleteAll(m_subgroups);
	qDeleteAll(m_listoptions);
}

void DrGroup::addOption(DrBase *opt)
{
	if (!opt->name().isEmpty())
	{
		m_options.insert(opt->name(),opt);
		m_listoptions.append(opt);
	}
}

void DrGroup::addGroup(DrGroup *grp)
{
	m_subgroups.append(grp);
}

void DrGroup::addObject(DrBase *optgrp)
{
	if (optgrp->isOption())
		addOption(optgrp);
	else if (optgrp->type() == DrBase::Group)
		addGroup(static_cast<DrGroup*>(optgrp));
}

void DrGroup::removeOption(const QString& name)
{
	if (m_options.contains(name))
	{
		DrBase	*opt = m_options.take(name);
		m_listoptions.removeAll(opt);
		delete opt;
	}
}

void DrGroup::removeGroup(DrGroup *grp)
{
	m_subgroups.removeAll(grp);
	delete grp;
}

bool DrGroup::isEmpty()
{
	return m_options.isEmpty() && m_subgroups.isEmpty();
}

DriverItem* DrGroup::createItem(DriverItem *parent, DriverItem *after)
{
	DriverItem	*item = DrBase::createItem(parent, after);
	createTree(item);
	return item;
}

void DrGroup::createTree(DriverItem *parent)
{
	DriverItem	*item(0);

	foreach (DrGroup* subgroup, m_subgroups)
		item = subgroup->createItem(parent, item);

	foreach (DrBase* option, m_listoptions)
		item = option->createItem(parent, item);
}

DrBase* DrGroup::findOption(const QString& name, DrGroup **parentGroup)
{
	DrBase	*opt(0);
	if (m_options.contains(name))
	{
		opt = m_options.value(name);
		if (parentGroup)
			*parentGroup = this;
	}
	else
	{
		QListIterator<DrGroup*>    it(m_subgroups);
		while (it.hasNext() && !opt)
			opt = it.next()->findOption(name, parentGroup);
	}
	return opt;
}

DrGroup* DrGroup::findGroup(DrGroup *grp, DrGroup ** parentGroup)
{
	DrGroup	*group(0);
	if (m_subgroups.contains(grp))
	{
		group = grp;
		if (parentGroup)
			*parentGroup = this;
	}
	else
	{
		QListIterator<DrGroup*>    it(m_subgroups);
		while (it.hasNext() && !group)
			group = it.next()->findGroup(grp, parentGroup);
	}
	return group;
}

void DrGroup::clearConflict()
{
	foreach (DrBase* option, m_options)
		option->setConflict(false);
	    
	foreach (DrGroup* subgroup, m_subgroups)
		subgroup->clearConflict();
}

void DrGroup::setOptions(const QMap<QString,QString>& opts)
{
	foreach (DrBase* option, m_options)
		option->setOptions(opts);

	foreach (DrGroup* subgroup, m_subgroups)
		subgroup->setOptions(opts);
}

void DrGroup::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	foreach (DrBase* option, m_options)
		option->getOptions(opts,incldef);
	
	foreach (DrGroup* subgroup, m_subgroups)
		subgroup->getOptions(opts,incldef);
}

void DrGroup::flattenGroup(QMap<QString, DrBase*>& optmap, int& index)
{
	foreach (DrGroup* subgroup, m_subgroups)
		subgroup->flattenGroup(optmap, index);
   
	foreach (DrBase* option, m_options)
		optmap[option->name()] = option;

	if (name().isEmpty())
		optmap[QString::fromLatin1("group%1").arg(index++)] = this;
	else
		optmap[name()] = this;

	m_subgroups.clear();
	m_options.clear();
	qDeleteAll(m_listoptions);
	m_listoptions.clear();
}

DrBase* DrGroup::clone()
{
	DrGroup	*grp = static_cast<DrGroup*>(DrBase::clone());

	foreach (DrGroup* subgroup, m_subgroups)
		grp->addGroup(static_cast<DrGroup*>(subgroup->clone()));

	foreach (DrBase* option, m_listoptions)
		grp->addOption(option->clone());
	
	return static_cast<DrBase*>(grp);
}

QString DrGroup::groupForOption( const QString& optname )
{
   QString grpname;
   if ( optname == "PageSize" ||
		 optname == "InputSlot" ||
		 optname == "ManualFeed" ||
		 optname == "MediaType" ||
		 optname == "MediaColor" ||
		 optname == "MediaWeight" ||
		 optname == "Duplex" ||
		 optname == "DoubleSided" ||
		 optname == "Copies" )
      grpname = i18n( "General" );
   else if ( optname.startsWith(  "stp" ) ||
			  optname == "Cyan" ||
			  optname == "Yellow" ||
			  optname == "Magenta" ||
			  optname == "Black" ||
			  optname == "Density" ||
			  optname == "Contrast" )
      grpname = i18n( "Adjustments" );
   else if (  optname.startsWith(  "JCL" ) )
      grpname = i18n( "JCL" );
   else
      grpname = i18n( "Others" );
	return grpname;
}

/*************************
 * DrChoiceGroup members *
 *************************/

DrChoiceGroup::DrChoiceGroup()
: DrGroup()
{
	m_type = DrBase::ChoiceGroup;
}

DrChoiceGroup::~DrChoiceGroup()
{
}

DriverItem* DrChoiceGroup::createItem(DriverItem *parent, DriverItem*)
{
	createTree(parent);
	return NULL;
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
        m_current = 0;
}

DrListOption::~DrListOption()
{
	qDeleteAll(m_choices);
}

QString DrListOption::valueText()
{
	if (m_current)
		return m_current->name();
	else
		return QString::null;
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
	if (!m_current)
	{
		bool	ok;
		int	index = s.toInt(&ok);
		if (ok)
			setChoice(index);
	}
}

DrBase* DrListOption::findChoice(const QString& txt)
{
	foreach (DrBase* choice, m_choices)
	    if (choice->name() == txt)
		return choice;
	return 0;
}

DrBase* DrListOption::clone()
{
	DrListOption	*opt = static_cast<DrListOption*>(DrBase::clone());

	foreach (DrBase* choice, m_choices)
		opt->addChoice(choice->clone());

	opt->setValueText(valueText());

	return static_cast<DrBase*>(opt);
}

void DrListOption::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	DrBase::getOptions(opts, incldef);
	if (currentChoice() && currentChoice()->type() == DrBase::ChoiceGroup)
		currentChoice()->getOptions(opts, incldef);
}

void DrListOption::setOptions(const QMap<QString,QString>& opts)
{
	DrBase::setOptions(opts);
	if (currentChoice() && currentChoice()->type() == DrBase::ChoiceGroup)
		currentChoice()->setOptions(opts);
}

DriverItem* DrListOption::createItem(DriverItem *parent, DriverItem *after)
{
	DriverItem	*item = DrBase::createItem(parent, after);
	/*if (currentChoice() && currentChoice()->type() == DrBase::ChoiceGroup)
	{
		currentChoice()->createItem(item);
	}*/
	return item;
}

void DrListOption::setChoice(int choicenum)
{
	if (choicenum >= 0 && choicenum < (int)m_choices.count())
	{
		setValueText(m_choices.at(choicenum)->name());
	}
}

/************************
 * DrConstraint members *
 ************************/

DrConstraint::DrConstraint(const QString& o1, const QString& o2, const QString& c1, const QString& c2)
: m_opt1(o1), m_opt2(o2), m_choice1(c1), m_choice2(c2), m_option1(0), m_option2(0)
{
}

DrConstraint::DrConstraint(const DrConstraint& d)
: m_opt1(d.m_opt1), m_opt2(d.m_opt2), m_choice1(d.m_choice1), m_choice2(d.m_choice2), m_option1(0), m_option2(0)
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

DrPageSize::DrPageSize(const QString& s, float width, float height, float left, float bottom, float right, float top)
: m_name(s),
  m_width( width ),
  m_height( height ),
  m_left( left ),
  m_bottom( bottom ),
  m_right( right ),
  m_top( top )
{
}

DrPageSize::DrPageSize(const DrPageSize& d)
: m_name(d.m_name),
  m_width( d.m_width ),
  m_height( d.m_height ),
  m_left( d.m_left ),
  m_bottom( d.m_bottom ),
  m_right( d.m_right ),
  m_top( d.m_top )
{
}

QSize DrPageSize::pageSize() const
{
	return QSize( ( int )m_width, ( int )m_height );
}

QRect DrPageSize::pageRect() const
{
	return QRect( ( int )( m_left+0.5 ), ( int )( m_top+0.5 ), ( int )( m_width-m_left-m_right ), ( int )( m_height-m_top-m_bottom ) );
}

QSize DrPageSize::margins() const
{
	return QSize( ( int )( m_left+0.5 ), ( int )( m_top+0.5 ) );
}
