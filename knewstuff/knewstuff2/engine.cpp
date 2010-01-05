/*
    This file is part of KNewStuff2.
    Copyright (c) 2008 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "knewstuff2/engine.h"

#include "knewstuff2/ui/downloaddialog.h"
#include "knewstuff2/ui/uploaddialog.h"
#include "knewstuff2/ui/providerdialog.h"

#include "knewstuff2/core/entryhandler.h" // tmp

#include <kcomponentdata.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kwindowsystem.h>

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

    static QHash<QString, QPointer<KDialog> > s_dialogs;

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

public Q_SLOTS:
    void slotDownloadDialogDestroyed();

private Q_SLOTS:
    /** stop the event loop */
    void stopLoop();

    void slotProviderLoaded(KNS::Provider *provider);

    /** slot for when entries are changed, so we can return a list
     * of them from the static methods */
    void slotEntryChanged(KNS::Entry *entry);

    void slotHandleUpload();
    void slotEntriesFinished();

    void slotDownloadDialogClosed();
};


QHash<QString, QPointer<KDialog> > KNS::EnginePrivate::s_dialogs;

Engine::Engine(QWidget* parent)
        : d(new EnginePrivate(parent))
{
}

Engine::~Engine()
{
    //kDebug() << d->m_downloaddialog;
    if (d->m_downloaddialog) {
        d->slotDownloadDialogDestroyed();
    }

    delete d;
}

void EnginePrivate::workflow()
{
    disconnect(this, 0, this, 0);
    if ((m_command == command_upload) || (m_command == command_download)) {
        connect(this,
                SIGNAL(signalProviderLoaded(KNS::Provider*)),
                SLOT(slotProviderLoaded(KNS::Provider*)));
    }

    if (m_command == command_upload) {
        connect(this,
                SIGNAL(signalProvidersFinished()),
                SLOT(slotHandleUpload()));
        connect(this,
                SIGNAL(signalProvidersFailed()),
                SLOT(stopLoop()));

        m_uploadedEntry = NULL;
    }

    if (m_command == command_download) {
        m_downloaddialog = new DownloadDialog(this, m_parent);
        //kDebug() << "adding!";
        s_dialogs.insert(componentName(), m_downloaddialog);

        connect(this, SIGNAL(signalEntriesFinished()),
                SLOT(slotEntriesFinished()));
        connect(this,
                SIGNAL(signalEntryChanged(KNS::Entry *)),
                SLOT(slotEntryChanged(KNS::Entry *)));
        connect(this,
                SIGNAL(signalProvidersFailed()),
                SLOT(slotDownloadDialogClosed()));
        connect(m_downloaddialog,
                SIGNAL(destroyed(QObject*)),
                SLOT(slotDownloadDialogDestroyed()));
        connect(m_downloaddialog, SIGNAL(finished()), SLOT(slotDownloadDialogClosed()));
        //kDebug() << "done adding!";

        m_downloaddialog->show();
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

        if (m_downloaddialog) {
            slotDownloadDialogDestroyed();
        }
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

KNS::Entry::List Engine::downloadDialogModal(QWidget* parent)
{
    //kDebug() << "Engine: downloadDialogModal";
    KDialog *existingDialog = EnginePrivate::s_dialogs.value(d->componentName());
    if (existingDialog) {
        existingDialog->show();
        KWindowSystem::setOnDesktop(existingDialog->winId(), KWindowSystem::currentDesktop());
        KWindowSystem::activateWindow(existingDialog->winId());
        return QList<KNS::Entry*>();
    }

    d->m_command = EnginePrivate::command_download;
    d->m_modal = true;
    if (parent) {
        d->m_parent = parent;
    }

    d->workflow();

    return QList<KNS::Entry*>::fromSet(d->m_changedEntries);
}

void Engine::downloadDialog()
{
    //kDebug() << "Engine: downloadDialog";
    KDialog *existingDialog = EnginePrivate::s_dialogs.value(d->componentName());
    if (existingDialog) {
        //kDebug() << "got an existing dialog";
        existingDialog->show();
        KWindowSystem::setOnDesktop(existingDialog->winId(), KWindowSystem::currentDesktop());
        KWindowSystem::activateWindow(existingDialog->winId());
        return;
    }

    if (d->m_command != EnginePrivate::command_none) {
        kError() << "Engine: asynchronous workflow already going on" << endl;
        return;
    }

    d->m_command = EnginePrivate::command_download;
    d->m_modal = false;

    d->workflow();
}

void Engine::downloadDialog(QObject * receiver, const char * slot)
{
    QObject::disconnect(d, SIGNAL(signalDownloadDialogDone(KNS::Entry::List)), receiver, slot);
    QObject::connect(d, SIGNAL(signalDownloadDialogDone(KNS::Entry::List)), receiver, slot);
    downloadDialog();
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
        return;
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

void EnginePrivate::slotHandleUpload()
{
    // NOTE: this is only connected when we are doing an upload
    //kDebug() << "Engine: slotProvidersFinished";

    //Provider *fakeprovider = new Provider();
    //fakeprovider->setName(QString("Fake Provider"));
    //fakeprovider->setUploadUrl(KUrl("http://localhost/dav/"));
    //fakeprovider->setUploadUrl(KUrl("webdav://localhost/uploads/"));
    
    
    // let the user select the provider
    QPointer<ProviderDialog> provdialog = new ProviderDialog(NULL);
    for (Provider::List::Iterator it = m_providers.begin(); it != m_providers.end(); ++it) {
        Provider *provider = (*it);
        provdialog->addProvider(provider);
    }
    //provdialog.addProvider(fakeprovider);
    if (provdialog->exec() == QDialog::Rejected) {
        stopLoop();
        return;
    }

    KNS::Provider *provider = provdialog->provider();

    // fill in the details of the upload (name, author...)
    QPointer<UploadDialog> uploaddialog = new UploadDialog(NULL);
    uploaddialog->setPayloadFile(KUrl(m_uploadfile));
    if (uploaddialog->exec() == QDialog::Rejected) {
        stopLoop();
        return;
    }

    Entry *entry = uploaddialog->entry();
    if (!entry) {
        stopLoop();
        return;
    }
    
    KTranslatable payload;
    // add all the translations to the payload
    QStringList langs = entry->name().languages();
    for (QStringList::const_iterator it = langs.constBegin(); it != langs.constEnd(); ++it) {
        payload.addString(*it, m_uploadfile);
    }
    entry->setPayload(payload);

    EntryHandler eh(*entry);
    QDomElement xml = eh.entryXML();
    QByteArray ar;
    QTextStream txt(&ar);
    txt << xml;
    //kDebug() << "Upload: " << QString(ar);

    connect(this, SIGNAL(signalEntryUploaded()), SLOT(stopLoop()));
    connect(this, SIGNAL(signalEntryFailed()), SLOT(stopLoop()));

    uploadEntry(provider, entry);
    m_uploadedEntry=entry;
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

void EnginePrivate::slotDownloadDialogDestroyed()
{
    //kDebug() << m_downloaddialog << "is destroyed!" << s_dialogs.count() << s_dialogs.keys();
    QHash<QString, QPointer<KDialog> >::iterator it = s_dialogs.begin();
    while (it != s_dialogs.end()) {
        if (it.value() == m_downloaddialog) {
            //kDebug() << "found it!";
            it = s_dialogs.erase(it);
        }

        if (it != s_dialogs.end()) {
            ++it;
        }
    }

    //kDebug() << s_dialogs.count() << s_dialogs.keys();
}

void EnginePrivate::slotDownloadDialogClosed()
{
    //kDebug() << sender() << m_downloaddialog;
    disconnect(m_downloaddialog, SIGNAL(destroyed(QObject*)),
               this, SLOT(slotDownloadDialogDestroyed()));
    slotDownloadDialogDestroyed();
    m_downloaddialog->deleteLater();
    m_downloaddialog = NULL;
    stopLoop();
    emit signalDownloadDialogDone(QList<KNS::Entry*>::fromSet(m_changedEntries));
}

#include "engine.moc"
