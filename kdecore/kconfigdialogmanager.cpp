/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+kdelibs at meyerhome dot net)
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
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

#include "kconfigdialogmanager.h"

#include <qlabel.h>
#include <qobjectlist.h>
#include <qbuttongroup.h>
#include <qsqlpropertymap.h>

#include <kapplication.h>
#include <kconfigskeleton.h>
#include <kglobal.h>

#include <assert.h>

class KConfigDialogManager::Private {

public:
  Private() { }

public:
  QDict<QWidget> knownWidget;
  QDict<QWidget> buddyWidget;
};

KConfigDialogManager::KConfigDialogManager(QWidget *parent, KConfigSkeleton *conf, const char *name)
 : QObject(parent, name), m_conf(conf), m_dialog(parent) 
{
  d = new Private();
  
  kapp->installKDEPropertyMap();
  propertyMap = QSqlPropertyMap::defaultMap();
  
  init(true);
}

KConfigDialogManager::~KConfigDialogManager()
{
  delete d;
}

void KConfigDialogManager::init(bool trackChanges)
{
  if(trackChanges)
  {
    // QT
    changedMap.insert("QButton", SIGNAL(stateChanged(int)));
    changedMap.insert("QCheckBox", SIGNAL(stateChanged(int)));
    changedMap.insert("QPushButton", SIGNAL(stateChanged(int)));
    changedMap.insert("QRadioButton", SIGNAL(stateChanged(int)));
    changedMap.insert("QButtonGroup", SIGNAL(clicked(int)));
    changedMap.insert("QComboBox", SIGNAL(activated (int)));
    //qsqlproperty map doesn't store the text, but the value!
    //changedMap.insert("QComboBox", SIGNAL(textChanged(const QString &)));
    changedMap.insert("QDateEdit", SIGNAL(valueChanged(const QDate &)));
    changedMap.insert("QDateTimeEdit", SIGNAL(valueChanged(const QDateTime &)));
    changedMap.insert("QDial", SIGNAL(valueChanged (int)));
    changedMap.insert("QLineEdit", SIGNAL(textChanged(const QString &)));
    changedMap.insert("QSlider", SIGNAL(valueChanged(int)));
    changedMap.insert("QSpinBox", SIGNAL(valueChanged(int)));
    changedMap.insert("QTimeEdit", SIGNAL(valueChanged(const QTime &)));
    changedMap.insert("QTextEdit", SIGNAL(textChanged()));
    changedMap.insert("QTextBrowser", SIGNAL(sourceChanged(const QString &)));
    changedMap.insert("QMultiLineEdit", SIGNAL(textChanged()));
    changedMap.insert("QListBox", SIGNAL(selectionChanged()));
    changedMap.insert("QTabWidget", SIGNAL(currentChanged(QWidget *)));

    // KDE
    changedMap.insert( "KComboBox", SIGNAL(activated (int)));
    changedMap.insert( "KFontCombo", SIGNAL(activated (int)));
    changedMap.insert( "KFontRequester", SIGNAL(fontSelected(const QFont &)));
    changedMap.insert( "KFontChooser",  SIGNAL(fontSelected(const QFont &)));
    changedMap.insert( "KHistoryCombo", SIGNAL(activated (int)));

    changedMap.insert( "KColorButton", SIGNAL(changed(const QColor &)));
    changedMap.insert( "KDatePicker", SIGNAL(dateSelected (QDate)));
    changedMap.insert( "KEditListBox", SIGNAL(changed()));
    changedMap.insert( "KListBox", SIGNAL(selectionChanged()));
    changedMap.insert( "KLineEdit", SIGNAL(textChanged(const QString &)));
    changedMap.insert( "KPasswordEdit", SIGNAL(textChanged(const QString &)));
    changedMap.insert( "KRestrictedLine", SIGNAL(textChanged(const QString &)));
    changedMap.insert( "KTextBrowser", SIGNAL(sourceChanged(const QString &)));
    changedMap.insert( "KTextEdit", SIGNAL(textChanged()));
    changedMap.insert( "KURLRequester",  SIGNAL(textChanged (const QString& )));
    changedMap.insert( "KIntNumInput", SIGNAL(valueChanged (int)));
    changedMap.insert( "KIntSpinBox", SIGNAL(valueChanged (int)));
    changedMap.insert( "KDoubleNumInput", SIGNAL(valueChanged (double)));
  }

  // Go through all of the children of the widgets and find all known widgets
  (void) parseChildren(m_dialog, trackChanges);
}

void KConfigDialogManager::addWidget(QWidget *widget)
{
  (void) parseChildren(widget, true);
}

bool KConfigDialogManager::parseChildren(const QWidget *widget, bool trackChanges)
{
  bool valueChanged = false;
  const QObjectList *listOfChildren = widget->children();
  if(!listOfChildren)
    return valueChanged;

  QObject *object;
  for( QPtrListIterator<QObject> it( *listOfChildren );
       (object = it.current()); ++it )
  {
    if(!object->isWidgetType())
      continue; // Skip non-widgets

    QWidget *childWidget = (QWidget *)object;

    const char *widgetName = childWidget->name(0);
    bool bParseChildren = true;
    
    if (widgetName && (strncmp(widgetName, "kcfg_", 5) == 0))
    {
      // This is one of our widgets!
      QString configId = widgetName+5;
      KConfigSkeletonItem *item = m_conf->findItem(configId);
      if (item)
      {
        qWarning("Found %s !!", configId.latin1());
        d->knownWidget.insert(configId, childWidget);

        QMap<QString, QCString>::const_iterator changedIt = changedMap.find(childWidget->className());
        if (changedIt == changedMap.end())
        {
          qWarning("Don't know how to monitor widget '%s' for changes!", childWidget->className());
        }
        else
        {
          if (!childWidget->isA("QButtonGroup"))
            bParseChildren = false;
          connect(childWidget, *changedIt,
                  this, SIGNAL(widgetModified()));
        }
      }
      else
      {
        qWarning("Not found %s !!", configId.latin1());
        assert(false);
      }
    }
    else if (childWidget->inherits("QLabel"))
    {
      QLabel *label = static_cast<QLabel *>(childWidget);
      QWidget *buddy = label->buddy();
      if (!buddy)
        continue;
      const char *buddyName = buddy->name(0);
      if (buddyName && (strncmp(buddyName, "kcfg_", 5) == 0))
      {
        // This is one of our widgets!
        QString configId = buddyName+5;
        d->buddyWidget.insert(configId, childWidget);
      }
    }
    else if (widgetName)
    {
      QMap<QString, QCString>::const_iterator changedIt = changedMap.find(childWidget->className());
      if (changedIt != changedMap.end())
        qWarning("Skipping %s (%s)!", widgetName, childWidget->className());
    }
    
    if(bParseChildren)
    {
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      valueChanged |= parseChildren(childWidget, trackChanges);
    }
  }
  return valueChanged;
}

void KConfigDialogManager::updateWidgets()
{
  bool bSignalsBlocked = signalsBlocked();
  blockSignals(true);

  QWidget *widget;
  for( QDictIterator<QWidget> it( d->knownWidget );
       (widget = it.current()); ++it )
  {
     KConfigSkeletonItem *item = m_conf->findItem(it.currentKey());
     if (!item)
     {
        qWarning("knownWidget went missing!");
        continue;
     }

     QVariant p = item->property();
     if (p != property(widget))
     {
        setProperty(widget, p);
     }
     if (item->isImmutable())
     {
        widget->setEnabled(false);
        QWidget *buddy = d->buddyWidget.find(it.currentKey());
        if (buddy)
           buddy->setEnabled(false);
     }
  }
  blockSignals(bSignalsBlocked);
}

void KConfigDialogManager::updateWidgetsDefault()
{
  bool bUseDefaults = m_conf->useDefaults(true);
  updateWidgets();
  m_conf->useDefaults(bUseDefaults);
}

void KConfigDialogManager::updateSettings()
{
  bool changed = false;

  QWidget *widget;
  for( QDictIterator<QWidget> it( d->knownWidget );
       (widget = it.current()); ++it )
  {
     KConfigSkeletonItem *item = m_conf->findItem(it.currentKey());
     if (!item)
     {
        qWarning("knownWidget went missing!");
        continue;
     }

     QVariant p = property(widget);
     if (p != item->property())
     {
        item->setProperty(p);
        changed = true;
     }
  }
  if (changed)
  {
     m_conf->writeConfig();
     emit settingsChanged();
  }
}

void KConfigDialogManager::setProperty(QWidget *w, const QVariant &v)
{
  if (w->isA("QButtonGroup"))
  {
    QButtonGroup *bg = static_cast<QButtonGroup *>(w);
    bg->setButton(v.toInt());
    return;
  }

  propertyMap->setProperty(w, v);
}

QVariant KConfigDialogManager::property(QWidget *w)
{
  if (w->isA("QButtonGroup"))
  {
    QButtonGroup *bg = static_cast<QButtonGroup *>(w);
    return QVariant(bg->selectedId());
  }

  return propertyMap->property(w);
}

bool KConfigDialogManager::hasChanged()
{

  QWidget *widget;
  for( QDictIterator<QWidget> it( d->knownWidget );
       (widget = it.current()); ++it )
  {
     KConfigSkeletonItem *item = m_conf->findItem(it.currentKey());
     if (!item)
     {
        qWarning("knownWidget went missing!");
        continue;
     }

     QVariant p = property(widget);
     if (p != item->property())
     {
        return true;
     }
  }
  return false;
}

bool KConfigDialogManager::isDefault()
{
  bool bUseDefaults = m_conf->useDefaults(true);
  bool result = !hasChanged();
  m_conf->useDefaults(bUseDefaults);
  return result;
}

#include "kconfigdialogmanager.moc"

