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
#include <QFile>

#include <QCoreApplication>
#include <QSettings>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

#include <cstdio>

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

    QSettings ini(QFile::decodeName(argv[1]), QSettings::IniFormat);
    ini.setIniCodec("UTF-8");
    if (ini.status()) {
        qCritical("Error loading file: %s", argv[1]);
        return 1;
    }

    if (argc == 3) {
        // Support an optional 2nd argument pointing to the output file
        //
        // This is safer to use in build systems than
        // "kauth-policy-gen foo.actions > foo.policy" because when using a
        // redirection "foo.policy" is created even if kauth-policy-gen fails.
        // This means the first call to make fails, but a second call succeeds
        // because an empty "foo.policy" exists.
        if (!freopen(argv[2], "w", stdout)) {
            qCritical("Failed to open %s for writing: %s", argv[2], strerror(errno));
            return 1;
        }
    }

    output(parse(ini), parseDomain(ini));
}

QList<Action> parse(QSettings &ini)
{
    QList<Action> actions;
    QRegExp actionExp(QLatin1String("[0-9a-z]+(\\.[0-9a-z]+)*"));
    QRegExp descriptionExp(QLatin1String("description(?:\\[(\\w+)\\])?"));
    QRegExp nameExp(QLatin1String("name(?:\\[(\\w+)\\])?"));
    QRegExp policyExp(QLatin1String("yes|no|auth_self|auth_admin"));

    descriptionExp.setCaseSensitivity(Qt::CaseInsensitive);
    nameExp.setCaseSensitivity(Qt::CaseInsensitive);

    Q_FOREACH(const QString &name, ini.childGroups()) {
        Action action;

        if (name == QLatin1String("Domain")) {
            continue;
        }

        if (!actionExp.exactMatch(name)) {
            qCritical("Wrong action syntax: %s\n", name.toLatin1().data());
            exit(1);
        }

        action.name = name;
        ini.beginGroup(name);

        Q_FOREACH(const QString &key, ini.childKeys()) {
            if (descriptionExp.exactMatch(key)) {
                QString lang = descriptionExp.capturedTexts().at(1);

                if (lang.isEmpty())
                    lang = QString::fromLatin1("en");

                action.descriptions.insert(lang, ini.value(key).toString());

            } else if (nameExp.exactMatch(key)) {
                QString lang = nameExp.capturedTexts().at(1);

                if (lang.isEmpty())
                    lang = QString::fromLatin1("en");

                action.messages.insert(lang, ini.value(key).toString());

            } else if (key.toLower() == QLatin1String("policy")) {
                QString policy = ini.value(key).toString();
                if (!policyExp.exactMatch(policy)) {
                    qCritical("Wrong policy: %s", policy.toLatin1().data());
                    exit(1);
                }
                action.policy = policy;

            } else if (key.toLower() == QLatin1String("policyinactive")) {
                QString policyInactive = ini.value(key).toString();
                if (!policyExp.exactMatch(policyInactive)) {
                    qCritical("Wrong policy: %s", policyInactive.toLatin1().data());
                    exit(1);
                }
                action.policyInactive = policyInactive;

            } else if (key.toLower() == QLatin1String("persistence")) {
                QString persistence = ini.value(key).toString();
                if (persistence != QLatin1String("session") && persistence != QLatin1String("always")) {
                    qCritical("Wrong persistence: %s", persistence.toLatin1().data());
                    exit(1);
                }
                action.persistence = persistence;
            }
        }

        if (action.policy.isEmpty() || action.messages.isEmpty() || action.descriptions.isEmpty()) {
            qCritical("Missing option in action: %s", name.toLatin1().data());
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

    if (ini.childGroups().contains(QString::fromLatin1("Domain"))) {
        if (ini.contains(QString::fromLatin1("Domain/Name"))) {
            rethash[QString::fromLatin1("vendor")] = ini.value(QString::fromLatin1("Domain/Name")).toString();
        }
        if (ini.contains(QString::fromLatin1("Domain/URL"))) {
            rethash[QString::fromLatin1("vendorurl")] = ini.value(QString::fromLatin1("Domain/URL")).toString();
        }
        if (ini.contains(QString::fromLatin1("Domain/Icon"))) {
            rethash[QString::fromLatin1("icon")] = ini.value(QString::fromLatin1("Domain/Icon")).toString();
        }
    }

    return rethash;
}


