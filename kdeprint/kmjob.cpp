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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kmjob.h"

#include <klocale.h>

KMJob::KMJob()
: KMObject()
{
	init();
}

KMJob::KMJob(const KMJob& j)
: KMObject()
{
	init();
	copy(j);
}

KMJob& KMJob::operator=(const KMJob& j)
{
	init();
	copy(j);
	return *this;
}

void KMJob::init()
{
	m_ID = -1;
	m_state = KMJob::Error;
	m_size = m_processedsize = 0;
	m_type = KMJob::System;
	m_pages = m_processedpages = 0;
	m_remote = false;
	m_attributes.insert(0, "");
	//m_attributes.resize(1, 0);
}

void KMJob::copy(const KMJob& j)
{
	m_ID = j.m_ID;
	m_name = j.m_name;
	m_printer = j.m_printer;
	m_owner = j.m_owner;
	m_state = j.m_state;
	m_size = j.m_size;
	m_uri = j.m_uri;
	m_type = j.m_type;
	m_pages = j.m_pages;
	m_processedsize = j.m_processedsize;
	m_processedpages = j.m_processedpages;
	m_remote = j.m_remote;
	m_attributes = j.m_attributes;

	setDiscarded(false);
}

QString KMJob::pixmap()
{
	// special case
	if (m_type == KMJob::Threaded)
		return QLatin1String("exec");

	// normal case
	QString	str("kdeprint_job");
	switch (m_state)
	{
		case KMJob::Printing:
			str.append("_process");
			break;
		case KMJob::Held:
			str.append("_stopped");
			break;
		case KMJob::Error:
			str.append("_error");
			break;
		case KMJob::Completed:
			str.append("_completed");
			break;
		case KMJob::Cancelled:
			str.append("_cancelled");
			break;
		default:
			break;
	}
	return str;
}

QString KMJob::stateString()
{
	QString	str;
	switch (m_state)
	{
		case KMJob::Printing:
			str = i18n("Processing...");
			break;
		case KMJob::Queued:
			str = i18n("Queued");
			break;
		case KMJob::Held:
			str = i18n("Held");
			break;
		case KMJob::Error:
			str = i18n("Error");
			break;
		case KMJob::Cancelled:
			str = i18n("Canceled");
			break;
		case KMJob::Aborted:
			str = i18n("Aborted");
			break;
		case KMJob::Completed:
			str = i18n("Completed");
			break;
		default:
			str = i18n("Unknown State", "Unknown");
			break;
	}
	return str;
}
