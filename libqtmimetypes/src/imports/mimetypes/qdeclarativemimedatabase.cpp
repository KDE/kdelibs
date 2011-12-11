/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtMimeTypes addon of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativemimedatabase_p.h"   // Basis

#include "qdeclarativemimetype_p.h"

#include <QtCore/QDebug>

// ------------------------------------------------------------------------------------------------

extern bool isQMimeDatabaseDebuggingActivated;

#ifndef QT_NO_DEBUG_OUTPUT
#define DBG() if (isQMimeDatabaseDebuggingActivated) qDebug() << Q_FUNC_INFO
#else
#define DBG() if (0) qDebug() << Q_FUNC_INFO
#endif

// ------------------------------------------------------------------------------------------------

/*!
    \qmlclass MimeDatabase QDeclarativeMimeDatabase
    \brief The QML MimeDatabase element maintains a database of MIME types.
 */

// ------------------------------------------------------------------------------------------------

/*!
    \class QDeclarativeMimeDatabase
    \brief The QDeclarativeMimeDatabase class is the QML wrapper for the class QMimeDatabase which maintains a database of MIME types.
    \inherits QObject
 */

// ------------------------------------------------------------------------------------------------

/*!
    \fn QDeclarativeMimeDatabase::QDeclarativeMimeDatabase(QObject *theParent);
    \brief Performs default initialization of the contained MimeDatabase, and attaches the object to the specified parent for destruction.
 */
QDeclarativeMimeDatabase::QDeclarativeMimeDatabase(QObject *theParent) :
        QObject(theParent),
        m_MimeDatabase()
{
    DBG() << "mimeTypeNames():" << mimeTypeNames();
}

// ------------------------------------------------------------------------------------------------

/*!
    \fn QDeclarativeMimeDatabase::~QDeclarativeMimeDatabase();
    \brief Destroys the the contained MimeDatabase.
 */
QDeclarativeMimeDatabase::~QDeclarativeMimeDatabase()
{
    DBG() << "mimeTypeNames():" << mimeTypeNames();
}

// ------------------------------------------------------------------------------------------------

/*!
    \fn QMimeDatabase &QDeclarativeMimeDatabase::mimeDatabase()
    \brief Returns the contained MimeDatabase.
 */
QMimeDatabase &QDeclarativeMimeDatabase::mimeDatabase()
{
    return m_MimeDatabase;
}

// ------------------------------------------------------------------------------------------------

/*!
    \qmlproperty QVariantList MimeDatabase::mimeTypeNames
    list of registered MIME types
 */

// ------------------------------------------------------------------------------------------------

QVariantList QDeclarativeMimeDatabase::mimeTypeNames() const
{
    QVariantList result;

    foreach (const QMimeType &mimeType, m_MimeDatabase.allMimeTypes()) {
        result << mimeType.name();
    }

    return result;
}

// ------------------------------------------------------------------------------------------------

/*!
    \qmlmethod MimeType MimeDatabase::mimeTypeForName(string nameOrAlias)
    \brief Returns a MIME type for \a nameOrAlias or an invalid one if none found.
 */

// ------------------------------------------------------------------------------------------------

/*!
    \property QDeclarativeMimeDatabase::mimeTypeNames
    \brief the list of registered MIME types
 */

// ------------------------------------------------------------------------------------------------

QDeclarativeMimeType *QDeclarativeMimeDatabase::mimeTypeForName (
                                                    const QString &nameOrAlias
                                                )
{
    return new QDeclarativeMimeType (
                   m_MimeDatabase.mimeTypeForName(nameOrAlias),
                   this   // <- The new object will be released later
                          //    when this registry is released.
               );
}

// ------------------------------------------------------------------------------------------------

/*!
    \qmlmethod MimeType MimeDatabase::findByName(string fileName)
    \brief Returns a MIME type for the file \a fileName.

    A valid MIME type is always returned. If the file name doesn't match any
    known pattern, the default MIME type (application/octet-stream)
    is returned.

    This function does not try to open the file. To also use the content
    when determining the MIME type, use findByFile().
*/
QDeclarativeMimeType *QDeclarativeMimeDatabase::findByName (
                                                    const QString &fileName
                                                )
{
    return new QDeclarativeMimeType (
                   m_MimeDatabase.findByName(fileName),
                   this   // <- The new object will be released later
                          //    when this registry is released.
               );
}

// ------------------------------------------------------------------------------------------------

/*!
    \qmlmethod MimeType MimeDatabase::findByFile(string fileName)
    \brief Returns a MIME type for \a fileName.

    This method looks at both the file name and the file contents,
    if necessary. The file extension has priority over the contents,
    but the contents will be used if the file extension is unknown, or
    matches multiple MIME types.

    A valid MIME type is always returned. If the file doesn't match any
    known pattern or data, the default MIME type (application/octet-stream)
    is returned.

    The \a fileName can also include an absolute or relative path.
*/
QDeclarativeMimeType *QDeclarativeMimeDatabase::findByFile (
                                                    const QString &fileName
                                                )
{
    return new QDeclarativeMimeType (
                   m_MimeDatabase.findByFile(fileName),
                   this   // <- The new object will be released later
                          //    when this registry is released.
               );
}

// ------------------------------------------------------------------------------------------------

#undef DBG
