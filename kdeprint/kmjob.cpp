#include "kmjob.h"

#include <klocale.h>

KMJob::KMJob()
: KMObject(), m_ID(-1), m_state(KMJob::Error), m_size(0)
{
}

KMJob::KMJob(const KMJob& j)
: KMObject(), m_ID(-1), m_state(KMJob::Error), m_size(0)
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

	setDiscarded(false);
}

QString KMJob::pixmap(int state)
{
	QString	str("kdeprint_job");
	switch (state)
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

QString KMJob::stateString(int state)
{
	QString	str;
	switch (state)
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
			str = i18n("Unknown");
			break;
	}
	return str;
}
