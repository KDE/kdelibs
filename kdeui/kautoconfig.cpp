#include "kautoconfig.h"
#include "kautoconfigwidgets.h"

#include <qobjectlist.h> 
#include <qwidget.h>
#include <kconfig.h>

KAutoConfigWidget::KAutoConfigWidget( KConfig *kconfig, QWidget *widget) :
                                      config(kconfig) {
  if(widget)
    widgetName = widget->name();
};

// For KAutoConfigPrivate
#include <qptrlist.h>
#include <qmap.h>
#include <qasciidict.h> 

class KAutoConfig::KAutoConfigPrivate {
public:
  KAutoConfigPrivate() : changed(false) { }
 
  // Known widgets that can be configured
  QMap<QWidget*, QPtrList<KAutoConfigWidget> > autoWidgets;
  // List of foudn widgets of the QPtrList widgets
  QAsciiDict<KAutoConfigWidget> knownKAutoCWidgets;
  
  // Widgets to parse
  QPtrList<QWidget> widgets;
  // Child widgets of widgets to ignore
  QPtrList<QWidget> ignore;
  
  // Name of the group that KConfig should be set to for each widget. 
  QMap<QWidget*, QString> groups;
  // reset to false after saveSettings returns true.
  bool changed;
};

KAutoConfig::KAutoConfig(QObject *parent, const char *name,
		         KConfig *kconfig, bool loadAllKnown) :
			 QObject(parent, name), config(kconfig) {
  d = new KAutoConfigPrivate();
  if(loadAllKnown){
    // These are the basic supported as listed in autowidgets.h
    d->knownKAutoCWidgets.insert("QCheckBox", new KAutoConfig_QCheckBox(config));
    d->knownKAutoCWidgets.insert("QRadioButton", new KAutoConfig_QRadioButton(config));
    d->knownKAutoCWidgets.insert("QSpinBox", new KAutoConfig_QSpinBox(config) );
    d->knownKAutoCWidgets.insert("QLineEdit", new KAutoConfig_QLineEdit(config) );
    d->knownKAutoCWidgets.insert("QComboBox", new KAutoConfig_QComboBox(config) );
    d->knownKAutoCWidgets.insert("QSlider", new KAutoConfig_QSlider(config) );
    d->knownKAutoCWidgets.insert("KColorButton", new KAutoConfig_KColorButton(config) );
    d->knownKAutoCWidgets.setAutoDelete(true);
  }
};

KAutoConfig::~KAutoConfig(){
  delete d;
}

void KAutoConfig::addKAutoConfigWidget(const char* name,
		KAutoConfigWidget *widget){
  widget->setConfig(config);
  d->knownKAutoCWidgets.insert(name, widget);
}

void KAutoConfig::addWidget(QWidget *widget, const QString &group){
  d->widgets.append(widget);
  d->groups.insert(widget, group);
  QPtrList<KAutoConfigWidget> newAutoConfigWidget;
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
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);

    // Go through the known autowidgets of this widget and save
    QPtrListIterator<KAutoConfigWidget> it( d->autoWidgets[widget] );
    KAutoConfigWidget *autoWidget;
    bool widgetChanged = false;
    while ( (autoWidget = it.current()) != 0 ){
      ++it;
      widgetChanged |= autoWidget->saveSettings();
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
  // Go through all of the widgets
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    // Go through the known autowidgets of this widget and reset
    QPtrListIterator<KAutoConfigWidget> it( d->autoWidgets[widget] );
    KAutoConfigWidget *autoWidget;
    while ( (autoWidget = it.current()) != 0 ) {
      ++it;
      d->changed |= autoWidget->resetSettings();
    }
  }
}

void KAutoConfig::parseChildren(const QWidget *widget,
		                QPtrList<KAutoConfigWidget>& currentGroup){
  const QPtrList<QObject> *listOfChildren = widget->children();
  if(!listOfChildren)
    return;
  
  QPtrListIterator<QObject> it( *listOfChildren );
  QObject *object;
  while ( (object = it.current()) != 0 ) {
    ++it;
    if(!object->isWidgetType())
      continue;
    QWidget *childWidget = (QWidget *)object;
    if(d->ignore.containsRef(childWidget))
      continue;
    
    const KAutoConfigWidget *isKnown=d->knownKAutoCWidgets[object->className()];
    if(isKnown != NULL){
      // We know about it so set the initial value.
      KAutoConfigWidget *newKAutoConfigWidget =
	      isKnown->makeKAutoConfigWidget(childWidget);
      newKAutoConfigWidget->retrieveSettings();
      if(config->entryIsImmutable( object->className())){
	// If we can't change anything don't even bother keeping track of it.
        delete newKAutoConfigWidget;
	childWidget->setEnabled(false); 
      }
      else
        currentGroup.append(newKAutoConfigWidget);
    }
    else{
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      parseChildren(childWidget, currentGroup);
    }
  }
}

