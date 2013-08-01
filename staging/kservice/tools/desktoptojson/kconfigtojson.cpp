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


#include <qcommandlineparser.h>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <kdesktopfile.h>
#include <kconfiggroup.h>


static QTextStream cout(stdout);
static QTextStream cerr(stderr);

static const QString INPUT = QStringLiteral("input");
static const QString OUTPUT = QStringLiteral("output");


KConfigToJson::KConfigToJson(int &argc, char **argv, QCommandLineParser *parser)
  : m_parser(parser)
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)
}

KConfigToJson::~KConfigToJson()
{
}

int KConfigToJson::runMain()
{
    if (m_parser->isSet(INPUT)) {
        if (!resolveFiles()) {
            cerr << "Failed to resolve filenames" << m_inFile << m_outFile << endl;;
            return 1;
        };
        if (convert(m_inFile, m_outFile)) {
            return 0;
        } else {
            return 1;
        }
    } else {
        cout << "Usage --help. In short: desktoptojson -i inputfile.desktop -o outputfile.json" << endl;
        return 1;
    }
}

bool KConfigToJson::resolveFiles()
{
    if (m_parser->isSet(INPUT)) {
        m_inFile = m_parser->value(INPUT);
        if (QFile::exists(m_inFile)) {
            if (!m_inFile.startsWith('/')) {
                m_inFile = QDir::currentPath() + '/' + m_inFile;
            }
        } else {
            cerr << "File not found: " + m_inFile;
            return false;
        }
    }
    if (m_parser->isSet(OUTPUT)) {
        m_outFile = m_parser->value(OUTPUT);
    } else {
        if (!m_inFile.isEmpty()) {
            m_outFile = m_inFile;
            m_outFile.replace(QStringLiteral(".desktop"), QStringLiteral(".json"));
        }
    }

    return m_inFile != m_outFile && !m_inFile.isEmpty() && !m_outFile.isEmpty();
}

bool KConfigToJson::convert(const QString &src, const QString &dest)
{
    KDesktopFile df(src);
    KConfigGroup c = df.desktopGroup();

    static const QSet<QString> boolkeys = QSet<QString>()
            << QStringLiteral("Hidden") << QStringLiteral("X-KDE-PluginInfo-EnabledByDefault");
    static const QSet<QString> stringlistkeys = QSet<QString>()
            << QStringLiteral("X-KDE-ServiceTypes") << QStringLiteral("X-KDE-PluginInfo-Depends");

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
        cerr << "Failed to open " << dest << endl;
        return false;
    }

    file.write(jdoc.toJson());
    cout << "Converted " << src << " to " << dest << endl;;
    return true;
}


