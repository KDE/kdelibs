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

#ifndef QDECLARATIVEMIMETYPE_P_H_INCLUDED
#define QDECLARATIVEMIMETYPE_P_H_INCLUDED

#include <QtCore/QObject>
#include <qmimetype.h>
#include <QtDeclarative/qdeclarative.h>

QT_BEGIN_NAMESPACE class QString; QT_END_NAMESPACE
QT_BEGIN_NAMESPACE class QStringList; QT_END_NAMESPACE

// ------------------------------------------------------------------------------------------------

class QDeclarativeMimeType : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name
               READ name
               WRITE setName)

#if 0
    Q_PROPERTY(QVariantList aliases
               READ aliases
               WRITE setAliases
               STORED false)

    Q_PROPERTY(QString comment
               READ comment
               WRITE setComment)
#endif

    Q_PROPERTY(QString genericIconName
               READ genericIconName
               WRITE setGenericIconName)

    Q_PROPERTY(QString iconName
               READ iconName
               WRITE setIconName)

    Q_PROPERTY(QVariantList globPatterns
               READ globPatterns
               WRITE setGlobPatterns
               STORED false)

    Q_PROPERTY(QVariantList suffixes
               READ suffixes
               STORED false)

    Q_PROPERTY(QString preferredSuffix
               READ preferredSuffix
               STORED false)

    Q_PROPERTY(bool isValid
               READ isValid
               STORED false)

protected:
    // We keep this destructor with its default value of 0 protected since
    // only QDeclarativePrivate::QDeclarativeElement<T> needs it:
    QDeclarativeMimeType(QObject *theParent = 0);

public:
    // We don't allow theParent to have a default value of 0 because in all
    // likelyhood we want to force the caller to specify its QObject so the
    // object will get destroyed in the caller's destructor:
    QDeclarativeMimeType(const QMimeType &other, QObject *theParent);
#ifdef Q_COMPILER_RVALUE_REFS
    QDeclarativeMimeType(QMimeType &&other, QObject *theParent);
#endif

    ~QDeclarativeMimeType();

    Q_INVOKABLE void assign(QDeclarativeMimeType *other);
    Q_INVOKABLE bool equals(QDeclarativeMimeType *other) const;

    Q_INVOKABLE QVariantMap properties() const;
    Q_INVOKABLE void assignProperties(const QVariantMap &other);
    Q_INVOKABLE bool equalsProperties(const QVariantMap &other) const;

    QMimeType mimeType() const;

    bool isValid() const;

    QString name() const;
    void setName(const QString &newName);
#if 0
    QVariantList aliases() const;
    void setAliases(const QVariantList &newAliases);
    QString comment() const;
    void setComment(const QString &newComment);
#endif
    QString genericIconName() const;
    void setGenericIconName(const QString &newGenericIconName);
    QString iconName() const;
    void setIconName(const QString &newIconName);
    QVariantList globPatterns() const;
    void setGlobPatterns(const QVariantList &newGlobPatterns);
    QVariantList suffixes() const;
    QString preferredSuffix() const;

private:
    QMimeType m_MimeType;
};

QML_DECLARE_TYPE(QDeclarativeMimeType)

#endif   // QDECLARATIVEMIMETYPE_P_H_INCLUDED
