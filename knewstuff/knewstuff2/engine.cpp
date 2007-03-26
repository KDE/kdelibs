#include "knewstuff2/engine.h"

#include "knewstuff2/ui/newstuff.h"
#include "knewstuff2/ui/uploaddialog.h"
#include "knewstuff2/ui/providerdialog.h"

#include "knewstuff2/core/entryhandler.h" // tmp

#include <kapplication.h> // kapp-ptr

#include <kdebug.h>

using namespace KNS;

Engine::Engine()
: DxsEngine()
{
	m_command = none;
	m_uploaddialog = NULL;
	m_downloaddialog = NULL;
	setDxsPolicy(DxsEngine::DxsNever); // FIXME: until KIO/cDXS gets fixed!
	m_entry = NULL;
}

Engine::~Engine()
{
}

KNS::Entry::List Engine::downloadDialogModal()
{
	return KNS::Entry::List();
}

KNS::Entry *Engine::uploadDialogModal(QString file)
{
	m_uploadfile = file;
	m_entry = NULL;

	m_command = upload;

	connect(this,
		SIGNAL(signalProviderLoaded(KNS::Provider*)),
		SLOT(slotProviderLoaded(KNS::Provider*)));
	connect(this,
		SIGNAL(signalProvidersFinished()),
		SLOT(slotProvidersFinished()));
	connect(this,
		SIGNAL(signalProvidersFailed()),
		SLOT(slotProvidersFailed()));

	start();

	while(m_command == upload)
	{
		kapp->processEvents();
	}

	return m_entry;
}

void Engine::downloadDialog()
{
	kDebug(550) << "Engine: downloadDialog" << endl;

	if(m_downloaddialog)
	{
		// ???
	}

	m_downloaddialog = new DownloadDialog(0);
	m_downloaddialog->setEngine(this);
	m_downloaddialog->show();

	connect(this,
		SIGNAL(signalProviderLoaded(KNS::Provider*)),
		SLOT(slotProviderLoaded(KNS::Provider*)));
	connect(this,
		SIGNAL(signalProvidersFailed()),
		SLOT(slotProvidersFailed()));
	connect(this,
		SIGNAL(signalEntryLoaded(KNS::Entry*)),
		SLOT(slotEntryLoaded(KNS::Entry*)));
	connect(this,
		SIGNAL(signalEntriesFailed()),
		SLOT(slotEntriesFailed()));
	connect(this,
		SIGNAL(signalEntriesFinished()),
		SLOT(slotEntriesFinished()));

	m_command = download;
	start();
}

void Engine::uploadDialog()
{
	kDebug(550) << "Engine: uploadDialog" << endl;

	if(m_uploaddialog)
	{
		// ???
	}

	m_uploaddialog = new UploadDialog(0);
//	m_uploaddialog->setEngine(this);
	m_uploaddialog->show();

//	connect(this,
//		SIGNAL(signalProviderLoaded(KNS::Provider*)),
//		SLOT(slotProviderLoaded(KNS::Provider*)));
//	connect(this,
//		SIGNAL(signalProvidersFailed()),
//		SLOT(slotProvidersFailed()));

	m_command = upload;
	start();
}

void Engine::slotProviderLoaded(KNS::Provider *provider)
{
	kDebug(550) << "Engine: slotProviderLoaded" << endl;

	if(m_command == download)
	{
		this->loadEntries(provider);
	}
	else if(m_command == upload)
	{
		// FIXME: inject into upload dialog
		// FIXME: dialog could do this by itself!

		// FIXME: for the modal dialog, do nothing here
		// ... and wait for slotProvidersFinished()
		m_providers.append(provider);
	}
	else
	{
		kError(550) << "Engine: invalid command" << endl;
	}
}

void Engine::slotProvidersFailed()
{
	kDebug(550) << "Engine: slotProvidersFailed" << endl;

	m_command = none;
}

void Engine::slotEntryLoaded(KNS::Entry *entry)
{
	kDebug(550) << "Engine: slotEntryLoaded" << endl;

	//m_downloaddialog->slotEntries(...);
	m_downloaddialog->addEntry(entry);
}

void Engine::slotEntriesFailed()
{
	kDebug(550) << "Engine: slotEntriesFailed" << endl;
}

void Engine::slotEntryUploaded()
{
	kDebug(550) << "Engine: slotEntryUploaded" << endl;

	m_command = none;

	//m_entry = ...; // FIXME: where do we get it from now?
	// FIXME: we cannot assign it earlier, probably need m_delayedentry
}

void Engine::slotEntryFailed()
{
	kDebug(550) << "Engine: slotEntryFailed" << endl;

	m_command = none;
}

void Engine::slotProvidersFinished()
{
	kDebug(550) << "Engine: slotProvidersFinished" << endl;

	int ret;

	//Provider *fakeprovider = new Provider();
	//fakeprovider->setName(QString("Fake Provider"));
	//fakeprovider->setUploadUrl(KUrl("http://localhost/dav/"));
	//fakeprovider->setUploadUrl(KUrl("webdav://localhost/uploads/"));

	ProviderDialog provdialog(0);
	for(Provider::List::Iterator it = m_providers.begin(); it != m_providers.end(); it++)
	{
		Provider *provider = (*it);
		provdialog.addProvider(provider);
	}
	//provdialog.addProvider(fakeprovider);
	ret = provdialog.exec();
	if(ret == QDialog::Rejected)
	{
		m_command = none;
		return;
	}

	KNS::Provider *provider = provdialog.provider();

	UploadDialog uploaddialog(0);
	uploaddialog.setPayloadFile(KUrl(m_uploadfile));
	ret = uploaddialog.exec();
	if(ret == QDialog::Rejected)
	{
		m_command = none;
		return;
	}

	Entry *entry = uploaddialog.entry();
	entry->setPayload(m_uploadfile);
	if(!entry)
	{
		m_command = none;
		return;
	}

	EntryHandler eh(*entry);
	QDomElement xml = eh.entryXML();
	QByteArray ar;
	QTextStream txt(&ar);
	txt << xml;
	kDebug(550) << "Upload: " << QString(ar) << endl;

	connect(this,
		SIGNAL(signalEntryUploaded()),
		SLOT(slotEntryUploaded()));
	connect(this,
		SIGNAL(signalEntryFailed()),
		SLOT(slotEntryFailed()));

	uploadEntry(provider, entry);
}

void Engine::slotEntriesFinished()
{
	kDebug(550) << "Engine: slotEntriesFinished" << endl;

	m_downloaddialog->refresh();
}

#include "engine.moc"
