#include "kwhatsthis.h"
#include "qwhatsthis.h"
#include <qvariant.h>
#include <kdebug.h>
#include <qtextedit.h>
#include <klocale.h>
#include <kapplication.h>

KWhatsThisManager *KWhatsThisManager::s_instance = 0;

class KWhatsThisUndefined : public QWhatsThis
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
    : QWhatsThis (w)
{
    m_widget = w;
}

QString KWhatsThisUndefined::text (const QPoint &)
{
    if (!m_widget)
        return "";
    kdDebug () << "widget: " << m_widget << endl;
    QString txt = i18n ("<b>Not Defined</b><br>There is no \"What's This\""
            " help assigned to this widget. If you want to help us and "
            " describe the widget, you are welcome to <a href=\"submit"
            "-whatsthis\">send us \"What's This\" help</a> for it.");
    QString parent;
    if (m_widget -> parentWidget ())
        parent = QWhatsThis::textFor (m_widget -> parentWidget ());
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
        dsc . append (QString (" (") + m_widget -> className () + ")\n");
        for (w = m_widget; w && w != m_widget -> topLevelWidget (); w = w -> parentWidget ()) {
            dsc . append (w -> name ());
            dsc . append (QString (" (") + w -> className () + ")\n");
        }
        w = m_widget -> topLevelWidget ();
        if (w) {
            dsc . append ("toplevel --> ");
            dsc . append (w -> name ());
            dsc . append (QString (" (") + w -> className () + ")\n");
        }
        body . append (dsc);
        QString subj ("What's This submission: ");
        subj . append (qApp -> argv () [0]);
        body . append ("\nPlease put type in your what's this help between these lines: "
                "\n--%-----------------------------------------------------------------------\n"
                "\n--%-----------------------------------------------------------------------");
        kapp -> invokeMailer ("mailto:quality-whatsthis@kde.org", "", "", subj, body);
    }
    return TRUE;
}

void KWhatsThisManager::init ()
{
    if (s_instance)
        return;
    s_instance = new KWhatsThisManager;
}

KWhatsThisManager::KWhatsThisManager ()
{
    qApp -> installEventFilter (this);
}

bool KWhatsThisManager::eventFilter (QObject *o, QEvent *e)
{
    if (e -> type () == QEvent::ChildInserted) {
        QChildEvent *ce = (QChildEvent *)e;
        // kdDebug () << "new qobject:" << ce -> child () << endl;
        if (ce -> child () -> isWidgetType ()) {
            QWidget *w = (QWidget *) (ce -> child ());
            // kdDebug () << "new qwidget:" << w << endl;
            if (QWhatsThis::textFor (w) . isEmpty ())
                new KWhatsThisUndefined (w);
        }
    }
    return false;
}
