/***************************************************************************
                        knewstuffsecure.h  -  description
                             -------------------
    begin               : Tue Jun 22 12:19:55 2004
    copyright          : (C) 2004, 2005 by Andras Mantia <amantia@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; version 2 of the License.  *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWSTUFFSECURE_H
#define KNEWSTUFFSECURE_H

//qt includes
#include <qobject.h>

//kde includes
#include "knewstuff.h"

class KTempDir;
/**
Makes possible downloading and installing signed resource files from a server.
You must subclass it and implement the @ref installResource() pure
virtual method to install a resource. For uploading you must create a resource
tarball (which is installabale by @ref installResource()) and call the 
@ref uploadResource() method with this tarball as the argument.
Signing and verification is done by the gpg application, so the user must
have it installed, otherwise this class does not give any extra security compared
to the standard KNewStuff class.

@since 3.4

@author Andras Mantia <amantia@kde.org>
*/
class KDE_EXPORT KNewStuffSecure : public  QObject, public KNewStuff
{
  Q_OBJECT

public:
    /** Constructor.

      @param type type of data to be handled, should be something like
             korganizer/calendar, kword/template, kdesktop/wallpaper
      @param parentWidget parent widget of dialogs opened by the KNewStuff
              engine
     */
    KNewStuffSecure(const QString &type,  QWidget *parentWidget=0);
    virtual ~KNewStuffSecure();

    /** Installs the downloaded resource. Do not call or reimplement directly. 
        It's reimplemented from KNewStuff for internal reasons. 
    */
    bool install( const QString &fileName );
    
    /** Reimplemented for internal reasons. */
    bool createUploadFile(const QString &fileName);
    
    /** Initiates a download. This is the method that must be called in
    * order to download a signed resource. */
    void downloadResource();
    
    /** Signs the file and uploads to the central server.
    * @param fileName The file to be signed and uploaded
    */
    void uploadResource(const QString &fileName);
    

private Q_SLOTS:
    /** Checks the validity of the downloaded tarball and installs it*/
    void slotValidated(int result);
    /** The file is signed, so it can be uploaded.*/
    void slotFileSigned(int result);
    /** Called when the upload has finished. 
        @param result the result of the upload
        Be careful if you reimplement it, as it deletes the temporary directory
        m_tempDir used for upload. You must also delete it (call the parent's method)
        if you reimplement it.
    */
    void slotUploadFinished(bool result);  

Q_SIGNALS:
    void installFinished();
    
protected:
   /** Installs the resource specified by m_tarName. Implement it in the subclass. */
    virtual void installResource() = 0;
    /** Removes the temporary directory m_tempDir. */
    void removeTempDirectory();    
    
    KTempDir *m_tempDir;
    QString m_tarName; 
    QString m_signedFileName;
    QMap<QString, QString> m_installedResources;
};

#endif
