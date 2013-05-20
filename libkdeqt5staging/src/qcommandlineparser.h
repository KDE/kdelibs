/****************************************************************************
**
** Copyright (C) 2012 author Laszlo Papp <lpapp@kde.org>
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QCOMMANDLINEPARSER_H
#define QCOMMANDLINEPARSER_H

#include <qstringlist.h>
#include <qcommandlineoption.h>
#include <qcoreapplication.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QCommandLineParserPrivate;

class KDEQT5STAGING_EXPORT QCommandLineParser
{
    Q_DECLARE_TR_FUNCTIONS(QCommandLineParser)
public:
    bool addOption(const QCommandLineOption &commandLineOption);

    void addVersionOption(const QString &version);
    QString applicationVersion() const;

    void addHelpOption(const QString &description);
    QString applicationDescription() const;

    bool isSet(const QString &name) const;

    QString argument(const QString &name) const;

    QStringList arguments(const QString &name) const;

    QStringList remainingArguments() const;

    QStringList optionNames() const;

    void setAbortOnUnknownOptions(bool b);
    QStringList unknownOptionNames() const;

    void showHelp();

protected:
    friend class QCoreApplication;

private:
    Q_DISABLE_COPY(QCommandLineParser)

public: // HACK FOR KDEQT5STAGING
    QCommandLineParser();
    ~QCommandLineParser();
private:

    QCommandLineParserPrivate* const d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCOMMANDLINEPARSER_H
