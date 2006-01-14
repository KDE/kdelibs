/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KNEWSTUFF_ENGINE_H
#define KNEWSTUFF_ENGINE_H

#include <qmap.h>
#include <qobject.h>
#include <qstring.h>

#include "entry.h"
#include "provider.h"

namespace KIO { class Job; }

class KNewStuff;

namespace KNS {

class DownloadDialog;
class UploadDialog;
class ProviderDialog;

/**
 * @short Central class combining all possible KNewStuff operations.
 *
 * In most cases, Engine objects are built and used internally.
 * Using this class explicitely does however give fine-grained control about the
 * upload and download operations.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class KDE_EXPORT Engine : public QObject
{
    Q_OBJECT
    struct Private;
  public:
    /**
      Constructor.

      @param newStuff a KNewStuff object
      @param type the Hotstuff data type such as "korganizer/calendar"
      @param parentWidget the parent window
    */
    Engine( KNewStuff *newStuff, const QString &type, QWidget *parentWidget = 0 );
    /**
      Constructor.

      @param newStuff a KNewStuff object
      @param type the Hotstuff data type such as "korganizer/calendar"
      @param providerList the URL of the provider list
      @param parentWidget the parent window
    */
    Engine( KNewStuff *newStuff, const QString &type, const QString &providerList, QWidget *parentWidget = 0 );

    /**
      Destructor.
    */
    virtual ~Engine();

    /**
      Returns the previously set data type.

      @return the Hotstuff data type
    */
    QString type() const { return mType; }

    /**
      Returns the previously set parent widget.

      @return parent widget
    */
    QWidget *parentWidget() const { return mParentWidget; }

    /**
      Initiates the download process, retrieving provider lists and invoking
      the download dialog.
    */
    void download();

    /**
      Initiates the upload process, invoking the provider selection dialog
      and the file upload dialog.

      @param fileName name of the payload data file
      @param previewName name of the preview image file
    */
    void upload( const QString &fileName = QString(), const QString &previewName = QString() );

    /**
      Downloads the specified data file.

      @param entry the Hotstuff data object to be downloaded
    */
    void download( Entry *entry );

    /**
      Asynchronous lookup of provider information such as upload and
      download locations, icon etc.

      @param provider the Hotstuff provider to request information from
    */
    void requestMetaInformation( Provider *provider );

    /**
      Uploads the specified data file to the provider-dependent location.

      @param entry the Hotstuff data object to be uploaded
    */
    void upload( Entry *entry );
    
    /**
    Ignores the return value of the install method. Used internally to
    avoid showing of the success/failure dialog when installation is done
    in another place, like in @ref KNewStuffSecure
     */
    void ignoreInstallResult(bool ignore);   

  signals:
    /** Emitted when the upload has finished.
      @param result indicates the success/failure of the upload
    */
    void uploadFinished( bool result );
  protected slots:
    void getMetaInformation( Provider::List *providers );
    void selectUploadProvider( Provider::List *providers );

    void slotNewStuffJobData( KIO::Job *job, const QByteArray &data );
    void slotNewStuffJobResult( KIO::Job *job );

    void slotDownloadJobResult( KIO::Job *job );

    void slotUploadPayloadJobResult( KIO::Job *job );
    void slotUploadPreviewJobResult (KIO::Job *job );
    void slotUploadMetaJobResult( KIO::Job *job );

  protected:
    bool createMetaFile( Entry * );
    
  private:
    QWidget *mParentWidget;

    ProviderLoader *mProviderLoader;

    QMap<KIO::Job *,QString> mNewStuffJobData;
    QMap<KIO::Job *,Provider *> mProviderJobs;

    QList<Entry*> mNewStuffList;

    DownloadDialog *mDownloadDialog;
    UploadDialog *mUploadDialog;
    ProviderDialog *mProviderDialog;

    QString mDownloadDestination;

    Provider *mUploadProvider;

    QString mUploadMetaFile;
    QString mUploadFile;
    QString mPreviewFile;
    QString mProviderList;

    Private* const d;

    QString mType;
};

}

#endif
