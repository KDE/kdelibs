/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001,2002 Michael Goffioul <goffioul@imec.be>
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

#ifndef LPRSETTINGS_H
#define LPRSETTINGS_H

#include <qobject.h>
#include "kpreloadobject.h"

class LprSettings : public QObject, public KPReloadObject
{
public:
	LprSettings(QObject *parent = 0, const char *name = 0);
	~LprSettings();

	static LprSettings* self();

	enum Mode { LPR, LPRng };
	Mode mode() const		{ return m_mode; }
	void setMode(Mode m)	{ m_mode = m; }
	QString printcapFile();
	bool isLocalPrintcap()		{ return m_local; }
	QString baseSpoolDir()		{ return m_spooldir; }

protected:
	void init();
	void reload();
	void configChanged();

private:
	static LprSettings*	m_self;
	Mode	m_mode;
	QString	m_printcapfile;
	bool	m_local;
	QString	m_spooldir;
};

#endif
