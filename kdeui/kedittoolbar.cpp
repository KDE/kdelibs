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

#include <kstddirs.h>
#include <klocale.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kxmlgui.h>
#include <kseparator.h>
#include <kconfig.h>
#include <klistview.h>

#include <qtextstream.h>
#include <qfile.h>
#include <kdebug.h>

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
  ToolbarItem(KListView *parent, const QString& name)
    : QListViewItem(parent),
      m_name(name)
  {
  }

  ToolbarItem(KListView *parent, QListViewItem *item, const QString& name)
    : QListViewItem(parent, item),
      m_name(name)
  {
  }

  QString internalName() const
  {
    return m_name;
  }

private:
  QString m_name;
};

class ToolbarStyleItem
{
public:
  ToolbarStyleItem(int icon_size = 1, int icontext = 0, int pos = 0)
    : m_iconSize(icon_size), m_iconText(icontext), m_position(pos)
  {
    m_sizeChanged = false;
    m_textChanged = false;
    m_posChanged  = false;
  }

  int m_iconSize;
  int m_iconText;
  int m_position;

  bool m_sizeChanged;
  bool m_textChanged;
  bool m_posChanged;
};
typedef QMap<QString, ToolbarStyleItem> StyleMap;

class KEditToolbarWidgetPrivate
{
public:
  KEditToolbarWidgetPrivate(KInstance *instance)
  {
    m_instance = instance;
    m_isPart   = false;
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
    {
      QString abs_xml( locate("data", QString(m_instance->instanceName()) +
                                      "/" + xml_file) );
      raw_xml = KXMLGUIFactory::readConfigFile(abs_xml);
    }

    return raw_xml;
  }

  /**
   * Return a list of toolbar elements given a toplevel element
   */
  ToolbarList findToolbars(QDomElement& elem)
  {
    static QString tagToolbar = QString::fromLatin1( "ToolBar" );
    ToolbarList list;

    for( ; !elem.isNull(); elem = elem.nextSibling().toElement() )
    {
      if (elem.tagName() == tagToolbar)
        list.append(elem);

      QDomElement child = elem.firstChild().toElement();
      list += findToolbars(child);
    }

    return list;
  }


  KActionCollection m_collection;
  KInstance         *m_instance;

  XmlData     m_currentXmlData;
  QDomElement m_currentToolbarElem;

  QString            m_xmlFile;
  QString            m_globalFile;
  QString            m_rcFile;
  QDomDocument       m_localDoc;
  bool               m_isPart;

  ToolbarList m_barList;
  StyleMap    m_styleMap;

  XmlDataList m_xmlFiles;
};

KEditToolbar::KEditToolbar(KActionCollection *collection, const QString& file,
                           bool global, QWidget* parent, const char* name)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Ok|Cancel, Cancel, parent, name),
      m_widget(new KEditToolbarWidget(collection, file, global, this))
{
    setMainWidget(m_widget);

    connect(m_widget, SIGNAL(enableOk(bool)),
            this,     SLOT(enableButtonOK(bool)));
    enableButtonOK(false);
}

KEditToolbar::KEditToolbar(KXMLGUIFactory* factory, QWidget* parent, const char* name)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Ok|Cancel, Cancel, parent, name),
      m_widget(new KEditToolbarWidget(factory, this))
{
    setMainWidget(m_widget);

    connect(m_widget, SIGNAL(enableOk(bool)),
            this,     SLOT(enableButtonOK(bool)));
    enableButtonOK(false);
}
void KEditToolbar::slotOk()
{
  // just the fact that we got here means that we've been modified.
  // so we save
  if (!m_widget->save())
  {
    // some error box here is needed
  }
  else
  {
    accept();
  }
}

KEditToolbarWidget::KEditToolbarWidget(KActionCollection *collection,
                                       const QString& file,
                                       bool global, QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance()))
{
  // let's not forget the stuff that's not xml specific
  d->m_collection = *collection;

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
}

KEditToolbarWidget::KEditToolbarWidget( KXMLGUIFactory* factory,
                                        QWidget *parent)
  : QWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance()))
{
  // reusable vars
  QDomElement elem;

  setFactory( factory );

  // add all of the client data
  QValueList<KXMLGUIClient*> clients(factory->clients());
  QValueList<KXMLGUIClient*>::ConstIterator it(clients.begin());
  for( ; it != clients.end(); ++it)
  {
    KXMLGUIClient *client = (*it);

    if (client->xmlFile().isNull())
      continue;

    XmlData data;
    data.m_xmlFile = client->xmlFile();
    if ( it == clients.begin() )
      data.m_type = XmlData::Shell;
    else
      data.m_type = XmlData::Part;
    data.m_document.setContent(d->loadXMLFile(client->xmlFile()));
    elem = data.m_document.documentElement().toElement();
    data.m_barList = d->findToolbars(elem);
    d->m_xmlFiles.append(data);

    d->m_collection += *client->actionCollection();
  }

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();

  // now load in our toolbar combo box
  loadToolbarCombo();
}

KEditToolbarWidget::~KEditToolbarWidget()
{
}

bool KEditToolbarWidget::save()
{
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

  QValueList<KXMLGUIClient*> clients(factory()->clients());

  // remove the elements starting from the last going to the first
  QValueList<KXMLGUIClient*>::Iterator client(clients.fromLast());
  for( ; client != clients.begin(); --client)
    factory()->removeClient( *client );

  // also remove the first element (can't do it in the loop)
  factory()->removeClient( *clients.begin() );

  // now, rebuild the gui from the first to the last
  for ( client = clients.begin(); client != clients.end(); ++client )
  {
    // passing an empty stream forces the clients to reread the XML
    (*client)->setContainerStates( QMap<QString,QByteArray>() );

    // and this forces it to use the *new* XML file
    (*client)->reloadXML();

    // finally, do all the real work
    factory()->addClient( *client );
  }

  return true;
}

void KEditToolbarWidget::setupLayout()
{
  // the toolbar name combo
  QLabel *toolbar_label = new QLabel(i18n("Toolbar:"), this);
  m_toolbarCombo = new QComboBox(this);
  m_toolbarCombo->setEnabled(false);
  connect(m_toolbarCombo, SIGNAL(activated(const QString&)),
          this,           SLOT(slotToolbarSelected(const QString&)));

  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
  new_toolbar->setPixmap(BarIcon("filenew", KIcon::SizeSmall));
  new_toolbar->setEnabled(false); // disabled until implemented
  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
  del_toolbar->setPixmap(BarIcon("editdelete", KIcon::SizeSmall));
  del_toolbar->setEnabled(false); // disabled until implemented

  // our list of inactive actions
  QLabel *inactive_label = new QLabel(i18n("Available actions:"), this);
  m_inactiveList = new KListView(this);
  m_inactiveList->setAllColumnsShowFocus(true);
  m_inactiveList->header()->hide();
  m_inactiveList->addColumn("");
  m_inactiveList->addColumn("");
  connect(m_inactiveList, SIGNAL(selectionChanged(QListViewItem *)),
          this,           SLOT(slotInactiveSelected(QListViewItem *)));

  // our list of active actions
  QLabel *active_label = new QLabel(i18n("Current actions:"), this);
  m_activeList = new KListView(this);
  m_activeList->setAllColumnsShowFocus(true);
  m_activeList->header()->hide();
  m_activeList->addColumn("");
  m_activeList->addColumn("");
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

  // setup the toolbar options
  QLabel *text_label = new QLabel(i18n("Text position:"), this);
  m_textCombo = new QComboBox(this);
  m_textCombo->insertItem(i18n("No text"));
  m_textCombo->insertItem(i18n("Text aside icon"));
  m_textCombo->insertItem(i18n("Text only"));
  m_textCombo->insertItem(i18n("Text under icon"));
  connect(m_textCombo, SIGNAL(highlighted(int)),
          this,        SLOT(slotTextClicked(int)));

  QLabel *icon_label = new QLabel(i18n("Icon size:"), this);
  m_iconCombo = new QComboBox(this);
  // the sizes are generated later
  connect(m_iconCombo, SIGNAL(highlighted(int)),
          this,        SLOT(slotIconClicked(int)));

  QLabel *pos_label = new QLabel(i18n("Toolbar position:"), this);
  m_posCombo = new QComboBox(this);
  m_posCombo->insertItem(i18n("Top (normal)"));
  m_posCombo->insertItem(i18n("Left"));
  m_posCombo->insertItem(i18n("Right"));
  m_posCombo->insertItem(i18n("Bottom"));
  m_posCombo->insertItem(i18n("Floating"));
  m_posCombo->insertItem(i18n("Flat"));
  connect(m_posCombo, SIGNAL(highlighted(int)),
          this,       SLOT(slotPosClicked(int)));

  // initially, disable the styles
  m_textCombo->setEnabled(false);
  m_iconCombo->setEnabled(false);
  m_posCombo->setEnabled(false);

  // now start with our layouts
  QVBoxLayout *top_layout = new QVBoxLayout(this, 5);

  QHBoxLayout *name_layout = new QHBoxLayout;
  QHBoxLayout *list_layout = new QHBoxLayout;

  QVBoxLayout *inactive_layout = new QVBoxLayout;
  QVBoxLayout *active_layout = new QVBoxLayout;

  QGridLayout *button_layout = new QGridLayout(5, 3, 0);

  QGridLayout *options_layout = new QGridLayout(2, 5);

  name_layout->addWidget(toolbar_label);
  name_layout->addWidget(m_toolbarCombo, 1);
  name_layout->addWidget(new_toolbar);
  name_layout->addWidget(del_toolbar);

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

  options_layout->addWidget(text_label, 0, 0);
  options_layout->addWidget(m_textCombo, 0, 1);
  options_layout->setColStretch(2, 1);
  options_layout->addWidget(pos_label, 0, 3);
  options_layout->addWidget(m_posCombo, 0, 4);
  options_layout->addWidget(icon_label, 1, 0);
  options_layout->addWidget(m_iconCombo, 1, 1);

  top_layout->addLayout(name_layout);
  top_layout->addWidget(new KSeparator(this));
  top_layout->addLayout(list_layout);
  top_layout->addWidget(new KSeparator(this));
  top_layout->addLayout(options_layout);
  top_layout->addWidget(new KSeparator(this));
}

void KEditToolbarWidget::loadToolbarCombo()
{
  static QString attrName = QString::fromLatin1( "name" );

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
      QString name( (*it).attribute( attrName ) );;
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

void KEditToolbarWidget::loadToolbarStyles(QDomElement& elem)
{
  static QString attrName      = QString::fromLatin1( "name" );
  static QString attrIconText  = QString::fromLatin1( "iconText" );
  static QString attrIconSize  = QString::fromLatin1( "iconSize" );
  static QString attrPosition  = QString::fromLatin1( "position" );

  QString name(elem.attribute(attrName));
  QString icon_text(elem.attribute(attrIconText).lower());
  QString position(elem.attribute(attrPosition).lower());
  QString icon_str(elem.attribute(attrIconSize));
  int icon_size = (icon_str.isEmpty()) ? -1 : icon_str.toInt();

  m_textCombo->setEnabled(true);
  m_iconCombo->setEnabled(true);
  m_posCombo->setEnabled(true);

  if (icon_text == QString::fromLatin1("icononly"))
    m_textCombo->setCurrentItem(0);
  else if (icon_text == QString::fromLatin1("icontextright"))
    m_textCombo->setCurrentItem(1);
  else if (icon_text == QString::fromLatin1("textonly"))
    m_textCombo->setCurrentItem(2);
  else if (icon_text == QString::fromLatin1("icontextbottom"))
    m_textCombo->setCurrentItem(3);
  else if ( name == "mainToolBar")
  {
    KConfig *config = KGlobal::config();
    KConfigGroupSaver saver(config, QString::fromLatin1("Toolbar style"));
    int index = config->readNumEntry(QString::fromLatin1("IconText"), 0);
    m_textCombo->setCurrentItem(index);
  }
  else
    m_textCombo->setCurrentItem(0);

  // load the sizes now
  m_iconCombo->clear();
  KIconTheme *theme = instance()->iconLoader()->theme();
  QValueList<int> sizes = theme->querySizes( name == "mainToolBar" ?
                                             KIcon::MainToolbar :
                                             KIcon::Toolbar );
  QValueList<int>::Iterator it(sizes.begin());
  for ( ; it != sizes.end(); ++it )
  {
      QString text;
      if ( *it < 20 )
          text = i18n("Small (%1x%2)").arg(*it).arg(*it);
      else if (*it < 25)
          text = i18n("Medium (%1x%2)").arg(*it).arg(*it);
      else
          text = i18n("Large (%1x%2)").arg(*it).arg(*it);
      m_iconCombo->insertItem(text);
  }

  if ( (icon_size == -1) && (name == "mainToolBar") )
  {
    // the new icon loader doesn't respect the IconSize field in the
    // config file... so neither will we
    icon_size = instance()->iconLoader()->currentSize( name == "mainToolBar" ?
                                                       KIcon::MainToolbar :
                                                       KIcon::Toolbar );
  }

  if ( icon_size == -1 )
    m_iconCombo->setCurrentItem(1);
  else if ( icon_size < 20)
    m_iconCombo->setCurrentItem(0);
  else if ( icon_size < 25 )
    m_iconCombo->setCurrentItem(1);
  else
    m_iconCombo->setCurrentItem(2);

  if ( position.isEmpty() && name == "mainToolBar" )
  {
    KConfig *config = KGlobal::config();
    KConfigGroupSaver saver(config, QString::fromLatin1("Toolbar style"));
    position = config->readEntry(QString::fromLatin1("Position")).lower();
  }
  if (position == QString::fromLatin1("top"))
    m_posCombo->setCurrentItem(0);
  else if (position == QString::fromLatin1("left"))
    m_posCombo->setCurrentItem(1);
  else if (position == QString::fromLatin1("right"))
    m_posCombo->setCurrentItem(2);
  else if (position == QString::fromLatin1("bottom"))
    m_posCombo->setCurrentItem(3);
  else if (position == QString::fromLatin1("floating"))
    m_posCombo->setCurrentItem(4);
  else if (position == QString::fromLatin1("flat"))
    m_posCombo->setCurrentItem(5);
  else
    m_posCombo->setCurrentItem(0);
}

void KEditToolbarWidget::loadActionList(QDomElement& elem)
{
  static QString tagSeparator = QString::fromLatin1( "Separator" );
  static QString tagMerge     = QString::fromLatin1( "Merge" );
  static QString attrName     = QString::fromLatin1( "name" );

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
      ToolbarItem *act = new ToolbarItem(m_activeList, sep_name.arg(sep_num++));
      act->setText(1, "-----");
      it.setAttribute( attrName, act->internalName() );
      continue;
    }

    if (it.tagName() == tagMerge)
    {
      ToolbarItem *act = new ToolbarItem(m_activeList, "merge");
      act->setText(1, "<Merge>");
      continue;
    }

    // iterate through all of our actions
    for (unsigned int i = 0;  i < d->m_collection.count(); i++)
    {
      KAction *action = d->m_collection.action(i);

      // do we have a match?
      if (it.attribute( attrName ) == action->name())
      {
        // we have a match!
        ToolbarItem *act = new ToolbarItem(m_activeList, action->name());
        act->setText(1, action->plainText());
        if (!action->iconName().isNull())
          act->setPixmap(0, BarIcon(action->iconName(), 16));

        active_list.insert(action->name(), true);
        break;
      }
    }
  }

  // go through the rest of the collection
  for (int i = d->m_collection.count() - 1; i > -1; --i)
  {
    KAction *action = d->m_collection.action(i);

    // skip our active ones
    if (active_list.contains(action->name()))
      continue;

    // insert this into the inactive list
    // for now, only deal with buttons with icons.. later, we'll need
    // to look into actions a LOT more carefully
    if ( action->iconName().isNull() )
      continue;

    ToolbarItem *act = new ToolbarItem(m_inactiveList, action->name());
    act->setText(1, action->plainText());
    act->setPixmap(0, BarIcon(action->iconName(), 16));
  }

  // finally, add a default separator to the inactive list
  ToolbarItem *act = new ToolbarItem(m_inactiveList, sep_name.arg(sep_num++));
  act->setText(1, "-----");
}

KActionCollection *KEditToolbarWidget::actionCollection() const
{
  return &d->m_collection;
}

void KEditToolbarWidget::slotToolbarSelected(const QString& _text)
{
  static QString attrName = QString::fromLatin1( "name" );

  // iterate through everything
  XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    // each xml file may have any number of toolbars
    ToolbarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      QString name( (*it).attribute( attrName ) );;
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

        // we do not want to load in the styles for parts
        if ( (*xit).m_type != XmlData::Part )
          loadToolbarStyles(d->m_currentToolbarElem);
        else
        {
          m_textCombo->setEnabled(false);
          m_iconCombo->setEnabled(false);
          m_posCombo->setEnabled(false);
        }

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
    m_insertAction->setEnabled(true);
  else
    m_insertAction->setEnabled(false);
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
  }
  else
  {
    m_removeAction->setEnabled(false);
    m_upAction->setEnabled(false);
    m_downAction->setEnabled(false);
  }
}

void KEditToolbarWidget::slotInsertButton()
{
  static QString tagAction    = QString::fromLatin1( "Action" );
  static QString tagSeparator = QString::fromLatin1( "Separator" );
  static QString attrName     = QString::fromLatin1( "name" );

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
      if (elem.attribute(attrName) == act_item->internalName())
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
  static QString attrName    = QString::fromLatin1( "name" );
  static QString attrNoMerge = QString::fromLatin1( "noMerge" );

  // we're modified, so let this change
  enableOk(true);

  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

  // now iterate through to find the child to nuke
  QDomElement elem = d->m_currentToolbarElem.firstChild().toElement();
  for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
  {
    if (item->internalName() == elem.attribute(attrName))
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

  static QString attrName    = QString::fromLatin1( "name" );
  static QString attrNoMerge = QString::fromLatin1( "noMerge" );

  // we're modified, so let this change
  enableOk(true);

  // now iterate through to find where we are
  QDomElement elem = d->m_currentToolbarElem.firstChild().toElement();
  for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
  {
    if (item->internalName() == elem.attribute(attrName))
    {
      // cool, i found me.  now clone myself
      ToolbarItem *clone = new ToolbarItem(m_activeList,
                                           item->itemAbove()->itemAbove(),
                                           item->internalName());
      clone->setText(1, item->text(1));

      // only set new pixmap if exists
      if( item->pixmap(0) )
        clone->setPixmap(0, *item->pixmap(0));

      // remove the old me
      m_activeList->takeItem(item);

      // select my clone
      m_activeList->setSelected(clone, true);

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

  // make sure we're not the top item already
  if (!item->itemBelow())
    return;

  static QString attrName    = QString::fromLatin1( "name" );
  static QString attrNoMerge = QString::fromLatin1( "noMerge" );

  // we're modified, so let this change
  enableOk(true);

  // now iterate through to find where we are
  QDomElement elem = d->m_currentToolbarElem.firstChild().toElement();
  for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
  {
    if (item->internalName() == elem.attribute(attrName))
    {
      // cool, i found me.  now clone myself
      ToolbarItem *clone = new ToolbarItem(m_activeList,
                                           item->itemBelow(),
                                           item->internalName());
      clone->setText(1, item->text(1));

      // only set new pixmap if exists
      if( item->pixmap(0) )
        clone->setPixmap(0, *item->pixmap(0));

      // remove the old me
      m_activeList->takeItem(item);

      // select my clone
      m_activeList->setSelected(clone, true);

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

void KEditToolbarWidget::slotTextClicked(int index)
{
  static QString attrIconText = QString::fromLatin1( "iconText" );
  static QString attrName     = QString::fromLatin1( "name" );
  static QString attrNoMerge  = QString::fromLatin1( "noMerge" );

  enableOk(true);

  d->m_currentToolbarElem.setAttribute(attrNoMerge, "1");
  switch (index)
  {
  case 0:
  default:
    d->m_currentToolbarElem.setAttribute(attrIconText, QString::fromLatin1("IconOnly"));
    break;
  case 1:
    d->m_currentToolbarElem.setAttribute(attrIconText, QString::fromLatin1("IconTextRight"));
    break;
  case 2:
    d->m_currentToolbarElem.setAttribute(attrIconText, QString::fromLatin1("TextOnly"));
    break;
  case 3:
    d->m_currentToolbarElem.setAttribute(attrIconText, QString::fromLatin1("IconTextBottom"));
  break;
  }
  updateLocal(d->m_currentToolbarElem);
}

void KEditToolbarWidget::slotPosClicked(int index)
{
  static QString attrPosition = QString::fromLatin1( "position" );
  static QString attrName     = QString::fromLatin1( "name" );
  static QString attrNoMerge  = QString::fromLatin1( "noMerge" );

  enableOk(true);

  d->m_currentToolbarElem.setAttribute(attrNoMerge, "1");
  switch (index)
  {
  case 0:
  default:
    d->m_currentToolbarElem.setAttribute(attrPosition, QString::fromLatin1("Top"));
    break;
  case 1:
    d->m_currentToolbarElem.setAttribute(attrPosition, QString::fromLatin1("Left"));
    break;
  case 2:
    d->m_currentToolbarElem.setAttribute(attrPosition, QString::fromLatin1("Right"));
    break;
  case 3:
    d->m_currentToolbarElem.setAttribute(attrPosition, QString::fromLatin1("Bottom"));
    break;
  case 4:
    d->m_currentToolbarElem.setAttribute(attrPosition, QString::fromLatin1("Floating"));
    break;
  case 5:
    d->m_currentToolbarElem.setAttribute(attrPosition, QString::fromLatin1("Flat"));
    break;
  }
  updateLocal(d->m_currentToolbarElem);
}

void KEditToolbarWidget::slotIconClicked(int index)
{
  static QString attrIconSize = QString::fromLatin1( "iconSize" );
  static QString attrName     = QString::fromLatin1( "name" );
  static QString attrNoMerge  = QString::fromLatin1( "noMerge" );

  enableOk(true);
  d->m_currentToolbarElem.setAttribute(attrNoMerge, "1");

  KIconTheme *theme = instance()->iconLoader()->theme();
  QString name(d->m_currentToolbarElem.attribute( attrName ));
  QValueList<int> sizes = theme->querySizes( name == "mainToolBar" ?
                                             KIcon::MainToolbar :
                                             KIcon::Toolbar );

  QString size;
  size.setNum(sizes[index]);
  d->m_currentToolbarElem.setAttribute(attrIconSize, size);

  updateLocal(d->m_currentToolbarElem);
}

void KEditToolbarWidget::updateLocal(QDomElement& elem)
{
  static QString attrName = QString::fromLatin1( "name" );

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

#include "kedittoolbar.moc"
