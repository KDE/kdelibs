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
#include <kaction.h>

#include <qheader.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qvaluelist.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kxmlguifactory.h>
#include <kseparator.h>
#include <kconfig.h>
#include <klistview.h>

#include <qtextstream.h>
#include <qfile.h>
#include <kdebug.h>
#include <kdebugclasses.h>

static void dump_xml(const QDomDocument& doc)
{
    QString str;
    QTextStream ts(&str, IO_WriteOnly);
    ts << doc;
    kdDebug() << str << endl;
}

typedef QValueList<QDomElement> ToolbarList;

class XmlData
{
public:
  enum XmlType { Shell = 0, Part, Local, Merged };
  XmlData()
  {
    m_isModified = false;
  }

  QString      m_xmlFile;
  QDomDocument m_document;
  XmlType      m_type;
  bool         m_isModified;

  ToolbarList  m_barList;
};

typedef QValueList<XmlData> XmlDataList;

class ToolbarItem : public QListViewItem
{
public:
  ToolbarItem(KListView *parent, const QString& tag, const QString& name, const QString& statusText)
    : QListViewItem(parent),
      m_tag(tag),
      m_name(name),
      m_statusText(statusText)
  {
  }

  ToolbarItem(KListView *parent, QListViewItem *item, const QString &tag, const QString& name, const QString& statusText)
    : QListViewItem(parent, item),
      m_tag(tag),
      m_name(name),
      m_statusText(statusText)
  {
  }

  QString internalTag() const { return m_tag; }
  QString internalName() const { return m_name; }
  QString statusText() const { return m_statusText; }
private:
  QString m_tag;
  QString m_name;
  QString m_statusText;
};

class KEditToolbarWidgetPrivate
{
public:
  KEditToolbarWidgetPrivate(KInstance *instance)
  //: m_collection( parent, "KEditToolbarWidgetPrivate::m_collection" )
  {
    m_instance = instance;
    m_isPart   = false;
    m_helpArea = 0L;
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

    if ( xml_file[0] == '/' )
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file);
    else
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file, m_instance);

    return raw_xml;
  }

  /**
   * Return a list of toolbar elements given a toplevel element
   */
  ToolbarList findToolbars(QDomElement elem)
  {
    static const QString &tagToolbar = KGlobal::staticQString( "ToolBar" );
    static const QString &attrNoEdit = KGlobal::staticQString( "noEdit" );
    ToolbarList list;

    for( ; !elem.isNull(); elem = elem.nextSibling().toElement() )
    {
      if (elem.tagName() == tagToolbar && elem.attribute( attrNoEdit ) != "true" )
        list.append(elem);

      QDomElement child = elem.firstChild().toElement();
      list += findToolbars(child);
    }

    return list;
  }

  QValueList<KAction*> m_actionList;
  //KActionCollection m_collection;
  KInstance         *m_instance;

  XmlData     m_currentXmlData;
  QDomElement m_currentToolbarElem;

  QString            m_xmlFile;
  QString            m_globalFile;
  QString            m_rcFile;
  QDomDocument       m_localDoc;
  bool               m_isPart;

  ToolbarList        m_barList;

  XmlDataList m_xmlFiles;

  QLabel * m_helpArea;

};

class KEditToolbarPrivate {
public:
    bool m_accept;
};

KEditToolbar::KEditToolbar(KActionCollection *collection, const QString& file,
                           bool global, QWidget* parent, const char* name)
  : KDialogBase(Swallow, i18n("Configure Toolbars"), Ok|Apply|Cancel, Ok, parent, name),
    m_widget(new KEditToolbarWidget(collection, file, global, this))
{
    init();
}

KEditToolbar::KEditToolbar(KXMLGUIFactory* factory, QWidget* parent, const char* name)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Ok|Apply|Cancel, Ok, parent, name),
      m_widget(new KEditToolbarWidget(factory, this))
{
    init();
}

void KEditToolbar::init()
{
    d = new KEditToolbarPrivate();
    d->m_accept = false;

    setMainWidget(m_widget);

    connect(m_widget, SIGNAL(enableOk(bool)), SLOT(acceptOK(bool)));
    connect(m_widget, SIGNAL(enableOk(bool)), SLOT(enableButtonApply(bool)));
    enableButtonApply(false);

    setMinimumSize(sizeHint());
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

KEditToolbarWidget::KEditToolbarWidget(KActionCollection *collection,
                                       const QString& file,
                                       bool global, QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance()))
{
  actionCollection()->setWidget( this );
  // let's not forget the stuff that's not xml specific
  //d->m_collection = *collection;
  d->m_actionList = collection->actions();

  // handle the merging
  if (global)
    setXMLFile(locate("config", "ui/ui_standards.rc"));
  setXML(d->loadXMLFile(file), true);

  // reusable vars
  QDomElement elem;

  // first, get all of the necessary info for our local xml
  XmlData local;
  local.m_xmlFile = d->xmlFile(file);
  local.m_type    = XmlData::Local;
  local.m_document.setContent(d->loadXMLFile(file));
  elem = local.m_document.documentElement().toElement();
  KXMLGUIFactory::removeDOMComments( elem );
  local.m_barList = d->findToolbars(elem);
  d->m_xmlFiles.append(local);

  // then, the merged one
  XmlData merge;
  merge.m_xmlFile  = QString::null;
  merge.m_type     = XmlData::Merged;
  merge.m_document = domDocument();
  elem = merge.m_document.documentElement().toElement();
  merge.m_barList  = d->findToolbars(elem);
  d->m_xmlFiles.append(merge);

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();

  // now load in our toolbar combo box
  loadToolbarCombo();
  adjustSize();

  kdDebug() << "kedittoolbarwidget " << sizeHint() << endl;
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::KEditToolbarWidget( KXMLGUIFactory* factory,
                                        QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance()))
{
  // reusable vars
  QDomElement elem;

  setFactory( factory );
  actionCollection()->setWidget( this );

  // add all of the client data
  QPtrList<KXMLGUIClient> clients(factory->clients());
  QPtrListIterator<KXMLGUIClient> it( clients );
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
    KXMLGUIFactory::removeDOMComments( elem );
    data.m_barList = d->findToolbars(elem);
    d->m_xmlFiles.append(data);

    d->m_actionList += client->actionCollection()->actions();
  }

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();
  setMinimumSize(sizeHint());

  // now load in our toolbar combo box
  loadToolbarCombo();
}

KEditToolbarWidget::~KEditToolbarWidget()
{
    delete d;
}

bool KEditToolbarWidget::save()
{
  //kdDebug() << "KEditToolbarWidget::save" << endl;
  XmlDataList::Iterator it = d->m_xmlFiles.begin();
  for ( ; it != d->m_xmlFiles.end(); ++it)
  {
    // let's not save non-modified files
    if ( (*it).m_isModified == false )
      continue;

    // let's also skip (non-existent) merged files
    if ( (*it).m_type == XmlData::Merged )
      continue;

    dump_xml((*it).m_document.toDocument());

    // if we got this far, we might as well just save it
    KXMLGUIFactory::saveConfigFile((*it).m_document, (*it).m_xmlFile);
  }

  if ( !factory() )
    return true;

  QPtrList<KXMLGUIClient> clients(factory()->clients());

  // remove the elements starting from the last going to the first
  KXMLGUIClient *client = clients.last();
  while ( client )
  {
    //kdDebug() << "factory->removeClient " << client << endl;
    factory()->removeClient( client );
    client = clients.prev();
  }

  client = clients.first();
  // now, rebuild the gui from the first to the last
  //kdDebug() << "rebuildling the gui" << endl;
  for (; client; client = clients.next() )
  {
    // passing an empty stream forces the clients to reread the XML
    client->setXMLGUIBuildDocument( QDomDocument() );

    // and this forces it to use the *new* XML file
    client->reloadXML();

    //kdDebug() << "factory->addClient " << client << endl;
    // finally, do all the real work
    factory()->addClient( client );
  }

  return true;
}

void KEditToolbarWidget::setupLayout()
{
  // the toolbar name combo
  QLabel *toolbar_label = new QLabel(i18n("&Toolbar:"), this);
  m_toolbarCombo = new QComboBox(this);
  m_toolbarCombo->setEnabled(false);
  toolbar_label->setBuddy(m_toolbarCombo);
  connect(m_toolbarCombo, SIGNAL(activated(const QString&)),
          this,           SLOT(slotToolbarSelected(const QString&)));

//  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
//  new_toolbar->setPixmap(BarIcon("filenew", KIcon::SizeSmall));
//  new_toolbar->setEnabled(false); // disabled until implemented
//  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
//  del_toolbar->setPixmap(BarIcon("editdelete", KIcon::SizeSmall));
//  del_toolbar->setEnabled(false); // disabled until implemented

  // our list of inactive actions
  QLabel *inactive_label = new QLabel(i18n("&Available actions:"), this);
  m_inactiveList = new KListView(this);
  m_inactiveList->setAllColumnsShowFocus(true);
  m_inactiveList->setMinimumSize(180, 250);
  m_inactiveList->header()->hide();
  m_inactiveList->addColumn("");
  int column2 = m_inactiveList->addColumn("");
  m_inactiveList->setSorting( column2 );
  inactive_label->setBuddy(m_inactiveList);
  connect(m_inactiveList, SIGNAL(selectionChanged(QListViewItem *)),
          this,           SLOT(slotInactiveSelected(QListViewItem *)));

  // our list of active actions
  QLabel *active_label = new QLabel(i18n("Curr&ent actions:"), this);
  m_activeList = new KListView(this);
  m_activeList->setAllColumnsShowFocus(true);
  m_activeList->setMinimumWidth(m_inactiveList->minimumWidth());
  m_activeList->header()->hide();
  m_activeList->addColumn("");
  m_activeList->addColumn("");
  m_activeList->setSorting (-1);
  active_label->setBuddy(m_activeList);

  connect(m_activeList, SIGNAL(selectionChanged(QListViewItem *)),
          this,         SLOT(slotActiveSelected(QListViewItem *)));

  m_upAction     = new QPushButton(QString::null, this);
  m_upAction->setPixmap(BarIcon("up", KIcon::SizeSmall));
  m_upAction->setEnabled(false);
  connect(m_upAction, SIGNAL(clicked()),
          this,       SLOT(slotUpButton()));

  m_insertAction = new QPushButton(QString::null, this);
  m_insertAction->setPixmap(BarIcon("forward", KIcon::SizeSmall));
  m_insertAction->setEnabled(false);
  connect(m_insertAction, SIGNAL(clicked()),
          this,           SLOT(slotInsertButton()));

  m_removeAction = new QPushButton(QString::null, this);
  m_removeAction->setPixmap(BarIcon("back", KIcon::SizeSmall));
  m_removeAction->setEnabled(false);
  connect(m_removeAction, SIGNAL(clicked()),
          this,           SLOT(slotRemoveButton()));

  m_downAction   = new QPushButton(QString::null, this);
  m_downAction->setPixmap(BarIcon("down", KIcon::SizeSmall));
  m_downAction->setEnabled(false);
  connect(m_downAction, SIGNAL(clicked()),
          this,         SLOT(slotDownButton()));

  d->m_helpArea = new QLabel(this);
  d->m_helpArea->setAlignment( Qt::WordBreak );

  // now start with our layouts
  QVBoxLayout *top_layout = new QVBoxLayout(this, 0, KDialog::spacingHint());

  QVBoxLayout *name_layout = new QVBoxLayout(KDialog::spacingHint());
  QHBoxLayout *list_layout = new QHBoxLayout(KDialog::spacingHint());

  QVBoxLayout *inactive_layout = new QVBoxLayout(KDialog::spacingHint());
  QVBoxLayout *active_layout = new QVBoxLayout(KDialog::spacingHint());

  QGridLayout *button_layout = new QGridLayout(5, 3, 0);

  name_layout->addWidget(toolbar_label);
  name_layout->addWidget(m_toolbarCombo);
//  name_layout->addWidget(new_toolbar);
//  name_layout->addWidget(del_toolbar);

  button_layout->addWidget(m_upAction, 1, 1);
  button_layout->addWidget(m_removeAction, 2, 0);
  button_layout->addWidget(m_insertAction, 2, 2);
  button_layout->addWidget(m_downAction, 3, 1);

  inactive_layout->addWidget(inactive_label);
  inactive_layout->addWidget(m_inactiveList, 1);

  active_layout->addWidget(active_label);
  active_layout->addWidget(m_activeList, 1);

  list_layout->addLayout(inactive_layout);
  list_layout->addLayout(button_layout);
  list_layout->addLayout(active_layout);

  top_layout->addLayout(name_layout);
  top_layout->addWidget(new KSeparator(this));
  top_layout->addLayout(list_layout,10);
  top_layout->addWidget(d->m_helpArea);
  top_layout->addWidget(new KSeparator(this));
}

void KEditToolbarWidget::loadToolbarCombo()
{
  static const QString &attrName = KGlobal::staticQString( "name" );
  static const QString &tagText = KGlobal::staticQString( "text" );
  static const QString &tagText2 = KGlobal::staticQString( "Text" );

  // just in case, we clear our combo
  m_toolbarCombo->clear();

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
      QString name;
      QCString txt( (*it).namedItem( tagText ).toElement().text().utf8() );
      if ( txt.isEmpty() )
          txt = (*it).namedItem( tagText2 ).toElement().text().utf8();
      if ( txt.isEmpty() )
          name = (*it).attribute( attrName );
      else
          name = i18n( txt );

      // the name of the toolbar might depend on whether or not
      // it is in kparts
      if ( ( (*xit).m_type == XmlData::Shell ) ||
           ( (*xit).m_type == XmlData::Part ) )
      {
        QString doc_name((*xit).m_document.documentElement().attribute( attrName ));
        name += " <" + doc_name + ">";
      }

      m_toolbarCombo->setEnabled( true );
      m_toolbarCombo->insertItem( name );
    }
  }

  // we want to the first item selected and its actions loaded
  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::loadActionList(QDomElement& elem)
{
  static const QString &tagSeparator = KGlobal::staticQString( "Separator" );
  static const QString &tagMerge     = KGlobal::staticQString( "Merge" );
  static const QString &tagActionList= KGlobal::staticQString( "ActionList" );
  static const QString &attrName     = KGlobal::staticQString( "name" );

  int     sep_num = 0;
  QString sep_name("separator_%1");

  // clear our lists
  m_inactiveList->clear();
  m_activeList->clear();
  m_insertAction->setEnabled(false);
  m_removeAction->setEnabled(false);
  m_upAction->setEnabled(false);
  m_downAction->setEnabled(false);

  // store the names of our active actions
  QMap<QString, bool> active_list;

  // see if our current action is in this toolbar
  QDomElement it = elem.lastChild().toElement();
  for( ; !it.isNull(); it = it.previousSibling().toElement() )
  {
    if (it.tagName() == tagSeparator)
    {
      ToolbarItem *act = new ToolbarItem(m_activeList, tagSeparator, sep_name.arg(sep_num++), QString::null);
      act->setText(1, "-----");
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

    // iterate through all of our actions
    for (unsigned int i = 0;  i < d->m_actionList.count(); i++)
    {
      KAction *action = d->m_actionList[i];

      // do we have a match?
      if (it.attribute( attrName ) == action->name())
      {
        // we have a match!
        ToolbarItem *act = new ToolbarItem(m_activeList, it.tagName(), action->name(), action->statusText());
        act->setText(1, action->plainText());
        if (action->hasIcon())
          act->setPixmap(0, BarIcon(action->icon(), 16));

        active_list.insert(action->name(), true);
        break;
      }
    }
  }

  // go through the rest of the collection
  for (int i = d->m_actionList.count() - 1; i > -1; --i)
  {
    KAction *action = d->m_actionList[i];

    // skip our active ones
    if (active_list.contains(action->name()))
      continue;

    // insert this into the inactive list
    // for now, only deal with buttons with icons.. later, we'll need
    // to look into actions a LOT more carefully
    // Hmm, we also accept non-basic-KActions that have no icon
    // (e.g. konqueror's location bar)
    if ( action->icon().isEmpty() && action->isA("KAction") )
      continue;

    ToolbarItem *act = new ToolbarItem(m_inactiveList, tagActionList, action->name(), action->statusText());
    act->setText(1, action->plainText());
    if (!action->icon().isEmpty())
        act->setPixmap(0, BarIcon(action->icon(), 16));
  }

  // finally, add a default separator to the inactive list
  ToolbarItem *act = new ToolbarItem(m_inactiveList, tagSeparator, sep_name.arg(sep_num++), QString::null);
  act->setText(1, "-----");
}

/*KActionCollection *KEditToolbarWidget::actionCollection() const
{
  return &d->m_collection;
}*/

void KEditToolbarWidget::slotToolbarSelected(const QString& _text)
{
  static const QString &attrName = KGlobal::staticQString( "name" );
  static const QString &tagText = KGlobal::staticQString( "text" );
  static const QString &tagText2 = KGlobal::staticQString( "Text" );

  // iterate through everything
  XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    // each xml file may have any number of toolbars
    ToolbarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      QString name;
      QCString txt( (*it).namedItem( tagText ).toElement().text().utf8() );
      if ( txt.isEmpty() )
          txt = (*it).namedItem( tagText2 ).toElement().text().utf8();
      if ( txt.isEmpty() )
          name = (*it).attribute( attrName );
      else
          name = i18n( txt );

      // the name of the toolbar might depend on whether or not
      // it is in kparts
      if ( ( (*xit).m_type == XmlData::Shell ) ||
           ( (*xit).m_type == XmlData::Part ) )
      {
        QString doc_name((*xit).m_document.documentElement().attribute( attrName ));
        name += " <" + doc_name + ">";
      }

      // is this our toolbar?
      if ( name == _text )
      {
        // save our current settings
        d->m_currentXmlData     = (*xit);
        d->m_currentToolbarElem = (*it);

        // load in our values
        loadActionList(d->m_currentToolbarElem);

        if ((*xit).m_type == XmlData::Part || (*xit).m_type == XmlData::Shell)
          setXML(KXMLGUIFactory::documentToXML((*xit).m_document.toDocument()));
        return;
      }
    }
  }
}

void KEditToolbarWidget::slotInactiveSelected(QListViewItem *item)
{
  if (item)
  {
    m_insertAction->setEnabled(true);
    QString statusText = static_cast<ToolbarItem *>(item)->statusText();
    d->m_helpArea->setText( statusText );
  }
  else
  {
    m_insertAction->setEnabled(false);
    d->m_helpArea->setText( QString::null );
  }
}

void KEditToolbarWidget::slotActiveSelected(QListViewItem *item)
{
  if (item)
  {
    m_removeAction->setEnabled(true);

    if (item->itemAbove())
      m_upAction->setEnabled(true);
    else
      m_upAction->setEnabled(false);

    if (item->itemBelow())
      m_downAction->setEnabled(true);
    else
      m_downAction->setEnabled(false);
    QString statusText = static_cast<ToolbarItem *>(item)->statusText();
    d->m_helpArea->setText( statusText );
  }
  else
  {
    m_removeAction->setEnabled(false);
    m_upAction->setEnabled(false);
    m_downAction->setEnabled(false);
    d->m_helpArea->setText( QString::null );
  }
}

void KEditToolbarWidget::slotInsertButton()
{
  static const QString &tagAction    = KGlobal::staticQString( "Action" );
  static const QString &tagSeparator = KGlobal::staticQString( "Separator" );
  static const QString &attrName     = KGlobal::staticQString( "name" );

  // we're modified, so let this change
  enableOk(true);

  ToolbarItem *item = (ToolbarItem*)m_inactiveList->currentItem();

  QDomElement new_item;
  // let's handle the separator specially
  if (item->text(1) == "-----")
    new_item = domDocument().createElement(tagSeparator);
  else
    new_item = domDocument().createElement(tagAction);
  new_item.setAttribute(attrName, item->internalName());

  if (m_activeList->currentItem())
  {
    // we have a selected item in the active list.. so let's try
    // our best to add our new item right after the selected one
    ToolbarItem *act_item = (ToolbarItem*)m_activeList->currentItem();
    QDomElement elem = d->m_currentToolbarElem.firstChild().toElement();
    for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
    {
      if ((elem.attribute(attrName) == act_item->internalName()) &&
          (elem.tagName() == act_item->internalTag()))
      {
        d->m_currentToolbarElem.insertAfter(new_item, elem);
        break;
      }
    }
  }
  else
  {
    // just stick it at the end of this
    d->m_currentToolbarElem.appendChild(new_item);
  }

  // and set this container as a noMerge
  d->m_currentToolbarElem.setAttribute(QString::fromLatin1("noMerge"), "1");

  // update the local doc
  updateLocal(d->m_currentToolbarElem);

  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotRemoveButton()
{
  static const QString &attrName    = KGlobal::staticQString( "name" );
  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );

  // we're modified, so let this change
  enableOk(true);

  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

  // now iterate through to find the child to nuke
  QDomElement elem = d->m_currentToolbarElem.firstChild().toElement();
  for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
  {
    if ((elem.attribute(attrName) == item->internalName()) &&
        (elem.tagName() == item->internalTag()))
    {
      // nuke myself!
      d->m_currentToolbarElem.removeChild(elem);

      // and set this container as a noMerge
      d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

      // update the local doc
      updateLocal(d->m_currentToolbarElem);

      break;
    }
  }

  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotUpButton()
{
  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

  // make sure we're not the top item already
  if (!item->itemAbove())
    return;

  static const QString &attrName    = KGlobal::staticQString( "name" );
  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );

  // we're modified, so let this change
  enableOk(true);

  // now iterate through to find where we are
  QDomElement elem = d->m_currentToolbarElem.firstChild().toElement();
  for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
  {
    if ((elem.attribute(attrName) == item->internalName()) &&
        (elem.tagName() == item->internalTag()))
    {
      // cool, i found me.  now clone myself
      ToolbarItem *clone = new ToolbarItem(m_activeList,
                                           item->itemAbove()->itemAbove(),
                                           item->internalTag(),
                                           item->internalName(),
                                           item->statusText());
      clone->setText(1, item->text(1));

      // only set new pixmap if exists
      if( item->pixmap(0) )
        clone->setPixmap(0, *item->pixmap(0));

      // remove the old me
      m_activeList->takeItem(item);
      delete item;

      // select my clone
      m_activeList->setSelected(clone, true);

      // make clone visible
      m_activeList->ensureItemVisible(clone);

      // and do the real move in the DOM
      d->m_currentToolbarElem.insertBefore(elem, elem.previousSibling());

      // and set this container as a noMerge
      d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

      // update the local doc
      updateLocal(d->m_currentToolbarElem);

      break;
    }
  }
}

void KEditToolbarWidget::slotDownButton()
{
  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

  // make sure we're not the bottom item already
  if (!item->itemBelow())
    return;

  static const QString &attrName    = KGlobal::staticQString( "name" );
  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );

  // we're modified, so let this change
  enableOk(true);

  // now iterate through to find where we are
  QDomElement elem = d->m_currentToolbarElem.firstChild().toElement();
  for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
  {
    if ((elem.attribute(attrName) == item->internalName()) &&
        (elem.tagName() == item->internalTag()))
    {
      // cool, i found me.  now clone myself
      ToolbarItem *clone = new ToolbarItem(m_activeList,
                                           item->itemBelow(),
                                           item->internalTag(),
                                           item->internalName(),
                                           item->statusText());
      clone->setText(1, item->text(1));

      // only set new pixmap if exists
      if( item->pixmap(0) )
        clone->setPixmap(0, *item->pixmap(0));

      // remove the old me
      m_activeList->takeItem(item);
      delete item;

      // select my clone
      m_activeList->setSelected(clone, true);

      // make clone visible
      m_activeList->ensureItemVisible(clone);

      // and do the real move in the DOM
      d->m_currentToolbarElem.insertAfter(elem, elem.nextSibling());

      // and set this container as a noMerge
      d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

      // update the local doc
      updateLocal(d->m_currentToolbarElem);

      break;
    }
  }
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
      if ( d->m_currentXmlData.m_xmlFile == (*xit).m_xmlFile )
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
      QString name( (*it).attribute( attrName ) );;
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

void KEditToolbar::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KEditToolbarWidget::virtual_hook( int id, void* data )
{ KXMLGUIClient::virtual_hook( id, data ); }

#include "kedittoolbar.moc"
