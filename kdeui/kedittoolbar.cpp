// -*- mode: c++; c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <kedittoolbar.h>

#include <qdom.h>
#include <qlayout.h>
#include <qdir.h>
#include <qfile.h>
#include <q3header.h>
#include <qcombobox.h>
#include <q3dragobject.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <q3valuelist.h>
#include <qapplication.h>
#include <qtextstream.h>

#include <kaction.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kseparator.h>
#include <kconfig.h>
#include <klistview.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kprocio.h>
#include <QDropEvent>

static const char * const lineseparatorstring = I18N_NOOP("--- line separator ---");
static const char * const separatorstring = I18N_NOOP("--- separator ---");

#define LINESEPARATORSTRING i18n(lineseparatorstring)
#define SEPARATORSTRING i18n(separatorstring)

static void dump_xml(const QDomDocument& doc)
{
    QString str;
    QTextStream ts(&str, QIODevice::WriteOnly);
    ts << doc;
    kdDebug() << str << endl;
}

typedef Q3ValueList<QDomElement> ToolbarList;

namespace
{
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

  ToolbarList  m_barList;
};

typedef Q3ValueList<XmlData> XmlDataList;

class ToolbarItem : public Q3ListViewItem
{
public:
  ToolbarItem(KListView *parent, const QString& tag = QString::null, const QString& name = QString::null, const QString& statusText = QString::null)
    : Q3ListViewItem(parent),
      m_tag(tag),
      m_name(name),
      m_statusText(statusText)
  {
  }

  ToolbarItem(KListView *parent, Q3ListViewItem *item, const QString &tag = QString::null, const QString& name = QString::null, const QString& statusText = QString::null)
    : Q3ListViewItem(parent, item),
      m_tag(tag),
      m_name(name),
      m_statusText(statusText)
  {
  }

  virtual QString key(int column, bool) const
  {
    QString s = text( column );
    if ( s == LINESEPARATORSTRING )
      return "0";
    if ( s == SEPARATORSTRING )
      return "1";
    return "2" + s;
  }

  void setInternalTag(const QString &tag) { m_tag = tag; }
  void setInternalName(const QString &name) { m_name = name; }
  void setStatusText(const QString &text) { m_statusText = text; }
  QString internalTag() const { return m_tag; }
  QString internalName() const { return m_name; }
  QString statusText() const { return m_statusText; }
private:
  QString m_tag;
  QString m_name;
  QString m_statusText;
};

#define TOOLBARITEMMIMETYPE "data/x-kde.toolbar.item"
class ToolbarItemDrag : public Q3StoredDrag
{
public:
  ToolbarItemDrag(ToolbarItem *toolbarItem,
                    QWidget *dragSource = 0, const char *name = 0)
    : Q3StoredDrag( TOOLBARITEMMIMETYPE, dragSource, name )
  {
    if (toolbarItem) {
      QByteArray data;
      QDataStream out(&data, QIODevice::WriteOnly);
      out << toolbarItem->internalTag();
      out << toolbarItem->internalName();
      out << toolbarItem->statusText();
      out << toolbarItem->text(1); // separators need this.
      setEncodedData(data);
    }
  }

  static bool canDecode(QMimeSource* e)
  {
    return e->provides(TOOLBARITEMMIMETYPE);
  }

  static bool decode( const QMimeSource* e, ToolbarItem& item )
  {
    if (!e)
      return false;

    QByteArray data = e->encodedData(TOOLBARITEMMIMETYPE);
    if ( data.isEmpty() )
      return false;

    QString internalTag, internalName, statusText, text;
    QDataStream in(data);
    in >> internalTag;
    in >> internalName;
    in >> statusText;
    in >> text;

    item.setInternalTag( internalTag );
    item.setInternalName( internalName );
    item.setStatusText( statusText );
    item.setText(1, text);

    return true;
  }
};

class ToolbarListView : public KListView
{
public:
  ToolbarListView(QWidget *parent=0)
    : KListView(parent)
  {
  }
protected:
  virtual Q3DragObject *dragObject()
  {
    ToolbarItem *item = dynamic_cast<ToolbarItem*>(selectedItem());
    if ( item ) {
      ToolbarItemDrag *obj = new ToolbarItemDrag(item,
                                 this, "ToolbarAction drag item");
      const QPixmap *pm = item->pixmap(0);
      if( pm )
        obj->setPixmap( *pm );
      return obj;
    }
    return 0;
  }

  virtual bool acceptDrag(QDropEvent *event) const
  {
    return ToolbarItemDrag::canDecode( event );
  }
};
} // namespace

class KEditToolbarWidgetPrivate
{
public:
    /**
     *
     * @param collection In a non-KParts application, this is the collection passed
     * to the KEditToolbar constructor.
     * In a KParts application we let create a KXMLGUIClient create a dummy one,
     * but it probably isn't used.
     */
  KEditToolbarWidgetPrivate(KInstance *instance, KActionCollection* collection)
      : m_collection( collection )
  {
    m_instance = instance;
    m_isPart   = false;
    m_helpArea = 0L;
    m_kdialogProcess = 0;
  }
  ~KEditToolbarWidgetPrivate()
  {
  }

  QString xmlFile(const QString& xml_file)
  {
    return xml_file.isNull() ? QString(m_instance->instanceName()) + "ui.rc" :
                               xml_file;
  }

  /**
   * Load in the specified XML file and dump the raw xml
   */
  QString loadXMLFile(const QString& _xml_file)
  {
    QString raw_xml;
    QString xml_file = xmlFile(_xml_file);
    //kdDebug() << "loadXMLFile xml_file=" << xml_file << endl;

    if ( !QDir::isRelativePath(xml_file) )
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file);
    else
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file, m_instance);

    return raw_xml;
  }

  /**
   * Return a list of toolbar elements given a toplevel element
   */
  ToolbarList findToolbars(QDomNode n)
  {
    static const QString &tagToolbar = KGlobal::staticQString( "ToolBar" );
    static const QString &attrNoEdit = KGlobal::staticQString( "noEdit" );
    ToolbarList list;

    for( ; !n.isNull(); n = n.nextSibling() )
    {
      QDomElement elem = n.toElement();
      if (elem.isNull())
        continue;

      if (elem.tagName() == tagToolbar && elem.attribute( attrNoEdit ) != "true" )
        list.append(elem);

      list += findToolbars(elem.firstChild());
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
      QByteArray txt( it.namedItem( tagText ).toElement().text().utf8() );
      if ( txt.isEmpty() )
          txt = it.namedItem( tagText2 ).toElement().text().utf8();
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
        name += " <" + doc_name + ">";
      }
      return name;
  }
  /**
   * Look for a given item in the current toolbar
   */
  QDomElement findElementForToolbarItem( const ToolbarItem* item ) const
  {
    static const QString &attrName    = KGlobal::staticQString( "name" );
    for(QDomNode n = m_currentToolbarElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
      QDomElement elem = n.toElement();
      if ((elem.attribute(attrName) == item->internalName()) &&
          (elem.tagName() == item->internalTag()))
        return elem;
    }
    return QDomElement();
  }

#ifndef NDEBUG
  void dump()
  {
    static const char* s_XmlTypeToString[] = { "Shell", "Part", "Local", "Merged" };
    XmlDataList::Iterator xit = m_xmlFiles.begin();
    for ( ; xit != m_xmlFiles.end(); ++xit )
    {
        kdDebug(240) << "XmlData type " << s_XmlTypeToString[(*xit).m_type] << " xmlFile: " << (*xit).m_xmlFile << endl;
        for( Q3ValueList<QDomElement>::Iterator it = (*xit).m_barList.begin();
             it != (*xit).m_barList.end(); ++it ) {
            kdDebug(240) << "    Toolbar: " << toolbarName( *xit, *it ) << endl;
        }
        if ( (*xit).m_actionCollection )
            kdDebug(240) << "    " << (*xit).m_actionCollection->count() << " actions in the collection." << endl;
        else
            kdDebug(240) << "    no action collection." << endl;
    }
  }
#endif

  //QValueList<KAction*> m_actionList;
  KActionCollection* m_collection;
  KInstance         *m_instance;

  XmlData*     m_currentXmlData;
  QDomElement m_currentToolbarElem;

  QString            m_xmlFile;
  QString            m_globalFile;
  QString            m_rcFile;
  QDomDocument       m_localDoc;
  bool               m_isPart;

  ToolbarList        m_barList;

  XmlDataList m_xmlFiles;

  QLabel     *m_comboLabel;
  KSeparator *m_comboSeparator;
  QLabel * m_helpArea;
  KPushButton* m_changeIcon;
  KProcIO* m_kdialogProcess;
  bool m_hasKDialog;
};

class KEditToolbarPrivate {
public:
    bool m_accept;

    // Save parameters for recreating widget after resetting toolbar
    bool m_global;
    KActionCollection* m_collection;
    QString m_file;
    KXMLGUIFactory* m_factory;
};

const char *KEditToolbar::s_defaultToolbar = 0L;

KEditToolbar::KEditToolbar(KActionCollection *collection, const QString& file,
                           bool global, QWidget* parent)
  : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent),
    m_widget(new KEditToolbarWidget(QLatin1String(s_defaultToolbar), collection, file, global, this))
{
    init();
    d->m_global = global;
    d->m_collection = collection;
    d->m_file = file;
}

KEditToolbar::KEditToolbar(const QString& defaultToolbar, KActionCollection *collection,
                           const QString& file, bool global,
                           QWidget* parent)
  : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent ),
    m_widget(new KEditToolbarWidget(defaultToolbar, collection, file, global, this))
{
    init();
    d->m_global = global;
    d->m_collection = collection;
    d->m_file = file;
}

KEditToolbar::KEditToolbar(KXMLGUIFactory* factory, QWidget* parent)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent),
      m_widget(new KEditToolbarWidget(QLatin1String(s_defaultToolbar), factory, this))
{
    init();
    d->m_factory = factory;
}

KEditToolbar::KEditToolbar(const QString& defaultToolbar,KXMLGUIFactory* factory,
                           QWidget* parent)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent),
      m_widget(new KEditToolbarWidget(defaultToolbar, factory, this))
{
    init();
    d->m_factory = factory;
}

void KEditToolbar::init()
{
    d = new KEditToolbarPrivate();
    d->m_accept = false;
    d->m_factory = 0;

    setMainWidget(m_widget);

    connect(m_widget, SIGNAL(enableOk(bool)), SLOT(acceptOK(bool)));
    connect(m_widget, SIGNAL(enableOk(bool)), SLOT(enableButtonApply(bool)));
    enableButtonApply(false);

    setMinimumSize(sizeHint());
    s_defaultToolbar = 0L;
}

KEditToolbar::~KEditToolbar()
{
    delete d;
}

void KEditToolbar::acceptOK(bool b)
{
    enableButtonOK(b);
    d->m_accept = b;
}

void KEditToolbar::slotDefault()
{
    if ( KMessageBox::warningContinueCancel(this, i18n("Do you really want to reset all toolbars of this application to their default? The changes will be applied immediately."), i18n("Reset Toolbars"),i18n("Reset"))!=KMessageBox::Continue )
        return;

    delete m_widget;
    d->m_accept = false;

    if ( d->m_factory )
    {
        const QString localPrefix = locateLocal("data", "");
        Q3PtrList<KXMLGUIClient> clients(d->m_factory->clients());
        Q3PtrListIterator<KXMLGUIClient> it( clients );

        for( ; it.current(); ++it)
        {
            KXMLGUIClient *client = it.current();
            QString file = client->xmlFile();

            if (file.isNull())
                continue;

            if (QDir::isRelativePath(file))
            {
                const KInstance *instance = client->instance() ? client->instance() : KGlobal::instance();
                file = locateLocal("data", QLatin1String( instance->instanceName() + '/' ) + file);
            }
            else
            {
                if (!file.startsWith(localPrefix))
                    continue;
            }

            if ( QFile::exists( file ) )
                if ( !QFile::remove( file ) )
                    kdWarning() << "Could not delete " << file << endl;
        }

        m_widget = new KEditToolbarWidget(QString::null, d->m_factory, this);
        m_widget->rebuildKXMLGUIClients();
    }
    else
    {
        int slash = d->m_file.findRev('/')+1;
        if (slash)
            d->m_file = d->m_file.mid(slash);
        QString xml_file = locateLocal("data", QLatin1String( KGlobal::instance()->instanceName() + '/' ) + d->m_file);

        if ( QFile::exists( xml_file ) )
            if ( !QFile::remove( xml_file ) )
                kdWarning() << "Could not delete " << xml_file << endl;

        m_widget = new KEditToolbarWidget(QString::null, d->m_collection, d->m_file, d->m_global, this);
    }

    setMainWidget(m_widget);
    m_widget->show();

    connect(m_widget, SIGNAL(enableOk(bool)), SLOT(acceptOK(bool)));
    connect(m_widget, SIGNAL(enableOk(bool)), SLOT(enableButtonApply(bool)));

    enableButtonApply(false);
    emit newToolbarConfig();
}

void KEditToolbar::slotOk()
{
  if (!d->m_accept) {
      reject();
      return;
  }

  if (!m_widget->save())
  {
    // some error box here is needed
  }
  else
  {
    emit newToolbarConfig();
    accept();
  }
}

void KEditToolbar::slotApply()
{
    (void)m_widget->save();
    enableButtonApply(false);
    emit newToolbarConfig();
}

void KEditToolbar::setDefaultToolbar(const char *toolbarName)
{
    s_defaultToolbar = toolbarName;
}

KEditToolbarWidget::KEditToolbarWidget(KActionCollection *collection,
                                       const QString& file,
                                       bool global, QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), collection))
{
  initNonKPart(collection, file, global);
  // now load in our toolbar combo box
  loadToolbarCombo();
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::KEditToolbarWidget(const QString& defaultToolbar,
                                       KActionCollection *collection,
                                       const QString& file, bool global,
                                       QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), collection))
{
  initNonKPart(collection, file, global);
  // now load in our toolbar combo box
  loadToolbarCombo(defaultToolbar);
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::KEditToolbarWidget( KXMLGUIFactory* factory,
                                        QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), KXMLGUIClient::actionCollection() /*create new one*/))
{
  initKPart(factory);
  // now load in our toolbar combo box
  loadToolbarCombo();
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::KEditToolbarWidget( const QString& defaultToolbar,
                                        KXMLGUIFactory* factory,
                                        QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), KXMLGUIClient::actionCollection() /*create new one*/))
{
  initKPart(factory);
  // now load in our toolbar combo box
  loadToolbarCombo(defaultToolbar);
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::~KEditToolbarWidget()
{
    delete d;
}

void KEditToolbarWidget::initNonKPart(KActionCollection *collection,
                                      const QString& file, bool global)
{
  //d->m_actionList = collection->actions();

  // handle the merging
  if (global)
    setXMLFile(locate("config", "ui/ui_standards.rc"));
  QString localXML = d->loadXMLFile(file);
  setXML(localXML, true);

  // reusable vars
  QDomElement elem;

  // first, get all of the necessary info for our local xml
  XmlData local;
  local.m_xmlFile = d->xmlFile(file);
  local.m_type    = XmlData::Local;
  local.m_document.setContent(localXML);
  elem = local.m_document.documentElement().toElement();
  local.m_barList = d->findToolbars(elem);
  local.m_actionCollection = collection;
  d->m_xmlFiles.append(local);

  // then, the merged one (ui_standards + local xml)
  XmlData merge;
  merge.m_xmlFile  = QString::null;
  merge.m_type     = XmlData::Merged;
  merge.m_document = domDocument();
  elem = merge.m_document.documentElement().toElement();
  merge.m_barList  = d->findToolbars(elem);
  merge.m_actionCollection = collection;
  d->m_xmlFiles.append(merge);

#ifndef NDEBUG
  //d->dump();
#endif

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();
}

void KEditToolbarWidget::initKPart(KXMLGUIFactory* factory)
{
  // reusable vars
  QDomElement elem;

  setFactory( factory );
  actionCollection()->setWidget( this );

  // add all of the client data
  Q3PtrList<KXMLGUIClient> clients(factory->clients());
  Q3PtrListIterator<KXMLGUIClient> it( clients );
  for( ; it.current(); ++it)
  {
    KXMLGUIClient *client = it.current();

    if (client->xmlFile().isNull())
      continue;

    XmlData data;
    data.m_xmlFile = client->localXMLFile();
    if ( it.atFirst() )
      data.m_type = XmlData::Shell;
    else
      data.m_type = XmlData::Part;
    data.m_document.setContent( KXMLGUIFactory::readConfigFile( client->xmlFile(), client->instance() ) );
    elem = data.m_document.documentElement().toElement();
    data.m_barList = d->findToolbars(elem);
    data.m_actionCollection = client->actionCollection();
    d->m_xmlFiles.append(data);

    //d->m_actionList += client->actionCollection()->actions();
  }

#ifndef NDEBUG
  //d->dump();
#endif

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();
}

bool KEditToolbarWidget::save()
{
  //kdDebug(240) << "KEditToolbarWidget::save" << endl;
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

    kdDebug(240) << "Saving " << (*it).m_xmlFile << endl;
    // if we got this far, we might as well just save it
    KXMLGUIFactory::saveConfigFile((*it).m_document, (*it).m_xmlFile);
  }

  if ( !factory() )
    return true;

  rebuildKXMLGUIClients();

  return true;
}

void KEditToolbarWidget::rebuildKXMLGUIClients()
{
  if ( !factory() )
    return;

  Q3PtrList<KXMLGUIClient> clients(factory()->clients());
  //kdDebug(240) << "factory: " << clients.count() << " clients" << endl;

  // remove the elements starting from the last going to the first
  KXMLGUIClient *client = clients.last();
  while ( client )
  {
    //kdDebug(240) << "factory->removeClient " << client << endl;
    factory()->removeClient( client );
    client = clients.prev();
  }

  KXMLGUIClient *firstClient = clients.first();

  // now, rebuild the gui from the first to the last
  //kdDebug(240) << "rebuilding the gui" << endl;
  Q3PtrListIterator<KXMLGUIClient> cit( clients );
  for( ; cit.current(); ++cit)
  {
    KXMLGUIClient* client = cit.current();
    //kdDebug(240) << "updating client " << client << " " << client->instance()->instanceName() << "  xmlFile=" << client->xmlFile() << endl;
    QString file( client->xmlFile() ); // before setting ui_standards!
    if ( !file.isEmpty() )
    {
        // passing an empty stream forces the clients to reread the XML
        client->setXMLGUIBuildDocument( QDomDocument() );

        // for the shell, merge in ui_standards.rc
        if ( client == firstClient ) // same assumption as in the ctor: first==shell
            client->setXMLFile(locate("config", "ui/ui_standards.rc"));

        // and this forces it to use the *new* XML file
        client->setXMLFile( file, client == firstClient /* merge if shell */ );
    }
  }

  // Now we can add the clients to the factory
  // We don't do it in the loop above because adding a part automatically
  // adds its plugins, so we must make sure the plugins were updated first.
  cit.toFirst();
  for( ; cit.current(); ++cit)
    factory()->addClient( cit.current() );
}

void KEditToolbarWidget::setupLayout()
{
  // the toolbar name combo
  d->m_comboLabel = new QLabel(i18n("&Toolbar:"), this);
  m_toolbarCombo = new QComboBox(this);
  m_toolbarCombo->setEnabled(false);
  d->m_comboLabel->setBuddy(m_toolbarCombo);
  d->m_comboSeparator = new KSeparator(this);
  connect(m_toolbarCombo, SIGNAL(activated(const QString&)),
          this,           SLOT(slotToolbarSelected(const QString&)));

//  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
//  new_toolbar->setPixmap(BarIcon("filenew", KIcon::SizeSmall));
//  new_toolbar->setEnabled(false); // disabled until implemented
//  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
//  del_toolbar->setPixmap(BarIcon("editdelete", KIcon::SizeSmall));
//  del_toolbar->setEnabled(false); // disabled until implemented

  // our list of inactive actions
  QLabel *inactive_label = new QLabel(i18n("A&vailable actions:"), this);
  m_inactiveList = new ToolbarListView(this);
  m_inactiveList->setDragEnabled(true);
  m_inactiveList->setAcceptDrops(true);
  m_inactiveList->setDropVisualizer(false);
  m_inactiveList->setAllColumnsShowFocus(true);
  m_inactiveList->setMinimumSize(180, 250);
  m_inactiveList->header()->hide();
  m_inactiveList->addColumn(""); // icon
  int column2 = m_inactiveList->addColumn(""); // text
  m_inactiveList->setSorting( column2 );
  inactive_label->setBuddy(m_inactiveList);
  connect(m_inactiveList, SIGNAL(selectionChanged(Q3ListViewItem *)),
          this,           SLOT(slotInactiveSelected(Q3ListViewItem *)));
  connect(m_inactiveList, SIGNAL( doubleClicked( Q3ListViewItem *, const QPoint &, int  )),
          this,           SLOT(slotInsertButton()));

  // our list of active actions
  QLabel *active_label = new QLabel(i18n("Curr&ent actions:"), this);
  m_activeList = new ToolbarListView(this);
  m_activeList->setDragEnabled(true);
  m_activeList->setAcceptDrops(true);
  m_activeList->setDropVisualizer(true);
  m_activeList->setAllColumnsShowFocus(true);
  m_activeList->setMinimumWidth(m_inactiveList->minimumWidth());
  m_activeList->header()->hide();
  m_activeList->addColumn(""); // icon
  m_activeList->addColumn(""); // text
  m_activeList->setSorting(-1);
  active_label->setBuddy(m_activeList);

  connect(m_inactiveList, SIGNAL(dropped(KListView*,QDropEvent*,Q3ListViewItem*)),
          this,              SLOT(slotDropped(KListView*,QDropEvent*,Q3ListViewItem*)));
  connect(m_activeList, SIGNAL(dropped(KListView*,QDropEvent*,Q3ListViewItem*)),
          this,            SLOT(slotDropped(KListView*,QDropEvent*,Q3ListViewItem*)));
  connect(m_activeList, SIGNAL(selectionChanged(Q3ListViewItem *)),
          this,         SLOT(slotActiveSelected(Q3ListViewItem *)));
  connect(m_activeList, SIGNAL( doubleClicked( Q3ListViewItem *, const QPoint &, int  )),
          this,           SLOT(slotRemoveButton()));

  // "change icon" button
  d->m_changeIcon = new KPushButton( i18n( "Change &Icon..." ), this );
  QString kdialogExe = KStandardDirs::findExe(QLatin1String("kdialog"));
  d->m_hasKDialog = !kdialogExe.isEmpty();
  d->m_changeIcon->setEnabled( d->m_hasKDialog );

  connect( d->m_changeIcon, SIGNAL( clicked() ),
           this, SLOT( slotChangeIcon() ) );

  // The buttons in the middle
  QIcon iconSet;

  m_upAction     = new QToolButton(this);
  iconSet = SmallIconSet( "up" );
  m_upAction->setIcon( iconSet );
  m_upAction->setEnabled(false);
  m_upAction->setAutoRepeat(true);
  connect(m_upAction, SIGNAL(clicked()), SLOT(slotUpButton()));

  m_insertAction = new QToolButton(this);
  iconSet = QApplication::reverseLayout() ? SmallIconSet( "back" ) : SmallIconSet( "forward" );
  m_insertAction->setIcon( iconSet );
  m_insertAction->setEnabled(false);
  connect(m_insertAction, SIGNAL(clicked()), SLOT(slotInsertButton()));

  m_removeAction = new QToolButton(this);
  iconSet = QApplication::reverseLayout() ? SmallIconSet( "forward" ) : SmallIconSet( "back" );
  m_removeAction->setIcon( iconSet );
  m_removeAction->setEnabled(false);
  connect(m_removeAction, SIGNAL(clicked()), SLOT(slotRemoveButton()));

  m_downAction   = new QToolButton(this);
  iconSet = SmallIconSet( "down" );
  m_downAction->setIcon( iconSet );
  m_downAction->setEnabled(false);
  m_downAction->setAutoRepeat(true);
  connect(m_downAction, SIGNAL(clicked()), SLOT(slotDownButton()));

  d->m_helpArea = new QLabel(this);
  d->m_helpArea->setAlignment( Qt::TextWordWrap );

  // now start with our layouts
  QVBoxLayout *top_layout = new QVBoxLayout(this, 0, KDialog::spacingHint());

  QVBoxLayout *name_layout = new QVBoxLayout(KDialog::spacingHint());
  QHBoxLayout *list_layout = new QHBoxLayout(KDialog::spacingHint());

  QVBoxLayout *inactive_layout = new QVBoxLayout(KDialog::spacingHint());
  QVBoxLayout *active_layout = new QVBoxLayout(KDialog::spacingHint());
  QHBoxLayout *changeIcon_layout = new QHBoxLayout(KDialog::spacingHint());

  QGridLayout *button_layout = new QGridLayout(5, 3, 0);

  name_layout->addWidget(d->m_comboLabel);
  name_layout->addWidget(m_toolbarCombo);
//  name_layout->addWidget(new_toolbar);
//  name_layout->addWidget(del_toolbar);

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
  changeIcon_layout->addWidget( d->m_changeIcon );
  changeIcon_layout->addStretch( 1 );

  list_layout->addLayout(inactive_layout);
  list_layout->addLayout(button_layout);
  list_layout->addLayout(active_layout);

  top_layout->addLayout(name_layout);
  top_layout->addWidget(d->m_comboSeparator);
  top_layout->addLayout(list_layout,10);
  top_layout->addWidget(d->m_helpArea);
  top_layout->addWidget(new KSeparator(this));
}

void KEditToolbarWidget::loadToolbarCombo(const QString& defaultToolbar)
{
  static const QString &attrName = KGlobal::staticQString( "name" );
  // just in case, we clear our combo
  m_toolbarCombo->clear();

  int defaultToolbarId = -1;
  int count = 0;
  // load in all of the toolbar names into this combo box
  XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    // skip the local one in favor of the merged
    if ( (*xit).m_type == XmlData::Local )
      continue;

    // each xml file may have any number of toolbars
    ToolbarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      QString name = d->toolbarName( *xit, *it );
      m_toolbarCombo->setEnabled( true );
      m_toolbarCombo->insertItem( name );
      if (defaultToolbarId == -1 && (name == defaultToolbar || defaultToolbar == (*it).attribute( attrName )))
          defaultToolbarId = count;
      count++;
    }
  }
  bool showCombo = (count > 1);
  d->m_comboLabel->setShown(showCombo);
  d->m_comboSeparator->setShown(showCombo);
  m_toolbarCombo->setShown(showCombo);
  if (defaultToolbarId == -1)
      defaultToolbarId = 0;
  // we want to the specified item selected and its actions loaded
  m_toolbarCombo->setCurrentItem(defaultToolbarId);
  slotToolbarSelected(m_toolbarCombo->currentText());
}

void KEditToolbarWidget::loadActionList(QDomElement& elem)
{
  static const QString &tagSeparator = KGlobal::staticQString( "Separator" );
  static const QString &tagMerge     = KGlobal::staticQString( "Merge" );
  static const QString &tagActionList= KGlobal::staticQString( "ActionList" );
  static const QString &attrName     = KGlobal::staticQString( "name" );
  static const QString &attrLineSeparator = KGlobal::staticQString( "lineSeparator" );

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
  KActionCollection* actionCollection = d->m_currentXmlData->m_actionCollection;

  // store the names of our active actions
  QMap<QString, bool> active_list;

  // see if our current action is in this toolbar
  QDomNode n = elem.lastChild();
  for( ; !n.isNull(); n = n.previousSibling() )
  {
    QDomElement it = n.toElement();
    if (it.isNull()) continue;
    if (it.tagName() == tagSeparator)
    {
      ToolbarItem *act = new ToolbarItem(m_activeList, tagSeparator, sep_name.arg(sep_num++), QString::null);
      bool isLineSep = ( it.attribute(attrLineSeparator, "true").toLower() == QLatin1String("true") );
      if(isLineSep)
        act->setText(1, LINESEPARATORSTRING);
      else
        act->setText(1, SEPARATORSTRING);
      it.setAttribute( attrName, act->internalName() );
      continue;
    }

    if (it.tagName() == tagMerge)
    {
      // Merge can be named or not - use the name if there is one
      QString name = it.attribute( attrName );
      ToolbarItem *act = new ToolbarItem(m_activeList, tagMerge, name, i18n("This element will be replaced with all the elements of an embedded component."));
      if ( name.isEmpty() )
          act->setText(1, i18n("<Merge>"));
      else
          act->setText(1, i18n("<Merge %1>").arg(name));
      continue;
    }

    if (it.tagName() == tagActionList)
    {
      ToolbarItem *act = new ToolbarItem(m_activeList, tagActionList, it.attribute(attrName), i18n("This is a dynamic list of actions. You can move it, but if you remove it you won't be able to re-add it.") );
      act->setText(1, i18n("ActionList: %1").arg(it.attribute(attrName)));
      continue;
    }

    // iterate through this client's actions
    // This used to iterate through _all_ actions, but we don't support
    // putting any action into any client...
    for (unsigned int i = 0;  i < actionCollection->count(); i++)
    {
      KAction *action = actionCollection->action( i );

      // do we have a match?
      if (it.attribute( attrName ) == action->objectName())
      {
        // we have a match!
        ToolbarItem *act = new ToolbarItem(m_activeList, it.tagName(), action->objectName(), action->toolTip());
        act->setText(1, action->plainText());
        if (action->hasIcon())
          if (!action->icon().isEmpty())
            act->setPixmap(0, BarIcon(action->icon(), 16));
          else // Has iconset
            act->setPixmap(0, action->iconSet(KIcon::Toolbar).pixmap());

        active_list.insert(action->objectName(), true);
        break;
      }
    }
  }

  // go through the rest of the collection
  for (int i = actionCollection->count() - 1; i > -1; --i)
  {
    KAction *action = actionCollection->action( i );

    // skip our active ones
    if (active_list.contains(action->objectName()))
      continue;

    ToolbarItem *act = new ToolbarItem(m_inactiveList, tagActionList, action->objectName(), action->toolTip());
    act->setText(1, action->plainText());
    if (action->hasIcon())
      if (!action->icon().isEmpty())
        act->setPixmap(0, BarIcon(action->icon(), 16));
      else // Has iconset
        act->setPixmap(0, action->iconSet(KIcon::Toolbar).pixmap());
  }

  // finally, add default separators to the inactive list
  ToolbarItem *act = new ToolbarItem(m_inactiveList, tagSeparator, sep_name.arg(sep_num++), QString::null);
  act->setText(1, LINESEPARATORSTRING);
  act = new ToolbarItem(m_inactiveList, tagSeparator, sep_name.arg(sep_num++), QString::null);
  act->setText(1, SEPARATORSTRING);
}

KActionCollection *KEditToolbarWidget::actionCollection() const
{
  return d->m_collection;
}

void KEditToolbarWidget::slotToolbarSelected(const QString& _text)
{
  // iterate through everything
  XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    // each xml file may have any number of toolbars
    ToolbarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      QString name = d->toolbarName( *xit, *it );
      // is this our toolbar?
      if ( name == _text )
      {
        // save our current settings
        d->m_currentXmlData     = & (*xit);
        d->m_currentToolbarElem = (*it);

        // load in our values
        loadActionList(d->m_currentToolbarElem);

        if ((*xit).m_type == XmlData::Part || (*xit).m_type == XmlData::Shell)
          setDOMDocument( (*xit).m_document );
        return;
      }
    }
  }
}

void KEditToolbarWidget::slotInactiveSelected(Q3ListViewItem *item)
{
  ToolbarItem* toolitem = static_cast<ToolbarItem *>(item);
  if (item)
  {
    m_insertAction->setEnabled(true);
    QString statusText = toolitem->statusText();
    d->m_helpArea->setText( statusText );
  }
  else
  {
    m_insertAction->setEnabled(false);
    d->m_helpArea->setText( QString::null );
  }
}

void KEditToolbarWidget::slotActiveSelected(Q3ListViewItem *item)
{
  ToolbarItem* toolitem = static_cast<ToolbarItem *>(item);
  m_removeAction->setEnabled( item );

  static const QString &tagAction = KGlobal::staticQString( "Action" );
  d->m_changeIcon->setEnabled( item &&
                               d->m_hasKDialog &&
                               toolitem->internalTag() == tagAction );

  if (item)
  {
    if (item->itemAbove())
      m_upAction->setEnabled(true);
    else
      m_upAction->setEnabled(false);

    if (item->itemBelow())
      m_downAction->setEnabled(true);
    else
      m_downAction->setEnabled(false);
    QString statusText = toolitem->statusText();
    d->m_helpArea->setText( statusText );
  }
  else
  {
    m_upAction->setEnabled(false);
    m_downAction->setEnabled(false);
    d->m_helpArea->setText( QString::null );
  }
}

void KEditToolbarWidget::slotDropped(KListView *list, QDropEvent *e, Q3ListViewItem *after)
{
  ToolbarItem *item = new ToolbarItem(m_inactiveList); // needs parent, use inactiveList temporarily
  if(!ToolbarItemDrag::decode(e, *item)) {
    delete item;
    return;
  }

  if (list == m_activeList) {
    if (e->source() == m_activeList) {
      // has been dragged within the active list (moved).
      moveActive(item, after);
    }
    else
      insertActive(item, after, true);
  } else if (list == m_inactiveList) {
    // has been dragged to the inactive list -> remove from the active list.
    removeActive(item);
  }

  delete item; item = 0; // not neded anymore

  // we're modified, so let this change
  emit enableOk(true);

  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotInsertButton()
{
  ToolbarItem *item = (ToolbarItem*)m_inactiveList->currentItem();
  insertActive(item, m_activeList->currentItem(), false);

  // we're modified, so let this change
  emit enableOk(true);

  // TODO: #### this causes #97572.
  // It would be better to just "delete item; loadActions( ... , ActiveListOnly );" or something.
  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotRemoveButton()
{
  removeActive( dynamic_cast<ToolbarItem*>(m_activeList->currentItem()) );

  // we're modified, so let this change
  emit enableOk(true);

  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::insertActive(ToolbarItem *item, Q3ListViewItem *before, bool prepend)
{
  if (!item)
    return;

  static const QString &tagAction    = KGlobal::staticQString( "Action" );
  static const QString &tagSeparator = KGlobal::staticQString( "Separator" );
  static const QString &attrName     = KGlobal::staticQString( "name" );
  static const QString &attrLineSeparator = KGlobal::staticQString( "lineSeparator" );
  static const QString &attrNoMerge  = KGlobal::staticQString( "noMerge" );

  QDomElement new_item;
  // let's handle the separator specially
  if (item->text(1) == LINESEPARATORSTRING) {
    new_item = domDocument().createElement(tagSeparator);
  } else if (item->text(1) == SEPARATORSTRING) {
    new_item = domDocument().createElement(tagSeparator);
    new_item.setAttribute(attrLineSeparator, "false");
  } else
    new_item = domDocument().createElement(tagAction);
  new_item.setAttribute(attrName, item->internalName());

  if (before)
  {
    // we have the item in the active list which is before the new
    // item.. so let's try our best to add our new item right after it
    ToolbarItem *act_item = (ToolbarItem*)before;
    QDomElement elem = d->findElementForToolbarItem( act_item );
    Q_ASSERT( !elem.isNull() );
    d->m_currentToolbarElem.insertAfter(new_item, elem);
  }
  else
  {
    // simply put it at the beginning or the end of the list.
    if (prepend)
      d->m_currentToolbarElem.insertBefore(new_item, d->m_currentToolbarElem.firstChild());
    else
      d->m_currentToolbarElem.appendChild(new_item);
  }

  // and set this container as a noMerge
  d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

  // update the local doc
  updateLocal(d->m_currentToolbarElem);
}

void KEditToolbarWidget::removeActive(ToolbarItem *item)
{
  if (!item)
    return;

  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );

  // we're modified, so let this change
  emit enableOk(true);

  // now iterate through to find the child to nuke
  QDomElement elem = d->findElementForToolbarItem( item );
  if ( !elem.isNull() )
  {
    // nuke myself!
    d->m_currentToolbarElem.removeChild(elem);

    // and set this container as a noMerge
    d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

    // update the local doc
    updateLocal(d->m_currentToolbarElem);
  }
}

void KEditToolbarWidget::slotUpButton()
{
  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

  // make sure we're not the top item already
  if (!item->itemAbove())
    return;

  // we're modified, so let this change
  emit enableOk(true);

  moveActive( item, item->itemAbove()->itemAbove() );
  delete item;
}

void KEditToolbarWidget::moveActive( ToolbarItem* item, Q3ListViewItem* before )
{
  QDomElement e = d->findElementForToolbarItem( item );

  if ( e.isNull() )
    return;

  // cool, i found me.  now clone myself
  ToolbarItem *clone = new ToolbarItem(m_activeList,
                                       before,
                                       item->internalTag(),
                                       item->internalName(),
                                       item->statusText());

  clone->setText(1, item->text(1));

  // only set new pixmap if exists
  if( item->pixmap(0) )
    clone->setPixmap(0, *item->pixmap(0));

  // select my clone
  m_activeList->setSelected(clone, true);

  // make clone visible
  m_activeList->ensureItemVisible(clone);

  // and do the real move in the DOM
  if ( !before )
    d->m_currentToolbarElem.insertBefore(e, d->m_currentToolbarElem.firstChild() );
  else
    d->m_currentToolbarElem.insertAfter(e, d->findElementForToolbarItem( (ToolbarItem*)before ));

  // and set this container as a noMerge
  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );
  d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

  // update the local doc
  updateLocal(d->m_currentToolbarElem);
}

void KEditToolbarWidget::slotDownButton()
{
  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

  // make sure we're not the bottom item already
  if (!item->itemBelow())
    return;

  // we're modified, so let this change
  emit enableOk(true);

  moveActive( item, item->itemBelow() );
  delete item;
}

void KEditToolbarWidget::updateLocal(QDomElement& elem)
{
  static const QString &attrName = KGlobal::staticQString( "name" );

  XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    if ( (*xit).m_type == XmlData::Merged )
      continue;

    if ( (*xit).m_type == XmlData::Shell ||
         (*xit).m_type == XmlData::Part )
    {
      if ( d->m_currentXmlData->m_xmlFile == (*xit).m_xmlFile )
      {
        (*xit).m_isModified = true;
        return;
      }

      continue;
    }

    (*xit).m_isModified = true;

    ToolbarList::Iterator it = (*xit).m_barList.begin();
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

void KEditToolbarWidget::slotChangeIcon()
{
  // We can't use KIconChooser here, since it's in libkio
  // ##### KDE4: reconsider this, e.g. move KEditToolbar to libkio
  d->m_kdialogProcess = new KProcIO;
  QString kdialogExe = KStandardDirs::findExe(QLatin1String("kdialog"));
  (*d->m_kdialogProcess) << kdialogExe;
  (*d->m_kdialogProcess) << "--embed";
  (*d->m_kdialogProcess) << QString::number( (ulong)topLevelWidget()->winId() );
  (*d->m_kdialogProcess) << "--geticon";
  (*d->m_kdialogProcess) << "Toolbar";
  (*d->m_kdialogProcess) << "Actions";
  if ( !d->m_kdialogProcess->start( KProcess::NotifyOnExit ) ) {
    kdError(240) << "Can't run " << kdialogExe << endl;
    delete d->m_kdialogProcess;
    d->m_kdialogProcess = 0;
    return;
  }

  m_activeList->setEnabled( false ); // don't change the current item
  m_toolbarCombo->setEnabled( false ); // don't change the current toolbar

  connect( d->m_kdialogProcess, SIGNAL( processExited( KProcess* ) ),
           this, SLOT( slotProcessExited( KProcess* ) ) );
}

void KEditToolbarWidget::slotProcessExited( KProcess* )
{
  m_activeList->setEnabled( true );
  m_toolbarCombo->setEnabled( true );

  QString icon;
  if ( !d->m_kdialogProcess->normalExit() ||
       d->m_kdialogProcess->exitStatus() ||
       d->m_kdialogProcess->readln(icon, true) <= 0 ) {
    delete d->m_kdialogProcess;
    d->m_kdialogProcess = 0;
    return;
  }

  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();
  if(item){
    item->setPixmap(0, BarIcon(icon, 16));

    Q_ASSERT( d->m_currentXmlData->m_type != XmlData::Merged );

    d->m_currentXmlData->m_isModified = true;

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement( d->m_currentXmlData->m_document );
    // Find or create an element for this action
    QDomElement act_elem = KXMLGUIFactory::findActionByName( elem, item->internalName(), true /*create*/ );
    Q_ASSERT( !act_elem.isNull() );
    act_elem.setAttribute( "icon", icon );

    // we're modified, so let this change
    emit enableOk(true);
  }

  delete d->m_kdialogProcess;
  d->m_kdialogProcess = 0;
}

void KEditToolbar::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KEditToolbarWidget::virtual_hook( int id, void* data )
{ KXMLGUIClient::virtual_hook( id, data ); }

#include "kedittoolbar.moc"
