/*
    knewstuff3/client.cpp.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2008-2009 Jeremy Whiting <jpwhiting@kde.org>

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
#include "client.h"

#include "engine.h"

#include "knewstuff3/ui/downloaddialog.h"
#include "knewstuff3/ui/uploaddialog.h"
#include "knewstuff3/ui/providerdialog.h"

//#include "knewstuff3/core/entryhandler.h" // tmp

#include <kcomponentdata.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kwindowsystem.h>

#include <qeventloop.h>
#include <qpointer.h>

using namespace KNS3;

class KNS3::ClientPrivate
{
public:
    ClientPrivate(QWidget * parent, Client * client)
        : m_command(ClientPrivate::command_none),
        m_uploaddialog(NULL),
        m_downloaddialog(NULL),
        m_uploadedEntry(NULL),
        m_modal(false),
	m_parent(parent),
	p(client),
        m_loop(0),
        m_engine(new Engine(parent))
    {
    }

    enum Command {
        command_none,
        command_upload,
        command_download
    };

	bool init(const QString & config);

    void workflow();
    KNS3::Entry upload(const QString& file);

    static QHash<QString, QPointer<KDialog> > s_dialogs;

    Command m_command;
    QPointer<UploadDialog> m_uploaddialog;
    QPointer<DownloadDialog> m_downloaddialog;
    QString m_uploadfile;
    KNS3::Entry *m_uploadedEntry;
    KNS3::Provider::List m_providers;
    bool m_modal;
    QWidget * m_parent;
    Client * p;
    QSet<KNS3::Entry> m_changedEntries;
    QEventLoop* m_loop;
    KNS3::Engine * m_engine;

public:
    void slotDownloadDialogDestroyed();

    /** stop the event loop */
    void stopLoop();

private:
    void slotProviderLoaded(KNS3::Provider *provider);

    /** slot for when entries are changed, so we can return a list
     * of them from the static methods */
    void slotEntryChanged(KNS3::Entry entry);

    void slotHandleUpload();
    void slotEntriesFinished();

    void slotDownloadDialogClosed();
};


QHash<QString, QPointer<KDialog> > KNS3::ClientPrivate::s_dialogs;

Client::Client(QWidget* parent)
        : d(new ClientPrivate(parent,this))
{
}

Client::~Client()
{
    //kDebug() << d->m_downloaddialog;
    if (d->m_downloaddialog) {
        d->slotDownloadDialogDestroyed();
    }

    delete d;
}

bool Client::init(const QString& config)
{
    return d->m_engine->init(config);
}

void ClientPrivate::workflow()
{
//    //disconnect(this, 0, this, 0);
//    if ((m_command == command_upload) || (m_command == command_download)) {
//        //connect(this,
//        //        SIGNAL(signalProviderLoaded(KNS::Provider*)),
//        //        SLOT(slotProviderLoaded(KNS::Provider*)));
//    }

//    if (m_command == command_upload) {
//        //connect(this,
//        //        SIGNAL(signalProvidersFinished()),
//        //        SLOT(slotHandleUpload()));
//        //connect(this,
//        //        SIGNAL(signalProvidersFailed()),
//        //        SLOT(stopLoop()));

//        m_uploadedEntry = NULL;
//    }

    if (m_command == command_download) {
        m_downloaddialog = new DownloadDialog(m_engine, m_parent);
        //kDebug() << "adding!";
        //s_dialogs.insert(componentName(), m_downloaddialog);

        //connect(this, SIGNAL(signalEntriesFinished()),
        //        SLOT(slotEntriesFinished()));
        //connect(this,
        //        SIGNAL(signalEntryChanged(KNS::Entry *)),
        //        SLOT(slotEntryChanged(KNS::Entry *)));
        //connect(this,
        //        SIGNAL(signalProvidersFailed()),
        //        SLOT(slotDownloadDialogClosed()));
        //connect(m_downloaddialog,
        //        SIGNAL(destroyed(QObject*)),
        //        SLOT(slotDownloadDialogDestroyed()));
        QObject::connect(m_downloaddialog, SIGNAL(finished()), p, SLOT(slotDownloadDialogClosed()));
        //kDebug() << "done adding!";

        m_downloaddialog->show();
    }

//    start();

    if (m_modal) {
        QEventLoop loop;
        m_loop = &loop;
        loop.exec();
    }
}

void ClientPrivate::stopLoop()
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

//KNS::Entry::List Client::download()
//{
//    KNS::Entry::List entries;

//    Client *client = new Client(0);

//    KComponentData component = KGlobal::activeComponent();
//    QString name = component.componentName();

//    bool ret = client->init(name + ".knsrc");
//    if (!ret) {
//        delete client;
//        return entries;
//    }

//    KNS::Entry::List tempList = client->downloadDialogModal(0);

//    // copy the list since the entries will be deleted when we delete the client
//    foreach(Entry * entry, tempList) {
//        entries << new Entry(*entry);
//    }
//    delete client;

//    return entries;
//}

KNS3::Entry::List Client::downloadDialogModal(QWidget* parent)
{
    kDebug() << "Client: downloadDialogModal";
    KDialog *existingDialog = d->m_downloaddialog;
    if (existingDialog) {
        existingDialog->show();
        KWindowSystem::setOnDesktop(existingDialog->winId(), KWindowSystem::currentDesktop());
        KWindowSystem::activateWindow(existingDialog->winId());
        return Entry::List(); // return an empty list, there's already a dialog showing
    }

    d->m_command = ClientPrivate::command_download;
    d->m_modal = true;

    d->workflow();

    return Entry::List::fromSet(d->m_changedEntries);
}

void Client::downloadDialog(QWidget * parent)
{
    //kDebug() << "Client: downloadDialog";
    KDialog *existingDialog = ClientPrivate::s_dialogs.value(d->m_engine->componentName());
    if (existingDialog) {
        //kDebug() << "got an existing dialog";
        existingDialog->show();
        KWindowSystem::setOnDesktop(existingDialog->winId(), KWindowSystem::currentDesktop());
        KWindowSystem::activateWindow(existingDialog->winId());
        return;
    }

    if (d->m_command != ClientPrivate::command_none) {
        kError() << "Client: asynchronous workflow already going on" << endl;
        return;
    }

    d->m_command = ClientPrivate::command_download;
    d->m_modal = false;

    d->workflow();
}

//KNS::Entry *ClientPrivate::upload(const QString& file, QWidget * parent = 0)
//{
//    KNS::Entry *entry = NULL;

//    Client client(0);

//    KComponentData component = KGlobal::activeComponent();
//    QString name = component.componentName();

//    bool ret = client.init(name + ".knsrc");
//    if (!ret) return entry;

//    entry = client.uploadDialogModal(file);

//    // FIXME: refcounting?
//    return entry;
//}

//KNS::Entry *Client::upload(const QString& file)
//{
//#ifdef __GNUC__
//#warning KNS::Client::upload() not implemented!
//#endif
//#if 0
//    return d->upload(file);
//#else
//    Q_UNUSED(file);
//#endif
//    Q_ASSERT(false);
//    return 0;
//}

KNS3::Entry *Client::uploadDialogModal(const QString& file, QWidget * parent)
{
    //kDebug() << "Client: uploadDialogModal";

    d->m_command = ClientPrivate::command_upload;
    d->m_modal = true;
    d->m_uploadfile = file;

    d->workflow();

    return d->m_uploadedEntry;
}

void Client::uploadDialog(const QString& file, QWidget * parent)
{
    //kDebug() << "Client: uploadDialog";

    if (d->m_command != ClientPrivate::command_none) {
        kError() << "Client: asynchronous workflow already going on" << endl;
        return;
    }

    d->m_command = ClientPrivate::command_upload;
    d->m_modal = false;
    d->m_uploadfile = file;

    d->workflow();
}

void Client::slotDownloadDialogClosed()
{
    d->m_downloaddialog->deleteLater();
	d->stopLoop();
}

void Client::slotUploadDialogClosed()
{
	d->m_uploaddialog->deleteLater();
	d->stopLoop();
}

//void ClientPrivate::slotProviderLoaded(KNS::Provider *provider)
//{
//    if (m_command == command_download) {
//        loadEntries(provider);
//    } else if (m_command == command_upload) {
//        // FIXME: inject into upload dialog
//        // FIXME: dialog could do this by itself!

//        // FIXME: for the modal dialog, do nothing here
//        // ... and wait for slotProvidersFinished()
//        m_providers.append(provider);
//    } else {
//        kError() << "Client: invalid command" << endl;
//    }
//}

void ClientPrivate::slotHandleUpload()
{
    // NOTE: this is only connected when we are doing an upload
    //kDebug() << "Client: slotProvidersFinished";

    //Provider *fakeprovider = new Provider();
    //fakeprovider->setName(QString("Fake Provider"));
    //fakeprovider->setUploadUrl(KUrl("http://localhost/dav/"));
    //fakeprovider->setUploadUrl(KUrl("webdav://localhost/uploads/"));
    
    
    // let the user select the provider
    //QPointer<ProviderDialog> provdialog = new ProviderDialog(NULL);
    //for (Provider::List::Iterator it = m_providers.begin(); it != m_providers.end(); ++it) {
    //    Provider *provider = (*it);
    //    provdialog->addProvider(provider);
    //}
    ////provdialog.addProvider(fakeprovider);
    //if (provdialog->exec() == QDialog::Rejected) {
    //    stopLoop();
    //    return;
    //}

    //KNS::Provider *provider = provdialog->provider();

    //// fill in the details of the upload (name, author...)
    //QPointer<UploadDialog> uploaddialog = new UploadDialog(NULL);
    //uploaddialog->setPayloadFile(KUrl(m_uploadfile));
    //if (uploaddialog->exec() == QDialog::Rejected) {
    //    stopLoop();
    //    return;
    //}

    //Entry *entry = uploaddialog->entry();
    //if (!entry) {
    //    stopLoop();
    //    return;
    //}
    
    //KTranslatable payload;
    //// add all the translations to the payload
    //QStringList langs = entry->name().languages();
    //for (QStringList::const_iterator it = langs.constBegin(); it != langs.constEnd(); ++it) {
    //    payload.addString(*it, m_uploadfile);
    //}
    //entry->setPayload(payload);

    //EntryHandler eh(*entry);
    //QDomElement xml = eh.entryXML();
    //QByteArray ar;
    //QTextStream txt(&ar);
    //txt << xml;
    ////kDebug() << "Upload: " << QString(ar);

    //connect(this, SIGNAL(signalEntryUploaded()), SLOT(stopLoop()));
    //connect(this, SIGNAL(signalEntryFailed()), SLOT(stopLoop()));

    //uploadEntry(provider, entry);
    //m_uploadedEntry=entry;
}

void ClientPrivate::slotEntryChanged(KNS3::Entry entry)
{
    //kDebug() << "adding entries to list of changed entries";
    
    m_changedEntries.insert(entry);
}

// BIGFIXME: make this method go away when we are using goya
void ClientPrivate::slotEntriesFinished()
{
    //m_downloaddialog->refresh();
}

void ClientPrivate::slotDownloadDialogDestroyed()
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

void ClientPrivate::slotDownloadDialogClosed()
{
    //kDebug() << sender() << m_downloaddialog;
    //disconnect(m_downloaddialog, SIGNAL(destroyed(QObject*)),
    //           this, SLOT(slotDownloadDialogDestroyed()));
    slotDownloadDialogDestroyed();
    m_downloaddialog->deleteLater();
    stopLoop();
    //emit signalDownloadDialogDone(QList<KNS::Entry*>::fromSet(m_changedEntries));
}


#include "client.moc"
