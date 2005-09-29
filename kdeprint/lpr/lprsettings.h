/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001,2002 Michael Goffioul <kdeprint@swing.be>
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

#ifndef LPRSETTINGS_H
#define LPRSETTINGS_H

#if !defined( _KDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a KDEPrint developer
#endif

#include <qobject.h>
#include <kdeprint/kpreloadobject.h>

/**
 * @internal
 * This class is internal to KDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a KDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
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
	QString defaultRemoteHost();
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
	QString m_defaultremotehost;
};

#endif
