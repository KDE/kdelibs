/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Koos Vriezen <koos.vriezen@xs4all.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <stdio.h>

#ifdef KDE_USE_FINAL
#undef Always
#include <qdir.h>
#endif
#include <qtable.h>
#include <qpair.h>

#include <klibloader.h>
#include <kaboutdata.h>
#include <kstaticdeleter.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>

#include "kjavaappletwidget.h"
#include "kjavaappletviewer.h"
#include "kjavaappletserver.h"


K_EXPORT_COMPONENT_FACTORY (kjavaappletviewer, KJavaAppletViewerFactory)

KInstance *KJavaAppletViewerFactory::s_instance = 0;

KJavaAppletViewerFactory::KJavaAppletViewerFactory () {
    s_instance = new KInstance ("KJavaAppletViewer");
}

KJavaAppletViewerFactory::~KJavaAppletViewerFactory () {
    delete s_instance;
}

KParts::Part *KJavaAppletViewerFactory::createPartObject
  (QWidget *wparent, const char *wname,
   QObject *parent, const char * name, const char *, const QStringList & args) {
    return new KJavaAppletViewer (wparent, wname, parent, name, args);
}

//-----------------------------------------------------------------------------

class KJavaServerMaintainer;
static KJavaServerMaintainer * serverMaintainer = 0;

class KJavaServerMaintainer {
public:
    KJavaServerMaintainer () { }

    KJavaAppletContext * getContext (QObject*, const QString &);
    void releaseContext (QObject*, const QString &);
public:
    typedef QMap <QPair <QObject*, QString>, QPair <KJavaAppletContext*, int> >
            ContextMap;
    ContextMap m_contextmap;
};

KJavaAppletContext * KJavaServerMaintainer::getContext (QObject * w, const QString & doc) {
    ContextMap::key_type key = qMakePair (w, doc);
    ContextMap::iterator it = m_contextmap.find (key);
    if (it != m_contextmap.end ()) {
        (*it).second++;
        return (*it).first;
    }
    KJavaAppletContext * context = new KJavaAppletContext ();
    m_contextmap.insert (key, qMakePair(context, 1));
    return context;
}

void KJavaServerMaintainer::releaseContext (QObject * w, const QString & doc) {
    ContextMap::iterator it = m_contextmap.find (qMakePair (w, doc));
    if (it != m_contextmap.end () && --(*it).second <= 0) {
        kdDebug(6100) << "KJavaServerMaintainer::releaseContext" << endl;
        (*it).first->deleteLater ();
        m_contextmap.remove (it);
    }
}

static KStaticDeleter <KJavaServerMaintainer> serverMaintainerDeleter;

//-----------------------------------------------------------------------------

AppletParameterDialog::AppletParameterDialog (KJavaAppletWidget * parent)
    //: KDialogBase (parent, "paramdialog", true, i18n ("Applet Parameters"), 
    : KDialogBase (parent, "paramdialog", true, "Applet Parameters", 
                   KDialogBase::Close, KDialogBase::Close, true),
      m_appletWidget (parent) {
    KJavaApplet * applet = parent->applet ();
    table = new QTable (30, 2, this);
    table->setMinimumSize (QSize (600, 400));
    table->setColumnWidth (0, 200);
    table->setColumnWidth (1, 340);
    QHeader *header = table->horizontalHeader();
    //header->setLabel (0, i18n ("Parameter"));
    header->setLabel (0, "Parameter");
    //header->setLabel (1, i18n ("Value"));
    header->setLabel (1, "Value");
    //QTableItem * tit = new QTableItem (table, QTableItem::Never, i18n("Class"));
    QTableItem * tit = new QTableItem (table, QTableItem::Never, "Class");
    table->setItem (0, 0, tit);
    tit = new QTableItem(table, QTableItem::Always, applet->appletClass());
    table->setItem (0, 1, tit);
    //tit = new QTableItem (table, QTableItem::Never, i18n ("Base URL"));
    tit = new QTableItem (table, QTableItem::Never, "Base URL");
    table->setItem (1, 0, tit);
    tit = new QTableItem(table, QTableItem::Always, applet->baseURL());
    table->setItem (1, 1, tit);
    //tit = new QTableItem (table, QTableItem::Never, i18n ("Archives"));
    tit = new QTableItem (table, QTableItem::Never, "Archives");
    table->setItem (2, 0, tit);
    tit = new QTableItem(table, QTableItem::Always, applet->archives());
    table->setItem (2, 1, tit);
    QMap<QString,QString>::iterator it = applet->getParams().begin ();
    for (int count = 2; it != applet->getParams().end (); ++it) {
        tit = new QTableItem (table, QTableItem::Always, it.key ());
        table->setItem (++count, 0, tit);
        tit = new QTableItem(table, QTableItem::Always, it.data ());
        table->setItem (count, 1, tit);
    }
    setMainWidget (table);
}

void AppletParameterDialog::slotClose () {
    table->selectCells (0, 0, 0, 0);
    KJavaApplet * applet = m_appletWidget->applet ();
    applet->setAppletClass (table->item (0, 1)->text ());
    applet->setBaseURL (table->item (1, 1)->text ());
    applet->setArchives (table->item (2, 1)->text ());
    for (int i = 3; i < table->numRows (); ++i) {
        if (table->item (i, 0) && table->item (i, 1) && !table->item (i, 0)->text ().isEmpty ())
            applet->setParameter (table->item (i, 0)->text (),
                                  table->item (i, 1)->text ());
    }
    hide ();
}
//-----------------------------------------------------------------------------

KJavaAppletViewer::KJavaAppletViewer (QWidget * wparent, const char *,
                 QObject * parent, const char * name, const QStringList & args)
 : KParts::ReadOnlyPart (parent, name),
   m_browserextension (new KJavaAppletViewerBrowserExtension (this))
{
    if (!serverMaintainer) {
        serverMaintainerDeleter.setObject (serverMaintainer,
                                           new KJavaServerMaintainer);
    }
    m_view = new KJavaAppletViewerWidget (wparent);

    QString classname, classid, codebase;
    int width = -1;
    int height = -1;
    KJavaApplet * applet = m_view->applet ();
    QStringList::const_iterator it = args.begin ();
    for ( ; it != args.end (); ++it) {
        int equalPos = (*it).find("=");
        if (equalPos > 0) {
            QString name = (*it).left (equalPos).upper ();
            QString value = (*it).right ((*it).length () - equalPos - 1);
            if (value.at(0)=='\"')
                value = value.right (value.length () - 1);
            if (value.at (value.length () - 1) == '\"')
                value = value.left(value.length()-1);
            kdDebug(6100) << "name=" << name << " value=" << value << endl;
            if (!name.isEmpty()) {
                if (name == "__KHTML__PLUGINBASEURL")
                    baseurl = value;
                else if (name == "__KHTML__CODEBASE" ||
                         name.lower()==QString::fromLatin1("codebase") ||
                         name.lower()==QString::fromLatin1("java_codebase")) {
                    if (!value.isEmpty ())
                        codebase = value;
                } else if (name == "__KHTML__CLASSID")
                //else if (name.lower()==QString::fromLatin1("classid"))
                    classid = value;
                else if (name.lower()==QString::fromLatin1("code") ||
                         name.lower()==QString::fromLatin1("java_code") ||
                         name.lower()==QString::fromLatin1("src"))
                    classname = value;
                else if (name.lower()==QString::fromLatin1("archive") ||
                         name.lower()==QString::fromLatin1("java_archieve"))
                    applet->setArchives (value);
                else if (name.lower()==QString::fromLatin1("name"))
                    applet->setAppletName (value);
                else if (name.lower()==QString::fromLatin1("width"))
                    width = value.toInt();
                else if (name.lower()==QString::fromLatin1("height"))
                    height = value.toInt();
                else {
                    applet->setParameter (name, value);
                }
            }
        }
    }
    if (!classid.isEmpty ()) {
        applet->setParameter ("CLSID", classid);
        kdDebug(6100) << "classid=" << classid << classid.startsWith("clsid:")<< endl;
        if (classid.startsWith ("clsid:"))
            // codeBase contains the URL to the plugin page
            codebase = baseurl;
        else if (classname.isEmpty () && classid.startsWith ("java:"))
            classname = classid.mid(5);
    }

    if (width > 0 && height > 0)
        applet->setSize (QSize(width, height));
    applet->setBaseURL (baseurl);
    applet->setCodeBase (codebase);
    applet->setAppletClass (classname);
    applet->setAppletContext (serverMaintainer->getContext (parent, baseurl));

    setInstance (KJavaAppletViewerFactory::instance ());
    KParts::Part::setWidget (m_view);
    insertChild (applet->getLiveConnectExtension ()); // hack

    connect (applet->getContext(), SIGNAL(appletLoaded()), this, SLOT(appletLoaded()));
    connect (applet->getContext(), SIGNAL(showDocument(const QString&, const QString&)), m_browserextension, SLOT(showDocument(const QString&, const QString&)));
    connect (applet->getContext(), SIGNAL(showStatus(const QString &)), this, SLOT(infoMessage(const QString &)));
}

KJavaAppletViewer::~KJavaAppletViewer () {
    delete m_view;
    serverMaintainer->releaseContext (parent(), baseurl);
    delete m_browserextension;
}

bool KJavaAppletViewer::openURL (const KURL & url) {
    if (!m_view) return false;
    KJavaApplet * applet = m_view->applet ();
    if (applet->isCreated ())
        applet->stop ();
    if (applet->appletClass ().isEmpty ()) {
        // preview without setting a class?
        if (applet->baseURL ().isEmpty ()) {
            applet->setAppletClass (url.fileName ());
            applet->setBaseURL (url.upURL ().url ());
        } else
            applet->setAppletClass (url.url ());
        AppletParameterDialog (m_view).exec ();
        applet->setSize (m_view->sizeHint());
    }
    // delay showApplet if size is unknown and m_view not shown
    if (applet->size().width() > 0 || m_view->isVisible())
        m_view->showApplet ();
    emit started (0L);
    return url.isValid ();
}

bool KJavaAppletViewer::openFile () {
    return false;
}

void KJavaAppletViewer::appletLoaded () {
    KJavaApplet * applet = m_view->applet ();
    if (applet->isAlive() || applet->failed())
        emit completed();
}

void KJavaAppletViewer::infoMessage (const QString & msg) {
    m_browserextension->infoMessage(msg);
}

KAboutData* KJavaAppletViewer::createAboutData () {
    //return new KAboutData("KJavaAppletViewer", I18N_NOOP("KDE Java Applet Plugin"), "1.0");
    return new KAboutData("KJavaAppletViewer", "KDE Java Applet Plugin", "1.0");
}

//---------------------------------------------------------------------

KJavaAppletViewerBrowserExtension::KJavaAppletViewerBrowserExtension (KJavaAppletViewer * parent)
  : KParts::BrowserExtension (parent, "KJavaAppletViewer Browser Extension") {
}

void KJavaAppletViewerBrowserExtension::urlChanged (const QString & url) {
    emit setLocationBarURL (url);
}

void KJavaAppletViewerBrowserExtension::setLoadingProgress (int percentage) {
    emit loadingProgress (percentage);
}

void KJavaAppletViewerBrowserExtension::setURLArgs (const KParts::URLArgs & /*args*/) {
}

void KJavaAppletViewerBrowserExtension::saveState (QDataStream & stream) {
    KJavaApplet * applet = static_cast<KJavaAppletViewer*>(parent())->view()->applet ();
    stream << applet->appletClass();
    stream << applet->baseURL();
    stream << applet->archives();
    stream << applet->getParams().size ();
    QMap<QString,QString>::iterator it = applet->getParams().begin ();
    for ( ; it != applet->getParams().end (); ++it) {
        stream << it.key ();
        stream << it.data ();
    }
}

void KJavaAppletViewerBrowserExtension::restoreState (QDataStream & stream) {
    KJavaAppletViewer * viewer = static_cast<KJavaAppletViewer*>(parent());
    KJavaApplet * applet = viewer->view()->applet ();
    QString key, val;
    int paramcount;
    stream >> val;
    applet->setAppletClass (val);
    stream >> val;
    applet->setBaseURL (val);
    stream >> val;
    applet->setArchives (val);
    stream >> paramcount;
    for (int i = 0; i < paramcount; ++i) {
        stream >> key;
        stream >> val;
        applet->setParameter (key, val);
        kdDebug(6100) << "restoreState key:" << key << " val:" << val << endl;
    }
    applet->setSize (viewer->view ()->sizeHint ());
    if (viewer->view ()->isVisible())
        viewer->view ()->showApplet ();
}

void KJavaAppletViewerBrowserExtension::showDocument (const QString & doc,
                                                      const QString & frame) {
    KURL url (doc);
    KParts::URLArgs args;
    args.frameName = frame;
    emit openURLRequest (url, args);
}
        
//-----------------------------------------------------------------------------
// TODO move this to kjavaappletwidget

KJavaAppletViewerWidget::KJavaAppletViewerWidget(QWidget* parent, const char* name)
  : KJavaAppletWidget(parent, name) {}

void KJavaAppletViewerWidget::showEvent (QShowEvent * e) {
    KJavaAppletWidget::showEvent(e);
    if (!applet()->isCreated() && !applet()->appletClass().isEmpty()) {
        // delayed showApplet
        if (applet()->size().width() <= 0)
            applet()->setSize (sizeHint());
        showApplet();
    }
}

#include "kjavaappletviewer.moc"
