/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben+kdelibs at meyerhome dot net)
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
#include "kautoconfig.h"

#include <kglobal.h>
#include <qsqlpropertymap.h>
#include <qobjectlist.h>
#include <kconfig.h>
#include <kapplication.h>

/**
 * Macro function to warn developers when they are making calls
 * that can never return anything of value
 */ 
#ifndef NDEBUG
#include "kdebug.h"
#define functionCallOrderCheck(functionName, returnValue) \
  if(!d->retrievedSettings){ \
      kdDebug(180) << "KAutoConfig::"functionName"() was called before " \
      "KAutoConfig::retrieveSettings().  This should NEVER happen.  " \
      "Please Fix." << endl; \
    return returnValue; \
  }
#else
#define functionCallOrderCheck(functionName, returnValue)
#endif

class KAutoConfig::KAutoConfigPrivate {

public:
  KAutoConfigPrivate() : changed(false)
#ifndef NDEBUG
    , retrievedSettings(false)
#endif
  { init(); }

  // Widgets to parse
  QPtrList<QWidget> widgets;
  // Name of the group that KConfig should be set to for each widget.
  QMap<QWidget*, QString> groups;

  // Child widgets of widgets to ignore
  QPtrList<QWidget> ignore;

  // Reset to false after saveSettings returns true.
  bool changed;

#ifndef NDEBUG
  // Many functions require this to be true to be of any value.
  bool retrievedSettings;
#endif

  // Known widgets that can be configured
  QMap<QWidget*, QPtrList<QWidget> > autoWidgets;
  // Default values for the widgets.
  QMap<QWidget*, QVariant> defaultValues;
  // Widgets to not get properties on (QLabel etc)
  QAsciiDict<int> ignoreTheseWidgets;

  void init(){
    ignoreTheseWidgets.insert("QLabel", new int(1));
    ignoreTheseWidgets.insert("QFrame", new int(2));
    ignoreTheseWidgets.insert("QGroupBox", new int(3));
    ignoreTheseWidgets.insert("QButtonGroup", new int(4));
    ignoreTheseWidgets.insert("QWidget", new int(5));
    ignoreTheseWidgets.setAutoDelete(true);

    static bool defaultKDEPropertyMapInstalled = false;
    if ( !defaultKDEPropertyMapInstalled && kapp ) {
      kapp->installKDEPropertyMap();
      defaultKDEPropertyMapInstalled = true;
    }
  }
};

KAutoConfig::KAutoConfig(KConfig *kconfig, QObject *parent,
    const char *name) : QObject(parent, name), config(kconfig) {
  d = new KAutoConfigPrivate();
}

KAutoConfig::KAutoConfig(QObject *parent, const char *name) :
    QObject(parent, name), config(KGlobal::config()) {
  d = new KAutoConfigPrivate();
}

KAutoConfig::~KAutoConfig(){
  delete d;
}

void KAutoConfig::addWidget(QWidget *widget, const QString &group){
  d->groups.insert(widget, group);
  d->widgets.append(widget);
  QPtrList<QWidget> newAutoConfigWidget;
  d->autoWidgets.insert(widget, newAutoConfigWidget );
}

void KAutoConfig::ignoreSubWidget(QWidget *widget){
  d->ignore.append(widget);
}

bool KAutoConfig::retrieveSettings(bool trackChanges){
#ifndef NDEBUG
  if(d->retrievedSettings){
      kdDebug(180) << "This should not happen.  Function "
       "KAutoConfig::retrieveSettings() was called more then once, returning "
       "false.  Please fix." << endl;
    return false;
  }
  d->retrievedSettings = true;
#endif
  
  if(trackChanges){
    // QT
    changedMap.insert("QButton", SIGNAL(stateChanged(int)));
    changedMap.insert("QCheckBox", SIGNAL(stateChanged(int)));
    changedMap.insert("QPushButton", SIGNAL(stateChanged(int)));
    changedMap.insert("QRadioButton", SIGNAL(stateChanged(int)));
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
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  bool usingDefaultValues = false;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    usingDefaultValues |= parseChildren(widget, d->autoWidgets[widget], trackChanges);
  }
  return usingDefaultValues;
}

bool KAutoConfig::saveSettings() {
  functionCallOrderCheck("saveSettings", false);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);

    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    bool widgetChanged = false;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      QVariant currentValue = propertyMap->property(groupWidget);

      if(!config->hasDefault(groupWidget->name()) && currentValue == defaultValue){
        config->revertToDefault(groupWidget->name());
        widgetChanged = true;
      }
      else{
        QVariant savedValue = config->readPropertyEntry(groupWidget->name(),
                                                             defaultValue);
        if(savedValue != currentValue){
          config->writeEntry(groupWidget->name(), currentValue);
          widgetChanged = true;
        }
      }
    }
    d->changed |= widgetChanged;
    if(widgetChanged)
      emit( settingsChanged(widget) );
  }

  if(d->changed){
    emit( settingsChanged() );
    d->changed = false;
    config->sync();
    return true;
  }
  return false;
}

bool KAutoConfig::hasChanged() const {
  functionCallOrderCheck("hasChanged", false);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      QVariant currentValue = propertyMap->property(groupWidget);
      QVariant savedValue = config->readPropertyEntry(groupWidget->name(),
      defaultValue);

      // Return once just one item is found to have changed.
      if((currentValue == defaultValue && savedValue != currentValue) ||
         (savedValue != currentValue))
        return true;
    }
  }
  return false;
}

bool KAutoConfig::isDefault() const {
  functionCallOrderCheck("isDefault", false);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      QVariant currentValue = propertyMap->property(groupWidget);
      if(currentValue != defaultValue){
        //qDebug("groupWidget %s, has changed: default: %s new: %s", groupWidget->name(), defaultValue.toString().latin1(), currentValue.toString().latin1());
        return false;
      }
    }
  }
  return true;
}

void KAutoConfig::resetSettings() const {
  functionCallOrderCheck("resetSettings",);

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);

    // Go through the known autowidgets of this widget and save
    QPtrListIterator<QWidget> it( d->autoWidgets[widget] );
    QWidget *groupWidget;
    while ( (groupWidget = it.current()) != 0 ){
      ++it;
      QVariant defaultValue = d->defaultValues[groupWidget];
      if(defaultValue != propertyMap->property(groupWidget)){
        propertyMap->setProperty(groupWidget, defaultValue);
        d->changed = true;
      }
    }
  }
}

void KAutoConfig::reloadSettings() const {
  functionCallOrderCheck("reloadSettings", );

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *pageWidget;
  while ( (pageWidget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[pageWidget]);

    // Go through the known widgets of this page and reload
    QPtrListIterator<QWidget> it( d->autoWidgets[pageWidget] );
    QWidget *widget;
    while ( (widget = it.current()) != 0 ){
      ++it;
      QVariant defaultSetting = d->defaultValues[widget];
      QVariant setting = 
        config->readPropertyEntry(widget->name(), defaultSetting);
      propertyMap->setProperty(widget, setting);
    }
  }
  d->changed = false;
}

bool KAutoConfig::parseChildren(const QWidget *widget,
    QPtrList<QWidget>& currentGroup, bool trackChanges){
  bool valueChanged = false;
  const QPtrList<QObject> *listOfChildren = widget->children();
  if(!listOfChildren)
    return valueChanged;

  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  QPtrListIterator<QObject> it( *listOfChildren );
  QObject *object;
  while ( (object = it.current()) != 0 )
  {
    ++it;
    if(!object->isWidgetType()){
      continue;
    }
    QWidget *childWidget = (QWidget *)object;
    if(d->ignore.containsRef(childWidget)){
      continue;
    }

    bool parseTheChildren = true;
    if( d->ignoreTheseWidgets[childWidget->className()] == 0 &&  
      childWidget->name(0) != NULL )
    {
      QVariant defaultSetting = propertyMap->property(childWidget);
      if(defaultSetting.isValid())
      {
        parseTheChildren = false;
        // Disable the widget if it is immutable?
        if(config->entryIsImmutable( childWidget->name()))
          childWidget->setEnabled(false);
        else
        {
          // FOR THOSE WHO ARE LOOKING
          // Here is the code were the widget is actually marked to watch.
          //qDebug("KAutoConfig: Adding widget(%s)",childWidget->name()); 
          currentGroup.append(childWidget);
          d->defaultValues.insert(childWidget, defaultSetting);
        }
        // Get/Set settings and connect up the changed signal
        QVariant setting =
         config->readPropertyEntry(childWidget->name(), defaultSetting);
        if(setting != defaultSetting)
        {
          propertyMap->setProperty(childWidget, setting);
          valueChanged = true;
        }
        if(trackChanges && changedMap.find(childWidget->className()) !=
            changedMap.end())
        {
          connect(childWidget, changedMap[childWidget->className()],
                  this, SIGNAL(widgetModified()));
        }
#ifndef NDEBUG
        else if(trackChanges &&
          changedMap.find(childWidget->className()) == changedMap.end())
        {
          kdDebug(180) << "KAutoConfig::retrieveSettings, Unknown changed "
           "signal for widget:" << childWidget->className() << endl;
        }
#endif

      }
#ifndef NDEBUG
      else
      { 
          kdDebug(180) << "KAutoConfig::retrieveSettings, Unknown widget:" 
           << childWidget->className() << endl;
      }
#endif
    }
    if(parseTheChildren)
    {
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      valueChanged |= parseChildren(childWidget, currentGroup, trackChanges);
    }
  }
  return valueChanged;
}

#include "kautoconfig.moc"

