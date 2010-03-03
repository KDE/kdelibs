/*
    knewstuff3/ui/uploaddialog.h.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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
#ifndef KNEWSTUFF3_UI_UPLOADDIALOG_H
#define KNEWSTUFF3_UI_UPLOADDIALOG_H

#include <kdialog.h>
#include <kurl.h>

#include <knewstuff3/knewstuff_export.h>

class KComboBox;
class KLineEdit;
class KUrlRequester;
class KTextEdit;

namespace Attica {
    class BaseJob;
    class Provider;
}

namespace KNS3
{
/**
 * @short KNewStuff file upload dialog.
 *
 * Using this dialog, data can easily be uploaded to the Hotstuff servers.
 *
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 *
 * @since 4.4
 */
class KNEWSTUFF_EXPORT UploadDialog : public KDialog
{
    Q_OBJECT
public:
    /**
      Constructor.

      @param parent the parent window
    */
    explicit UploadDialog(QWidget *parent = 0);
    explicit UploadDialog(const QString& configFile, QWidget *parent = 0);

    /**
      Destructor.
    */
    ~UploadDialog();

    /**
      Set the file to be uploaded.

      @param payloadFile the payload data file
    */
    void setUploadFile(const KUrl& payloadFile);

    /**
      Set the suggested name for the upload.
      The application can suggest a title which can then be edited by the user before uploading.
      The name field will be left empty if no title was set.

      @param name the suggested name for the upload
    */
    void setUploadName(const QString& name);

public Q_SLOTS:
    virtual void accept();

private Q_SLOTS:
    void contentAdded(Attica::BaseJob*);

    void fileUploadFinished(Attica::BaseJob*);

    void doUpload(const QString& index, const QString& filePath);

    void providerAdded(const Attica::Provider& provider);
    void categoriesLoaded(Attica::BaseJob* job);

    void previewChanged(const KUrl&);

    void priceToggled(bool);

private:
    bool init(const QString &configfile);

    class Private;
    Private *const d;
};

}

#endif
