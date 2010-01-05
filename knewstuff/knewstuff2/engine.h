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
#ifndef KNEWSTUFF2_ENGINE_H
#define KNEWSTUFF2_ENGINE_H

#include <knewstuff2/core/entry.h>
#include <kdemacros.h>

namespace KNS
{


class EnginePrivate;

/**
 * @brief The KNewStuff2 engine is the top-level class to handle GHNS and DXS workflows.
 *
 * An engine implements GHNS workflows, which consist of discrete steps
 * performed by the inherited engine. Depending on the provider, traditional
 * GHNS or DXS are used transparently.
 * This class is the one which applications should use.
 * In most cases, either \ref upload() or \ref download() will be called by the
 * application to upload or download data.
 *
 * Deprecated, use knewstuff3!
 */
class KNEWSTUFF_EXPORT_DEPRECATED Engine
{
public:
    /**
     * @brief Engine constructor.
     *
     * As many engines as needed can be instantiated, although one should use
     * the static methods \ref download() and \ref upload() instead.
     */
    explicit Engine(QWidget* parent = 0);

    /**
     * \brief Engine destructor.
     *
     * Deletes an engine and frees all memory occupied by it.
     */
    ~Engine();

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
    KNS::Entry::List downloadDialogModal(QWidget* parent = 0);

    /**
     * @brief Recommended download workflow entry point.
     *
     * This method is a static convenience wrapper around \ref downloadDialogModal()
     * which does not require the manual construction of an engine object.
     * The engine will be configured to load appname.knsrc.
     * The resulting entry list must not be freed, as the engine will continue
     * to keep track of it.
     *
     * @return List of installed or deinstalled entries
     * NOTE: this is a copy of each entry because this call deletes its own
     * engine which deletes the original entries, so you need to delete the entries
     * returned in the list
     *
     * @see downloadDialogModal()
     */
    static KNS::Entry::List download();

    /**
     * @brief Synchronous way of starting the upload workflow.
     *
     * Starts the upload workflow. This workflow will offer provider
     * selection and afterwards upload all files associated with an entry.
     * This method is a modal one. It will return the uploaded entry.
     *
     * @return Uploaded entry, or \b null in case of failures
     */
    KNS::Entry * uploadDialogModal(const QString& file);

    /**
     * @brief Recommended upload workflow entry point.
     *
     * This method is a static convenience wrapper around \ref uploadDialogModal()
     * which does not require the manual construction of an engine object.
     * The engine will be configured to load appname.knsrc.
     * The resulting entry must not be freed, as the engine will continue
     * to keep track of it.
     *
     * @return Uploaded entry, or \b null in case of failures
     *
     * @see uploadDialogModal()
     */
    static KNS::Entry * upload(const QString& file);

    /**
     * @brief Asynchronous way of starting the download workflow.
     *
     * This method should be used whenever a blocking application with a
     * non-blocking GUI during GHNS operations is not suitable.
     *
     * @see downloadDialogModal()
     */
    void downloadDialog();

    /**
     * @brief Asynchronous way of starting the download workflow and getting feedback to a slot when the dialog closes.
     *
     * This method should be used whenever a blocking application with a
     * non-blocking GUI during GHNS operations is not suitable.
     * the slot passed in should have the signature (KNS::Entry::List) as its parameter
     *
     * @see downloadDialogModal()
     */
    void downloadDialog(QObject * receiver, const char * slot);

    /**
     * @brief Asynchronous way of starting the upload workflow.
     *
     * This method should be used whenever a blocking application with a
     * non-blocking GUI during GHNS operations is not suitable.
     * The affected entry will be reported by signals.
     *
     * @see uploadDialogModal()
     */
    void uploadDialog(const QString& file);

    bool init(const QString& config);

private:

    friend class EnginePrivate;
    EnginePrivate* const d;
    Q_DISABLE_COPY(Engine)
};

}

#endif
