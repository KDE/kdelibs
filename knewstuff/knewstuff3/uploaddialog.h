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
#include <attica/provider.h>

class KComboBox;
class KLineEdit;
class KUrlRequester;
class KTextEdit;

namespace Attica {
    class BaseJob;
}

namespace KNS3
{
/**
 * @short KNewStuff file upload dialog.
 *
 * Using this dialog, data can easily be uploaded to the Hotstuff servers.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 *
 * @internal
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
      Sets the payload filename.
      This is optional, but necessary if the application wants to support
      reusing previously filled out form data based on the filename.

      @param payloadFile the payload data file
    */
    void setUploadFile(const KUrl& payloadFile);

public Q_SLOTS:
    virtual void accept();

private Q_SLOTS:
    void contentAdded(Attica::BaseJob*);

    void fileUploadFinished(Attica::BaseJob*);

    void doUpload(const QString& index, const QString& filePath);

    void providerAdded(const Attica::Provider& provider);
    void categoriesLoaded(Attica::BaseJob* job);

    void previewChanged(const KUrl&);

private:
    bool init(const QString &configfile);

    class Private;
    Private *const d;
};

}

#endif
