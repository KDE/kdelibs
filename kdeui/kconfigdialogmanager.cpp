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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kconfigdialogmanager.h"

#include <q3buttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qmetaobject.h>
#include <qobject.h>
#include <q3sqlpropertymap.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfigskeleton.h>
#include <kdebug.h>
#include <kglobal.h>
#include <QHash>

#include <assert.h>

class KConfigDialogManager::Private {

public:
  Private() : insideGroupBox(false) { }

public:
  QHash<QString, QWidget *> knownWidget;
  QHash<QString, QWidget *> buddyWidget;
  bool insideGroupBox;
};

KConfigDialogManager::KConfigDialogManager(QWidget *parent, KConfigSkeleton *conf)
 : QObject(parent), m_conf(conf), m_dialog(parent)
{
  d = new Private();

  kapp->installKDEPropertyMap();
  propertyMap = Q3SqlPropertyMap::defaultMap();

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
    changedMap.insert("QRadioButton", SIGNAL(toggled(bool)));
    // We can only store one thing, so you can't have
    // a ButtonGroup that is checkable.
    changedMap.insert("QButtonGroup", SIGNAL(clicked(int)));
    changedMap.insert("QGroupBox", SIGNAL(toggled(bool)));
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
    changedMap.insert( "KDateWidget", SIGNAL(changed (QDate)));
    changedMap.insert( "KDateTimeWidget", SIGNAL(valueChanged (const QDateTime &)));
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

void KConfigDialogManager::setupWidget(QWidget *widget, KConfigSkeletonItem *item)
{
  QVariant minValue = item->minValue();
  if (minValue.isValid())
  {
    // Only q3datetimeedit is using this property we can remove it if we stop supporting Qt3Support
    if (widget->metaObject()->indexOfProperty("minValue") != -1)
       widget->setProperty("minValue", minValue);
    if (widget->metaObject()->indexOfProperty("minimum") != -1)
       widget->setProperty("minimum", minValue);
  }
  QVariant maxValue = item->maxValue();
  if (maxValue.isValid())
  {
    // Only q3datetimeedit is using that property we can remove it if we stop supporting Qt3Support
    if (widget->metaObject()->indexOfProperty("maxValue") != -1)
       widget->setProperty("maxValue", maxValue);
    if (widget->metaObject()->indexOfProperty("maximum") != -1)
       widget->setProperty("maximum", maxValue);
  }

  if (widget->whatsThis().isEmpty())
  {
    QString whatsThis = item->whatsThis();
    if ( !whatsThis.isEmpty() )
    {
      widget->setWhatsThis(whatsThis );
    }
  }
}

bool KConfigDialogManager::parseChildren(const QWidget *widget, bool trackChanges)
{
  bool valueChanged = false;
  const QList<QObject*> listOfChildren = widget->children();
  if(listOfChildren.count()==0) //?? XXX
    return valueChanged;

  foreach ( QObject *object, listOfChildren )
  {
    if(!object->isWidgetType())
      continue; // Skip non-widgets

    QWidget *childWidget = (QWidget *)object;

    const char *widgetName = childWidget->name(0);
    bool bParseChildren = true;
    bool bSaveInsideGroupBox = d->insideGroupBox;

    if (widgetName && (strncmp(widgetName, "kcfg_", 5) == 0))
    {
      // This is one of our widgets!
      QString configId = widgetName+5;
      KConfigSkeletonItem *item = m_conf->findItem(configId);
      if (item)
      {
        d->knownWidget.insert(configId, childWidget);

        setupWidget(childWidget, item);

        QMap<QString, QByteArray>::const_iterator changedIt = changedMap.find(childWidget->className());

        if (changedIt == changedMap.end())
        {
		   // If the class name of the widget wasn't in the monitored widgets map, then look for
		   // it again using the super class name. This fixes a problem with using QtRuby/Korundum
		   // widgets with KConfigXT where 'Qt::Widget' wasn't being seen a the real deal, even
		   // though it was a 'QWidget'.
          changedIt = changedMap.find(childWidget->metaObject()->superClassName());
        }

        if (changedIt == changedMap.end())
        {
          kdWarning(178) << "Don't know how to monitor widget '" << childWidget->className() << "' for changes!" << endl;
        }
        else
        {
          connect(childWidget, *changedIt,
                  this, SIGNAL(widgetModified()));

          Q3GroupBox *gb = dynamic_cast<Q3GroupBox *>(childWidget);
          if (!gb)
            bParseChildren = false;
          else
            d->insideGroupBox = true;

          QComboBox *cb = dynamic_cast<QComboBox *>(childWidget);
          if (cb && cb->editable())
            connect(cb, SIGNAL(textChanged(const QString &)),
                    this, SIGNAL(widgetModified()));
        }
      }
      else
      {
        kdWarning(178) << "A widget named '" << widgetName << "' was found but there is no setting named '" << configId << "'" << endl;
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
#ifndef NDEBUG
    else if (widgetName)
    {
      QMap<QString, QByteArray>::const_iterator changedIt = changedMap.find(childWidget->className());
      if (changedIt != changedMap.end())
      {
        if ((!d->insideGroupBox || !childWidget->inherits("QRadioButton")) &&
            !childWidget->inherits("QGroupBox"))
          kdDebug(178) << "Widget '" << widgetName << "' (" << childWidget->className() << ") remains unmanaged." << endl;
      }
    }
#endif

    if(bParseChildren)
    {
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      valueChanged |= parseChildren(childWidget, trackChanges);
    }
    d->insideGroupBox = bSaveInsideGroupBox;
  }
  return valueChanged;
}

void KConfigDialogManager::updateWidgets()
{
  bool changed = false;
  bool bSignalsBlocked = signalsBlocked();
  blockSignals(true);

  QWidget *widget;
  QHashIterator<QString, QWidget *> it( d->knownWidget );
  while(it.hasNext()) {
     it.next();
     widget = it.value();

     KConfigSkeletonItem *item = m_conf->findItem(it.key());
     if (!item)
     {
        kdWarning(178) << "The setting '" << it.key() << "' has disappeared!" << endl;
        continue;
     }

     QVariant p = item->property();
     if (p != property(widget))
     {
        setProperty(widget, p);
//        kdDebug(178) << "The setting '" << it.currentKey() << "' [" << widget->className() << "] has changed" << endl;
        changed = true;
     }
     if (item->isImmutable())
     {
        widget->setEnabled(false);
        QWidget *buddy = 0;
        if(d->buddyWidget.contains(it.key()))
           buddy = d->buddyWidget.value(it.key());
        if (buddy)
           buddy->setEnabled(false);
     }
  }
  blockSignals(bSignalsBlocked);

  if (changed)
    QTimer::singleShot(0, this, SIGNAL(widgetModified()));
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
  QHashIterator<QString, QWidget *> it( d->knownWidget );
  while(it.hasNext()) {
     it.next();
     widget = it.value();

     KConfigSkeletonItem *item = m_conf->findItem(it.key());
     if (!item)
     {
        kdWarning(178) << "The setting '" << it.key() << "' has disappeared!" << endl;
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
  Q3ButtonGroup *bg = dynamic_cast<Q3ButtonGroup *>(w);
  if (bg)
  {
    bg->setButton(v.toInt());
    return;
  }

  QComboBox *cb = dynamic_cast<QComboBox *>(w);
  if (cb && cb->editable())
  {
    cb->setCurrentText(v.toString());
    return;
  }

  propertyMap->setProperty(w, v);
}

QVariant KConfigDialogManager::property(QWidget *w)
{
  Q3ButtonGroup *bg = dynamic_cast<Q3ButtonGroup *>(w);
  if (bg)
    return QVariant(bg->selectedId());

  QComboBox *cb = dynamic_cast<QComboBox *>(w);
  if (cb && cb->editable())
      return QVariant(cb->currentText());

  return propertyMap->property(w);
}

bool KConfigDialogManager::hasChanged()
{

  QWidget *widget;
  QHashIterator<QString, QWidget *> it( d->knownWidget) ;
  while(it.hasNext()) {
     it.next();
     widget = it.value();

     KConfigSkeletonItem *item = m_conf->findItem(it.key());
     if (!item)
     {
        kdWarning(178) << "The setting '" << it.key() << "' has disappeared!" << endl;
        continue;
     }

     QVariant p = property(widget);
     if (p != item->property())
     {
//        kdDebug(178) << "Widget for '" << it.currentKey() << "' has changed." << endl;
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

