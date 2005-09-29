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

#include "lprsettings.h"
#include "kmmanager.h"
#include "kmfactory.h"

#include <kconfig.h>
#include <qfile.h>
#include <qtextstream.h>

#define LPDCONF "/etc/lpd.conf"
#define PRINTCAP "/etc/printcap"

LprSettings* LprSettings::m_self = 0;

LprSettings::LprSettings(QObject *parent, const char *name)
: QObject(parent, name), KPReloadObject(true)
{
	init();
}

LprSettings::~LprSettings()
{
	m_self = 0;
}

LprSettings* LprSettings::self()
{
	if (!m_self)
	{
		m_self = new LprSettings(KMManager::self(), "LprSettings");
	}
	return m_self;
}

void LprSettings::init()
{
	// LPR/LPRng mode
	KConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("LPR");
	QString	modestr = conf->readEntry("Mode");
	if (modestr == "LPRng")
		m_mode = LPRng;
	else if (modestr == "LPR")
		m_mode = LPR;
	else
	{
		// try to guess
		if (QFile::exists(LPDCONF))
			m_mode = LPRng;
		else
			m_mode = LPR;
	}

	// Printcap file
	m_printcapfile = QString::null;
	m_local = true;

	// Spool directory
	m_spooldir = "/var/spool/lpd";
}

QString LprSettings::printcapFile()
{
	if (m_printcapfile.isEmpty())
	{
		// default value
		m_printcapfile = PRINTCAP;
		if (m_mode == LPRng)
		{
			// look into /etc/lpd/conf file
			QFile cf(LPDCONF);
			if (cf.open(IO_ReadOnly))
			{
				QTextStream	t(&cf);
				QString	line;
				while (!t.atEnd())
				{
					line = t.readLine().stripWhiteSpace();
					if (line.startsWith("printcap_path"))
					{
						QString	filename = line.mid(14).stripWhiteSpace();
						if (filename[0] != '|')
							m_printcapfile = filename;
						else
						{
							// should download the printcap file
							// and set m_local to false
						}
					}
				}
			}
		}
	}
	return m_printcapfile;
}

QString LprSettings::defaultRemoteHost()
{
	if (m_defaultremotehost.isEmpty())
	{
		m_defaultremotehost = "localhost";
		QFile cf(LPDCONF);
		if (cf.open(IO_ReadOnly))
		{
			QTextStream	t(&cf);
			QString	line;
			while (!t.atEnd())
			{
				line = t.readLine().stripWhiteSpace();
				if (line.startsWith("default_remote_host"))
				{
					QString	hostname = line.mid(20).stripWhiteSpace();
					m_defaultremotehost = hostname;
				}
			}
		}		
	}
	return m_defaultremotehost;
}

void LprSettings::reload()
{
}

void LprSettings::configChanged()
{
	init();
}
