#include "dxsengine.h"

#include "dxs.h"

using namespace KNS;

DxsEngine::DxsEngine()
: CoreEngine()
{
	m_dxs = NULL;
}

DxsEngine::~DxsEngine()
{
	delete m_dxs;
}

void DxsEngine::loadEntries(Provider *provider)
{
	if(!m_dxs)
	{
		m_dxs = new Dxs();
	}

	// Ensure that the provider offers DXS at all
	if(provider->webService().isValid())
	{
		m_dxs->setEndpoint(provider->webService());
	}
	else
	{
		CoreEngine::loadEntries(provider);
		return;
	}

	// FIXME: load all categories first, then feeds second
	m_dxs->call_entries(QString(), QString());

	connect(m_dxs,
		SIGNAL(signalEntries(KNS::Entry::List*)),
		SLOT(slotEntriesLoaded(KNS::Entry::List*)));
	connect(m_dxs,
		SIGNAL(signalFault()),
		SLOT(slotEntriesFailed()));
	// FIXME: which one of signalFault()/signalError()? Or both?
}

void DxsEngine::slotEntriesLoaded(KNS::Entry::List *list)
{
	// FIXME: we circumvent the cache now...
	for(Entry::List::Iterator it = list->begin(); it != list->end(); it++)
	{
		Entry *entry = (*it);
		emit signalEntryLoaded(entry);
	}
}

void DxsEngine::slotEntriesFailed()
{
	emit signalEntriesFailed();
}

#include "dxsengine.moc"
