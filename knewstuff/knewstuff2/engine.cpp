/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "knewstuff2/engine.h"

#include "knewstuff2/ui/downloaddialog.h"
#include "knewstuff2/ui/uploaddialog.h"
#include "knewstuff2/ui/providerdialog.h"

#include "knewstuff2/core/entryhandler.h" // tmp

#include <kapplication.h> // kapp-ptr
#include <kglobal.h>

#include <kdebug.h>

using namespace KNS;

class KNS::EnginePrivate : public DxsEngine
{
    Q_OBJECT

public:
    EnginePrivate(QWidget* parent)
        : DxsEngine(parent)
    {
        m_command = EnginePrivate::command_none;
        m_uploaddialog = NULL;
        m_downloaddialog = NULL;
        setDxsPolicy(DxsEngine::DxsNever); // FIXME: until KIO/cDXS gets fixed!
        m_entry = NULL;
        m_modal = false;
        m_parent = parent;
    }

    enum Command
    {
        command_none,
        command_upload,
        command_download
    };

    void workflow();
    KNS::Entry* upload(const QString& file);

    Command m_command;
    UploadDialog *m_uploaddialog;
    DownloadDialog *m_downloaddialog;
    QString m_uploadfile;
    KNS::Entry *m_entry;
    KNS::Provider::List m_providers;
    bool m_modal;
    QWidget * m_parent;

  private Q_SLOTS:
    void slotProviderLoaded(KNS::Provider *provider);
    void slotProvidersFailed();
    void slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void slotEntriesFailed();
    void slotEntryUploaded();
    void slotEntryFailed();

    void slotProvidersFinished();
    void slotEntriesFinished();
    void slotEntriesFeedFinished(const KNS::Feed *feed);

    void slotDownloadDialogClosed();
};


Engine::Engine(QWidget* parent)
    : d(new EnginePrivate(parent))
{
}

Engine::~Engine()
{
    delete d;
}

void EnginePrivate::workflow()
{
    if((m_command == command_upload) || (m_command == command_download))
    {
        connect(this,
                SIGNAL(signalProviderLoaded(KNS::Provider*)),
                SLOT(slotProviderLoaded(KNS::Provider*)));
        connect(this,
                SIGNAL(signalProvidersFailed()),
                SLOT(slotProvidersFailed()));
    }

    if(m_command == command_upload)
    {
        connect(this,
                SIGNAL(signalProvidersFinished()),
                SLOT(slotProvidersFinished()));

        m_entry = NULL;
    }

    if(m_command == command_download)
    {
        connect(this,
                SIGNAL(signalEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)),
                SLOT(slotEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)));
        connect(this,
                SIGNAL(signalEntriesFailed()),
                SLOT(slotEntriesFailed()));
        connect(this,
                SIGNAL(signalEntriesFinished()),
                SLOT(slotEntriesFinished()));
        connect(this,
                SIGNAL(signalEntriesFeedFinished(const KNS::Feed*)),
                SLOT(slotEntriesFeedFinished(const KNS::Feed*)));

        m_downloaddialog = new DownloadDialog(this,m_parent);
        m_downloaddialog->show();

        connect(m_downloaddialog, SIGNAL(finished()), SLOT(slotDownloadDialogClosed()));
    }

    start();

    if(m_modal)
    {
        while((m_command == command_upload) || (m_command == command_download))
        {
            kapp->processEvents();
        }
    }
}

KNS::Entry::List Engine::download()
{
	KNS::Entry::List entries;

	Engine *engine = new Engine(0);

	KComponentData component = KGlobal::mainComponent();
	QString name = component.componentName();

	bool ret = engine->init(name + ".knsrc");
	if(!ret)
	{
		delete engine;
		return entries;
	}

	entries = engine->downloadDialogModal(0);
	delete engine;

	// FIXME: refcounting?
	return entries;
}

KNS::Entry::List Engine::downloadDialogModal(QWidget*)
{
	//kDebug(550) << "Engine: downloadDialogModal";

	d->m_command = EnginePrivate::command_download;
	d->m_modal = true;

	d->workflow();

	return KNS::Entry::List();
}

void Engine::downloadDialog()
{
	//kDebug(550) << "Engine: downloadDialog";

	if(d->m_command != EnginePrivate::command_none)
	{
		kError(550) << "Engine: asynchronous workflow already going on" << endl;
	}

	d->m_command = EnginePrivate::command_download;
	d->m_modal = false;

	d->workflow();
}

KNS::Entry *EnginePrivate::upload(const QString& file)
{
    KNS::Entry *entry = NULL;

    Engine engine(0);

    KComponentData component = KGlobal::mainComponent();
    QString name = component.componentName();

    bool ret = engine.init(name + ".knsrc");
    if(!ret) return entry;

    entry = engine.uploadDialogModal(file);

    // FIXME: refcounting?
    return entry;
}

bool Engine::init(const QString& config)
{
    return d->init(config);
}


KNS::Entry *Engine::upload(const QString& file)
{
#ifdef __GNUC__
#warning KNS::Engine::upload() not implemented!
#endif
#if 0
    return d->upload(file);
#else
    Q_UNUSED(file);
#endif
    Q_ASSERT(false);
    return 0;
}

KNS::Entry *Engine::uploadDialogModal(const QString& file)
{
    //kDebug(550) << "Engine: uploadDialogModal";

    d->m_command = EnginePrivate::command_upload;
    d->m_modal = true;
    d->m_uploadfile = file;

    d->workflow();

    return d->m_entry;
}

void Engine::uploadDialog(const QString& file)
{
    //kDebug(550) << "Engine: uploadDialog";

    if(d->m_command != EnginePrivate::command_none)
    {
        kError(550) << "Engine: asynchronous workflow already going on" << endl;
    }

    d->m_command = EnginePrivate::command_upload;
    d->m_modal = false;
    d->m_uploadfile = file;

    d->workflow();
}

void EnginePrivate::slotProviderLoaded(KNS::Provider *provider)
{
    //kDebug(550) << "Engine: slotProviderLoaded";

    if(m_command == command_download)
    {
        loadEntries(provider);
    }
    else if(m_command == command_upload)
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

void EnginePrivate::slotProvidersFailed()
{
    //kDebug(550) << "Engine: slotProvidersFailed";

    m_command = command_none;
}

void EnginePrivate::slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider)
{
    //kDebug(550) << "Engine: slotEntryLoaded";

    m_downloaddialog->addEntry(entry, feed, provider);
}

void EnginePrivate::slotEntriesFailed()
{
    //kDebug(550) << "Engine: slotEntriesFailed";
}

void EnginePrivate::slotEntryUploaded()
{
    //kDebug(550) << "Engine: slotEntryUploaded";

    m_command = command_none;

    //m_entry = ...; // FIXME: where do we get it from now?
    // FIXME: we cannot assign it earlier, probably need m_delayedentry
    // FIXME: if not modal, this must be a signal to the outside (is already the case?)
}

void EnginePrivate::slotEntryFailed()
{
    //kDebug(550) << "Engine: slotEntryFailed";

    m_command = command_none;
}

void EnginePrivate::slotProvidersFinished()
{
	//kDebug(550) << "Engine: slotProvidersFinished";

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
		m_command = EnginePrivate::command_none;
		return;
	}

	KNS::Provider *provider = provdialog.provider();

	UploadDialog uploaddialog(0);
	uploaddialog.setPayloadFile(KUrl(m_uploadfile));
	ret = uploaddialog.exec();
	if(ret == QDialog::Rejected)
	{
		m_command = EnginePrivate::command_none;
		return;
	}

	Entry *entry = uploaddialog.entry();
	entry->setPayload(m_uploadfile);
	if(!entry)
	{
		m_command = EnginePrivate::command_none;
		return;
	}

	EntryHandler eh(*entry);
	QDomElement xml = eh.entryXML();
	QByteArray ar;
	QTextStream txt(&ar);
	txt << xml;
	//kDebug(550) << "Upload: " << QString(ar);

	connect(this,
		SIGNAL(signalEntryUploaded()),
		SLOT(slotEntryUploaded()));
	connect(this,
		SIGNAL(signalEntryFailed()),
		SLOT(slotEntryFailed()));

	uploadEntry(provider, entry);
}

void EnginePrivate::slotEntriesFeedFinished(const KNS::Feed *feed)
{
    //kDebug(550) << "Engine: slotEntriesFeedFinished";

    Q_UNUSED(feed);
    //m_downloaddialog->refresh();
}

void EnginePrivate::slotEntriesFinished()
{
    //kDebug(550) << "Engine: slotEntriesFinished";

    m_downloaddialog->refresh();
}

void EnginePrivate::slotDownloadDialogClosed()
{
    m_downloaddialog->deleteLater();
    m_downloaddialog = NULL;

    m_command = command_none;
}

#include "engine.moc"
