/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#ifndef DRIVER_H
#define DRIVER_H

#if !defined( _KDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a KDEPrint developer
#endif

#include <qstring.h>
#include <qmap.h>
#include <qrect.h>
#include <qsize.h>
#include <QHash>
#include <QList>

#include <kdelibs_export.h>

class DriverItem;
class Q3ListView;

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

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrBase
{
public:
	enum Type { Base = 0, Main, ChoiceGroup, Group, String, Integer, Float, List, Boolean };

	DrBase();
	virtual ~DrBase();

	Type type() const 					{ return m_type; }
	bool isOption() const 					{ return (m_type >= DrBase::String); }

	QString get(const QString& key) const 		{ return m_map[key]; }
	void set(const QString& key, const QString& val)	{ m_map[key] = val; }
	bool has(const QString& key) const 			{ return m_map.contains(key); }
	const QString& name() const				{ return m_name; }
	void setName(const QString& s)				{ m_name = s; }
	bool conflict() const 					{ return m_conflict; }
	void setConflict(bool on)				{ m_conflict = on; }

	virtual QString valueText();
	virtual QString prettyText();
	virtual void setValueText(const QString&);
	virtual DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
	virtual void setOptions(const QMap<QString,QString>& opts);
	virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	virtual DrBase* clone();

protected:
	QMap<QString,QString>	m_map;
	QString			m_name;		// used as a search key, better to have defined directly
	Type			m_type;
	bool			m_conflict;
};

/**********************
 * Option group class *
 **********************/

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrGroup : public DrBase
{
public:
	DrGroup();
	~DrGroup();

	void addOption(DrBase *opt);
	void addGroup(DrGroup *grp);
	void addObject(DrBase *optgrp);
	void clearConflict();
	void removeOption(const QString& name);
	void removeGroup(DrGroup *grp);
	bool isEmpty();

	virtual DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
	DrBase* findOption(const QString& name, DrGroup **parentGroup = 0);
	DrGroup* findGroup(DrGroup *grp, DrGroup **parentGroup = 0);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	DrBase* clone();

	const QList<DrGroup*>& groups()	{ return m_subgroups; }
	const QList<DrBase*>& options()	{ return m_listoptions; }

	static QString groupForOption( const QString& optname );

protected:
	void createTree(DriverItem *parent);
	void flattenGroup(QMap<QString, DrBase*>&, int&);

protected:
	QList<DrGroup*>         m_subgroups;
	QHash<QString, DrBase*> m_options;
	QList<DrBase*>	        m_listoptions;	// keep track of order of appearance
};

/*********************
 * Main driver class *
 *********************/

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrMain : public DrGroup
{
public:
	DrMain();
	~DrMain();

	DriverItem* createTreeView(Q3ListView *parent);
	void addConstraint(DrConstraint *c)		{ m_constraints.append(c); }
	int checkConstraints();
	DrPageSize* findPageSize(const QString& name)	{ return m_pagesizes.value(name); }
	void addPageSize(DrPageSize *sz);
	void removeOptionGlobally(const QString& name);
	void removeGroupGlobally(DrGroup *grp);
	QMap<QString, DrBase*> flatten();
	DrMain* cloneDriver();

protected:
	QList<DrConstraint*>	    m_constraints;
	QHash<QString, DrPageSize*> m_pagesizes;
};

/**********************************************************
 * Choice group class: a choice that involve a sub-option *
 **********************************************************/

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class DrChoiceGroup : public DrGroup
{
public:
	DrChoiceGroup();
	~DrChoiceGroup();

	DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
};

/***********************
 * String option class *
 ***********************/

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrStringOption : public DrBase
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

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrIntegerOption : public DrBase
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

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrFloatOption : public DrBase
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

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrListOption : public DrBase
{
public:
	DrListOption();
	~DrListOption();

	void addChoice(DrBase *ch)	{ m_choices.append(ch); }
	const QList<DrBase*>& choices()	{ return m_choices; }
	DrBase* currentChoice() const 	{ return m_current; }
	DrBase* findChoice(const QString& txt);
	void setChoice(int choicenum);

	virtual QString valueText();
	virtual QString prettyText();
	virtual void setValueText(const QString& s);
	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
	DrBase* clone();

protected:
	QList<DrBase*>	m_choices;
	DrBase		*m_current;
};

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class KDEPRINT_EXPORT DrBooleanOption : public DrListOption
{
	/* just an overloaded class, with different type */
public:
	DrBooleanOption() : DrListOption() { m_type = DrBase::Boolean; }
	~DrBooleanOption() {}
};

/********************
 * Constraint class *
 ********************/

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class DrConstraint
{
public:
	DrConstraint(const QString& o1, const QString& o2, const QString& c1 = QString::null, const QString& c2 = QString::null);
	DrConstraint(const DrConstraint&);

	bool check(DrMain*);

protected:
	QString		m_opt1, m_opt2;
	QString		m_choice1, m_choice2;
	DrListOption	*m_option1, *m_option2;
};

/*******************
 * Page Size class *
 *******************/

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class DrPageSize
{
public:
	DrPageSize(const QString& s, float width, float height, float left, float bottom, float right, float top);
	DrPageSize(const DrPageSize&);

	/**
	 * All dimensions are int dot: 1/72th of an inch ( PostScript ).
	 * When rounded, the rounding is made safely: upward for a margin,
	 * downward for a page size.
	 */
	float pageWidth() const    { return m_width; }
	float pageHeight() const   { return m_height; }
	float leftMargin() const   { return m_left; }
	float rightMargin() const  { return m_right; }
	float topMargin() const    { return m_top; }
	float bottomMargin() const { return m_bottom; }
	QString pageName() const   { return m_name; }

	QSize pageSize() const;
	QRect pageRect() const;
	QSize margins() const;

protected:
	QString	m_name;
	float m_width, m_height, m_left, m_bottom, m_right, m_top;
};

#endif
