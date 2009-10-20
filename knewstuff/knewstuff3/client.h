/*
    knewstuff3/client.h.
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
#ifndef KNEWSTUFF3_CLIENT_H
#define KNEWSTUFF3_CLIENT_H

#include <knewstuff3/core/entry.h>

namespace KNS3
{

class ClientPrivate;

/**
 * @brief The KNewStuff3 client is the top-level class to handle GHNS in applications.
 *
 * A client implements GHNS workflows, which consist of discrete steps
 * performed by the engine. Depending on the providers, traditional
 * GHNS, DXS, or OpenDesktopServices are used transparently.
 * This class is the one which applications should use.
 * In most cases, either \ref uploadDialog() or \ref downloadDialog() will be called by the
 * application to upload or download data.
 */
class KNEWSTUFF_EXPORT Client: public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Client constructor.
     *
     * As many Clients as needed can be instantiated, although one should use
     * the static methods \ref download() and \ref upload() instead.
     */
    explicit Client(QWidget* parent = 0);

    /**
     * \brief Client destructor.
     *
     */
    ~Client();

	/**
	 * @brief initialize the client (and it's underlying engine) based on the given configfile name
     *	 
	 * Initialize the engine by reading the given config file, creates an engine internally. 
	 * Initializes any providers found therein, loads any cache if caching is enabled, reads the
	 * list of installed entries from kconfig.
	 * @param config filename of config file to load, usually appname.knsrc
	 * @returns 
	 */
	bool init(const QString& config);

    /**
     * @brief Synchronous way of starting the download workflow.
     *
     * Starts the download workflow. This workflow will turn up a dialog
     * where the user can select entries for installation and uninstallation.
     * This method is a modal one. It will return all affected entries as
     * a list.
     *
     * NOTE: this list must be queried to see which ones were installed and which were uninstalled
     * since both are in the one list. i.e. (entry->status() == KNS::Entry::Installed)
     * NOTE: the list returned is not a copy of each entry, so do not delete the entries returned here
     *
     * @return List of installed or deinstalled entries
     */
    KNS3::Entry::List downloadDialogModal(QWidget* parent = 0);

    /**
     * @brief Asynchronous way of starting the download workflow.
     *
     * This method should be used whenever a blocking application with a
     * non-blocking GUI during GHNS operations is not suitable.
     *
     * @see downloadDialogModal()
     */
    void downloadDialog(QWidget* parent = 0);

    /**
     * @brief Synchronous way of starting the upload workflow.
     *
     * Starts the upload workflow. This workflow will offer provider
     * selection and afterwards upload all files associated with an entry.
     * This method is a modal one. It will return the uploaded entry.
     *
     * @return Uploaded entry, or \b null in case of failures
     */
    KNS3::Entry *uploadDialogModal(const QString& file, QWidget *parent = 0);

    /**
     * @brief Asynchronous way of starting the upload workflow.
     *
     * This method should be used whenever a blocking application with a
     * non-blocking GUI during GHNS operations is not suitable.
     * The affected entry will be reported by signals.
     *
     * @see uploadDialogModal()
     */
    void uploadDialog(const QString& file, QWidget *parent = 0);

private Q_SLOTS:

	void slotDownloadDialogClosed();
	void slotUploadDialogClosed();
	
private:

    friend class ClientPrivate;
    ClientPrivate* const d;
    Q_DISABLE_COPY(Client)
};

}

#endif
