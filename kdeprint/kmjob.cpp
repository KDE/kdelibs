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

#include "kmjob.h"

#include <klocale.h>

KMJob::KMJob()
: KMObject(), m_ID(-1), m_state(KMJob::Error), m_size(0), m_type(KMJob::System)
{
}

KMJob::KMJob(const KMJob& j)
: KMObject(), m_ID(-1), m_state(KMJob::Error), m_size(0), m_type(KMJob::System)
{
	copy(j);
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

	setDiscarded(false);
}

QString KMJob::pixmap()
{
	// special case
	if (m_type == KMJob::Threaded)
		return QString::fromLatin1("exec");

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
		default:
			str = i18n("Unknown State", "Unknown");
			break;
	}
	return str;
}
