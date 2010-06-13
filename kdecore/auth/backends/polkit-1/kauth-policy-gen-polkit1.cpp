/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009-2010 Dario Freddi <drf@kde.org>
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

#include <auth/policy-gen/policy-gen.h>

#include <cstdio>
#include <QDebug>
#include <QTextStream>

const char header[] = ""
                      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                      "<!DOCTYPE policyconfig PUBLIC\n"
                      "\"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN\"\n"
                      "\"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd\">\n"
                      "<policyconfig>\n";

const char policy_tag[] = ""
                          "      <defaults>\n"
                          "         <allow_inactive>no</allow_inactive>\n"
                          "         <allow_active>%1</allow_active>\n"
                          "      </defaults>\n";

const char dent[] = "   ";

void output(QList<Action> actions, QHash<QString, QString> domain)
{
    QTextStream out(stdout);
    out.setCodec("UTF-8");

    out << header;

    // Blacklisted characters + replacements
    QHash< QChar, QString > blacklist;
    blacklist.insert(QChar('&'), "&amp;");

    if (domain.contains("vendor")) {
        QHash< QChar, QString >::const_iterator blI;
        QString vendor = domain["vendor"];
        for (blI = blacklist.constBegin(); blI != blacklist.constEnd(); ++blI) {
            vendor.replace(blI.key(), blI.value());
        }
        out << "<vendor>" << vendor << "</vendor>\n";
    }
    if (domain.contains("vendorurl")) {
        out << "<vendor_url>" << domain["vendorurl"] << "</vendor_url>\n";
    }
    if (domain.contains("icon")) {
        out << "<icon_name>" << domain["icon"] << "</icon_name>\n";
    }

    foreach (const Action &action, actions) {
        out << dent << "<action id=\"" << action.name << "\" >\n";

        // Not a typo, messages and descriptions are actually inverted
        for (QHash< QString, QString >::const_iterator i = action.messages.constBegin(); i != action.messages.constEnd(); ++i) {
            out << dent << dent << "<description";
            if (i.key() != "en") {
                out << " xml:lang=\"" << i.key() << '"';
            }

            QHash< QChar, QString >::const_iterator blI;
            QString description = i.value();
            for (blI = blacklist.constBegin(); blI != blacklist.constEnd(); ++blI) {
                description.replace(blI.key(), blI.value());
            }

            out << '>' << description << "</description>\n";
        }

            QHash< QString, QString >::const_iterator i;
        for (QHash< QString, QString >::const_iterator i = action.descriptions.constBegin();
             i != action.descriptions.constEnd();
             ++i) {
            out << dent << dent << "<message";
            if (i.key() != "en") {
                out << " xml:lang=\"" << i.key() << '"';
            }

            QHash< QChar, QString >::const_iterator blI;
            QString message = i.value();
            for (blI = blacklist.constBegin(); blI != blacklist.constEnd(); ++blI) {
                message.replace(blI.key(), blI.value());
            }

            out << '>' << message << "</message>\n";
        }

        QString policy = action.policy;
        if (!action.persistence.isEmpty() && policy != "yes" && policy != "no") {
            policy += "_keep";
        }

        out << QString(policy_tag).arg(policy);

        out << dent << "</action>\n";
    }

    out << "</policyconfig>\n";
}
