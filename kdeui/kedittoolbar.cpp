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

#include <qlistview.h>
#include <qheader.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <kseparator.h>

#include <kstddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kxmlgui.h>

#include <qtextstream.h>
#include <qfile.h>

static void dump_xml(const QDomDocument& doc)
{
    QString str;
    QTextStream ts(&str, IO_WriteOnly);
    ts << doc;
    qDebug("%s", str.ascii());
}

class ToolbarItem : public QListViewItem
{
public:
  ToolbarItem(QListView *parent, const QString& name)
    : QListViewItem(parent),
      m_name(name)
  {
  }

  ToolbarItem(QListView *parent, QListViewItem *item, const QString& name)
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
typedef QValueList<QAction*> ActionList;


KEditToolbar::KEditToolbar(QActionCollection *collection, const QString& file,
                           bool global)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Ok|Cancel, Cancel),
      m_widget(new KEditToolbarWidget(collection, file, global, this))
{
    setMainWidget(m_widget);

    connect(m_widget, SIGNAL(enableOk(bool)),
            this,     SLOT(enableButtonOK(bool)));
    enableButtonOK(false);
}

QDomDocument KEditToolbar::localDocument() const
{
  return m_widget->localDocument();
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
    accept();
}

KEditToolbarWidget::KEditToolbarWidget(QActionCollection *collection,
                                       const QString& file,
                                       bool global, QWidget *parent)
  : QWidget(parent),
    m_collection(collection),
    m_xmlFile(file)
{
  // construct our own filename if one is provided
  if (m_xmlFile.isNull())
    m_xmlFile = instance()->instanceName() + "ui.rc";
  
  // load in our local xml file for later use
  m_localDoc.setContent(KXMLGUIFactory::readConfigFile((locate("data", QString(instance()->instanceName()) +"/"+ m_xmlFile))));

  if (global)
    setXMLFile(locate("config", "ui/ui_standards.rc"));
  setXMLFile(m_xmlFile, true);

  QDomElement elem = document().documentElement().toElement();
  findToolbars(elem);

  setupLayout();

  loadComboBox();
}

KEditToolbarWidget::~KEditToolbarWidget()
{
}

bool KEditToolbarWidget::save()
{
  updateLocalDoc();

  dump_xml(m_localDoc.toDocument());

  QString file = m_xmlFile;
  if ( file[0] != '/' )
    file = locateLocal("data", QString(instance()->instanceName()) +"/"+ file);

  QFile rc_file( file );
  if (rc_file.open(IO_WriteOnly) == false)
  {
    // some error checking here
    return false;
  }

  // write out our document
  QTextStream ts(&rc_file);
  ts << m_localDoc.toDocument();

  rc_file.close();

  return true;
}

QDomDocument KEditToolbarWidget::localDocument() const
{
  return m_localDoc;
}

void KEditToolbarWidget::setupLayout()
{
  // the toolbar name combo
  QLabel *toolbar_label = new QLabel(i18n("Toolbar:"), this);
  m_toolbarCombo = new QComboBox(this);
  m_toolbarCombo->setEnabled(false);
  connect(m_toolbarCombo, SIGNAL(activated(const QString&)),
          this,           SLOT(slotComboClicked(const QString&)));

  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
  new_toolbar->setPixmap(BarIcon("filenew", KIconLoader::Small));
  new_toolbar->setEnabled(false); // disabled until implemented
  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
  del_toolbar->setPixmap(BarIcon("editdelete", KIconLoader::Small));
  del_toolbar->setEnabled(false); // disabled until implemented

  // our list of inactive actions
  QLabel *inactive_label = new QLabel(i18n("Available actions:"), this);
  m_inactiveList = new QListView(this);
  m_inactiveList->setAllColumnsShowFocus(true);
  m_inactiveList->header()->hide();
  m_inactiveList->addColumn("");
  m_inactiveList->addColumn("");
  connect(m_inactiveList, SIGNAL(selectionChanged(QListViewItem *)),
          this,           SLOT(slotInactiveSelected(QListViewItem *)));

  // our list of active actions
  QLabel *active_label = new QLabel(i18n("Current actions:"), this);
  m_activeList = new QListView(this);
  m_activeList->setAllColumnsShowFocus(true);
  m_activeList->header()->hide();
  m_activeList->addColumn("");
  m_activeList->addColumn("");
  connect(m_activeList, SIGNAL(selectionChanged(QListViewItem *)),
          this,         SLOT(slotActiveSelected(QListViewItem *)));

  m_upAction     = new QPushButton(QString::null, this);
  m_upAction->setPixmap(BarIcon("up", KIconLoader::Small));
  m_upAction->setEnabled(false);
  connect(m_upAction, SIGNAL(clicked()),
          this,       SLOT(slotUpButton()));

  m_insertAction = new QPushButton(QString::null, this);
  m_insertAction->setPixmap(BarIcon("forward", KIconLoader::Small));
  m_insertAction->setEnabled(false);
  connect(m_insertAction, SIGNAL(clicked()),
          this,           SLOT(slotInsertButton()));

  m_removeAction = new QPushButton(QString::null, this);
  m_removeAction->setPixmap(BarIcon("back", KIconLoader::Small));
  m_removeAction->setEnabled(false);
  connect(m_removeAction, SIGNAL(clicked()),
          this,           SLOT(slotRemoveButton()));

  m_downAction   = new QPushButton(QString::null, this);
  m_downAction->setPixmap(BarIcon("down", KIconLoader::Small));
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

  QLabel *icon_label = new QLabel(i18n("Icon size:"), this);
  m_iconCombo = new QComboBox(this);
  m_iconCombo->insertItem(i18n("Small icons"));
  m_iconCombo->insertItem(i18n("Normal icons"));
  m_iconCombo->insertItem(i18n("Large icons"));
  m_iconCombo->insertItem(i18n("Default size"));

  QLabel *pos_label = new QLabel(i18n("Toolbar position:"), this);
  m_posCombo = new QComboBox(this);
  m_posCombo->insertItem(i18n("Top (normal)"));
  m_posCombo->insertItem(i18n("Left"));
  m_posCombo->insertItem(i18n("Right"));
  m_posCombo->insertItem(i18n("Bottom"));
  m_posCombo->insertItem(i18n("Floating"));

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

  inactive_layout->addWidget(inactive_label, 1);
  inactive_layout->addWidget(m_inactiveList);

  active_layout->addWidget(active_label, 1);
  active_layout->addWidget(m_activeList);

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

void KEditToolbarWidget::loadComboBox()
{
  static QString attrName = QString::fromLatin1( "name" );

  // just in case, we clear our combo
  m_toolbarCombo->clear();

  // load in all of the toolbar names into this combo box
  ToolbarList::Iterator it = m_barList.begin();
  for ( ; it != m_barList.end(); ++it)
  {
    m_toolbarCombo->setEnabled(true);
    m_toolbarCombo->insertItem((*it).attribute( attrName ));
  }

  // we want to the first item selected and its actions loaded
  slotComboClicked(m_toolbarCombo->currentText());
}

void KEditToolbarWidget::loadActionList(QDomElement& elem)
{
  static QString attrSeparator = QString::fromLatin1( "separator" );
  static QString attrName      = QString::fromLatin1( "name" );

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
    if (it.tagName().lower() == attrSeparator)
    {
      ToolbarItem *act = new ToolbarItem(m_activeList, QString::null);
      act->setText(1, "-----");
      continue;
    }

    // iterate through all of our actions
    for (unsigned int i = 0;  i < m_collection->count(); i++)
    {
      QAction *action = m_collection->action(i);

      // do we have a match?
      if (it.attribute( attrName ) == action->name())
      {
        // we have a match!
        ToolbarItem *act = new ToolbarItem(m_activeList, action->name());
        act->setText(1, action->text());
        if (action->hasIconSet())
          act->setPixmap(0, action->iconSet().pixmap(QIconSet::Small, true));

        active_list.insert(action->name(), true);
        break;
      }
    }
  }

  // go through the rest of the collection
  for (int i = m_collection->count() - 1; i > -1; --i)
  {
    QAction *action = m_collection->action(i);

    // skip our active ones
    if (active_list.contains(action->name()))
      continue;

    // insert this into the inactive list
    // for now, only deal with buttons with icons.. later, we'll need
    // to look into actions a LOT more carefully
    if (action->hasIconSet() == false)
      continue;

    ToolbarItem *act = new ToolbarItem(m_inactiveList, action->name());
    act->setText(1, action->text());
    act->setPixmap(0, action->iconSet().pixmap(QIconSet::Small, true));
  }

  // finally, add a default separator to the inactive list
  ToolbarItem *act = new ToolbarItem(m_inactiveList, QString::null);
  act->setText(1, "-----");
}

QActionCollection *KEditToolbarWidget::actionCollection() const
{
  return m_collection;
}

void KEditToolbarWidget::findToolbars(QDomElement& elem)
{
  static QString tagToolbar = QString::fromLatin1( "toolbar" );

  for( ; !elem.isNull(); elem = elem.nextSibling().toElement() )
  {
    if (elem.tagName().lower() == tagToolbar)
      m_barList.append(elem);

    QDomElement child = elem.firstChild().toElement();
    findToolbars(child);
  }
}

void KEditToolbarWidget::slotComboClicked(const QString& _text)
{
  static QString attrName = QString::fromLatin1( "name" );

  ToolbarList::Iterator it = m_barList.begin();
  for ( ; it != m_barList.end(); ++it)
  {
    if ((*it).attribute(attrName) == _text)
    {
      loadActionList((*it));
      return;
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

  ToolbarList::Iterator it = m_barList.begin();
  for ( ; it != m_barList.end(); ++it)
  {
    QString toolbar_name((*it).attribute(attrName));
    QString current(m_toolbarCombo->currentText());

    // of course, we only want to deal with the current toolbar
    if (toolbar_name == current)
    {
      QDomElement new_item;
      // let's handle the separator specially
      if (item->text(1) != "-----")
      {
        new_item = document().createElement(tagAction);
        new_item.setAttribute(attrName, item->internalName());
      }
      else
        new_item = document().createElement(tagSeparator);

      if (m_activeList->currentItem())
      {
        // we have a selected item in the active list.. so let's try
        // our best to add our new item right after the selected one
        ToolbarItem *act_item = (ToolbarItem*)m_activeList->currentItem();
        QDomElement elem = (*it).firstChild().toElement();
        for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
        {
          if (elem.attribute(attrName) == act_item->internalName())
          {
            (*it).insertAfter(new_item, elem);
            break;
          }
        }
      }
      else
      {
        // just stick it at the end of this
        (*it).appendChild(new_item);

        // and set this container as a noMerge
        (*it).setAttribute(QString::fromLatin1("noMerge"), "1");
      }

      break;
    }
  }

  slotComboClicked( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotRemoveButton()
{
  static QString attrName = QString::fromLatin1( "name" );

  // we're modified, so let this change
  enableOk(true);

  ToolbarList::Iterator it = m_barList.begin();
  for ( ; it != m_barList.end(); ++it)
  {
    QString name((*it).attribute(attrName));
    QString current(m_toolbarCombo->currentText());

    // only deal with the current toolbar
    if (name == current)
    {
      ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

      // now iterate through to find the child to nuke
      QDomElement elem = (*it).firstChild().toElement();
      for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
      {
        if (item->internalName() == elem.attribute(attrName))
        {
          // nuke myself!
          (*it).removeChild(elem);

          // and set this container as a noMerge
          (*it).setAttribute(QString::fromLatin1("noMerge"), "1");

          break;
        }
      }
      break;
    }
  }
  slotComboClicked( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotUpButton()
{
  ToolbarItem *item = (ToolbarItem*)m_activeList->currentItem();

  // make sure we're not the top item already
  if (!item->itemAbove())
    return;

  static QString attrName = QString::fromLatin1( "name" );

  // we're modified, so let this change
  enableOk(true);

  ToolbarList::Iterator it = m_barList.begin();
  for ( ; it != m_barList.end(); ++it)
  {
    QString toolbar_name((*it).attribute(attrName));
    QString current_toolbar(m_toolbarCombo->currentText());

    // only deal with the current toolbar
    if (toolbar_name == current_toolbar)
    {
      // now iterate through to find where we are
      QDomElement elem = (*it).firstChild().toElement();
      for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
      {
        if (item->internalName() == elem.attribute(attrName))
        {
          // cool, i found me.  now clone myself
          ToolbarItem *clone = new ToolbarItem(m_activeList,
                                               item->itemAbove()->itemAbove(),
                                               item->internalName());
          clone->setText(1, item->text(1));
          clone->setPixmap(0, *item->pixmap(0));

          // remove the old me
          m_activeList->takeItem(item);

          // select my clone
          m_activeList->setSelected(clone, true);

          // and do the real move in the DOM
          (*it).insertBefore(elem, elem.previousSibling());

          // and set this container as a noMerge
          (*it).setAttribute(QString::fromLatin1("noMerge"), "1");

          break;
        }
      }

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

  static QString attrName = QString::fromLatin1( "name" );

  // we're modified, so let this change
  enableOk(true);

  ToolbarList::Iterator it = m_barList.begin();
  for ( ; it != m_barList.end(); ++it)
  {
    QString toolbar_name((*it).attribute(attrName));
    QString current_toolbar(m_toolbarCombo->currentText());

    // only deal with the current toolbar
    if (toolbar_name == current_toolbar)
    {
      // now iterate through to find where we are
      QDomElement elem = (*it).firstChild().toElement();
      for( ; !elem.isNull(); elem = elem.nextSibling().toElement())
      {
        if (item->internalName() == elem.attribute(attrName))
        {
          // cool, i found me.  now clone myself
          ToolbarItem *clone = new ToolbarItem(m_activeList,
                                               item->itemBelow(),
                                               item->internalName());
          clone->setText(1, item->text(1));
          clone->setPixmap(0, *item->pixmap(0));

          // remove the old me
          m_activeList->takeItem(item);

          // select my clone
          m_activeList->setSelected(clone, true);

          // and do the real move in the DOM
          (*it).insertAfter(elem, elem.nextSibling());

          // and set this container as a noMerge
          (*it).setAttribute(QString::fromLatin1("noMerge"), "1");

          break;
        }
      }

      break;
    }
  }
}

void KEditToolbarWidget::updateLocalDoc()
{
  // TODO: debug this!  it is currently much too buggy
  static QString tagToolBar = QString::fromLatin1( "ToolBar" );
  static QString attrNoMerge = QString::fromLatin1( "noMerge" );
  static QString attrName = QString::fromLatin1( "name" );

  QDomElement elem = m_localDoc.documentElement().toElement();
  for( elem = elem.firstChild().toElement(); !elem.isNull();
       elem = elem.nextSibling().toElement())
  {
    // only look for toolbars
    if (elem.tagName() == tagToolBar)
    {
      // search through *our* toolbars and see if we match
      ToolbarList::Iterator it = m_barList.begin();
      for (; it != m_barList.end(); ++it)
      {
        if ((*it).attribute(attrName) == elem.attribute(attrName))
        {
          // okay, they match.  now see if ours is modified
          if ((*it).attribute(attrNoMerge) == "1")
          {
            QDomElement toolbar = m_localDoc.documentElement().toElement();
            toolbar.replaceChild((*it), elem);
          }
        }
      }
    }
  }

  // finally, go through our toolbars again to see if there are any
  // new ones
  ToolbarList::Iterator it = m_barList.begin();
  for ( ; it != m_barList.end(); ++it)
  {
    if ((*it).attribute(attrNoMerge) == "1")
    {
      // if there are, just append them to the end
      QDomElement orig = m_localDoc.documentElement().toElement();
      orig.appendChild((*it));
    }
  }
}
