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
    std::cout << msg.toLocal8Bit().constData() << std::endl;
}

class KConfigToJsonPrivate {
public:
    QString pluginName;
    QCommandLineParser *parser;
    QString inFile;
    QString outFile;
};

KConfigToJson::KConfigToJson(int& argc, char** argv, QCommandLineParser *parser) :
    QCoreApplication(argc, argv)
{
    d = new KConfigToJsonPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, SLOT(runMain()));
}

KConfigToJson::~KConfigToJson()
{
    delete d;
}

void KConfigToJson::runMain()
{
    d->parser->process(*this);
    if (d->parser->isSet("input")) {
        if (!resolveFiles()) {
            qDebug() << "Failed to resolve filenames" << d->inFile << d->outFile;
            exit(1);
            return;
        };
        convert(d->inFile, d->outFile);
        exit(0);

    } else {
        // Let the event loop run once more to show help
        coutput("Usage --help. In short: kconfigtojson -i inputfile.desktop -o outputfile.json");
        QTimer::singleShot(50, this, SLOT(quit()));
    }
}

bool KConfigToJson::resolveFiles()
{
    if (d->parser->isSet("input")) {
        d->inFile = d->parser->value("input");
        if (QFile::exists(d->inFile)) {
            if (!d->inFile.startsWith('/')) {
                d->inFile = QDir::currentPath() + '/' + d->inFile;
            }
        } else {
            coutput("File not found: " + d->inFile);
            return false;
        }
    }
    if (d->parser->isSet("output")) {
        d->outFile = d->parser->value("output");
    } else {
        if (!d->inFile.isEmpty()) {
            d->outFile = d->inFile;
            d->outFile.replace(".desktop", ".json");
        }
    }

    return d->inFile != d->outFile && !d->inFile.isEmpty() && !d->outFile.isEmpty();
}

void KConfigToJson::convert(const QString& src, const QString& dest)
{
    KDesktopFile df(src);
    KConfigGroup c = df.desktopGroup();

    const QStringList boolkeys = QStringList()
            << "Hidden" << "X-KDE-PluginInfo-EnabledByDefault";
    const QStringList stringlistkeys = QStringList()
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
        exit(1);
        return;
    }

    QTextStream out(&file);
    out << jdoc.toJson();
    file.close();

    coutput("Converted " + src + " to " + dest);
}


