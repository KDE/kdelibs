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
#include <qguardedptr.h>

#include <klibloader.h>
#include <kaboutdata.h>
#include <kstaticdeleter.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kio/authinfo.h>
#include <dcopclient.h>

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
    ~KJavaServerMaintainer ();

    KJavaAppletContext * getContext (QObject*, const QString &);
    void releaseContext (QObject*, const QString &);
    void setServer (KJavaAppletServer * s);
private:
    typedef QMap <QPair <QObject*, QString>, QPair <KJavaAppletContext*, int> >
            ContextMap;
    ContextMap m_contextmap;
    QGuardedPtr <KJavaAppletServer> server;
};

KJavaServerMaintainer::~KJavaServerMaintainer () {
    delete server;
}

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

inline void KJavaServerMaintainer::setServer (KJavaAppletServer * s) {
    if (!server)
        server = s;
}

static KStaticDeleter <KJavaServerMaintainer> serverMaintainerDeleter;

//-----------------------------------------------------------------------------

AppletParameterDialog::AppletParameterDialog (KJavaAppletWidget * parent)
    : KDialogBase (parent, "paramdialog", true, i18n ("Applet Parameters"),
                   KDialogBase::Close, KDialogBase::Close, true),
      m_appletWidget (parent) {
    KJavaApplet * applet = parent->applet ();
    table = new QTable (30, 2, this);
    table->setMinimumSize (QSize (600, 400));
    table->setColumnWidth (0, 200);
    table->setColumnWidth (1, 340);
    QHeader *header = table->horizontalHeader();
    header->setLabel (0, i18n ("Parameter"));
    header->setLabel (1, i18n ("Value"));
    QTableItem * tit = new QTableItem (table, QTableItem::Never, i18n("Class"));
    table->setItem (0, 0, tit);
    tit = new QTableItem(table, QTableItem::Always, applet->appletClass());
    table->setItem (0, 1, tit);
    tit = new QTableItem (table, QTableItem::Never, i18n ("Base URL"));
    table->setItem (1, 0, tit);
    tit = new QTableItem(table, QTableItem::Always, applet->baseURL());
    table->setItem (1, 1, tit);
    tit = new QTableItem (table, QTableItem::Never, i18n ("Archives"));
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

class CoverWidget : public QWidget {
    KJavaAppletWidget * m_appletwidget;
public:
    CoverWidget (QWidget *);
    ~CoverWidget () {}
    KJavaAppletWidget * appletWidget () const;
protected:
    void resizeEvent (QResizeEvent * e);
};

inline CoverWidget::CoverWidget (QWidget * parent) : QWidget (parent) {
    m_appletwidget = new KJavaAppletWidget (this);
    setFocusProxy (m_appletwidget);
    hide ();
}

inline KJavaAppletWidget * CoverWidget::appletWidget () const {
    return m_appletwidget;
}

void CoverWidget::resizeEvent (QResizeEvent * e) {
    m_appletwidget->resize (e->size().width(), e->size().height());
}

//-----------------------------------------------------------------------------

KJavaAppletViewer::KJavaAppletViewer (QWidget * wparent, const char *,
                 QObject * parent, const char * name, const QStringList & args)
 : KParts::ReadOnlyPart (parent, name),
   m_browserextension (new KJavaAppletViewerBrowserExtension (this)),
   m_liveconnect (new KJavaAppletViewerLiveConnectExtension (this)),
   m_closed (true)
{
    if (!serverMaintainer) {
        serverMaintainerDeleter.setObject (serverMaintainer,
                                           new KJavaServerMaintainer);
    }
    m_view = new CoverWidget (wparent);
    QString classname, classid, codebase, khtml_codebase;
    int width = -1;
    int height = -1;
    KJavaApplet * applet = m_view->appletWidget()->applet ();
    QStringList::const_iterator it = args.begin ();
    for ( ; it != args.end (); ++it) {
        int equalPos = (*it).find("=");
        if (equalPos > 0) {
            QString name = (*it).left (equalPos).upper ();
            QString value = (*it).right ((*it).length () - equalPos - 1);
            if (value.at(0)=='\"')
                value = value.right (value.length () - 1);
            if (value.at (value.length () - 1) == '\"')
                value.truncate (value.length () - 1);
            kdDebug(6100) << "name=" << name << " value=" << value << endl;
            if (!name.isEmpty()) {
                QString name_lower = name.lower ();
                if (name == "__KHTML__PLUGINBASEURL") {
                    KURL url (value);
                    QString fn = url.fileName (false);
                    baseurl = fn.isEmpty () ?
                        value : value.left (value.length ()-fn.length ());
                } else if (name == "__KHTML__CODEBASE")
                    khtml_codebase = value;
                else if (name_lower == QString::fromLatin1("codebase") ||
                         name_lower == QString::fromLatin1("java_codebase")) {
                    if (!value.isEmpty ())
                        codebase = value;
                } else if (name == "__KHTML__CLASSID")
                //else if (name.lower()==QString::fromLatin1("classid"))
                    classid = value;
                else if (name_lower == QString::fromLatin1("code") ||
                         name_lower == QString::fromLatin1("java_code") ||
                         name_lower == QString::fromLatin1("src"))
                    classname = value;
                else if (name_lower == QString::fromLatin1("archive") ||
                         name_lower == QString::fromLatin1("java_archive") ||
                         name_lower.startsWith ("cache_archive"))
                    applet->setArchives (value);
                else if (name_lower == QString::fromLatin1("name"))
                    applet->setAppletName (value);
                else if (name_lower == QString::fromLatin1("width"))
                    width = value.toInt();
                else if (name_lower == QString::fromLatin1("height"))
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
            khtml_codebase = baseurl;
        else if (classname.isEmpty () && classid.startsWith ("java:"))
            classname = classid.mid(5);
    }
    if (codebase.isEmpty ())
        codebase = khtml_codebase;

    if (width > 0 && height > 0)
        m_view->resize (width, height);
    applet->setBaseURL (baseurl);
    // check codebase first
    KURL kbaseURL( baseurl );
    KURL newURL(kbaseURL, codebase);
    if (kapp->authorizeURLAction("redirect", KURL(baseurl), newURL))
        applet->setCodeBase (newURL.url());
    applet->setAppletClass (classname);
    KJavaAppletContext * cxt = serverMaintainer->getContext (parent, baseurl);
    applet->setAppletContext (cxt);

    KJavaAppletServer * server = cxt->getServer ();

    serverMaintainer->setServer (server);

    if (!server->usingKIO ()) {
        /* if this page needs authentication */
        KIO::AuthInfo info;
        QString errorMsg;
        QCString replyType;
        QByteArray params;
        QByteArray reply;
        KIO::AuthInfo authResult;

        //(void) dcopClient(); // Make sure to have a dcop client.
        info.url = baseurl;
        info.verifyPath = true;

        QDataStream stream(params, IO_WriteOnly);
        stream << info << m_view->topLevelWidget()->winId();

        if (!kapp->dcopClient ()->call( "kded", "kpasswdserver", "checkAuthInfo(KIO::AuthInfo, long int)", params, replyType, reply ) ) {
            kdWarning() << "Can't communicate with kded_kpasswdserver!" << endl;
        } else if ( replyType == "KIO::AuthInfo" ) {
            QDataStream stream2( reply, IO_ReadOnly );
            stream2 >> authResult;
            applet->setUser (authResult.username);
            applet->setPassword (authResult.password);
            applet->setAuthName (authResult.realmValue);
        }
    }

    /* install event filter for close events */
    if (wparent)
        wparent->topLevelWidget ()->installEventFilter (this);

    setInstance (KJavaAppletViewerFactory::instance ());
    KParts::Part::setWidget (m_view);

    connect (applet->getContext(), SIGNAL(appletLoaded()), this, SLOT(appletLoaded()));
    connect (applet->getContext(), SIGNAL(showDocument(const QString&, const QString&)), m_browserextension, SLOT(showDocument(const QString&, const QString&)));
    connect (applet->getContext(), SIGNAL(showStatus(const QString &)), this, SLOT(infoMessage(const QString &)));
    connect (applet, SIGNAL(jsEvent (const QStringList &)), m_liveconnect, SLOT(jsEvent (const QStringList &)));
}

bool KJavaAppletViewer::eventFilter (QObject *o, QEvent *e) {
    if (m_liveconnect->jsSessions () > 0) {
        switch (e->type()) {
            case QEvent::Destroy:
            case QEvent::Close:
            case QEvent::Quit:
                return true;
            default:
                break;
        }
    }
    return KParts::ReadOnlyPart::eventFilter(o,e);
}

KJavaAppletViewer::~KJavaAppletViewer () {
    m_view = 0L;
    serverMaintainer->releaseContext (parent(), baseurl);
}

bool KJavaAppletViewer::openURL (const KURL & url) {
    if (!m_view) return false;
    m_closed = false;
    KJavaAppletWidget * w = m_view->appletWidget ();
    KJavaApplet * applet = w->applet ();
    if (applet->isCreated ())
        applet->stop ();
    if (applet->appletClass ().isEmpty ()) {
        // preview without setting a class?
        if (applet->baseURL ().isEmpty ()) {
            applet->setAppletClass (url.fileName ());
            applet->setBaseURL (url.upURL ().url ());
        } else
            applet->setAppletClass (url.url ());
        AppletParameterDialog (w).exec ();
        applet->setSize (w->sizeHint());
    }
    // delay showApplet if size is unknown and m_view not shown
    if (applet->size().width() > 0 || m_view->isVisible())
        w->showApplet ();
    if (!applet->failed ())
        emit started (0L);
    return url.isValid ();
}

bool KJavaAppletViewer::closeURL () {
    kdDebug(6100) << "closeURL" << endl;
    m_closed = true;
    KJavaApplet * applet = m_view->appletWidget ()->applet ();
    if (applet->isCreated ())
        applet->stop ();
    applet->getContext()->getServer()->endWaitForReturnData();
    return true;
}

bool KJavaAppletViewer::appletAlive () const {
    return !m_closed && m_view &&
           m_view->appletWidget ()->applet () &&
           m_view->appletWidget ()->applet ()->isAlive ();
}

bool KJavaAppletViewer::openFile () {
    return false;
}

void KJavaAppletViewer::appletLoaded () {
    if (!m_view) return;
    KJavaApplet * applet = m_view->appletWidget ()->applet ();
    if (applet->isAlive() || applet->failed())
        emit completed();
}

void KJavaAppletViewer::infoMessage (const QString & msg) {
    m_browserextension->infoMessage(msg);
}

KAboutData* KJavaAppletViewer::createAboutData () {
    return new KAboutData("KJavaAppletViewer", I18N_NOOP("KDE Java Applet Plugin"), "1.0");
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
    KJavaApplet * applet = static_cast<KJavaAppletViewer*>(parent())->view()->appletWidget ()->applet ();
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
    KJavaAppletWidget * w = static_cast<KJavaAppletViewer*>(parent())->view()->appletWidget();
    KJavaApplet * applet = w->applet ();
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
    applet->setSize (w->sizeHint ());
    if (w->isVisible())
        w->showApplet ();
}

void KJavaAppletViewerBrowserExtension::showDocument (const QString & doc,
                                                      const QString & frame) {
    KURL url (doc);
    KParts::URLArgs args;
    args.frameName = frame;
    emit openURLRequest (url, args);
}

//-----------------------------------------------------------------------------

KJavaAppletViewerLiveConnectExtension::KJavaAppletViewerLiveConnectExtension(KJavaAppletViewer * parent)
    : KParts::LiveConnectExtension (parent), m_viewer (parent) {
}

bool KJavaAppletViewerLiveConnectExtension::get (
        const unsigned long objid, const QString & name,
        KParts::LiveConnectExtension::Type & type,
        unsigned long & rid, QString & value)
{
    if (!m_viewer->appletAlive ())
        return false;
    QStringList args, ret_args;
    KJavaApplet * applet = m_viewer->view ()->appletWidget ()->applet ();
    args.append (QString::number (applet->appletId ()));
    args.append (QString::number ((int) objid));
    args.append (name);
    m_jssessions++;
    bool ret = applet->getContext()->getMember (args, ret_args);
    m_jssessions--;
    if (!ret || ret_args.count() != 3) return false;
    bool ok;
    int itype = ret_args[0].toInt (&ok);
    if (!ok || itype < 0) return false;
    type = (KParts::LiveConnectExtension::Type) itype;
    rid = ret_args[1].toInt (&ok);
    if (!ok) return false;
    value = ret_args[2];
    return true;
}

bool KJavaAppletViewerLiveConnectExtension::put(const unsigned long objid, const QString & name, const QString & value)
{
    if (!m_viewer->appletAlive ())
        return false;
    QStringList args;
    KJavaApplet * applet = m_viewer->view ()->appletWidget ()->applet ();
    args.append (QString::number (applet->appletId ()));
    args.append (QString::number ((int) objid));
    args.append (name);
    args.append (value);
    m_jssessions++;
    bool ret = applet->getContext()->putMember (args);
    m_jssessions--;
    return ret;
}

bool KJavaAppletViewerLiveConnectExtension::call( const unsigned long objid, const QString & func, const QStringList & fargs, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, QString & value )
{
    if (!m_viewer->appletAlive ())
        return false;
    KJavaApplet * applet = m_viewer->view ()->appletWidget ()->applet ();
    QStringList args, ret_args;
    args.append (QString::number (applet->appletId ()));
    args.append (QString::number ((int) objid));
    args.append (func);
    for (QStringList::const_iterator it=fargs.begin(); it != fargs.end(); ++it)
        args.append(*it);
    m_jssessions++;
    bool ret = applet->getContext()->callMember (args, ret_args);
    m_jssessions--;
    if (!ret || ret_args.count () != 3) return false;
    bool ok;
    int itype = ret_args[0].toInt (&ok);
    if (!ok || itype < 0) return false;
    type = (KParts::LiveConnectExtension::Type) itype;
    retobjid = ret_args[1].toInt (&ok);
    if (!ok) return false;
    value = ret_args[2];
    return true;
}

void KJavaAppletViewerLiveConnectExtension::unregister(const unsigned long objid)
{
    if (!m_viewer->view () || !m_viewer->view ())
        return;
    KJavaApplet * applet = m_viewer->view ()->appletWidget ()->applet ();
    if (!applet || objid == 0) {
        // typically a gc after a function call on the applet,
        // no need to send to the jvm
        return;
    }
    QStringList args;
    args.append (QString::number (applet->appletId ()));
    args.append (QString::number ((int) objid));
    applet->getContext()->derefObject (args);
}

void KJavaAppletViewerLiveConnectExtension::jsEvent (const QStringList & args) {
    if (args.count () < 2 || !m_viewer->appletAlive ())
        return;
    bool ok;
    unsigned long objid = args[0].toInt(&ok);
    QString event = args[1];
    KParts::LiveConnectExtension::ArgList arglist;
    for (unsigned i = 2; i < args.count(); i += 2)
        // take a deep breath here
        arglist.push_back(KParts::LiveConnectExtension::ArgList::value_type((KParts::LiveConnectExtension::Type) args[i].toInt(), args[i+1]));
    emit partEvent (objid, event, arglist);
}

int KJavaAppletViewerLiveConnectExtension::m_jssessions = 0;

//-----------------------------------------------------------------------------

#include "kjavaappletviewer.moc"
