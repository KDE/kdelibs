/*
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef BROWSEROPENORSAVEQUESTION_H
#define BROWSEROPENORSAVEQUESTION_H

#include "kparts_export.h"
#include <kdialog.h>
#include "browserrun.h"
#include <kservice.h>

namespace KParts {

class BrowserOpenOrSaveQuestionPrivate;

/**
 * This class shows the dialog that asks the user whether to
 * save a url or open a url in another application.
 *
 * It also has the variant which asks "save or embed" (e.g. into konqueror).
 *
 * @since 4.4
 */
class KPARTS_EXPORT BrowserOpenOrSaveQuestion
{
public:
    /**
     * Constructor, for all kinds of dialogs shown in this class.
     * @param url the URL in question
     * @param mimeType the mimetype of the URL
     */
    BrowserOpenOrSaveQuestion(QWidget* parent, const KUrl& url, const QString& mimeType);
    ~BrowserOpenOrSaveQuestion();

    /**
     * Sets the suggested filename, shown in the dialog.
     * @param suggestedFileName optional file name suggested by the server (HTTP Content-Disposition)
     */
    void setSuggestedFileName(const QString& suggestedFileName);

    /**
     * Set of features that should be enabled in this dialog.
     * This allows to add features before making all applications ready for those features
     * (e.g. applications need to read selectedService() otherwise the dialog should not
     * show the service selection button)
     */
    enum Feature { BasicFeatures = 0, /**< Only the basic save, open, embed, cancel button */
                   ServiceSelection = 1 /**< Shows "Open With..." with the associated applications for the mimetype */
    };
    Q_DECLARE_FLAGS(Features, Feature)

    /**
     * Enables the given features in the dialog
     */
    void setFeatures(Features features);

    enum Result { Save, Open, Embed, Cancel };
    
    /**
     * Ask the user whether to save or open a url in another application.
     * @param parent parent widget for the dialog
     * @return Save, Open or Cancel.
     */
    Result askOpenOrSave();

    /**
     * Ask the user whether to save or open a url in another application.
     * @param parent parent widget for the dialog
     * @param flags set to BrowserRun::AttachmentDisposition if suggested by the server
     * This is used because by default text/html files are opened embedded in browsers, not saved.
     * But if the server said "attachment", it means the user is download a file for saving it.
     * @return Save, Embed or Cancel.
     */
    Result askEmbedOrSave(int flags = 0);
    // KDE5 TODO: move BrowserRun::AskEmbedOrSaveFlags to this class.
    
    // TODO askOpenEmbedOrSave

    /**
     * @return the service that was selected during askOpenOrSave,
     * if it returned Open.
     * In all other cases (no associated application, Save or Cancel
     * selected), this returns 0.
     *
     * Requires setFeatures(BrowserOpenOrSaveQuestion::ServiceSelection).
     */
    KService::Ptr selectedService() const;

private:
    BrowserOpenOrSaveQuestionPrivate* const d;
};

}

#endif /* BROWSEROPENORSAVEQUESTION_H */

