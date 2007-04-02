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
#ifndef KNEWSTUFF2_ENGINE_H
#define KNEWSTUFF2_ENGINE_H

#include <knewstuff2/dxs/dxsengine.h>

namespace KNS {

class UploadDialog;
class DownloadDialog;
class Feed;

/**
 * KNewStuff engine.
 * An engine implements GHNS workflows, which consist of discrete steps
 * performed by the inherited engine. Depending on the provider, traditional
 * GHNS or DXS are used transparently.
 * This class is the one which applications should use.
 */
class KNEWSTUFF_EXPORT Engine : public DxsEngine
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     */
    Engine();

    /**
     * Destructor.
     */
    ~Engine();

    /**
     * Starts the download workflow. This workflow will turn up a dialog
     * where the user can select entries for installation and deinstallation.
     * This method is a modal one. It will return all affected entries as
     * a list.
     *
     * @return List of installed or deinstalled entries
     */
    KNS::Entry::List downloadDialogModal();

    static KNS::Entry::List download();

    /**
     * Starts the upload workflow. This workflow will offer provider
     * selection and afterwards upload all files associated with an entry.
     * This method is a modal one. It will return the uploaded entry.
     *
     * @return Uploaded entry, or \b null in case of failures
     */
    KNS::Entry *uploadDialogModal(QString file);

    static KNS::Entry *upload(QString file);

    /**
     * Asynchronous way of starting the download workflow.
     * Affected entries will be reported by signals.
     *
     * @see downloadDialogModal
     */
    void downloadDialog();

    /**
     * Asynchronous way of starting the upload workflow.
     * The affected entry will be reported by signals.
     *
     * @see uploadDialogModal
     */
    void uploadDialog(QString file);

  private slots:
    void slotProviderLoaded(KNS::Provider *provider);
    void slotProvidersFailed();
    void slotEntryLoaded(KNS::Entry *entry, const Feed *feed, const Provider *provider);
    void slotEntriesFailed();
    void slotEntryUploaded();
    void slotEntryFailed();

    void slotProvidersFinished();
    void slotEntriesFinished();
    void slotEntriesFeedFinished(const Feed *feed);

  private:
    void workflow();

    enum Command
    {
        command_none,
        command_upload,
        command_download
    };

    Command m_command;
    UploadDialog *m_uploaddialog;
    DownloadDialog *m_downloaddialog;
    QString m_uploadfile;
    KNS::Entry *m_entry;
    KNS::Provider::List m_providers;
    bool m_modal;
};

}

#endif
