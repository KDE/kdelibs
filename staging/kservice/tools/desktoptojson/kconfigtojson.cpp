/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "kconfigtojson.h"

//#include <kqpluginfactory.h>
#include <klocalizedstring.h>


#include <qcommandlineparser.h>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTimer>
#include <QDebug>

#include <kconfig.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>

#include <iostream>
#include <iomanip>

void coutput(const QString &msg)
{
    QTextStream out(stdout);
    out << msg.toLocal8Bit().constData() << "\n";
}

KConfigToJson::KConfigToJson(int& argc, char** argv, QCommandLineParser *parser)
{
    m_parser = parser;
}

KConfigToJson::~KConfigToJson()
{
}

int KConfigToJson::runMain()
{
    if (m_parser->isSet(QStringLiteral("input"))) {
        if (!resolveFiles()) {
            qDebug() << "Failed to resolve filenames" << m_inFile << m_outFile;
            return 1;
        };
        if (convert(m_inFile, m_outFile)) {
            return 0;
        } else {
            return 1;
        }
    } else {
        coutput("Usage --help. In short: desktoptojson -i inputfile.desktop -o outputfile.json");
        return 1;
    }
}

bool KConfigToJson::resolveFiles()
{
    if (m_parser->isSet(QStringLiteral("input"))) {
        m_inFile = m_parser->value("input");
        if (QFile::exists(m_inFile)) {
            if (!m_inFile.startsWith('/')) {
                m_inFile = QDir::currentPath() + '/' + m_inFile;
            }
        } else {
            coutput("File not found: " + m_inFile);
            return false;
        }
    }
    if (m_parser->isSet(QStringLiteral("output"))) {
        m_outFile = m_parser->value("output");
    } else {
        if (!m_inFile.isEmpty()) {
            m_outFile = m_inFile;
            m_outFile.replace(".desktop", ".json");
        }
    }

    return m_inFile != m_outFile && !m_inFile.isEmpty() && !m_outFile.isEmpty();
}

bool KConfigToJson::convert(const QString& src, const QString& dest)
{
    KDesktopFile df(src);
    KConfigGroup c = df.desktopGroup();

    static const QStringList boolkeys = QStringList()
            << "Hidden" << "X-KDE-PluginInfo-EnabledByDefault";
    static const QStringList stringlistkeys = QStringList()
            << "X-KDE-ServiceTypes" << "X-KDE-PluginInfo-Depends";

    QVariantMap vm;
    foreach (const QString &k, c.keyList()) {
        if (boolkeys.contains(k)) {
            vm[k] = c.readEntry(k, false);
        } else if (stringlistkeys.contains(k)) {
            vm[k] = c.readEntry(k, QStringList());
        } else {
            vm[k] = c.readEntry(k, QString());
        }
    }

    QJsonObject jo = QJsonObject::fromVariantMap(vm);
    QJsonDocument jdoc;
    jdoc.setObject(jo);

    QFile file(dest);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        coutput("Failed to open " + dest);
        return false;
    }

    file.write(jdoc.toJson());
    coutput("Converted " + src + " to " + dest);
    return true;
}


