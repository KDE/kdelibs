/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
   Copyright     2007 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <kedittoolbar.h>
#include <kedittoolbar_p.h>


#include <QtXml/QDomDocument>
#include <QtGui/QLayout>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QHeaderView>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QtGui/QApplication>
#include <QMimeData>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kseparator.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kprocess.h>
#include <ktoolbar.h>
#include <kdeversion.h>
#include <kcombobox.h>
#include <kmainwindow.h>

#include "kaction.h"
#include "kactioncollection.h"

static const char * const separatorstring = I18N_NOOP("--- separator ---");

#define SEPARATORSTRING i18n(separatorstring)

static const char* const s_XmlTypeToString[] = { "Shell", "Part", "Local", "Merged" };

typedef QList<QDomElement> ToolBarList;

namespace KDEPrivate {

/**
 * Return a list of toolbar elements given a toplevel element
 */
static ToolBarList findToolBars(const QDomElement& start)
{
    static const QString &tagToolBar = KGlobal::staticQString( "ToolBar" );
    static const QString &tagMenuBar = KGlobal::staticQString( "MenuBar" );
    static const QString &attrNoEdit = KGlobal::staticQString( "noEdit" );
    ToolBarList list;

    for( QDomElement elem = start; !elem.isNull(); elem = elem.nextSiblingElement() ) {
        if (elem.tagName() == tagToolBar) {
            if ( elem.attribute( attrNoEdit ) != "true" )
                list.append(elem);
        } else {
            if (elem.tagName() != tagMenuBar) // there are no toolbars inside the menubar :)
                list += findToolBars(elem.firstChildElement()); // recursive
        }
    }

    return list;
}

class XmlData
{
public:
    enum XmlType { Shell = 0, Part, Local, Merged };

    explicit XmlData( XmlType xmlType, const QString& xmlFile, KActionCollection* collection )
        : m_isModified(false),
          m_xmlFile(xmlFile),
          m_type(xmlType),
          m_actionCollection(collection)
    {
    }
    void dump() const
    {
        kDebug(240) << "XmlData" << this << "type" << s_XmlTypeToString[m_type] << "xmlFile:" << m_xmlFile;
        foreach (const QDomElement& element, m_barList) {
            kDebug(240) << "    ToolBar:" << toolBarText( element );
        }
        if ( m_actionCollection )
            kDebug(240) << "    " << m_actionCollection->actions().count() << "actions in the collection.";
        else
            kDebug(240) << "    no action collection.";
    }
    QString xmlFile() const { return m_xmlFile; }
    XmlType type() const { return m_type; }
    KActionCollection* actionCollection() const { return m_actionCollection; }
    void setDomDocument(const QDomDocument& domDoc)
    {
        m_document = domDoc;
        m_barList = findToolBars(m_document.documentElement());
    }
    // Return reference, for e.g. actionPropertiesElement() to modify the document
    QDomDocument& domDocument() { return m_document; }
    const QDomDocument& domDocument() const { return m_document; }

    /**
     * Return the text (user-visible name) of a given toolbar
     */
    QString toolBarText( const QDomElement& it ) const;


    bool         m_isModified;
    ToolBarList& barList() { return m_barList; }
    const ToolBarList& barList() const { return m_barList; }

private:
    ToolBarList  m_barList;
    QString      m_xmlFile;
    QDomDocument m_document;
    XmlType      m_type;
    KActionCollection* m_actionCollection;
};

QString XmlData::toolBarText( const QDomElement& it ) const
{
    static const QString &tagText = KGlobal::staticQString( "text" );
    static const QString &tagText2 = KGlobal::staticQString( "Text" );
    static const QString &attrName = KGlobal::staticQString( "name" );

    QString name;
    QByteArray txt( it.namedItem( tagText ).toElement().text().toUtf8() );
    if ( txt.isEmpty() )
        txt = it.namedItem( tagText2 ).toElement().text().toUtf8();
    if ( txt.isEmpty() )
        name = it.attribute( attrName );
    else
        name = i18n( txt );

    // the name of the toolbar might depend on whether or not
    // it is in kparts
    if ( ( m_type == XmlData::Shell ) ||
         ( m_type == XmlData::Part ) ) {
        QString doc_name(m_document.documentElement().attribute( attrName ));
        name += " <" + doc_name + '>';
    }
    return name;
}


typedef QList<XmlData> XmlDataList;

class ToolBarItem : public QListWidgetItem
{
public:
    ToolBarItem(QListWidget *parent, const QString& tag = QString(), const QString& name = QString(), const QString& statusText = QString())
        : QListWidgetItem(parent),
          m_internalTag(tag),
          m_internalName(name),
          m_statusText(statusText),
          m_isSeparator(false)
    {
        // Drop between items, not onto items
        setFlags((flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
    }

    void setInternalTag(const QString &tag) { m_internalTag = tag; }
    void setInternalName(const QString &name) { m_internalName = name; }
    void setStatusText(const QString &text) { m_statusText = text; }
    void setSeparator(bool sep) { m_isSeparator = sep; }
    QString internalTag() const { return m_internalTag; }
    QString internalName() const { return m_internalName; }
    QString statusText() const { return m_statusText; }
    bool isSeparator() const { return m_isSeparator; }

    int index() const { return listWidget()->row(const_cast<ToolBarItem*>(this)); }

private:
    QString m_internalTag;
    QString m_internalName;
    QString m_statusText;
    bool m_isSeparator;
};

static QDataStream & operator<< ( QDataStream & s, const ToolBarItem & item ) {
    s << item.internalTag();
    s << item.internalName();
    s << item.statusText();
    s << item.isSeparator();
    return s;
}
static QDataStream & operator>> ( QDataStream & s, ToolBarItem & item ) {
    QString internalTag;
    s >> internalTag;
    item.setInternalTag(internalTag);
    QString internalName;
    s >> internalName;
    item.setInternalName(internalName);
    QString statusText;
    s >> statusText;
    item.setStatusText(statusText);
    bool sep;
    s >> sep;
    item.setSeparator(sep);
    return s;
}

////

ToolBarListWidget::ToolBarListWidget(QWidget *parent)
    : QListWidget(parent),
      m_activeList(true)
{
    setDragDropMode(QAbstractItemView::DragDrop); // no internal moves
}

QMimeData* ToolBarListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
    if (items.isEmpty())
        return 0;
    QMimeData* mimedata = new QMimeData();

    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        // we only support single selection
        ToolBarItem* item = static_cast<ToolBarItem *>(items.first());
        stream << *item;
    }

    mimedata->setData("application/x-kde-action-list", data);
    mimedata->setData("application/x-kde-source-treewidget", m_activeList ? "active" : "inactive");

    return mimedata;
}

bool ToolBarListWidget::dropMimeData(int index, const QMimeData * mimeData, Qt::DropAction action)
{
    Q_UNUSED(action)
    const QByteArray data = mimeData->data("application/x-kde-action-list");
    if (data.isEmpty())
        return false;
    QDataStream stream(data);
    const bool sourceIsActiveList = mimeData->data("application/x-kde-source-treewidget") == "active";
    ToolBarItem* item = new ToolBarItem(this); // needs parent, use this temporarily
    stream >> *item;
    emit dropped(this, index, item, sourceIsActiveList);
    return true;
}

ToolBarItem* ToolBarListWidget::currentItem() const
{
    return static_cast<ToolBarItem*>(QListWidget::currentItem());
}

class KEditToolBarWidgetPrivate
{
public:
    /**
     *
     * @param collection In a non-KParts application, this is the collection passed
     * to the KEditToolBar constructor.
     * In a KParts application we let create a KXMLGUIClient create a dummy one,
     * but it probably isn't used.
     */
    KEditToolBarWidgetPrivate(KEditToolBarWidget* widget,
                              const KComponentData &cData, KActionCollection* collection)
        : m_collection( collection ),
          m_widget (widget),
          m_loadedOnce( false )
    {
        m_componentData = cData;
        m_isPart   = false;
        m_helpArea = 0L;
        m_kdialogProcess = 0;
        // We want items with an icon to align with items without icon
        // So we use an empty QPixmap for that
        const int iconSize = widget->style()->pixelMetric(QStyle::PM_SmallIconSize);
        m_emptyIcon = QPixmap(iconSize, iconSize);
        m_emptyIcon.fill(Qt::transparent);
    }
    ~KEditToolBarWidgetPrivate()
    {
    }

    // private slots
    void slotToolBarSelected(int index);

    void slotInactiveSelectionChanged();
    void slotActiveSelectionChanged();

    void slotInsertButton();
    void slotRemoveButton();
    void slotUpButton();
    void slotDownButton();

    void slotChangeIcon();

    void slotProcessExited();

    void slotDropped(ToolBarListWidget* list, int index, ToolBarItem* item, bool sourceIsActiveList);


    void setupLayout();

    void initNonKPart( const QString& file, bool global, const QString& defaultToolbar );
    void initKPart( KXMLGUIFactory* factory, const QString& defaultToolbar );
    void loadToolBarCombo( const QString& defaultToolbar );
    void loadActions(const QDomElement& elem);

    QString xmlFile(const QString& xml_file) const
    {
        return xml_file.isEmpty() ? QString(m_componentData.componentName()) + "ui.rc" :
            xml_file;
    }

    /**
     * Load in the specified XML file and dump the raw xml
     */
    QString loadXMLFile(const QString& _xml_file)
    {
        QString raw_xml;
        QString xml_file = xmlFile(_xml_file);
        //kDebug() << "loadXMLFile xml_file=" << xml_file;

        if ( !QDir::isRelativePath(xml_file) )
            raw_xml = KXMLGUIFactory::readConfigFile(xml_file);
        else
            raw_xml = KXMLGUIFactory::readConfigFile(xml_file, m_componentData);

        return raw_xml;
    }

    /**
     * Look for a given item in the current toolbar
     */
    QDomElement findElementForToolBarItem( const ToolBarItem* item ) const
    {
        static const QString &attrName    = KGlobal::staticQString( "name" );
        //kDebug(240) << "looking for name=" << item->internalName() << "and tag=" << item->internalTag();
        for(QDomNode n = m_currentToolBarElem.firstChild(); !n.isNull(); n = n.nextSibling())
        {
            QDomElement elem = n.toElement();
            if ((elem.attribute(attrName) == item->internalName()) &&
                (elem.tagName() == item->internalTag()))
                return elem;
        }
        //kDebug(240) << "no item found in the DOM with name=" << item->internalName() << "and tag=" << item->internalTag();
        return QDomElement();
    }

    void insertActive(ToolBarItem *item, ToolBarItem *before, bool prepend = false);
    void removeActive(ToolBarItem *item);
    void moveActive(ToolBarItem *item, ToolBarItem *before);
    void updateLocal(QDomElement& elem);

#ifndef NDEBUG
    void dump() const
    {
        XmlDataList::const_iterator xit = m_xmlFiles.begin();
        for ( ; xit != m_xmlFiles.end(); ++xit ) {
            (*xit).dump();
        }
    }
#endif

    KComboBox *m_toolbarCombo;

    QToolButton *m_upAction;
    QToolButton *m_removeAction;
    QToolButton *m_insertAction;
    QToolButton *m_downAction;

    //QValueList<KAction*> m_actionList;
    KActionCollection* m_collection;
    KEditToolBarWidget* m_widget;
    KComponentData m_componentData;

    QPixmap m_emptyIcon;

    XmlData*     m_currentXmlData;
    QDomElement m_currentToolBarElem;

    QString            m_xmlFile;
    QString            m_globalFile;
    QString            m_rcFile;
    QDomDocument       m_localDoc;

    ToolBarList        m_barList;
    ToolBarListWidget *m_inactiveList;
    ToolBarListWidget *m_activeList;

    XmlDataList m_xmlFiles;

    QLabel     *m_comboLabel;
    KSeparator *m_comboSeparator;
    QLabel * m_helpArea;
    KPushButton* m_changeIcon;
    KProcess* m_kdialogProcess;
    bool m_isPart : 1;
    bool m_hasKDialog : 1;
    bool m_loadedOnce : 1;
};

}

using namespace KDEPrivate;


class KEditToolBarPrivate {
public:
    KEditToolBarPrivate(KEditToolBar *q): q(q),
      m_accept(false), m_global(false),
      m_collection(0), m_factory(0), m_widget(0) {}

    void init();

    void _k_slotOk();
    void _k_slotApply();
    void _k_acceptOK(bool);
    void _k_slotDefault();

    KEditToolBar *q;
    bool m_accept;
    // Save parameters for recreating widget after resetting toolbar
    bool m_global;
    KActionCollection* m_collection;
    QString m_file;
    QString m_defaultToolBar;
    KXMLGUIFactory* m_factory;
    KEditToolBarWidget *m_widget;
};

K_GLOBAL_STATIC(QString, s_defaultToolBarName)

KEditToolBar::KEditToolBar( KActionCollection *collection,
                            QWidget* parent )
  : KDialog(parent),
    d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget( collection, this);
    d->init();
    d->m_collection = collection;
}

KEditToolBar::KEditToolBar( KXMLGUIFactory* factory,
                            QWidget* parent )
    : KDialog(parent),
      d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget( this);
    d->init();
    d->m_factory = factory;
}

void KEditToolBarPrivate::init()
{
    m_accept = false;
    m_factory = 0;

    q->setDefaultToolBar( QString() );

    q->setCaption(i18n("Configure Toolbars"));
    q->setButtons(KDialog::Default|KDialog::Ok|KDialog::Apply|KDialog::Cancel);
    q->setDefaultButton(KDialog::Ok);

    q->setModal(false);

    q->setMainWidget(m_widget);

    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(_k_acceptOK(bool)));
    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(enableButtonApply(bool)));
    q->enableButtonApply(false);

    q->connect(q, SIGNAL(okClicked()), SLOT(_k_slotOk()));
    q->connect(q, SIGNAL(applyClicked()), SLOT(_k_slotApply()));
    q->connect(q, SIGNAL(defaultClicked()), SLOT(_k_slotDefault()));

    q->setMinimumSize(q->sizeHint());
}

void KEditToolBar::setResourceFile( const QString& file, bool global )
{
    d->m_file = file;
    d->m_global = global;
    d->m_widget->load( d->m_file, d->m_global, d->m_defaultToolBar );
}

KEditToolBar::~KEditToolBar()
{
    delete d;
    s_defaultToolBarName->clear();
}

void KEditToolBar::setDefaultToolBar( const QString& toolBarName )
{
    if ( toolBarName.isEmpty() ) {
        d->m_defaultToolBar = *s_defaultToolBarName;
    } else {
        d->m_defaultToolBar = toolBarName;
    }
}

void KEditToolBarPrivate::_k_acceptOK(bool b)
{
    q->enableButtonOk(b);
    m_accept = b;
}

void KEditToolBarPrivate::_k_slotDefault()
{
    if ( KMessageBox::warningContinueCancel(q, i18n("Do you really want to reset all toolbars of this application to their default? The changes will be applied immediately."), i18n("Reset Toolbars"),KGuiItem(i18n("Reset")))!=KMessageBox::Continue )
        return;

    delete m_widget;
    m_widget = 0;
    m_accept = false;

    if ( m_factory )
    {
        const QString localPrefix = KStandardDirs::locateLocal("data", "");
        foreach (KXMLGUIClient* client, m_factory->clients())
        {
            QString file = client->xmlFile();

            if (file.isNull()) // ##### should be isEmpty?
                continue;

            if (QDir::isRelativePath(file))
            {
                const KComponentData cData = client->componentData().isValid() ? client->componentData() : KGlobal::mainComponent();
                file = KStandardDirs::locateLocal("data", cData.componentName() + '/' + file);
            }
            else
            {
                if (!file.startsWith(localPrefix))
                    continue;
            }

            if ( QFile::exists( file ) )
                if ( !QFile::remove( file ) )
                    kWarning() << "Could not delete " << file;
        }

        m_widget = new KEditToolBarWidget( q );
        m_widget->load( m_factory, m_defaultToolBar );
        m_widget->rebuildKXMLGUIClients();
    }
    else
    {
        int slash = m_file.lastIndexOf('/')+1;
        if (slash)
            m_file = m_file.mid(slash);
        QString xml_file = KStandardDirs::locateLocal("data", KGlobal::mainComponent().componentName() + '/' + m_file);

        if ( QFile::exists( xml_file ) )
            if ( !QFile::remove( xml_file ) )
                kWarning() << "Could not delete " << xml_file;

        m_widget = new KEditToolBarWidget( m_collection, q );
        q->setResourceFile( m_file, m_global );
    }

    q->setMainWidget(m_widget);
    m_widget->show();

    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(_k_acceptOK(bool)));
    q->connect(m_widget, SIGNAL(enableOk(bool)), SLOT(enableButtonApply(bool)));

    q->enableButtonApply(false);
    emit q->newToolBarConfig();
    emit q->newToolbarConfig(); // compat
}

void KEditToolBarPrivate::_k_slotOk()
{
  if (!m_accept) {
      q->reject();
      return;
  }

  if (!m_widget->save())
  {
    // some error box here is needed
  }
  else
  {
    emit q->newToolBarConfig();
    emit q->newToolbarConfig(); // compat
    q->accept();
  }
}

void KEditToolBarPrivate::_k_slotApply()
{
    (void)m_widget->save();
    q->enableButtonApply(false);
    emit q->newToolBarConfig();
    emit q->newToolbarConfig(); // compat
}

void KEditToolBar::setGlobalDefaultToolBar(const char *toolbarName)
{
    *s_defaultToolBarName = QString::fromLatin1(toolbarName);
}

KEditToolBarWidget::KEditToolBarWidget( KActionCollection *collection,
                                        QWidget *parent )
  : QWidget(parent),
    d(new KEditToolBarWidgetPrivate(this, componentData(), collection))
{
    d->setupLayout();
}

KEditToolBarWidget::KEditToolBarWidget( QWidget *parent )
  : QWidget(parent),
    d(new KEditToolBarWidgetPrivate(this, componentData(), KXMLGUIClient::actionCollection() /*create new one*/))
{
    d->setupLayout();
}

KEditToolBarWidget::~KEditToolBarWidget()
{
    delete d;
}

void KEditToolBarWidget::load( const QString& file, bool global, const QString& defaultToolBar )
{
    d->initNonKPart( file, global, defaultToolBar );
}

void KEditToolBarWidget::load( KXMLGUIFactory* factory, const QString& defaultToolBar )
{
    d->initKPart( factory, defaultToolBar );
}

void KEditToolBarWidgetPrivate::initNonKPart( const QString& resourceFile,
                                              bool global,
                                              const QString& defaultToolBar )
{
    //TODO: make sure we can call this multiple times?
    if ( m_loadedOnce ) {
        return;
    }

    m_loadedOnce = true;
    //d->m_actionList = collection->actions();

    // handle the merging
    if (global)
        m_widget->setXMLFile(KStandardDirs::locate("config", "ui/ui_standards.rc"));
    const QString localXML = loadXMLFile( resourceFile );
    m_widget->setXML(localXML, global ? true /*merge*/ : false);

    // first, get all of the necessary info for our local xml
    XmlData local(XmlData::Local, xmlFile(resourceFile), m_collection);
    QDomDocument domDoc;
    domDoc.setContent(localXML);
    local.setDomDocument(domDoc);
    m_xmlFiles.append(local);

    // then, the merged one (ui_standards + local xml)
    XmlData merge(XmlData::Merged, QString(), m_collection);
    merge.setDomDocument(m_widget->domDocument());
    m_xmlFiles.append(merge);

#ifndef NDEBUG
    dump();
#endif

    // now load in our toolbar combo box
    loadToolBarCombo( defaultToolBar );
    m_widget->adjustSize();
    m_widget->setMinimumSize( m_widget->sizeHint() );
}

void KEditToolBarWidgetPrivate::initKPart( KXMLGUIFactory* factory,
                                           const QString& defaultToolBar )
{
    //TODO: make sure we can call this multiple times?
    if ( m_loadedOnce ) {
        return;
    }

    m_loadedOnce = true;

  // reusable vars
  QDomElement elem;

  m_widget->setFactory( factory );

  // add all of the client data
  bool first = true;
  foreach (KXMLGUIClient* client, factory->clients())
  {
    if (client->xmlFile().isEmpty())
      continue;

    XmlData::XmlType type = XmlData::Part;
    if ( first ) {
      type = XmlData::Shell;
      first = false;
    }

    XmlData data(type, client->localXMLFile(), client->actionCollection());
    QDomDocument domDoc;
    domDoc.setContent( KXMLGUIFactory::readConfigFile( client->xmlFile(), client->componentData() ) );
    data.setDomDocument(domDoc);
    m_xmlFiles.append(data);

    //d->m_actionList += client->actionCollection()->actions();
  }

#ifndef NDEBUG
  //d->dump();
#endif

  // now load in our toolbar combo box
  loadToolBarCombo( defaultToolBar );
  m_widget->adjustSize();
  m_widget->setMinimumSize( m_widget->sizeHint() );

  m_widget->actionCollection()->addAssociatedWidget( m_widget );
  foreach (QAction* action, m_widget->actionCollection()->actions())
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

bool KEditToolBarWidget::save()
{
  //kDebug(240) << "KEditToolBarWidget::save";
  XmlDataList::Iterator it = d->m_xmlFiles.begin();
  for ( ; it != d->m_xmlFiles.end(); ++it)
  {
    // let's not save non-modified files
    if ( !((*it).m_isModified) )
      continue;

    // let's also skip (non-existent) merged files
    if ( (*it).type() == XmlData::Merged )
      continue;

    kDebug() << (*it).domDocument().toString();

    kDebug(240) << "Saving " << (*it).xmlFile();
    // if we got this far, we might as well just save it
    KXMLGUIFactory::saveConfigFile((*it).domDocument(), (*it).xmlFile());
  }

  if ( !factory() )
    return true;

  // Now we need to rebuild the XML GUI clients. Before we start this process that will remove
  // and create all containers, we need to check which main windows had auto save enabled (for
  // disabling it temporarily, while we rebuild the GUI). Why is this needed ? We remove the
  // containers, let's say a KToolBar. KMainWindow is listening for changes on its childs (and
  // a KToolBar is one of them). After loadState() has been called after calling to createContainer(),
  // the appearance of the toolbar has probably changed. KMainWindow was listening to this change (if
  // auto save was enabled) and will trigger a save. So, the rc file of the app will save what the XMLGUI
  // file was describing, overriding the real settings.
  //
  // By temporarily disabling the auto save feature we are sure no writing will happen on the app rc file
  // which has more priority than the XMLGUI file (global < xmlgui < rc file in terms of priority).
  //
  // After all the rebuild has finished we set back the auto save feature for all those main windows
  // that had it enabled. (ereslibre)
  QList<KMainWindow*> windowList;
  QList<KConfigGroup> configGroupList;
  foreach (KMainWindow *mw, KMainWindow::memberList()) {
    if (mw->autoSaveSettings()) {
        windowList << mw;
        configGroupList << mw->autoSaveConfigGroup();
        mw->resetAutoSaveSettings();
    }
  }

  rebuildKXMLGUIClients();

  for (int i = 0; i < windowList.count(); ++i) {
    windowList[i]->setAutoSaveSettings(configGroupList.at(i));
  }

  return true;
}

void KEditToolBarWidget::rebuildKXMLGUIClients()
{
  if ( !factory() )
    return;

  QList<KXMLGUIClient*> clients = factory()->clients();
  //kDebug(240) << "factory: " << clients.count() << " clients";

  if (!clients.count())
    return;

  // remove the elements starting from the last going to the first
  QListIterator<KXMLGUIClient*> clientIterator = clients;
  clientIterator.toBack();
  while ( clientIterator.hasPrevious() )
  {
    //kDebug(240) << "factory->removeClient " << client;
    factory()->removeClient( clientIterator.previous() );
  }

  KXMLGUIClient *firstClient = clients.first();

  // now, rebuild the gui from the first to the last
  //kDebug(240) << "rebuilding the gui";
  foreach (KXMLGUIClient* client, clients)
  {
    //kDebug(240) << "updating client " << client << " " << client->componentData().componentName() << "  xmlFile=" << client->xmlFile();
    QString file( client->xmlFile() ); // before setting ui_standards!
    if ( !file.isEmpty() )
    {
        // passing an empty stream forces the clients to reread the XML
        client->setXMLGUIBuildDocument( QDomDocument() );

        // for the shell, merge in ui_standards.rc
        if ( client == firstClient ) // same assumption as in the ctor: first==shell
            client->setXMLFile(KStandardDirs::locate("config", "ui/ui_standards.rc"));

        // and this forces it to use the *new* XML file
        client->setXMLFile( file, client == firstClient /* merge if shell */ );
    }
  }

  // Now we can add the clients to the factory
  // We don't do it in the loop above because adding a part automatically
  // adds its plugins, so we must make sure the plugins were updated first.
  foreach (KXMLGUIClient* client, clients)
    factory()->addClient( client );
}

void KEditToolBarWidgetPrivate::setupLayout()
{
  // the toolbar name combo
  m_comboLabel = new QLabel(i18n("&Toolbar:"), m_widget);
  m_toolbarCombo = new KComboBox(m_widget);
  m_comboLabel->setBuddy(m_toolbarCombo);
  m_comboSeparator = new KSeparator(m_widget);
  QObject::connect(m_toolbarCombo, SIGNAL(activated(int)),
                   m_widget,       SLOT(slotToolBarSelected(int)));

//  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
//  new_toolbar->setPixmap(BarIcon("document-new", KIconLoader::SizeSmall));
//  new_toolbar->setEnabled(false); // disabled until implemented
//  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
//  del_toolbar->setPixmap(BarIcon("edit-delete", KIconLoader::SizeSmall));
//  del_toolbar->setEnabled(false); // disabled until implemented

  // our list of inactive actions
  QLabel *inactive_label = new QLabel(i18n("A&vailable actions:"), m_widget);
  m_inactiveList = new ToolBarListWidget(m_widget);
  m_inactiveList->setDragEnabled(true);
  m_inactiveList->setActiveList(false);
  m_inactiveList->setMinimumSize(180, 250);
  m_inactiveList->setDropIndicatorShown(false); // #165663
  inactive_label->setBuddy(m_inactiveList);
  QObject::connect(m_inactiveList, SIGNAL(itemSelectionChanged()),
                   m_widget,       SLOT(slotInactiveSelectionChanged()));
  QObject::connect(m_inactiveList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                   m_widget,       SLOT(slotInsertButton()));
  QObject::connect(m_inactiveList, SIGNAL(dropped(ToolBarListWidget*, int, ToolBarItem*, bool)),
                   m_widget,       SLOT(slotDropped(ToolBarListWidget*, int, ToolBarItem*, bool)));

  // our list of active actions
  QLabel *active_label = new QLabel(i18n("Curr&ent actions:"), m_widget);
  m_activeList = new ToolBarListWidget(m_widget);
  m_activeList->setDragEnabled(true);
  m_activeList->setActiveList(true);
  // With Qt-4.1 only setting MiniumWidth results in a 0-width icon column ...
  m_activeList->setMinimumSize(m_inactiveList->minimumWidth(), 100);
  active_label->setBuddy(m_activeList);

  QObject::connect(m_activeList, SIGNAL(itemSelectionChanged()),
                   m_widget,     SLOT(slotActiveSelectionChanged()));
  QObject::connect(m_activeList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                   m_widget,     SLOT(slotRemoveButton()));
  QObject::connect(m_activeList, SIGNAL(dropped(ToolBarListWidget*, int, ToolBarItem*, bool)),
                   m_widget,     SLOT(slotDropped(ToolBarListWidget*, int, ToolBarItem*, bool)));

  // "change icon" button
  m_changeIcon = new KPushButton(i18n( "Change &Icon..." ), m_widget);
  QString kdialogExe = KStandardDirs::findExe(QLatin1String("kdialog"));
  m_hasKDialog = !kdialogExe.isEmpty();
  m_changeIcon->setEnabled(m_hasKDialog && m_activeList->currentItem());

  QObject::connect( m_changeIcon, SIGNAL( clicked() ),
                    m_widget, SLOT( slotChangeIcon() ) );

  // The buttons in the middle

  m_upAction     = new QToolButton(m_widget);
  m_upAction->setIcon( KIcon("go-up") );
  m_upAction->setEnabled(false);
  m_upAction->setAutoRepeat(true);
  QObject::connect(m_upAction, SIGNAL(clicked()), m_widget, SLOT(slotUpButton()));

  m_insertAction = new QToolButton(m_widget);
  m_insertAction->setIcon( KIcon(QApplication::isRightToLeft() ? "go-previous" : "go-next") );
  m_insertAction->setEnabled(false);
  QObject::connect(m_insertAction, SIGNAL(clicked()), m_widget, SLOT(slotInsertButton()));

  m_removeAction = new QToolButton(m_widget);
  m_removeAction->setIcon( KIcon(QApplication::isRightToLeft() ? "go-next" : "go-previous") );
  m_removeAction->setEnabled(false);
  QObject::connect(m_removeAction, SIGNAL(clicked()), m_widget, SLOT(slotRemoveButton()));

  m_downAction   = new QToolButton(m_widget);
  m_downAction->setIcon( KIcon("go-down") );
  m_downAction->setEnabled(false);
  m_downAction->setAutoRepeat(true);
  QObject::connect(m_downAction, SIGNAL(clicked()), m_widget, SLOT(slotDownButton()));

  m_helpArea = new QLabel(m_widget);
  m_helpArea->setWordWrap(true);

  // now start with our layouts
  QVBoxLayout *top_layout = new QVBoxLayout(m_widget);
  top_layout->setMargin(0);
  top_layout->setSpacing(KDialog::spacingHint());

  QVBoxLayout *name_layout = new QVBoxLayout();
  name_layout->setSpacing(KDialog::spacingHint());
  QHBoxLayout *list_layout = new QHBoxLayout();
  list_layout->setSpacing(KDialog::spacingHint());

  QVBoxLayout *inactive_layout = new QVBoxLayout();
  inactive_layout->setSpacing(KDialog::spacingHint());
  QVBoxLayout *active_layout = new QVBoxLayout();
  active_layout->setSpacing(KDialog::spacingHint());
  QHBoxLayout *changeIcon_layout = new QHBoxLayout();
  changeIcon_layout->setSpacing(KDialog::spacingHint());

  QGridLayout *button_layout = new QGridLayout();

  name_layout->addWidget(m_comboLabel);
  name_layout->addWidget(m_toolbarCombo);
//  name_layout->addWidget(new_toolbar);
//  name_layout->addWidget(del_toolbar);

  button_layout->setSpacing( 0 );
  button_layout->setRowStretch( 0, 10 );
  button_layout->addWidget(m_upAction, 1, 1);
  button_layout->addWidget(m_removeAction, 2, 0);
  button_layout->addWidget(m_insertAction, 2, 2);
  button_layout->addWidget(m_downAction, 3, 1);
  button_layout->setRowStretch( 4, 10 );

  inactive_layout->addWidget(inactive_label);
  inactive_layout->addWidget(m_inactiveList, 1);

  active_layout->addWidget(active_label);
  active_layout->addWidget(m_activeList, 1);
  active_layout->addLayout(changeIcon_layout);

  changeIcon_layout->addStretch( 1 );
  changeIcon_layout->addWidget(m_changeIcon);
  changeIcon_layout->addStretch( 1 );

  list_layout->addLayout(inactive_layout);
  list_layout->addLayout(button_layout);
  list_layout->addLayout(active_layout);

  top_layout->addLayout(name_layout);
  top_layout->addWidget(m_comboSeparator);
  top_layout->addLayout(list_layout,10);
  top_layout->addWidget(m_helpArea);
  top_layout->addWidget(new KSeparator(m_widget));
}

void KEditToolBarWidgetPrivate::loadToolBarCombo( const QString& defaultToolBar )
{
  const QLatin1String attrName( "name" );
  // just in case, we clear our combo
  m_toolbarCombo->clear();

  int defaultToolBarId = -1;
  int count = 0;
  // load in all of the toolbar names into this combo box
  XmlDataList::const_iterator xit = m_xmlFiles.begin();
  for ( ; xit != m_xmlFiles.end(); ++xit)
  {
      // skip the merged one in favor of the local one,
      // so that we can change icons
      // This also makes the app-defined named for "mainToolBar" appear rather than the ui_standards-defined name.
    if ( (*xit).type() == XmlData::Merged )
      continue;

    // each xml file may have any number of toolbars
    ToolBarList::const_iterator it = (*xit).barList().begin();
    for ( ; it != (*xit).barList().end(); ++it)
    {
        const QString text = (*xit).toolBarText( *it );
        m_toolbarCombo->addItem( text );
        const QString name = (*it).attribute(attrName);
        if (defaultToolBarId == -1 && name == defaultToolBar)
            defaultToolBarId = count;
        count++;
    }
  }
  const bool showCombo = (count > 1);
  m_comboLabel->setVisible(showCombo);
  m_comboSeparator->setVisible(showCombo);
  m_toolbarCombo->setVisible(showCombo);
  if (defaultToolBarId == -1)
      defaultToolBarId = 0;
  // we want to the specified item selected and its actions loaded
  m_toolbarCombo->setCurrentIndex(defaultToolBarId);
  slotToolBarSelected(m_toolbarCombo->currentIndex());
}

void KEditToolBarWidgetPrivate::loadActions(const QDomElement& elem)
{
  const QLatin1String tagSeparator( "Separator" );
  const QLatin1String tagMerge( "Merge" );
  const QLatin1String tagActionList( "ActionList" );
  const QLatin1String tagAction( "Action" );
  const QLatin1String attrName( "name" );

  int     sep_num = 0;
  QString sep_name("separator_%1");

  // clear our lists
  m_inactiveList->clear();
  m_activeList->clear();
  m_insertAction->setEnabled(false);
  m_removeAction->setEnabled(false);
  m_upAction->setEnabled(false);
  m_downAction->setEnabled(false);

  // We'll use this action collection
  KActionCollection* actionCollection = m_currentXmlData->actionCollection();

  // store the names of our active actions
  QSet<QString> active_list;

  // i18n filtering message for action names
  KLocalizedString nameFilter = ki18nc("@item:intable Action name in toolbar editor", "%1");

  // see if our current action is in this toolbar
  QDomNode n = elem.firstChild();
  for( ; !n.isNull(); n = n.nextSibling() )
  {
    QDomElement it = n.toElement();
    if (it.isNull()) continue;
    if (it.tagName() == tagSeparator)
    {
      ToolBarItem *act = new ToolBarItem(m_activeList, tagSeparator, sep_name.arg(sep_num++), QString());
      act->setSeparator(true);
      act->setText(SEPARATORSTRING);
      it.setAttribute( attrName, act->internalName() );
      continue;
    }

    if (it.tagName() == tagMerge)
    {
      // Merge can be named or not - use the name if there is one
      QString name = it.attribute( attrName );
      ToolBarItem *act = new ToolBarItem(m_activeList, tagMerge, name, i18n("This element will be replaced with all the elements of an embedded component."));
      if ( name.isEmpty() )
          act->setText(i18n("<Merge>"));
      else
          act->setText(i18n("<Merge %1>", name));
      continue;
    }

    if (it.tagName() == tagActionList)
    {
      ToolBarItem *act = new ToolBarItem(m_activeList, tagActionList, it.attribute(attrName), i18n("This is a dynamic list of actions. You can move it, but if you remove it you will not be able to re-add it.") );
      act->setText(i18n("ActionList: %1", it.attribute(attrName)));
      continue;
    }

    // iterate through this client's actions
    // This used to iterate through _all_ actions, but we don't support
    // putting any action into any client...
    foreach (QAction* action, actionCollection->actions())
    {
      // do we have a match?
      if (it.attribute( attrName ) == action->objectName())
      {
        // we have a match!
        ToolBarItem *act = new ToolBarItem(m_activeList, it.tagName(), action->objectName(), action->toolTip());
        act->setText(nameFilter.subs(action->text().remove(QChar('&'))).toString());
        act->setIcon(!action->icon().isNull() ? action->icon() : m_emptyIcon);

        active_list.insert(action->objectName());
        break;
      }
    }
  }

  // go through the rest of the collection
  foreach (QAction* action, actionCollection->actions())
  {
    // skip our active ones
    if (active_list.contains(action->objectName()))
      continue;

    ToolBarItem *act = new ToolBarItem(m_inactiveList, tagAction, action->objectName(), action->toolTip());
    act->setText(nameFilter.subs(action->text().remove(QChar('&'))).toString());
    act->setIcon(!action->icon().isNull() ? action->icon() : m_emptyIcon);
  }

  m_inactiveList->sortItems(Qt::AscendingOrder);

  // finally, add default separators to the inactive list
  ToolBarItem *act = new ToolBarItem(0L, tagSeparator, sep_name.arg(sep_num++), QString());
  act->setSeparator(true);
  act->setText(SEPARATORSTRING);
  m_inactiveList->insertItem(0, act);
}

KActionCollection *KEditToolBarWidget::actionCollection() const
{
  return d->m_collection;
}

void KEditToolBarWidgetPrivate::slotToolBarSelected(int index)
{
    const QLatin1String attrName( "name" );
    // We need to find the XmlData and toolbar element for this index
    // To do that, we do the same iteration as the one which filled in the combobox.

    int toolbarNumber = 0;
    XmlDataList::iterator xit = m_xmlFiles.begin();
    for ( ; xit != m_xmlFiles.end(); ++xit) {

        // skip the merged one in favor of the local one,
        // so that we can change icons
        if ( (*xit).type() == XmlData::Merged )
            continue;

        // each xml file may have any number of toolbars
        ToolBarList::Iterator it = (*xit).barList().begin();
        for ( ; it != (*xit).barList().end(); ++it) {

            // is this our toolbar?
            if (toolbarNumber == index) {

                // save our current settings
                m_currentXmlData = & (*xit);
                m_currentToolBarElem = *it;

                kDebug() << "found toolbar" << m_currentXmlData->toolBarText(*it) << "m_currentXmlData set to";
                m_currentXmlData->dump();

                // If this is a Merged xmldata, clicking the "change icon" button would assert...
                Q_ASSERT( m_currentXmlData->type() != XmlData::Merged );

                // load in our values
                loadActions(m_currentToolBarElem);

                if ((*xit).type() == XmlData::Part || (*xit).type() == XmlData::Shell)
                    m_widget->setDOMDocument( (*xit).domDocument() );
                return;
            }
            ++toolbarNumber;

        }
    }
}

void KEditToolBarWidgetPrivate::slotInactiveSelectionChanged()
{
  if (m_inactiveList->selectedItems().count())
  {
    m_insertAction->setEnabled(true);
    QString statusText = static_cast<ToolBarItem*>(m_inactiveList->selectedItems().first())->statusText();
    m_helpArea->setText( i18nc("@label Action tooltip in toolbar editor, below the action list", "%1", statusText) );
  }
  else
  {
    m_insertAction->setEnabled(false);
    m_helpArea->setText( QString() );
  }
}

void KEditToolBarWidgetPrivate::slotActiveSelectionChanged()
{
  ToolBarItem* toolitem = 0;
  if (!m_activeList->selectedItems().isEmpty())
    toolitem = static_cast<ToolBarItem *>(m_activeList->selectedItems().first());

  m_removeAction->setEnabled( toolitem );

  m_changeIcon->setEnabled( toolitem &&
                            m_hasKDialog &&
                            toolitem->internalTag() == "Action" );

  if (toolitem)
  {
    m_upAction->setEnabled(toolitem->index() != 0);
    m_downAction->setEnabled(toolitem->index() != toolitem->listWidget()->count() - 1);

    QString statusText = toolitem->statusText();
    m_helpArea->setText( i18nc("@label Action tooltip in toolbar editor, below the action list", "%1", statusText) );
  }
  else
  {
    m_upAction->setEnabled(false);
    m_downAction->setEnabled(false);
    m_helpArea->setText( QString() );
  }
}

void KEditToolBarWidgetPrivate::slotInsertButton()
{
  insertActive(m_inactiveList->currentItem(), m_activeList->currentItem(), false);

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  // TODO: #### this causes #97572.
  // It would be better to just "delete item; loadActions( ... , ActiveListOnly );" or something.
  slotToolBarSelected( m_toolbarCombo->currentIndex() );
}

void KEditToolBarWidgetPrivate::slotRemoveButton()
{
  removeActive( m_activeList->currentItem() );

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  slotToolBarSelected( m_toolbarCombo->currentIndex() );
}

void KEditToolBarWidgetPrivate::insertActive(ToolBarItem *item, ToolBarItem *before, bool prepend)
{
  if (!item)
    return;

  static const QString &tagAction    = KGlobal::staticQString( "Action" );
  static const QString &tagSeparator = KGlobal::staticQString( "Separator" );
  static const QString &attrName     = KGlobal::staticQString( "name" );
  static const QString &attrNoMerge  = KGlobal::staticQString( "noMerge" );

  QDomElement new_item;
  // let's handle the separator specially
  if (item->isSeparator())
    new_item = m_widget->domDocument().createElement(tagSeparator);
  else
    new_item = m_widget->domDocument().createElement(tagAction);

  new_item.setAttribute(attrName, item->internalName());

  Q_ASSERT(!m_currentToolBarElem.isNull());

  if (before)
  {
    // we have the item in the active list which is before the new
    // item.. so let's try our best to add our new item right after it
    QDomElement elem = findElementForToolBarItem( before );
    Q_ASSERT( !elem.isNull() );
    m_currentToolBarElem.insertAfter(new_item, elem);
  }
  else
  {
    // simply put it at the beginning or the end of the list.
    if (prepend)
      m_currentToolBarElem.insertBefore(new_item, m_currentToolBarElem.firstChild());
    else
      m_currentToolBarElem.appendChild(new_item);
  }

  // and set this container as a noMerge
  m_currentToolBarElem.setAttribute( attrNoMerge, "1");

  // update the local doc
  updateLocal(m_currentToolBarElem);
}

void KEditToolBarWidgetPrivate::removeActive(ToolBarItem *item)
{
  if (!item)
    return;

  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  // now iterate through to find the child to nuke
  QDomElement elem = findElementForToolBarItem( item );
  if ( !elem.isNull() )
  {
    // nuke myself!
    m_currentToolBarElem.removeChild(elem);

    // and set this container as a noMerge
    m_currentToolBarElem.setAttribute( attrNoMerge, "1");

    // update the local doc
    updateLocal(m_currentToolBarElem);
  }
}

void KEditToolBarWidgetPrivate::slotUpButton()
{
  ToolBarItem *item = m_activeList->currentItem();

  if (!item) {
    Q_ASSERT(false);
    return;
  }

  int row = item->listWidget()->row(item) - 1;
  // make sure we're not the top item already
  if (row < 0) {
    Q_ASSERT(false);
    return;
  }

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  moveActive( item, static_cast<ToolBarItem*>(item->listWidget()->item(row - 1)) );
}

void KEditToolBarWidgetPrivate::moveActive( ToolBarItem* item, ToolBarItem* before )
{
  QDomElement e = findElementForToolBarItem( item );

  if ( e.isNull() )
    return;

  // remove item
  m_activeList->takeItem(m_activeList->row(item));

  // put it where it's supposed to go
  m_activeList->insertItem(m_activeList->row(before) + 1, item);

  // make it selected again
  m_activeList->setCurrentItem(item);

  // and do the real move in the DOM
  if ( !before )
    m_currentToolBarElem.insertBefore(e, m_currentToolBarElem.firstChild() );
  else
    m_currentToolBarElem.insertAfter(e, findElementForToolBarItem( (ToolBarItem*)before ));

  // and set this container as a noMerge
  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );
  m_currentToolBarElem.setAttribute( attrNoMerge, "1");

  // update the local doc
  updateLocal(m_currentToolBarElem);
}

void KEditToolBarWidgetPrivate::slotDownButton()
{
  ToolBarItem *item = m_activeList->currentItem();

  if (!item) {
    Q_ASSERT(false);
    return;
  }

  // make sure we're not the bottom item already
  int newRow = item->listWidget()->row(item) + 1;
  if (newRow >= item->listWidget()->count()) {
    Q_ASSERT(false);
    return;
  }

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  moveActive( item, static_cast<ToolBarItem*>(item->listWidget()->item(newRow)) );
}

void KEditToolBarWidgetPrivate::updateLocal(QDomElement& elem)
{
  static const QString &attrName = KGlobal::staticQString( "name" );

  XmlDataList::Iterator xit = m_xmlFiles.begin();
  for ( ; xit != m_xmlFiles.end(); ++xit)
  {
    if ( (*xit).type() == XmlData::Merged )
      continue;

    if ( (*xit).type() == XmlData::Shell ||
         (*xit).type() == XmlData::Part )
    {
      if ( m_currentXmlData->xmlFile() == (*xit).xmlFile() )
      {
        (*xit).m_isModified = true;
        return;
      }

      continue;
    }

    (*xit).m_isModified = true;

    ToolBarList::Iterator it = (*xit).barList().begin();
    for ( ; it != (*xit).barList().end(); ++it)
    {
      QString name( (*it).attribute( attrName ) );
      QString tag( (*it).tagName() );
      if ( (tag != elem.tagName()) || (name != elem.attribute(attrName)) )
        continue;

      QDomElement toolbar = (*xit).domDocument().documentElement().toElement();
      toolbar.replaceChild(elem, (*it));
      return;
    }

    // just append it
    QDomElement toolbar = (*xit).domDocument().documentElement().toElement();
    Q_ASSERT(!toolbar.isNull());
    toolbar.appendChild(elem);
  }
}

void KEditToolBarWidgetPrivate::slotChangeIcon()
{
  // We can't use KIconChooser here, since it's in libkio
  // ##### KDE4: reconsider this, e.g. move KEditToolBar to libkio,
  // ##### or better, dlopen libkfile from here like kio does.

  //if the process is already running (e.g. when somebody clicked the change button twice (see #127149)) - do nothing...
  //otherwise m_kdialogProcess will be overwritten and set to zero in slotProcessExited()...crash!
  if ( m_kdialogProcess && m_kdialogProcess->state() == QProcess::Running )
        return;

  m_currentXmlData->dump();
  Q_ASSERT( m_currentXmlData->type() != XmlData::Merged );

  m_kdialogProcess = new KProcess;
  QString kdialogExe = KStandardDirs::findExe(QLatin1String("kdialog"));
  (*m_kdialogProcess) << kdialogExe;
  (*m_kdialogProcess) << "--caption";
  (*m_kdialogProcess) << i18n( "Change Icon" );
  (*m_kdialogProcess) << "--embed";
  (*m_kdialogProcess) << QString::number( (ulong)m_widget->window()->winId() );
  (*m_kdialogProcess) << "--geticon";
  (*m_kdialogProcess) << "Toolbar";
  (*m_kdialogProcess) << "Actions";
  m_kdialogProcess->setOutputChannelMode(KProcess::OnlyStdoutChannel);
  m_kdialogProcess->setNextOpenMode( QIODevice::ReadOnly | QIODevice::Text );
  m_kdialogProcess->start();
  if ( !m_kdialogProcess->waitForStarted() ) {
    kError(240) << "Can't run " << kdialogExe << endl;
    delete m_kdialogProcess;
    m_kdialogProcess = 0;
    return;
  }

  m_activeList->setEnabled( false ); // don't change the current item
  m_toolbarCombo->setEnabled( false ); // don't change the current toolbar

  QObject::connect( m_kdialogProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ),
                    m_widget, SLOT( slotProcessExited() ) );
}

void KEditToolBarWidgetPrivate::slotProcessExited()
{
  m_activeList->setEnabled( true );
  m_toolbarCombo->setEnabled( true );

  QString icon;

  if (!m_kdialogProcess) {
         kError(240) << "Something is wrong here! m_kdialogProcess is zero!" << endl;
         return;
  }

  icon = QString::fromLocal8Bit( m_kdialogProcess->readLine() );
  icon = icon.left( icon.indexOf( '\n' ) );
  kDebug(240) << "icon=" << icon;
  if ( m_kdialogProcess->exitStatus() != QProcess::NormalExit ||
       icon.isEmpty() ) {
    delete m_kdialogProcess;
    m_kdialogProcess = 0;
    return;
  }

  ToolBarItem *item = m_activeList->currentItem();
  kDebug() << item;
  if(item){
    item->setIcon(KIcon(icon));

    Q_ASSERT( m_currentXmlData->type() != XmlData::Merged );

    m_currentXmlData->m_isModified = true;

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( m_currentXmlData->domDocument() );
    // Find or create an element for this action
    QDomElement act_elem = KXMLGUIFactory::findActionByName( elem, item->internalName(), true /*create*/ );
    Q_ASSERT( !act_elem.isNull() );
    act_elem.setAttribute( "icon", icon );

    // we're modified, so let this change
    emit m_widget->enableOk(true);
  }

  delete m_kdialogProcess;
  m_kdialogProcess = 0;
}

void KEditToolBarWidgetPrivate::slotDropped(ToolBarListWidget* list, int index, ToolBarItem* item, bool sourceIsActiveList)
{
    //kDebug() << "slotDropped list=" << (list==m_activeList?"activeList":"inactiveList")
    //         << "index=" << index << "sourceIsActiveList=" << sourceIsActiveList;
    if (list == m_activeList) {
        ToolBarItem* after = index > 0 ? static_cast<ToolBarItem *>(list->item(index-1)) : 0;
        //kDebug() << "after" << after->text() << after->internalTag();
        if (sourceIsActiveList) {
            // has been dragged within the active list (moved).
            moveActive(item, after);
        } else {
            // dragged from the inactive list to the active list
            insertActive(item, after, true);
        }
    } else if (list == m_inactiveList) {
        // has been dragged to the inactive list -> remove from the active list.
        removeActive(item);
    }

    delete item; // not needed anymore. must be deleted before slotToolBarSelected clears the lists

    // we're modified, so let this change
    emit m_widget->enableOk(true);

    slotToolBarSelected( m_toolbarCombo->currentIndex() );
}


void KEditToolBar::showEvent( QShowEvent * event )
{
  // The dialog has been shown, enable toolbar editing
    if ( d->m_factory ) {
        // call the kpart version
        d->m_widget->load( d->m_factory, d->m_defaultToolBar );
    } else {
        // call the action collection version
        d->m_widget->load( d->m_file, d->m_global, d->m_defaultToolBar );
    }

    KToolBar::setToolBarsEditable(true);
    KDialog::showEvent(event);
}

void KEditToolBar::hideEvent(QHideEvent* event)
{
  // The dialog has been hidden, disable toolbar editing
  KToolBar::setToolBarsEditable(false);

  KDialog::hideEvent(event);
}

#include "kedittoolbar.moc"
#include "kedittoolbar_p.moc"
