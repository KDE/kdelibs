/*
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007-2009 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "upload.h"

/*
bool Engine::uploadEntry(Provider *provider, const Entry& entry)
{
    //kDebug() << "Uploading " << entry.name().representation() << "...";

    //if (d->uploadedentry) {
    //    kError() << "Another upload is in progress!" << endl;
    //    return false;
    //}

    //if (!provider->uploadUrl().isValid()) {
    //    kError() << "The provider doesn't support uploads." << endl;
    //    return false;

    //    // FIXME: support for <noupload> will go here (file bundle creation etc.)
    //}

    //// FIXME: validate files etc.
    //d->uploadprovider = provider;
    //d->uploadedentry = entry;

    //KUrl sourcepayload = KUrl(entry.payload().representation());
    //KUrl destfolder = provider->uploadUrl();

    //destfolder.setFileName(sourcepayload.fileName());

    //KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepayload, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    //connect(fcjob,
    //        SIGNAL(result(KJob*)),
    //        SLOT(slotUploadPayloadResult(KJob*)));

    return true;
}
*/
/*
void Engine::slotUploadPayloadResult(KJob *job)
{
    //if (job->error()) {
    //    kError() << "Cannot upload payload file." << endl;
    //    kError() << job->errorString() << endl;

    //    d->uploadedentry = NULL;
    //    d->uploadprovider = NULL;

    //    emit signalEntryFailed();
    //    return;
    //}

    //if (d->uploadedentry.preview().representation().isEmpty()) {
    //    // FIXME: we abuse 'job' here for the shortcut if there's no preview
    //    slotUploadPreviewResult(job);
    //    return;
    //}

    //KUrl sourcepreview = KUrl(d->uploadedentry.preview().representation());
    //KUrl destfolder = d->uploadprovider->uploadUrl();

    //destfolder.setFileName(sourcepreview.fileName());

    //KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepreview, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    //connect(fcjob,
    //        SIGNAL(result(KJob*)),
    //        SLOT(slotUploadPreviewResult(KJob*)));
}

void Engine::slotUploadPreviewResult(KJob *job)
{
    //if (job->error()) {
    //    kError() << "Cannot upload preview file." << endl;
    //    kError() << job->errorString() << endl;

    //    d->uploadedentry = NULL;
    //    d->uploadprovider = NULL;

    //    emit signalEntryFailed();
    //    return;
    //}

    //// FIXME: the following save code is also in cacheEntry()
    //// when we upload, the entry should probably be cached!

    //// FIXME: adhere to meta naming rules as discussed
    //KUrl sourcemeta = QString(KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + ".meta");
    //KUrl destfolder = d->uploadprovider->uploadUrl();

    //destfolder.setFileName(sourcemeta.fileName());

    //EntryHandler eh(*d->uploadedentry);
    //QDomElement exml = eh.entryXML();

    //QDomDocument doc;
    //QDomElement root = doc.createElement("ghnsupload");
    //root.appendChild(exml);

    //QFile f(sourcemeta.path());
    //if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    //    kError() << "Cannot write meta information to '" << sourcemeta << "'." << endl;

    //    d->uploadedentry = NULL;
    //    d->uploadprovider = NULL;

    //    emit signalEntryFailed();
    //    return;
    //}
    //QTextStream metastream(&f);
    //metastream << root;
    //f.close();

    //KIO::FileCopyJob *fcjob = KIO::file_copy(sourcemeta, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
    //connect(fcjob,
    //        SIGNAL(result(KJob*)),
    //        SLOT(slotUploadMetaResult(KJob*)));
}
*/

/*
void Engine::slotUploadMetaResult(KJob *job)
{
    if (job->error()) {
        kError() << "Cannot upload meta file." << endl;
        kError() << job->errorString() << endl;

        d->uploadedentry = Entry();
        d->uploadprovider = NULL;

        emit signalEntryFailed();
        return;
    } else {
        d->uploadedentry = Entry();
        d->uploadprovider = NULL;

        //KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);
        emit signalEntryUploaded();
    }
}
*/


