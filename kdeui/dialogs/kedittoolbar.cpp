// -*- mode: c++; c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include <qdom.h>
#include <qlayout.h>
#include <qdir.h>
#include <qfile.h>
#include <QHeaderView>
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qtextstream.h>
#include <QTreeWidget>
#include <QMimeData>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kicon.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kseparator.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <k3procio.h>
#include <ktoolbar.h>

#include "kaction.h"
#include "kactioncollection.h"

static const char * const separatorstring = I18N_NOOP("--- separator ---");

#define SEPARATORSTRING i18n(separatorstring)

static void dump_xml(const QDomDocument& doc)
{
    QString str;
    QTextStream ts(&str, QIODevice::WriteOnly);
    ts << doc;
    kDebug() << str << endl;
}

typedef QList<QDomElement> ToolBarList;

class XmlData
{
public:
  enum XmlType { Shell = 0, Part, Local, Merged };
  XmlData()
  {
    m_isModified = false;
    m_actionCollection = 0;
  }

  QString      m_xmlFile;
  QDomDocument m_document;
  XmlType      m_type;
  bool         m_isModified;
  KActionCollection* m_actionCollection;

  ToolBarList  m_barList;
};

typedef QList<XmlData> XmlDataList;

class ToolBarItem : public QTreeWidgetItem
{
public:
  ToolBarItem(QTreeWidget *parent, const QString& tag = QString(), const QString& name = QString(), const QString& statusText = QString())
    : QTreeWidgetItem(parent)
  {
    setInternalTag(tag);
    setInternalName(name);
    setStatusText(statusText);

    setFlags(flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
  }

  void setInternalTag(const QString &tag) { setText(2, tag); }
  void setInternalName(const QString &name) { setText(3, name); }
  void setStatusText(const QString &text) { setText(4, text); }
  QString internalTag() const { return text(2); }
  QString internalName() const { return text(3); }
  QString statusText() const { return text(4); }

  int index() const { return treeWidget()->indexOfTopLevelItem(const_cast<ToolBarItem*>(this)); }
};

class ToolBarListView : public QTreeWidget
{
public:
  ToolBarListView(QWidget *parent=0)
    : QTreeWidget(parent)
  {
    setAcceptDrops(true);
    setDragEnabled(true);

    setColumnCount(5);
    header()->hideSection(2);
    header()->hideSection(3);
    header()->hideSection(4);
    header()->setStretchLastSection(true);
    header()->hide();

    setIndentation(0);
  }

  void makeVisible(QTreeWidgetItem* item)
  {
    scrollTo(indexFromItem(item));
  }

  ToolBarItem* currentItem() const
  {
    return static_cast<ToolBarItem*>(QTreeWidget::currentItem());
  }

protected:
  virtual Qt::DropActions supportedDropActions() const
  {
    return Qt::MoveAction;
  }

  virtual QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const
  {
    QMimeData* mimedata = new QMimeData();

    QByteArray data;
    {
      QDataStream stream(&data, QIODevice::WriteOnly);

      QStringList actionNames;
      foreach (QTreeWidgetItem* item, items) {
        if (!item->text(3).isEmpty())
          actionNames.append(item->text(3));
      }

      stream << actionNames;
    }

    mimedata->setData("application/x-kde-action-list", data);

    return mimedata;
  }
};

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
        m_widget (widget)
  {
    m_componentData = cData;
    m_isPart   = false;
    m_helpArea = 0L;
    m_kdialogProcess = 0;
  }
  ~KEditToolBarWidgetPrivate()
  {
  }

  // private slots
  void slotToolBarSelected(const QString& text);
  
  void slotInactiveSelectionChanged();
  void slotActiveSelectionChanged();
  
  void slotInsertButton();
  void slotRemoveButton();
  void slotUpButton();
  void slotDownButton();
  
  void slotChangeIcon();
  
  void slotProcessExited( K3Process* );
  



  void setupLayout();
  
  void initNonKPart(KActionCollection *collection, const QString& file, bool global);
  void initKPart(KXMLGUIFactory* factory);
  void loadToolBarCombo(const QString& defaultToolBar = QString());
  void loadActionList(QDomElement& elem);
  
  QString xmlFile(const QString& xml_file)
  {
    return xml_file.isNull() ? QString(m_componentData.componentName()) + "ui.rc" :
                               xml_file;
  }

  /**
   * Load in the specified XML file and dump the raw xml
   */
  QString loadXMLFile(const QString& _xml_file)
  {
    QString raw_xml;
    QString xml_file = xmlFile(_xml_file);
    //kDebug() << "loadXMLFile xml_file=" << xml_file << endl;

    if ( !QDir::isRelativePath(xml_file) )
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file);
    else
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file, m_componentData);

    return raw_xml;
  }

  /**
   * Return a list of toolbar elements given a toplevel element
   */
  ToolBarList findToolBars(QDomNode n)
  {
    static const QString &tagToolBar = KGlobal::staticQString( "ToolBar" );
    static const QString &attrNoEdit = KGlobal::staticQString( "noEdit" );
    ToolBarList list;

    for( ; !n.isNull(); n = n.nextSibling() )
    {
      QDomElement elem = n.toElement();
      if (elem.isNull())
        continue;

      if (elem.tagName() == tagToolBar && elem.attribute( attrNoEdit ) != "true" )
        list.append(elem);

      list += findToolBars(elem.firstChild());
    }

    return list;
  }

  /**
   * Return the name of a given toolbar
   */
  QString toolbarName( const XmlData& xmlData, const QDomElement& it ) const
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
      if ( ( xmlData.m_type == XmlData::Shell ) ||
           ( xmlData.m_type == XmlData::Part ) )
      {
        QString doc_name(xmlData.m_document.documentElement().attribute( attrName ));
        name += " <" + doc_name + '>';
      }
      return name;
  }
  /**
   * Look for a given item in the current toolbar
   */
  QDomElement findElementForToolBarItem( const ToolBarItem* item ) const
  {
    static const QString &attrName    = KGlobal::staticQString( "name" );
    for(QDomNode n = m_currentToolBarElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
      QDomElement elem = n.toElement();
      if ((elem.attribute(attrName) == item->internalName()) &&
          (elem.tagName() == item->internalTag()))
        return elem;
    }
    return QDomElement();
  }

  void insertActive(ToolBarItem *item, ToolBarItem *before, bool prepend = false);
  void removeActive(ToolBarItem *item);
  void moveActive(ToolBarItem *item, ToolBarItem *before);
  void updateLocal(QDomElement& elem);

#ifndef NDEBUG
  void dump()
  {
    static const char* s_XmlTypeToString[] = { "Shell", "Part", "Local", "Merged" };
    XmlDataList::Iterator xit = m_xmlFiles.begin();
    for ( ; xit != m_xmlFiles.end(); ++xit )
    {
        kDebug(240) << "XmlData type " << s_XmlTypeToString[(*xit).m_type] << " xmlFile: " << (*xit).m_xmlFile << endl;
        foreach (const QDomElement& element,  (*xit).m_barList) {
            kDebug(240) << "    ToolBar: " << toolbarName( *xit, element ) << endl;
        }
        if ( (*xit).m_actionCollection )
            kDebug(240) << "    " << (*xit).m_actionCollection->actions().count() << " actions in the collection." << endl;
        else
            kDebug(240) << "    no action collection." << endl;
    }
  }
#endif

  QComboBox *m_toolbarCombo;
  
  QToolButton *m_upAction;
  QToolButton *m_removeAction;
  QToolButton *m_insertAction;
  QToolButton *m_downAction;
  
  //QValueList<KAction*> m_actionList;
  KActionCollection* m_collection;
  KEditToolBarWidget* m_widget;
  KComponentData m_componentData;

  XmlData*     m_currentXmlData;
  QDomElement m_currentToolBarElem;

  QString            m_xmlFile;
  QString            m_globalFile;
  QString            m_rcFile;
  QDomDocument       m_localDoc;
  bool               m_isPart;

  ToolBarList        m_barList;
  ToolBarListView *m_inactiveList;
  ToolBarListView *m_activeList;

  XmlDataList m_xmlFiles;

  QLabel     *m_comboLabel;
  KSeparator *m_comboSeparator;
  QLabel * m_helpArea;
  KPushButton* m_changeIcon;
  K3ProcIO* m_kdialogProcess;
  bool m_hasKDialog;
};

class KEditToolBarPrivate {
public:
    KEditToolBarPrivate(KEditToolBar *q): q(q),
      m_accept(false), m_global(false),
      m_collection(0), m_factory(0), m_widget(0) {}
  
    KEditToolBar *q;
    bool m_accept;
    // Save parameters for recreating widget after resetting toolbar
    bool m_global;
    KActionCollection* m_collection;
    QString m_file;
    KXMLGUIFactory* m_factory;
    KEditToolBarWidget *m_widget;

    static const char *s_defaultToolBar;
};

const char *KEditToolBarPrivate::s_defaultToolBar = 0L;

KEditToolBar::KEditToolBar(KActionCollection *collection, const QString& file,
                           bool global, QWidget* parent)
  : KDialog(parent),
    d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget(QLatin1String(KEditToolBarPrivate::s_defaultToolBar), collection, file, global, this);
    init();
    d->m_global = global;
    d->m_collection = collection;
    d->m_file = file;
}

KEditToolBar::KEditToolBar(const QString& defaultToolBar, KActionCollection *collection,
                           const QString& file, bool global,
                           QWidget* parent)
  : KDialog(parent),
    d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget(defaultToolBar, collection, file, global, this);
    init();
    d->m_global = global;
    d->m_collection = collection;
    d->m_file = file;
}

KEditToolBar::KEditToolBar(KXMLGUIFactory* factory, QWidget* parent)
    : KDialog(parent),
      d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget(QLatin1String(KEditToolBarPrivate::s_defaultToolBar), factory, this);
    init();
    d->m_factory = factory;
}

KEditToolBar::KEditToolBar(const QString& defaultToolBar,KXMLGUIFactory* factory,
                           QWidget* parent)
    : KDialog(parent),
      d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget(defaultToolBar, factory, this);
    init();
    d->m_factory = factory;
}

void KEditToolBar::init()
{
    d->m_accept = false;
    d->m_factory = 0;

    setCaption(i18n("Configure ToolBars"));
    setButtons(Default|Ok|Apply|Cancel);
    setDefaultButton(Ok);

    setModal(false);

    setMainWidget(d->m_widget);

    connect(d->m_widget, SIGNAL(enableOk(bool)), SLOT(acceptOK(bool)));
    connect(d->m_widget, SIGNAL(enableOk(bool)), SLOT(enableButtonApply(bool)));
    enableButtonApply(false);

    connect(this, SIGNAL(okClicked()), SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), SLOT(slotApply()));
    connect(this, SIGNAL(defaultClicked()), SLOT(slotDefault()));

    setMinimumSize(sizeHint());
    KEditToolBarPrivate::s_defaultToolBar = 0L;
}

KEditToolBar::~KEditToolBar()
{
    delete d;
}

void KEditToolBar::acceptOK(bool b)
{
    enableButtonOk(b);
    d->m_accept = b;
}

void KEditToolBar::slotDefault()
{
    if ( KMessageBox::warningContinueCancel(this, i18n("Do you really want to reset all toolbars of this application to their default? The changes will be applied immediately."), i18n("Reset ToolBars"),KGuiItem(i18n("Reset")))!=KMessageBox::Continue )
        return;

    delete d->m_widget;
    d->m_widget = 0;
    d->m_accept = false;

    if ( d->m_factory )
    {
        const QString localPrefix = KStandardDirs::locateLocal("data", "");
        foreach (KXMLGUIClient* client, d->m_factory->clients())
        {
            QString file = client->xmlFile();

            if (file.isNull())
                continue;

            if (QDir::isRelativePath(file))
            {
                const KComponentData cData = client->componentData().isValid() ? client->componentData() : KGlobal::mainComponent();
                file = KStandardDirs::locateLocal("data", QLatin1String( cData.componentName() + '/' ) + file);
            }
            else
            {
                if (!file.startsWith(localPrefix))
                    continue;
            }

            if ( QFile::exists( file ) )
                if ( !QFile::remove( file ) )
                    kWarning() << "Could not delete " << file << endl;
        }

        d->m_widget = new KEditToolBarWidget(QString(), d->m_factory, this);
        d->m_widget->rebuildKXMLGUIClients();
    }
    else
    {
        int slash = d->m_file.lastIndexOf('/')+1;
        if (slash)
            d->m_file = d->m_file.mid(slash);
        QString xml_file = KStandardDirs::locateLocal("data", QLatin1String( KGlobal::mainComponent().componentName() + '/' ) + d->m_file);

        if ( QFile::exists( xml_file ) )
            if ( !QFile::remove( xml_file ) )
                kWarning() << "Could not delete " << xml_file << endl;

        d->m_widget = new KEditToolBarWidget(QString(), d->m_collection, d->m_file, d->m_global, this);
    }

    setMainWidget(d->m_widget);
    d->m_widget->show();

    connect(d->m_widget, SIGNAL(enableOk(bool)), SLOT(acceptOK(bool)));
    connect(d->m_widget, SIGNAL(enableOk(bool)), SLOT(enableButtonApply(bool)));

    enableButtonApply(false);
    emit newToolBarConfig();
    emit newToolbarConfig(); // compat
}

void KEditToolBar::slotOk()
{
  if (!d->m_accept) {
      reject();
      return;
  }

  if (!d->m_widget->save())
  {
    // some error box here is needed
  }
  else
  {
    emit newToolBarConfig();
    emit newToolbarConfig(); // compat
    accept();
  }
}

void KEditToolBar::slotApply()
{
    (void)d->m_widget->save();
    enableButtonApply(false);
    emit newToolBarConfig();
    emit newToolbarConfig(); // compat
}

void KEditToolBar::setDefaultToolBar(const char *toolbarName)
{
    KEditToolBarPrivate::s_defaultToolBar = toolbarName;
}

KEditToolBarWidget::KEditToolBarWidget(KActionCollection *collection,
                                       const QString& file,
                                       bool global, QWidget *parent)
  : QWidget(parent),
    d(new KEditToolBarWidgetPrivate(this, componentData(), collection))
{
  d->initNonKPart(collection, file, global);
  // now load in our toolbar combo box
  d->loadToolBarCombo();
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolBarWidget::KEditToolBarWidget(const QString& defaultToolBar,
                                       KActionCollection *collection,
                                       const QString& file, bool global,
                                       QWidget *parent)
  : QWidget(parent),
    d(new KEditToolBarWidgetPrivate(this, componentData(), collection))
{
  d->initNonKPart(collection, file, global);
  // now load in our toolbar combo box
  d->loadToolBarCombo(defaultToolBar);
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolBarWidget::KEditToolBarWidget( KXMLGUIFactory* factory,
                                        QWidget *parent)
  : QWidget(parent),
    d(new KEditToolBarWidgetPrivate(this, componentData(), KXMLGUIClient::actionCollection() /*create new one*/))
{
  d->initKPart(factory);
  // now load in our toolbar combo box
  d->loadToolBarCombo();
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolBarWidget::KEditToolBarWidget( const QString& defaultToolBar,
                                        KXMLGUIFactory* factory,
                                        QWidget *parent)
  : QWidget(parent),
    d(new KEditToolBarWidgetPrivate(this, componentData(), KXMLGUIClient::actionCollection() /*create new one*/))
{
  d->initKPart(factory);
  // now load in our toolbar combo box
  d->loadToolBarCombo(defaultToolBar);
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolBarWidget::~KEditToolBarWidget()
{
    delete d;
}

void KEditToolBarWidgetPrivate::initNonKPart(KActionCollection *collection,
                                      const QString& file, bool global)
{
  //d->m_actionList = collection->actions();

  // handle the merging
  if (global)
    m_widget->setXMLFile(KStandardDirs::locate("config", "ui/ui_standards.rc"));
  QString localXML = loadXMLFile(file);
  m_widget->setXML(localXML, true);

  // reusable vars
  QDomElement elem;

  // first, get all of the necessary info for our local xml
  XmlData local;
  local.m_xmlFile = xmlFile(file);
  local.m_type    = XmlData::Local;
  local.m_document.setContent(localXML);
  elem = local.m_document.documentElement().toElement();
  local.m_barList = findToolBars(elem);
  local.m_actionCollection = collection;
  m_xmlFiles.append(local);

  // then, the merged one (ui_standards + local xml)
  XmlData merge;
  merge.m_xmlFile.clear();
  merge.m_type     = XmlData::Merged;
  merge.m_document = m_widget->domDocument();
  elem = merge.m_document.documentElement().toElement();
  merge.m_barList  = findToolBars(elem);
  merge.m_actionCollection = collection;
  m_xmlFiles.append(merge);

#ifndef NDEBUG
  //d->dump();
#endif

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();
}

void KEditToolBarWidgetPrivate::initKPart(KXMLGUIFactory* factory)
{
  // reusable vars
  QDomElement elem;

  m_widget->setFactory( factory );
  m_widget->actionCollection()->setAssociatedWidget( m_widget );

  // add all of the client data
  bool first = true;
  foreach (KXMLGUIClient* client, factory->clients())
  {
    if (client->xmlFile().isNull())
      continue;

    XmlData data;
    data.m_xmlFile = client->localXMLFile();
    if ( first ) {
      data.m_type = XmlData::Shell;
      first = false;
    } else {
      data.m_type = XmlData::Part;
    }
    data.m_document.setContent( KXMLGUIFactory::readConfigFile( client->xmlFile(), client->componentData() ) );
    elem = data.m_document.documentElement().toElement();
    data.m_barList = findToolBars(elem);
    data.m_actionCollection = client->actionCollection();
    m_xmlFiles.append(data);

    //d->m_actionList += client->actionCollection()->actions();
  }

#ifndef NDEBUG
  //d->dump();
#endif

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();
}

bool KEditToolBarWidget::save()
{
  //kDebug(240) << "KEditToolBarWidget::save" << endl;
  XmlDataList::Iterator it = d->m_xmlFiles.begin();
  for ( ; it != d->m_xmlFiles.end(); ++it)
  {
    // let's not save non-modified files
    if ( !((*it).m_isModified) )
      continue;

    // let's also skip (non-existent) merged files
    if ( (*it).m_type == XmlData::Merged )
      continue;

    dump_xml((*it).m_document);

    kDebug(240) << "Saving " << (*it).m_xmlFile << endl;
    // if we got this far, we might as well just save it
    KXMLGUIFactory::saveConfigFile((*it).m_document, (*it).m_xmlFile);
  }

  if ( !factory() )
    return true;

  rebuildKXMLGUIClients();

  return true;
}

void KEditToolBarWidget::rebuildKXMLGUIClients()
{
  if ( !factory() )
    return;

  QList<KXMLGUIClient*> clients = factory()->clients();
  //kDebug(240) << "factory: " << clients.count() << " clients" << endl;

  if (!clients.count())
    return;

  // remove the elements starting from the last going to the first
  QListIterator<KXMLGUIClient*> clientIterator = clients;
  clientIterator.toBack();
  while ( clientIterator.hasPrevious() )
  {
    //kDebug(240) << "factory->removeClient " << client << endl;
    factory()->removeClient( clientIterator.previous() );
  }

  KXMLGUIClient *firstClient = clients.first();

  // now, rebuild the gui from the first to the last
  //kDebug(240) << "rebuilding the gui" << endl;
  foreach (KXMLGUIClient* client, clients)
  {
    //kDebug(240) << "updating client " << client << " " << client->componentData().componentName() << "  xmlFile=" << client->xmlFile() << endl;
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
  m_comboLabel = new QLabel(i18n("&ToolBar:"), m_widget);
  m_toolbarCombo = new QComboBox(m_widget);
  m_toolbarCombo->setEnabled(false);
  m_comboLabel->setBuddy(m_toolbarCombo);
  m_comboSeparator = new KSeparator(m_widget);
  QObject::connect(m_toolbarCombo, SIGNAL(activated(const QString&)),
                   m_widget,       SLOT(slotToolBarSelected(const QString&)));

//  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
//  new_toolbar->setPixmap(BarIcon("document-new", K3Icon::SizeSmall));
//  new_toolbar->setEnabled(false); // disabled until implemented
//  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
//  del_toolbar->setPixmap(BarIcon("edit-delete", K3Icon::SizeSmall));
//  del_toolbar->setEnabled(false); // disabled until implemented

  // our list of inactive actions
  QLabel *inactive_label = new QLabel(i18n("A&vailable actions:"), m_widget);
  m_inactiveList = new ToolBarListView(m_widget);
  m_inactiveList->setDragEnabled(true);
  //m_inactiveList->setAcceptDrops(true);

  //KDE4: no replacement?
  //m_inactiveList->setDropVisualizer(false);
  //m_inactiveList->setAllColumnsShowFocus(true);

  m_inactiveList->setMinimumSize(180, 250);
  inactive_label->setBuddy(m_inactiveList);
  QObject::connect(m_inactiveList, SIGNAL(itemSelectionChanged()),
                   m_widget,       SLOT(slotInactiveSelectionChanged()));
  QObject::connect(m_inactiveList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
                   m_widget,       SLOT(slotInsertButton()));

  // our list of active actions
  QLabel *active_label = new QLabel(i18n("Curr&ent actions:"), m_widget);
  m_activeList = new ToolBarListView(m_widget);
  m_activeList->setDragEnabled(true);
  //m_activeList->setAcceptDrops(true);

  //KDE4: no replacement?
  //m_activeList->setDropVisualizer(true);
  //m_activeList->setAllColumnsShowFocus(true);

  // With Qt-4.1 only setting MiniumWidth results in a 0-width icon column ...
  m_activeList->setMinimumSize(m_inactiveList->minimumWidth(), 100);
  active_label->setBuddy(m_activeList);

  QObject::connect(m_activeList, SIGNAL(itemSelectionChanged()),
                   m_widget,     SLOT(slotActiveSelectionChanged()));
  QObject::connect(m_activeList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
                   m_widget,     SLOT(slotRemoveButton()));

  // "change icon" button
  m_changeIcon = new KPushButton(i18n( "Change &Icon..." ), m_widget);
  QString kdialogExe = KStandardDirs::findExe(QLatin1String("kdialog"));
  m_hasKDialog = !kdialogExe.isEmpty();
  m_changeIcon->setEnabled(m_hasKDialog);

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

void KEditToolBarWidgetPrivate::loadToolBarCombo(const QString& defaultToolBar)
{
  const QLatin1String attrName( "name" );
  // just in case, we clear our combo
  m_toolbarCombo->clear();

  int defaultToolBarId = -1;
  int count = 0;
  // load in all of the toolbar names into this combo box
  XmlDataList::Iterator xit = m_xmlFiles.begin();
  for ( ; xit != m_xmlFiles.end(); ++xit)
  {
    // skip the local one in favor of the merged
    if ( (*xit).m_type == XmlData::Local )
      continue;

    // each xml file may have any number of toolbars
    ToolBarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      QString name = toolbarName( *xit, *it );
      m_toolbarCombo->setEnabled( true );
      m_toolbarCombo->addItem( name );
      if (defaultToolBarId == -1 && (name == defaultToolBar || defaultToolBar == (*it).attribute( attrName )))
          defaultToolBarId = count;
      count++;
    }
  }
  bool showCombo = (count > 1);
  m_comboLabel->setVisible(showCombo);
  m_comboSeparator->setVisible(showCombo);
  m_toolbarCombo->setVisible(showCombo);
  if (defaultToolBarId == -1)
      defaultToolBarId = 0;
  // we want to the specified item selected and its actions loaded
  m_toolbarCombo->setCurrentIndex(defaultToolBarId);
  slotToolBarSelected(m_toolbarCombo->currentText());
}

void KEditToolBarWidgetPrivate::loadActionList(QDomElement& elem)
{
  const QLatin1String tagSeparator( "Separator" );
  const QLatin1String tagMerge( "Merge" );
  const QLatin1String tagActionList( "ActionList" );
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
  KActionCollection* actionCollection = m_currentXmlData->m_actionCollection;

  // store the names of our active actions
  QMap<QString, bool> active_list;

  // see if our current action is in this toolbar
  QDomNode n = elem.firstChild();
  for( ; !n.isNull(); n = n.nextSibling() )
  {
    QDomElement it = n.toElement();
    if (it.isNull()) continue;
    if (it.tagName() == tagSeparator)
    {
      ToolBarItem *act = new ToolBarItem(m_activeList, tagSeparator, sep_name.arg(sep_num++), QString());
      act->setText(1, SEPARATORSTRING);
      it.setAttribute( attrName, act->internalName() );
      continue;
    }

    if (it.tagName() == tagMerge)
    {
      // Merge can be named or not - use the name if there is one
      QString name = it.attribute( attrName );
      ToolBarItem *act = new ToolBarItem(m_activeList, tagMerge, name, i18n("This element will be replaced with all the elements of an embedded component."));
      if ( name.isEmpty() )
          act->setText(1, i18n("<Merge>"));
      else
          act->setText(1, i18n("<Merge %1>", name));
      continue;
    }

    if (it.tagName() == tagActionList)
    {
      ToolBarItem *act = new ToolBarItem(m_activeList, tagActionList, it.attribute(attrName), i18n("This is a dynamic list of actions. You can move it, but if you remove it you will not be able to re-add it.") );
      act->setText(1, i18n("ActionList: %1", it.attribute(attrName)));
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
        act->setText(1, action->text().remove(QChar('&')));
        if (!action->icon().isNull())
          act->setIcon(0, action->icon());

        active_list.insert(action->objectName(), true);
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

    ToolBarItem *act = new ToolBarItem(m_inactiveList, tagActionList, action->objectName(), action->toolTip());
    act->setText(1, action->text().remove(QChar('&')));
    if (!action->icon().isNull())
      act->setIcon(0, action->icon());
  }

  m_inactiveList->sortItems(1, Qt::AscendingOrder);

  // finally, add default separators to the inactive list
  ToolBarItem *act = new ToolBarItem(0L, tagSeparator, sep_name.arg(sep_num++), QString());
  act->setText(1, SEPARATORSTRING);
  m_inactiveList->insertTopLevelItem(0, act);

  m_inactiveList->resizeColumnToContents(0);
  m_activeList->resizeColumnToContents(0);
}

KActionCollection *KEditToolBarWidget::actionCollection() const
{
  return d->m_collection;
}

void KEditToolBarWidgetPrivate::slotToolBarSelected(const QString& _text)
{
  // iterate through everything
  XmlDataList::Iterator xit = m_xmlFiles.begin();
  for ( ; xit != m_xmlFiles.end(); ++xit)
  {
    // each xml file may have any number of toolbars
    ToolBarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      QString name = toolbarName( *xit, *it );
      // is this our toolbar?
      if ( name == _text )
      {
        // save our current settings
        m_currentXmlData     = & (*xit);
        m_currentToolBarElem = (*it);

        // load in our values
        loadActionList(m_currentToolBarElem);

        if ((*xit).m_type == XmlData::Part || (*xit).m_type == XmlData::Shell)
          m_widget->setDOMDocument( (*xit).m_document );
        return;
      }
    }
  }
}

void KEditToolBarWidgetPrivate::slotInactiveSelectionChanged()
{
  if (m_inactiveList->selectedItems().count())
  {
    m_insertAction->setEnabled(true);
    QString statusText = static_cast<ToolBarItem*>(m_inactiveList->selectedItems().first())->statusText();
    m_helpArea->setText( statusText );
  }
  else
  {
    m_insertAction->setEnabled(false);
    m_helpArea->setText( QString() );
  }
}

void KEditToolBarWidgetPrivate::slotActiveSelectionChanged()
{
  ToolBarItem* toolitem = 0L;
  if (m_activeList->selectedItems().count())
    toolitem = static_cast<ToolBarItem *>(m_activeList->selectedItems().first());

  m_removeAction->setEnabled( toolitem );

  static const QString &tagAction = KGlobal::staticQString( "Action" );
  m_changeIcon->setEnabled( toolitem &&
                               m_hasKDialog &&
                               toolitem->internalTag() == tagAction );

  if (toolitem)
  {
    m_upAction->setEnabled(toolitem->index() != 0);
    m_downAction->setEnabled(toolitem->index() != toolitem->treeWidget()->topLevelItemCount() - 1);

    QString statusText = toolitem->statusText();
    m_helpArea->setText( statusText );
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
  slotToolBarSelected( m_toolbarCombo->currentText() );
}

void KEditToolBarWidgetPrivate::slotRemoveButton()
{
  removeActive( m_activeList->currentItem() );

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  slotToolBarSelected( m_toolbarCombo->currentText() );
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
  if (item->text(1) == SEPARATORSTRING)
    new_item = m_widget->domDocument().createElement(tagSeparator);
  else
    new_item = m_widget->domDocument().createElement(tagAction);

  new_item.setAttribute(attrName, item->internalName());

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

  int row = item->treeWidget()->indexOfTopLevelItem(item) - 1;
  // make sure we're not the top item already
  if (row < 0) {
    Q_ASSERT(false);
    return;
  }

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  moveActive( item, static_cast<ToolBarItem*>(item->treeWidget()->topLevelItem(row - 1)) );
}

void KEditToolBarWidgetPrivate::moveActive( ToolBarItem* item, ToolBarItem* before )
{
  QDomElement e = findElementForToolBarItem( item );

  if ( e.isNull() )
    return;

  // remove item
  m_activeList->takeTopLevelItem(m_activeList->indexOfTopLevelItem(item));

  // put it where it's supposed to go
  m_activeList->insertTopLevelItem(m_activeList->indexOfTopLevelItem(before) + 1, item);

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
  int newRow = item->treeWidget()->indexOfTopLevelItem(item) + 1;
  if (newRow >= item->treeWidget()->topLevelItemCount()) {
    Q_ASSERT(false);
    return;
  }

  // we're modified, so let this change
  emit m_widget->enableOk(true);

  moveActive( item, static_cast<ToolBarItem*>(item->treeWidget()->topLevelItem(newRow)) );
}

void KEditToolBarWidgetPrivate::updateLocal(QDomElement& elem)
{
  static const QString &attrName = KGlobal::staticQString( "name" );

  XmlDataList::Iterator xit = m_xmlFiles.begin();
  for ( ; xit != m_xmlFiles.end(); ++xit)
  {
    if ( (*xit).m_type == XmlData::Merged )
      continue;

    if ( (*xit).m_type == XmlData::Shell ||
         (*xit).m_type == XmlData::Part )
    {
      if ( m_currentXmlData->m_xmlFile == (*xit).m_xmlFile )
      {
        (*xit).m_isModified = true;
        return;
      }

      continue;
    }

    (*xit).m_isModified = true;

    ToolBarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      QString name( (*it).attribute( attrName ) );
      QString tag( (*it).tagName() );
      if ( (tag != elem.tagName()) || (name != elem.attribute(attrName)) )
        continue;

      QDomElement toolbar = (*xit).m_document.documentElement().toElement();
      toolbar.replaceChild(elem, (*it));
      return;
    }

    // just append it
    QDomElement toolbar = (*xit).m_document.documentElement().toElement();
    toolbar.appendChild(elem);
  }
}

void KEditToolBarWidgetPrivate::slotChangeIcon()
{
  // We can't use KIconChooser here, since it's in libkio
  // ##### KDE4: reconsider this, e.g. move KEditToolBar to libkio

  //if the process is already running (e.g. when somebody clicked the change button twice (see #127149)) - do nothing...
  //otherwise m_kdialogProcess will be overwritten and set to zero in slotProcessExited()...crash!
  if ( m_kdialogProcess && m_kdialogProcess->isRunning() )
        return;

  m_kdialogProcess = new K3ProcIO;
  QString kdialogExe = KStandardDirs::findExe(QLatin1String("kdialog"));
  (*m_kdialogProcess) << kdialogExe;
  (*m_kdialogProcess) << "--embed";
  (*m_kdialogProcess) << QString::number( (ulong)m_widget->topLevelWidget()->winId() );
  (*m_kdialogProcess) << "--geticon";
  (*m_kdialogProcess) << "Toolbar";
  (*m_kdialogProcess) << "Actions";
  if ( !m_kdialogProcess->start( K3Process::NotifyOnExit ) ) {
    kError(240) << "Can't run " << kdialogExe << endl;
    delete m_kdialogProcess;
    m_kdialogProcess = 0;
    return;
  }

  m_activeList->setEnabled( false ); // don't change the current item
  m_toolbarCombo->setEnabled( false ); // don't change the current toolbar

  QObject::connect( m_kdialogProcess, SIGNAL( processExited( K3Process* ) ),
                    m_widget, SLOT( slotProcessExited( K3Process* ) ) );
}

void KEditToolBarWidgetPrivate::slotProcessExited( K3Process* )
{
  m_activeList->setEnabled( true );
  m_toolbarCombo->setEnabled( true );

  QString icon;

  if (!m_kdialogProcess) {
         kError(240) << "Something is wrong here! m_kdialogProcess is zero!" << endl;
         return;
  }

  if ( !m_kdialogProcess->normalExit() ||
       m_kdialogProcess->exitStatus() ||
       m_kdialogProcess->readln(icon, true) <= 0 ) {
    delete m_kdialogProcess;
    m_kdialogProcess = 0;
    return;
  }

  ToolBarItem *item = m_activeList->currentItem();
  if(item){
    item->setIcon(0, BarIcon(icon, 16));

    Q_ASSERT( m_currentXmlData->m_type != XmlData::Merged );

    m_currentXmlData->m_isModified = true;

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( m_currentXmlData->m_document );
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

void KEditToolBar::showEvent( QShowEvent * event )
{
  // The dialog has been shown, enable toolbar editing
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
