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

#ifndef CUPSDPAGE_H
#define	CUPSDPAGE_H

#include <qwidget.h>

struct CupsdConf;

class CupsdPage : public QWidget
{
	Q_OBJECT
public:
	CupsdPage(QWidget *parent = 0, const char *name = 0);
	virtual ~CupsdPage();

	virtual bool loadConfig(CupsdConf *conf, QString& msg) = 0;
	virtual bool saveConfig(CupsdConf *conf, QString& msg) = 0;
        virtual void setInfos(CupsdConf*) {}
	
	QString pageLabel() const	{ return label_; }
	QString header() const		{ return header_; }
	QString pixmap() const		{ return pixmap_; }

protected:
	void setPageLabel(const QString& s)	{ label_ = s; }
	void setHeader(const QString& s)	{ header_ = s; }
	void setPixmap(const QString& s)	{ pixmap_ = s; }

protected:
	CupsdConf	*conf_;
	QString		label_;
	QString		header_;
	QString		pixmap_;
};

#endif
