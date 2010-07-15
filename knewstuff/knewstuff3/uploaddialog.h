/*
    knewstuff3/ui/uploaddialog.h.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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

namespace Attica {
    class BaseJob;
    class Provider;
}

// KDE5: this class should inherit from the wizard class - KAssistantDialog

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
      Create a new upload dialog.

      @param parent the parent window
    */
    explicit UploadDialog(QWidget *parent = 0);

    /**
      Create a new upload dialog.

      @param parent the parent window
    */
    explicit UploadDialog(const QString& configFile, QWidget *parent = 0);

    /**
      Destructor.
    */
    ~UploadDialog();

    /**
      Set the file to be uploaded.
      This has to be set for the dialog to work, before displaying the dialog.

      @param payloadFile the payload data file
    */
    void setUploadFile(const KUrl& payloadFile);

    /**
      Set the suggested title for the upload.
      The application can suggest a title which can then be edited by the user before uploading.
      The name field will be left empty if no title was set.

      @param name the suggested name for the upload
    */
    void setUploadName(const QString& name);

    /**
      Set the suggested version displayed in the upload dialog.
      The user can still change this.
      @param version
      */
    void setVersion(const QString& version);

    /**
      Set the suggested description displayed in the upload dialog.
      The user can still change this.
      @param description
      */
    void setDescription(const QString& description);

    /**
      Set the suggested changelog displayed in the upload dialog.
      The user can still change this.
      @param version version
      */
    void setChangelog(const QString& changelog);

    /* *
      Set the suggested license displayed in the upload dialog.
      The user can still change this.
      @param version version
      */
    // enum License {}; // see fd.o api spec
    // void setLicense(License license);

    /**
      Set one of the threee preview images displayed in the upload dialog.
      The user can still change this.
      @param number The number of the preview image to set, either 1, 2, or 3.
      @param file A URL to the file to be used as preview image
      @since 4.6
      */ 
    void setPreviewImageFile(uint number, const KUrl& file);

    /**
     Enable the UI to let the user to set a price for the uploaded item.
     @param enabled enable the price option - it is enabled by default
     @since 4.5
     */
    void setPriceEnabled(bool enabled);

    /**
      Set the suggested price displayed in the upload dialog.
      The user can still change this.
      @param version version
      */
    void setPrice(double price);

    /**
      Set the suggested rationale why this item costs something to download.
      The user can still change this.
      @param version version
      */
    void setPriceReason(const QString& reason);

    /**
      Set the suggested category for the upload.
      The .knsrc file may contain multiple upload categories, this sets which one is pre-selected.
      It does not add any new category to the list of available categories.

      @param category the suggested category for the upload
    */
    void selectCategory(const QString& category);

public Q_SLOTS:
    virtual void accept();

private:
    bool init(const QString &configfile);

    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void _k_nextPage() )
    Q_PRIVATE_SLOT( d, void _k_backPage() )
    Q_PRIVATE_SLOT( d, void _k_updatePage() )

    Q_PRIVATE_SLOT( d, void _k_providerChanged(QString) )
    Q_PRIVATE_SLOT( d, void _k_checkCredentialsFinished(bool) )
    Q_PRIVATE_SLOT( d, void _k_contentByCurrentUserLoaded(Attica::Content::List) )
    Q_PRIVATE_SLOT( d, void _k_providersLoaded(QStringList) )
    Q_PRIVATE_SLOT( d, void _k_categoriesLoaded(Attica::Category::List) )
    Q_PRIVATE_SLOT( d, void _k_licensesLoaded(Attica::License::List) )
    Q_PRIVATE_SLOT( d, void _k_currencyLoaded(QString) )
    Q_PRIVATE_SLOT( d, void _k_previewLoaded(int, QImage) )

    Q_PRIVATE_SLOT( d, void _k_changePreview1() )
    Q_PRIVATE_SLOT( d, void _k_changePreview2() )
    Q_PRIVATE_SLOT( d, void _k_changePreview3() )
    Q_PRIVATE_SLOT( d, void _k_priceToggled(bool) )
    Q_PRIVATE_SLOT( d, void _k_updateContentsToggled(bool update) )

    Q_PRIVATE_SLOT( d, void _k_startUpload() )
    Q_PRIVATE_SLOT( d, void _k_contentAdded(Attica::BaseJob*) )
    Q_PRIVATE_SLOT( d, void _k_fileUploadFinished(Attica::BaseJob*) )
    Q_PRIVATE_SLOT( d, void _k_preview1UploadFinished(Attica::BaseJob*) )
    Q_PRIVATE_SLOT( d, void _k_preview2UploadFinished(Attica::BaseJob*) )
    Q_PRIVATE_SLOT( d, void _k_preview3UploadFinished(Attica::BaseJob*) )

    Q_PRIVATE_SLOT( d, void _k_updatedContentFetched(Attica::Content) )
    Q_PRIVATE_SLOT( d, void _k_detailsLinkLoaded(QUrl) )

    Q_DISABLE_COPY( UploadDialog )
};

}

#endif
