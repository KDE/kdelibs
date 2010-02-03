/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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
    Q_UNUSED(domain)

    QTextStream out(stdout);
    out.setCodec("UTF-8");

    out << header;

    foreach(const Action &action, actions) {
        out << dent << "<action id=\"" << action.name << "\" >\n";

        foreach(const QString& lang, action.descriptions.keys()) {
            out << dent << dent << "<description";
            if (lang != "en")
                out << " xml:lang=\"" << lang << '"';
            out << '>' << action.messages.value(lang) << "</description>\n";
        }

        foreach(const QString& lang, action.messages.keys()) {
            out << dent << dent << "<message";
            if (lang != "en")
                out << " xml:lang=\"" << lang << '"';
            out << '>' << action.descriptions.value(lang) << "</message>\n";
        }

        QString policy = action.policy;
        if (!action.persistence.isEmpty())
            policy += "_keep_" + action.persistence;

        out << QString(policy_tag).arg(policy);

        out << dent << "</action>\n";
    }

    out << "</policyconfig>\n";
}
