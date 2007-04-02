#include "dxsengine.h"

#include "dxs.h"

#include <kdebug.h>

using namespace KNS;

DxsEngine::DxsEngine()
: CoreEngine()
{
	m_dxs = NULL;
	m_dxspolicy = DxsIfPossible;
}

DxsEngine::~DxsEngine()
{
	delete m_dxs;
}

void DxsEngine::setDxsPolicy(Policy policy)
{
	m_dxspolicy = policy;
}

void DxsEngine::loadEntries(Provider *provider)
{
	// Ensure that the provider offers DXS at all
	// Match DXS offerings with the engine's policy
	if(provider->webService().isValid())
	{
		if(m_dxspolicy == DxsNever)
		{
			CoreEngine::loadEntries(provider);
			return;
		}
	}
	else
	{
		if(m_dxspolicy != DxsAlways)
		{
			CoreEngine::loadEntries(provider);
			return;
		}
		else
		{
			kError(550) << "DxsEngine: DXS requested but not offered" << endl;
			return;
		}
	}

	// From here on, it's all DXS now

	if(!m_dxs)
	{
		m_dxs = new Dxs();
	}
	m_dxs->setEndpoint(provider->webService());

	// FIXME: load all categories first, then feeds second
	m_dxs->call_entries(QString(), QString());

	connect(m_dxs,
		SIGNAL(signalEntries(KNS::Entry::List)),
		SLOT(slotEntriesLoaded(KNS::Entry::List)));
	connect(m_dxs,
		SIGNAL(signalFault()),
		SLOT(slotEntriesFailed()));
	// FIXME: which one of signalFault()/signalError()? Or both?
}

void DxsEngine::slotEntriesLoaded(KNS::Entry::List list)
{
	// FIXME: we circumvent the cache now...
	for(Entry::List::Iterator it = list.begin(); it != list.end(); it++)
	{
		Entry *entry = (*it);
		// FIXME: the association to feed and provider is missing here
		emit signalEntryLoaded(entry, NULL, NULL);
	}
}

void DxsEngine::slotEntriesFailed()
{
	emit signalEntriesFailed();
}

#include "dxsengine.moc"
