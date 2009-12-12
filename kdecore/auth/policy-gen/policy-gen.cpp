/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "policy-gen.h"

#include <QCoreApplication>
#include <QSettings>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

using namespace std;

QList<Action> parse(QSettings &ini);
QHash<QString, QString> parseDomain(QSettings &ini);

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        qCritical("Too few arguments");
        return 1;
    }

    QSettings ini(argv[1], QSettings::IniFormat);
    if (ini.status()) {
        qCritical("Error loading file: %s", argv[1]);
        return 1;
    }

    output(parse(ini), parseDomain(ini));
}

QList<Action> parse(QSettings &ini)
{
    QList<Action> actions;
    QRegExp actionExp("[0-9a-z]+(\\.[0-9a-z]+)*");
    QRegExp descriptionExp("description(?:\\[(\\w+)\\])?");
    QRegExp nameExp("name(?:\\[(\\w+)\\])?");
    QRegExp policyExp("yes|no|auth_self|auth_admin");

    descriptionExp.setCaseSensitivity(Qt::CaseInsensitive);
    nameExp.setCaseSensitivity(Qt::CaseInsensitive);

    foreach(const QString &name, ini.childGroups()) {
        Action action;

        if (name == "Domain") {
            continue;
        }

        if (!actionExp.exactMatch(name)) {
            qCritical("Wrong action syntax: %s\n", name.toAscii().data());
            exit(1);
        }

        action.name = name;
        ini.beginGroup(name);

        foreach(const QString &key, ini.childKeys()) {
            if (descriptionExp.exactMatch(key)) {
                QString lang = descriptionExp.capturedTexts().at(1);

                if (lang.isEmpty())
                    lang = "en";

                action.descriptions.insert(lang, ini.value(key).toString());

            } else if (nameExp.exactMatch(key)) {
                QString lang = nameExp.capturedTexts().at(1);

                if (lang.isEmpty())
                    lang = "en";

                action.messages.insert(lang, ini.value(key).toString());

            } else if (key.toLower() == "policy") {
                QString policy = ini.value(key).toString();
                if (!policyExp.exactMatch(policy)) {
                    qCritical("Wrong policy: %s", policy.toAscii().data());
                    exit(1);
                }
                action.policy = policy;

            } else if (key.toLower() == "persistence") {
                QString persistence = ini.value(key).toString();
                if (persistence != "session" && persistence != "always") {
                    qCritical("Wrong persistence: %s", persistence.toAscii().data());
                    exit(1);
                }
                action.persistence = persistence;
            }
        }

        if (action.policy.isEmpty() || action.messages.isEmpty() || action.descriptions.isEmpty()) {
            qCritical("Missing option in action: %s", name.toAscii().data());
            exit(1);
        }
        ini.endGroup();

        actions.append(action);
    }

    return actions;
}


QHash<QString, QString> parseDomain(QSettings& ini)
{
    QHash<QString, QString> rethash;

    if (ini.childGroups().contains("Domain")) {
        if (ini.contains("Domain/Name")) {
            rethash["vendor"] = ini.value("Domain/Name").toString();
        }
        if (ini.contains("Domain/URL")) {
            rethash["vendorurl"] = ini.value("Domain/URL").toString();
        }
        if (ini.contains("Domain/Icon")) {
            rethash["icon"] = ini.value("Domain/Icon").toString();
        }
    }

    return rethash;
}


