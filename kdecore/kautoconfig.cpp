#include "kautoconfig.h"

#include <kglobal.h>
#include <qsqlpropertymap.h>
#include <qobjectlist.h>
#ifndef NDEBUG
#include "kdebug.h"
#endif
#include <kconfig.h>
#include <kapplication.h>

class KAutoConfig::KAutoConfigPrivate {

public:
  KAutoConfigPrivate() : changed(false)
#ifndef NDEBUG
	  , retrievedSettings(false)
#endif
  { }

  // Widgets to parse
  QPtrList<QWidget> widgets;
  // Name of the group that KConfig should be set to for each widget.
  QMap<QWidget*, QString> groups;

  // Child widgets of widgets to ignore
  QPtrList<QWidget> ignore;

  // reset to false after saveSettings returns true.
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
    ignoreTheseWidgets.setAutoDelete(true);

    static bool defaultKDEPropertyMapInstalled = false;
    if ( !defaultKDEPropertyMapInstalled && kapp ) {
      kapp->installKDEPropertyMap();
      defaultKDEPropertyMapInstalled = true;
    }
  }
};

KAutoConfig::KAutoConfig(KConfig *kconfig, QObject *parent, const char *name):
			 QObject(parent, name), config(kconfig) {
  d = new KAutoConfigPrivate();
  d->init();
}

KAutoConfig::KAutoConfig(QObject *parent, const char *name) :
			 QObject(parent, name), config(KGlobal::config()) {
  d = new KAutoConfigPrivate();
  d->init();
}

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

bool KAutoConfig::retrieveSettings(bool trackChanges){
#ifndef NDEBUG
  if(d->retrievedSettings){
      kdDebug() << "This should not happen.  Function KAutoConfig::retrieveSettings() called more then once, returning first.  It is not recomended unless you know the implimentation details of KAutoConfig and what this breaks.  Please fix." << endl;
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

    // KDE
    changedMap.insert( "KComboBox", SIGNAL(activated (int)));
    changedMap.insert( "KFontCombo", SIGNAL(activated (int)));
    changedMap.insert( "KFontRequester", SIGNAL(fontSelected(const QFont &)));
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
    changedMap.insert( "KURLRequester",  SIGNAL(textChanged ( const QString& )));
    changedMap.insert( "KIntNumInput", SIGNAL(valueChanged (int)));
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
#ifndef NDEBUG
  if(!d->retrievedSettings){
      kdDebug() << "This should NEVER happen.  Function KAutoConfig::saveSettings() called before retrieveSettings. Please Fix." << endl;
    return false;
  }
#endif

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
    // Because no app has ever segfaulted.
    config->sync();
    return true;
  }
  return false;
}

bool KAutoConfig::hasChanged() const {
#ifndef NDEBUG
  if(!d->retrievedSettings){
      kdDebug() << "This should NEVER happen.  Function KAutoConfig::hasChanged() called before retrieveSettings. Please Fix." << endl;
    return false;
  }
#endif

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

      if((currentValue == defaultValue && savedValue != currentValue) ||
         (savedValue != currentValue))
        return true;
    }
  }
  return false;
}

bool KAutoConfig::isDefault() const {
#ifndef NDEBUG
  if(!d->retrievedSettings){
      kdDebug() << "This should NEVER happen.  Function KAutoConfig::hasChanged() called before retrieveSettings. Please Fix." << endl;
    return false;
  }
#endif

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

void KAutoConfig::resetSettings(){
#ifndef NDEBUG
  if(!d->retrievedSettings){
      kdDebug() << "This should NEVER happen.  Function KAutoConfig::resetSettings() called before retrieveSettings. Please Fix." << endl;
    return;
  }
#endif

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
        emit (widgetModified());
	d->changed = true;
      }
    }
  }
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
  while ( (object = it.current()) != 0 ) {
    ++it;
    if(!object->isWidgetType())
      continue;
    QWidget *childWidget = (QWidget *)object;
    if(d->ignore.containsRef(childWidget))
      continue;

    bool parseTheChildren = true;
    if( d->ignoreTheseWidgets[childWidget->className()] == 0 &&  
	childWidget->name(0) != NULL ){
      QVariant defaultSetting = propertyMap->property(childWidget);
      if(defaultSetting.isValid()){
        parseTheChildren = false;
	if(config->entryIsImmutable( childWidget->name()))
	  childWidget->setEnabled(false);
        else{
          currentGroup.append(childWidget);
	  d->defaultValues.insert(childWidget, defaultSetting);
        }
        QVariant setting =
	  config->readPropertyEntry(childWidget->name(), defaultSetting);
	if(setting != defaultSetting){
	  propertyMap->setProperty(childWidget, setting);
	  valueChanged = true;
	  if(trackChanges) emit void widgetModified();
        }

	if(trackChanges && changedMap.find(childWidget->className()) != changedMap.end())
	  connect(childWidget, changedMap[childWidget->className()], SIGNAL(widgetModified()));
#ifndef NDEBUG
	else if(trackChanges && changedMap.find(childWidget->className()) == changedMap.end())
            kdDebug() << "KAutoConfig::retrieveSettings, Unknown changed signal for widget:" << childWidget->className() << endl;
#endif

      }
#ifndef NDEBUG
      else
          kdDebug() << "KAutoConfig::retrieveSettings, Unknown widget:" << childWidget->className() << endl;
#endif
    }
    if(parseTheChildren){
      // this widget is not known as something we can store.
      // Maybe we can store one of its children.
      valueChanged |= parseChildren(childWidget, currentGroup, trackChanges);
    }
  }
  return valueChanged;
}

#include "kautoconfig.moc"

