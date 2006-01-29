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

#include <Q3ButtonGroup>
#include <QComboBox>
#include <QLabel>
#include <QMetaObject>
#include <QTimer>
#include <QRadioButton>
#include <QHash>

#include <kconfigskeleton.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstaticdeleter.h>

#include <assert.h>

static QMap<QByteArray, QByteArray> *s_propertyMap = 0;
static QMap<QString, QByteArray> *s_changedMap = 0;

static KStaticDeleter< QMap<QByteArray, QByteArray> > s_propertyMapDeleter;
static KStaticDeleter< QMap<QString, QByteArray> > s_changedMapDeleter;
	    
class KConfigDialogManager::Private {

public:
  Private() : insideGroupBox(false) { }

public:
  QHash<QString, QWidget *> knownWidget;
  QHash<QString, QWidget *> buddyWidget;
  bool insideGroupBox;
  bool trackChanges;
};

KConfigDialogManager::KConfigDialogManager(QWidget *parent, KConfigSkeleton *conf)
 : QObject(parent), m_conf(conf), m_dialog(parent)
{
  d = new Private();

  init(true);
}

KConfigDialogManager::~KConfigDialogManager()
{
  delete d;
}

void KConfigDialogManager::initMaps()
{
  if ( s_propertyMap == 0 ) {
    s_propertyMapDeleter.setObject( s_propertyMap, new QMap<QByteArray,QByteArray> );
     
    //Qt
    s_propertyMap->insert( "Q3DateEdit", "date" );
    s_propertyMap->insert( "Q3DateTimeEdit", "dateTime" );
    s_propertyMap->insert( "Q3ListBox", "currentItem" );
    s_propertyMap->insert( "Q3TimeEdit", "time" );
    s_propertyMap->insert( "QAbstractButton", "text" );
    s_propertyMap->insert( "QCheckBox", "checked" );
    s_propertyMap->insert( "QRadioButton", "checked" );
    s_propertyMap->insert( "QComboBox", "currentIndex" );
    s_propertyMap->insert( "QDateTimeEdit", "dateTime" );
    s_propertyMap->insert( "QDial", "value" );
    s_propertyMap->insert( "QLabel", "text" );
    s_propertyMap->insert( "QLCDNumber", "value" );
    s_propertyMap->insert( "QLineEdit", "text" );
    s_propertyMap->insert( "QPushButton", "text" );
    s_propertyMap->insert( "QScrollBar", "value" );
    s_propertyMap->insert( "QSlider", "value" );
    s_propertyMap->insert( "QSpinBox", "value" );
    s_propertyMap->insert( "QTabBar", "currentTab" );
    s_propertyMap->insert( "QTabWidget", "currentPage" );
    s_propertyMap->insert( "QTextBrowser", "source" );
    s_propertyMap->insert( "QTextEdit", "text" );
    s_propertyMap->insert( "QGroupBox", "checked" );

    //KDE
    s_propertyMap->insert( "KColorButton", "color" );
    s_propertyMap->insert( "KComboBox", "currentIndex" );
    s_propertyMap->insert( "KDatePicker", "date" );
    s_propertyMap->insert( "KDateWidget", "date" );
    s_propertyMap->insert( "KDateTimeWidget", "dateTime" );
    s_propertyMap->insert( "KEditListBox", "items" );
    s_propertyMap->insert( "KFontCombo", "family" );
    s_propertyMap->insert( "KFontRequester", "font" );
    s_propertyMap->insert( "KFontChooser", "font" );
    s_propertyMap->insert( "KHistoryCombo", "currentItem" );
    s_propertyMap->insert( "KListBox", "currentItem" );
    s_propertyMap->insert( "KLineEdit", "text" );
    s_propertyMap->insert( "KRestrictedLine", "text" );
    s_propertyMap->insert( "KTextBrowser", "source" );
    s_propertyMap->insert( "KTextEdit", "text" );
    s_propertyMap->insert( "KUrlRequester", "url" );
    s_propertyMap->insert( "KPasswordEdit", "password" );
    s_propertyMap->insert( "KIntNumInput", "value" );
    s_propertyMap->insert( "KIntSpinBox", "value" );
    s_propertyMap->insert( "KDoubleNumInput", "value" );
  }
  									       
  if( s_changedMap == 0 )
  {
    s_changedMapDeleter.setObject( s_changedMap, new QMap<QString,QByteArray> );
    // QT
    s_changedMap->insert("QButton", SIGNAL(stateChanged(int)));
    s_changedMap->insert("QCheckBox", SIGNAL(stateChanged(int)));
    s_changedMap->insert("QPushButton", SIGNAL(stateChanged(int)));
    s_changedMap->insert("QRadioButton", SIGNAL(toggled(bool)));
    // We can only store one thing, so you can't have
    // a ButtonGroup that is checkable.
    s_changedMap->insert("QButtonGroup", SIGNAL(clicked(int)));
    s_changedMap->insert("QGroupBox", SIGNAL(toggled(bool)));
    s_changedMap->insert("QComboBox", SIGNAL(activated (int)));
    //qsqlproperty map doesn't store the text, but the value!
    //s_changedMap->insert("QComboBox", SIGNAL(textChanged(const QString &)));
    s_changedMap->insert("QDateEdit", SIGNAL(valueChanged(const QDate &)));
    s_changedMap->insert("QDateTimeEdit", SIGNAL(valueChanged(const QDateTime &)));
    s_changedMap->insert("QDial", SIGNAL(valueChanged (int)));
    s_changedMap->insert("QLineEdit", SIGNAL(textChanged(const QString &)));
    s_changedMap->insert("QSlider", SIGNAL(valueChanged(int)));
    s_changedMap->insert("QSpinBox", SIGNAL(valueChanged(int)));
    s_changedMap->insert("QTimeEdit", SIGNAL(valueChanged(const QTime &)));
    s_changedMap->insert("QTextEdit", SIGNAL(textChanged()));
    s_changedMap->insert("QTextBrowser", SIGNAL(sourceChanged(const QString &)));
    s_changedMap->insert("QMultiLineEdit", SIGNAL(textChanged()));
    s_changedMap->insert("QListBox", SIGNAL(selectionChanged()));
    s_changedMap->insert("QTabWidget", SIGNAL(currentChanged(QWidget *)));

    // KDE
    s_changedMap->insert( "KComboBox", SIGNAL(activated (int)));
    s_changedMap->insert( "KFontCombo", SIGNAL(activated (int)));
    s_changedMap->insert( "KFontRequester", SIGNAL(fontSelected(const QFont &)));
    s_changedMap->insert( "KFontChooser",  SIGNAL(fontSelected(const QFont &)));
    s_changedMap->insert( "KHistoryCombo", SIGNAL(activated (int)));

    s_changedMap->insert( "KColorButton", SIGNAL(changed(const QColor &)));
    s_changedMap->insert( "KDatePicker", SIGNAL(dateSelected (QDate)));
    s_changedMap->insert( "KDateWidget", SIGNAL(changed (QDate)));
    s_changedMap->insert( "KDateTimeWidget", SIGNAL(valueChanged (const QDateTime &)));
    s_changedMap->insert( "KEditListBox", SIGNAL(changed()));
    s_changedMap->insert( "KListBox", SIGNAL(selectionChanged()));
    s_changedMap->insert( "KLineEdit", SIGNAL(textChanged(const QString &)));
    s_changedMap->insert( "KPasswordEdit", SIGNAL(textChanged(const QString &)));
    s_changedMap->insert( "KRestrictedLine", SIGNAL(textChanged(const QString &)));
    s_changedMap->insert( "KTextBrowser", SIGNAL(sourceChanged(const QString &)));
    s_changedMap->insert( "KTextEdit", SIGNAL(textChanged()));
    s_changedMap->insert( "KUrlRequester",  SIGNAL(textChanged (const QString& )));
    s_changedMap->insert( "KIntNumInput", SIGNAL(valueChanged (int)));
    s_changedMap->insert( "KIntSpinBox", SIGNAL(valueChanged (int)));
    s_changedMap->insert( "KDoubleNumInput", SIGNAL(valueChanged (double)));
  }
}
	  
QMap<QByteArray, QByteArray> *KConfigDialogManager::propertyMap()
{
  initMaps();
  return s_propertyMap;
}

QMap<QString, QByteArray> *KConfigDialogManager::changedMap()
{
  initMaps();
  return s_changedMap;
}

void KConfigDialogManager::init(bool trackChanges)
{
  initMaps();
  d->trackChanges = trackChanges;

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

        if ( d->trackChanges ) {
	  QMap<QString, QByteArray>::const_iterator changedIt = s_changedMap->find(childWidget->className());

          if (changedIt == s_changedMap->end())
          {
		   // If the class name of the widget wasn't in the monitored widgets map, then look for
		   // it again using the super class name. This fixes a problem with using QtRuby/Korundum
		   // widgets with KConfigXT where 'Qt::Widget' wasn't being seen a the real deal, even
		   // though it was a 'QWidget'.
            changedIt = s_changedMap->find(childWidget->metaObject()->superClassName());
          }

          if (changedIt == s_changedMap->end())
          {
            kdWarning(178) << "Don't know how to monitor widget '" << childWidget->className() << "' for changes!" << endl;
          }
          else
          {
            connect(childWidget, *changedIt,
                  this, SIGNAL(widgetModified()));

            QComboBox *cb = dynamic_cast<QComboBox *>(childWidget);
            if (cb && cb->editable())
              connect(cb, SIGNAL(textChanged(const QString &)),
                    this, SIGNAL(widgetModified()));
	  }	 
        }
        Q3GroupBox *gb = dynamic_cast<Q3GroupBox *>(childWidget);
        if (!gb)
          bParseChildren = false;
        else
          d->insideGroupBox = true;
      }
      else
      {
        kdWarning(178) << "A widget named '" << widgetName << "' was found but there is no setting named '" << configId << "'" << endl;
        assert(false);
      }
    }
    else if (QLabel *label = qobject_cast<QLabel*>(childWidget))
    {
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
    else if (widgetName && d->trackChanges)
    {
      QMap<QString, QByteArray>::const_iterator changedIt = s_changedMap->find(childWidget->className());
      if (changedIt != s_changedMap->end())
      {
        if ((!d->insideGroupBox || !qobject_cast<QRadioButton*>(childWidget)) &&
            !qobject_cast<QGroupBox*>(childWidget))
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

  const QMetaObject* mo = w->metaObject();
  while (mo && !s_propertyMap->contains(mo->className()))
    mo = mo->superClass();
  if (!mo) {
    kdWarning(178) << w->metaObject()->className() << " widget not handled!" << endl;
    return;
  }
  w->setProperty(s_propertyMap->value(mo->className()), v);
}

QVariant KConfigDialogManager::property(QWidget *w)
{
  Q3ButtonGroup *bg = dynamic_cast<Q3ButtonGroup *>(w);
  if (bg)
    return QVariant(bg->selectedId());

  QComboBox *cb = dynamic_cast<QComboBox *>(w);
  if (cb && cb->editable())
      return QVariant(cb->currentText());

  const QMetaObject* mo = w->metaObject();
  while (mo && !s_propertyMap->contains(mo->className()))
    mo = mo->superClass();
		    
  if (!mo) {
    kdWarning(178) << w->metaObject()->className() << " widget not handled!" << endl;
    return QVariant();
  }
  return w->property(s_propertyMap->value(mo->className()));
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

