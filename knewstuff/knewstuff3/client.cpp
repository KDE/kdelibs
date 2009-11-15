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
    ClientPrivate(QWidget* parent_, Client* client)
        : parent(parent_),
        p(client),
        command(ClientPrivate::command_none),
        uploaddialog(0),
        downloaddialog(0),
        modal(false),
        loop(0),
        engine(new Engine)
    {
    }

    ~ClientPrivate()
    {
        delete engine;
    }

    enum Command {
        command_none,
        command_upload,
        command_download
    };

    Command command;
    
    bool init(const QString & config);

    void workflow();
    Entry upload(const QString& file);

    static QHash<QString, QPointer<KDialog> > s_dialogs;

    QWidget* parent;
    QPointer<UploadDialog> uploaddialog;
    QPointer<DownloadDialog> downloaddialog;
    QString m_uploadfile;
    Entry uploadedEntry;
    Provider::List providers;
    bool modal;
    Client* p;
    QSet<Entry> changedEntries;
    QEventLoop* loop;
    Engine* engine;
};


QHash<QString, QPointer<KDialog> > KNS3::ClientPrivate::s_dialogs;

Client::Client(QWidget* parent)
        : QObject(parent), d(new ClientPrivate(parent,this))
{
}

Client::~Client()
{
    //kDebug() << d->downloaddialog;
    if (d->downloaddialog) {
        slotDownloadDialogDestroyed();
    }

    delete d;
}

bool Client::init(const QString& config)
{
    return d->engine->init(config);
}

void Client::workflow()
{
    if (d->command == ClientPrivate::command_download) {
        d->downloaddialog = new DownloadDialog(d->engine, d->parent);
        connect(d->engine, SIGNAL(signalEntryChanged(const KNS3::Entry&)),
               this, SLOT(slotEntryChanged(KNS3::Entry))); 
        QObject::connect(d->downloaddialog, SIGNAL(finished()), this, SLOT(slotDownloadDialogClosed()));
        d->downloaddialog->show();
    }

    if (d->modal) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
    }
}

void Client::stopLoop()
{
    d->command = KNS3::ClientPrivate::command_none;

    if (d->loop) {
        d->loop->exit();
        d->loop = 0;

        if (d->downloaddialog) {
            slotDownloadDialogDestroyed();
        }
    }
}

KNS3::Entry::List Client::downloadDialogModal(QWidget* parent)
{
    kDebug() << "Client: downloadDialogModal";
    KDialog *existingDialog = d->downloaddialog;
    if (existingDialog) {
        existingDialog->show();
        KWindowSystem::setOnDesktop(existingDialog->winId(), KWindowSystem::currentDesktop());
        KWindowSystem::activateWindow(existingDialog->winId());
        return Entry::List(); // return an empty list, there's already a dialog showing
    }

    d->command = ClientPrivate::command_download;
    d->modal = true;

    workflow();

    return Entry::List::fromSet(d->changedEntries);
}

void Client::downloadDialog(QWidget * parent)
{
    KDialog *existingDialog = ClientPrivate::s_dialogs.value(d->engine->componentName());
    if (existingDialog) {
        existingDialog->show();
        KWindowSystem::setOnDesktop(existingDialog->winId(), KWindowSystem::currentDesktop());
        KWindowSystem::activateWindow(existingDialog->winId());
        return;
    }

    if (d->command != ClientPrivate::command_none) {
        kError() << "Client: asynchronous workflow already going on" << endl;
        return;
    }

    d->command = ClientPrivate::command_download;
    d->modal = false;

    workflow();
}

void Client::slotEntryChanged(const KNS3::Entry& entry)
{
    //kDebug() << "adding entries to list of changed entries";
    d->changedEntries.insert(entry);
}

void Client::slotDownloadDialogDestroyed()
{
    //kDebug() << d->downloaddialog << "is destroyed!" << s_dialogs.count() << s_dialogs.keys();
    QHash<QString, QPointer<KDialog> >::iterator it = d->s_dialogs.begin();
    while (it != d->s_dialogs.end()) {
        if (it.value() == d->downloaddialog) {
            //kDebug() << "found it!";
            it = d->s_dialogs.erase(it);
        }

        if (it != d->s_dialogs.end()) {
            ++it;
        }
    }
}

void Client::slotDownloadDialogClosed()
{
    //kDebug() << sender() << d->downloaddialog;
    //disconnect(d->downloaddialog, SIGNAL(destroyed(QObject*)),
    //           this, SLOT(slotDownloadDialogDestroyed()));
    slotDownloadDialogDestroyed();
    d->downloaddialog->deleteLater();
    stopLoop();
    //emit signalDownloadDialogDone(QList<KNS::Entry*>::fromSet(m_changedEntries));
}


#include "client.moc"
