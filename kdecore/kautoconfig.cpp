#include "kautoconfig.h"

#include <qobjectlist.h> 
#include <qwidget.h>
#include <kconfig.h>
#include <kapplication.h>

#include <qsqlpropertymap.h>
#include <qvariant.h>

// For KAutoConfigPrivate
#include <qptrlist.h>
#include <qmap.h>
#include <qasciidict.h> 

class KAutoConfig::KAutoConfigPrivate {
public:
  KAutoConfigPrivate() : changed(false) { }
 
  // Widgets to parse
  QPtrList<QWidget> widgets;
  // Name of the group that KConfig should be set to for each widget. 
  QMap<QWidget*, QString> groups;
  
  // Child widgets of widgets to ignore
  QPtrList<QWidget> ignore;
  
  // reset to false after saveSettings returns true.
  bool changed;
  
  // Known widgets that can be configured
  QMap<QWidget*, QPtrList<QWidget> > autoWidgets;
  // Default values for the widgets.
  QMap<QWidget*, QVariant> defaultValues;
  // Widgets to not get properties on (QLabel etc)
  QAsciiDict<int> ignoreTheseWidgets;
};

KAutoConfig::KAutoConfig(QObject *parent, const char *name,
		         KConfig *kconfig) :
			 QObject(parent, name), config(kconfig) {
  
  static bool defaultKDEPropertyMapInstalled = false;
  if ( !defaultKDEPropertyMapInstalled && kapp ) {
    kapp->installKDEPropertyMap();
    defaultKDEPropertyMapInstalled = true;
  }
  
  d = new KAutoConfigPrivate();
  
  d->ignoreTheseWidgets.insert("QLabel", new int(1));		 
  d->ignoreTheseWidgets.insert("QFrame", new int(2));
  d->ignoreTheseWidgets.insert("QGroupBox", new int(3));
  d->ignoreTheseWidgets.setAutoDelete(true);
};

KAutoConfig::~KAutoConfig(){
  delete d;
}

void KAutoConfig::addWidget(QWidget *widget, const QString &group){
  d->widgets.append(widget);
  d->groups.insert(widget, group);
  QPtrList<QWidget> newAutoConfigWidget;
  d->autoWidgets.insert(widget, newAutoConfigWidget );
}

void KAutoConfig::ignoreSubWidget(QWidget *widget){
  d->ignore.append(widget);
}

void KAutoConfig::retrieveSettings(){
  // Go through all of the children of the widgets and find all known widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    parseChildren(widget, d->autoWidgets[widget]);
  }
}

bool KAutoConfig::saveSettings() {
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
      QVariant savedValue = config->readPropertyEntry(groupWidget->name(),
		      defaultValue);
      
      if(currentValue == defaultValue && savedValue != currentValue){
        config->deleteEntry(groupWidget->name());
        widgetChanged = true;
      }
      else{
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
    return true;
  }
  return false;
}

void KAutoConfig::resetSettings(){
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

void KAutoConfig::parseChildren(const QWidget *widget,
		                QPtrList<QWidget>& currentGroup){
  const QPtrList<QObject> *listOfChildren = widget->children();
  if(!listOfChildren)
    return;
 
  QSqlPropertyMap *propertyMap = QSqlPropertyMap::defaultMap();
  QPtrListIterator<QObject> it( *listOfChildren );
  QObject *object;
  while ( (object = it.current()) != 0 ) {
    ++it;
    if(!object->isWidgetType())
      continue;
    QWidget *childWidget = (QWidget *)object;
    if(d->ignore.containsRef(childWidget))
      continue;
   
    bool parseTheChildren = true;
    if( d->ignoreTheseWidgets[childWidget->className()] == 0){
      QVariant p = propertyMap->property(childWidget);
      if(p.isValid()){
        parseTheChildren = false;
	if(config->entryIsImmutable( childWidget->name()))
	  childWidget->setEnabled(false); 
        else{
          currentGroup.append(childWidget);
	  d->defaultValues.insert(childWidget, p);
        }
        propertyMap->setProperty(childWidget,
		       config->readPropertyEntry(childWidget->name(), p));
      }
    }
    if(parseTheChildren){
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      parseChildren(childWidget, currentGroup);
    }
  }
}

