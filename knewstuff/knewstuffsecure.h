/***************************************************************************
                          qnewstuff.h  -  description
                             -------------------
    begin                : Tue Jun 22 12:19:55 2004
    copyright          : (C) 2004 by Andras Mantia <amantia@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; version 2 of the License.  *
 *                                                                         *
 ***************************************************************************/

#ifndef QNEWSTUFF_H
#define QNEWSTUFF_H

//qt includes
#include <qobject.h>

//kde includes
#include <knewstuff/knewstuff.h>

class KTempDir;
/**
Makes possible downloading and installing resource files from a server.
You must subclass it and implement the @ref installResource() pure
virtual method.

@author Andras Mantia
*/
class QNewStuff : public  QObject, public KNewStuff
{
  Q_OBJECT

public:
    QNewStuff(const QString &type,  QWidget *parentWidget=0);
    ~QNewStuff();

    /** Installs the downloaded resource */
    bool install( const QString &fileName );
    
    /** Overriden for internal reasons. */
    bool createUploadFile(const QString &fileName);
    
    /** Initiates a download. This is the method that must be called in
    * order to download a signed resource */
    void downloadResource();
    
    /** Signs the file and uploads to the central server. Returns the
    * temporary directory used to create the final tarball. This directory
    * should be cleaned and deleted on application exit, but not sooner,
    * as there is a risk of deleting it before it's actually uploaded.
    * @param fileName The file to be signed and uploaded
    */
    KTempDir* uploadResource(const QString &fileName);

private slots:
    /** Checks the validity of the downloaded tarball and installs it*/
    void slotValidated(int result);
    /** The file is signed, so it can be uploaded.*/
    void slotFileSigned(int result);

signals:
    void installFinished();

protected:
  /** Installs the resource specified by @ref m_tarName. Implement it in the subclass. */
    virtual void installResource() = 0;

    KTempDir *m_tempDir;
    QString m_tarName;
    QString m_signedFileName;
    QMap<QString, QString> m_installedResources;
};

#endif
