/*  This file is part of the KDE Libraries
 *  Copyright (C) 2004 Peter Rockai (mornfall) <mornfall@danill.sk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */
#include "kwhatsthismanager_p.h"
#include "q3whatsthis.h"
#include <qvariant.h>
#include <kdebug.h>
#include <q3textedit.h>
#include <klocale.h>
#include <ktoolinvocation.h>

KWhatsThisManager *KWhatsThisManager::s_instance = 0;

class KWhatsThisUndefined : public Q3WhatsThis
{
    public:
        KWhatsThisUndefined (QWidget *);
        QString text (const QPoint &);
    public slots:
        bool clicked (const QString &);
    protected:
        QWidget *m_widget;
};

KWhatsThisUndefined::KWhatsThisUndefined (QWidget *w)
    : Q3WhatsThis (w)
{
    m_widget = w;
}

QString KWhatsThisUndefined::text (const QPoint &)
{
    if (!m_widget)
        return "";
    QString txt = i18n ("<b>Not Defined</b><br>There is no \"What's This\""
            " help assigned to this widget. If you want to help us and "
            " describe the widget, you are welcome to <a href=\"submit"
            "-whatsthis\">send us \"What's This\" help</a> for it.");
    QString parent;
    if (m_widget -> parentWidget ())
        parent = m_widget -> parentWidget () -> whatsThis ();
        if (parent != txt)
            if (! parent . isEmpty ())
                return parent;
    return txt;
}

bool KWhatsThisUndefined::clicked (const QString& href)
{
    if (href == "submit-whatsthis") {
        QWidget *w = m_widget;
        QString body;
        body . append ("Widget text: '" + (m_widget -> property ("text") . toString ()) + "'\n");
        QString dsc = QString ("current --> ") + m_widget -> name ();
        dsc . append (QString (" (") + m_widget -> metaObject () -> className () + ")\n");
        for (w = m_widget; w && w != m_widget -> topLevelWidget (); w = w -> parentWidget ()) {
            dsc . append (w -> name ());
            dsc . append (QString (" (") + w -> metaObject () -> className () + ")\n");
        }
        w = m_widget -> topLevelWidget ();
        if (w) {
            dsc . append ("toplevel --> ");
            dsc . append (w -> name ());
            dsc . append (QString (" (") + w -> metaObject () -> className () + ")\n");
        }
        body . append (dsc);
        QString subj ("What's This submission: ");
        subj . append (qApp -> argv () [0]);
        body . append ("\nPlease type in your what's this help between these lines: "
                "\n--%-----------------------------------------------------------------------\n"
                "\n--%-----------------------------------------------------------------------");
        KToolInvocation::invokeMailer ("quality-whatsthis@kde.org", "", "", subj, body);
    }
    return true;
}

void KWhatsThisManager::init ()
{
    if (s_instance)
        return;
    s_instance = new KWhatsThisManager;
}

KWhatsThisManager::KWhatsThisManager ()
{
    // go away...
    // qApp -> installEventFilter (this);
}

bool KWhatsThisManager::eventFilter (QObject * /*o*/, QEvent *e)
{
    if (e -> type () == QEvent::ChildInserted) {
        QChildEvent *ce = (QChildEvent *)e;
        // kdDebug () << "new qobject:" << ce -> child () << endl;
        if (ce -> child () -> isWidgetType ()) {
            QWidget *w = (QWidget *) (ce -> child ());
            // kdDebug () << "new qwidget:" << w << endl;
            if (w -> whatsThis () . isEmpty ())
                new KWhatsThisUndefined (w);
        }
    }
    return false;
}

#include "kwhatsthismanager_p.moc"

