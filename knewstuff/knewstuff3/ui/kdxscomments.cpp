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

#include "kdxscomments.h"

#include <klocale.h>
#include <ktextbrowser.h>

#include <QtGui/QLayout>
#include <QtGui/QApplication>

#include <QtGui/QCursor>

KDXSComments::KDXSComments(QWidget *parent)
        : KDialog(parent)
{
    setCaption(i18n("User comments"));
    setButtons(KDialog::Close);

    m_log = new KTextBrowser(this);
    setMainWidget(m_log);

    connect(m_log, SIGNAL(anchorClicked(const QUrl&)),
            SLOT(slotUrl(const QUrl&)));
}

void KDXSComments::slotUrl(const QUrl& url)
{
    if (!url.isEmpty()) {
        qDebug("SHOW %s!", qPrintable(url.toString()));
    }
}

void KDXSComments::addComment(const QString& username, const QString& comment)
{
    // FIXME: get email address??
    QString t;

    t += m_log->toHtml();

    QString email = "spillner@kde.org";

    t += "<a href='" + email + "'>" + Qt::escape(username) + "</a>"
         + "<table class='itemBox'>"
         + "<tr>"
         + "<td class='contentsColumn'>"
         + "<table class='contentsHeader' cellspacing='2' cellpadding='0'><tr>"
         + "<td>Comment!</td>"
         + "</tr></table>"
         + "<div class='contentsBody'>"
         + Qt::escape(comment)
         + "</div>"
         + "<div class='contentsFooter'>"
         + "<em>" + Qt::escape(username) + "</em>"
         + "</div>"
         + "</td>"
         + "</tr>"
         + "</table>";

    m_log->setHtml(t);
}

/*
void urlSelected(const QString & link, int, int, const QString &, KParts::URLArgs)
{
KURL url(link);
QString urlProtocol = url.protocol();
QString urlPath = url.path();
if(urlProtocol == "mailto")
{
kapp->invokeMailer( url );
}
}
*/

#include "kdxscomments.moc"
