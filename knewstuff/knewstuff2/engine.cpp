/*
    This file is part of KNewStuff2.
    Copyright (c) 2008 Jeremy Whiting <jeremy@scitools.com>
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

#include <kcomponentdata.h>
#include <kglobal.h>
#include <kdebug.h>

#include <qeventloop.h>

using namespace KNS;

class KNS::EnginePrivate : public DxsEngine
{
    Q_OBJECT

public:
    EnginePrivate(QWidget* parent)
            : DxsEngine(parent) {
        m_command = EnginePrivate::command_none;
        m_uploaddialog = NULL;
        m_downloaddialog = NULL;
        m_uploadedEntry = NULL;
        m_modal = false;
        m_parent = parent;
        m_loop = 0;
    }

    enum Command {
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
    KNS::Entry *m_uploadedEntry;
    KNS::Provider::List m_providers;
    bool m_modal;
    QWidget * m_parent;
    QSet<KNS::Entry*> m_changedEntries;
    QEventLoop* m_loop;

private Q_SLOTS:
    /** stop the event loop */
    void stopLoop();

    void slotProviderLoaded(KNS::Provider *provider);

    /** slot for when entries are changed, so we can return a list
     * of them from the static methods */
    void slotEntryChanged(KNS::Entry *entry);

    void slotProvidersFinished();
    void slotEntriesFinished();

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
    if ((m_command == command_upload) || (m_command == command_download)) {
        connect(this,
                SIGNAL(signalProviderLoaded(KNS::Provider*)),
                SLOT(slotProviderLoaded(KNS::Provider*)));
        connect(this,
                SIGNAL(signalProvidersFailed()),
                SLOT(stopLoop()));
    }

    if (m_command == command_upload) {
        connect(this,
                SIGNAL(signalProvidersFinished()),
                SLOT(slotProvidersFinished()));

        m_uploadedEntry = NULL;
    }

    if (m_command == command_download) {
        m_downloaddialog = new DownloadDialog(this, m_parent);

        connect(this, SIGNAL(signalEntriesFinished()),
                SLOT(slotEntriesFinished()));
        connect(this,
                SIGNAL(signalEntryChanged(KNS::Entry *)),
                SLOT(slotEntryChanged(KNS::Entry *)));

        m_downloaddialog->show();

        connect(m_downloaddialog, SIGNAL(finished()), SLOT(slotDownloadDialogClosed()));
    }

    start();

    if (m_modal) {
        QEventLoop loop;
        m_loop = &loop;
        loop.exec();
    }
}

void EnginePrivate::stopLoop()
{
    m_command = command_none;

    if (m_loop) {
        m_loop->exit();
        m_loop = 0;
    }
}

KNS::Entry::List Engine::download()
{
    KNS::Entry::List entries;

    Engine *engine = new Engine(0);

    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();

    bool ret = engine->init(name + ".knsrc");
    if (!ret) {
        delete engine;
        return entries;
    }

    KNS::Entry::List tempList = engine->downloadDialogModal(0);

    // copy the list since the entries will be deleted when we delete the engine
    foreach(Entry * entry, tempList) {
        entries << new Entry(*entry);
    }
    delete engine;

    return entries;
}

KNS::Entry::List Engine::downloadDialogModal(QWidget*)
{
    //kDebug() << "Engine: downloadDialogModal";

    d->m_command = EnginePrivate::command_download;
    d->m_modal = true;

    d->workflow();

    return QList<KNS::Entry*>::fromSet(d->m_changedEntries);
}

void Engine::downloadDialog()
{
    //kDebug() << "Engine: downloadDialog";

    if (d->m_command != EnginePrivate::command_none) {
        kError() << "Engine: asynchronous workflow already going on" << endl;
    }

    d->m_command = EnginePrivate::command_download;
    d->m_modal = false;

    d->workflow();
}

KNS::Entry *EnginePrivate::upload(const QString& file)
{
    KNS::Entry *entry = NULL;

    Engine engine(0);

    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();

    bool ret = engine.init(name + ".knsrc");
    if (!ret) return entry;

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
    //kDebug() << "Engine: uploadDialogModal";

    d->m_command = EnginePrivate::command_upload;
    d->m_modal = true;
    d->m_uploadfile = file;

    d->workflow();

    return d->m_uploadedEntry;
}

void Engine::uploadDialog(const QString& file)
{
    //kDebug() << "Engine: uploadDialog";

    if (d->m_command != EnginePrivate::command_none) {
        kError() << "Engine: asynchronous workflow already going on" << endl;
    }

    d->m_command = EnginePrivate::command_upload;
    d->m_modal = false;
    d->m_uploadfile = file;

    d->workflow();
}

void EnginePrivate::slotProviderLoaded(KNS::Provider *provider)
{
    if (m_command == command_download) {
        loadEntries(provider);
    } else if (m_command == command_upload) {
        // FIXME: inject into upload dialog
        // FIXME: dialog could do this by itself!

        // FIXME: for the modal dialog, do nothing here
        // ... and wait for slotProvidersFinished()
        m_providers.append(provider);
    } else {
        kError() << "Engine: invalid command" << endl;
    }
}

void EnginePrivate::slotProvidersFinished()
{
    // NOTE: this is only connected when we are doing an upload
    //kDebug() << "Engine: slotProvidersFinished";

    int ret;

    //Provider *fakeprovider = new Provider();
    //fakeprovider->setName(QString("Fake Provider"));
    //fakeprovider->setUploadUrl(KUrl("http://localhost/dav/"));
    //fakeprovider->setUploadUrl(KUrl("webdav://localhost/uploads/"));

    ProviderDialog provdialog(0);
    for (Provider::List::Iterator it = m_providers.begin(); it != m_providers.end(); ++it) {
        Provider *provider = (*it);
        provdialog.addProvider(provider);
    }
    //provdialog.addProvider(fakeprovider);
    ret = provdialog.exec();
    if (ret == QDialog::Rejected) {
        stopLoop();
        return;
    }

    KNS::Provider *provider = provdialog.provider();

    UploadDialog uploaddialog(0);
    uploaddialog.setPayloadFile(KUrl(m_uploadfile));
    ret = uploaddialog.exec();
    if (ret == QDialog::Rejected) {
        stopLoop();
        return;
    }

    Entry *entry = uploaddialog.entry();
    entry->setPayload(m_uploadfile);
    if (!entry) {
        stopLoop();
        return;
    }

    EntryHandler eh(*entry);
    QDomElement xml = eh.entryXML();
    QByteArray ar;
    QTextStream txt(&ar);
    txt << xml;
    //kDebug() << "Upload: " << QString(ar);

    connect(this,
            SIGNAL(signalEntryUploaded()),
            SLOT(stopLoop()));
    connect(this,
            SIGNAL(signalEntryFailed()),
            SLOT(stopLoop()));

    uploadEntry(provider, entry);
}

void EnginePrivate::slotEntryChanged(KNS::Entry * entry)
{
    //kDebug() << "adding entries to list of changed entries";
    m_changedEntries << entry;
}

// BIGFIXME: make this method go away when we are using goya
void EnginePrivate::slotEntriesFinished()
{
    //m_downloaddialog->refresh();
}

void EnginePrivate::slotDownloadDialogClosed()
{
    m_downloaddialog->deleteLater();
    m_downloaddialog = NULL;

    stopLoop();
}

#include "engine.moc"
