/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kdxschanges.h"

#include <klocale.h>
#include <ktextbrowser.h>

#include <QtGui/QTextDocument>
#include <QtGui/QLayout>
#include <QtGui/QApplication>

KDXSChanges::KDXSChanges(QWidget *parent)
        : KDialog(parent)
{
    setCaption(i18n("Changelog"));
    setButtons(KDialog::Close);

    QVBoxLayout *vbox;

    m_log = new KTextBrowser(this);
    setMainWidget(m_log);

    vbox = new QVBoxLayout(m_log);
    vbox->addWidget(m_log);
}

void KDXSChanges::addChangelog(const QString& version, const QString& log)
{
    QString t = QLatin1String("<qt>");

    t += m_log->toHtml();

    t += QLatin1String("<table class='itemBox'>")
         + "<tr>"
         +   "<td class='contentsColumn'>"
         +     "<table class='contentsHeader' cellspacing='2' cellpadding='0'><tr>"
         +     "<td>" + i18n("Version %1", Qt::escape(version)) + "</td>"
         + "</tr></table>"
         + "<div class='contentsBody'>"
         + Qt::escape(log)
         + "</div>";

    m_log->setHtml(t);
}

#include "kdxschanges.moc"
