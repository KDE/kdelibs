#ifndef KAUTOCONFIG_H
#define KAUTOCONFIG_H

#include <qobject.h>
#include <kglobal.h>
#include <qptrlist.h>

class KConfig;
class QWidget;
class QSqlPropertyMap;

/**
 * @author Benjamin C Meyer <ben@meyerhome.net>
 * 
 * The KAutoConfig class provides a means of automaticly retrieving,
 * saving and reseting basic settings.
 *
 * When told to retrieve settings KAutoConfig will traverse the specified
 * widgets building a list of all known widgets that haven't been marked to
 * be ignore.  If a setting is immutible the value is loaded and it is disabled.
 * 
 * When save or reset is called KAutoConfig goes through the list of known
 * widgets and performs the operation on each of them.
 *
 * KAutoConfig uses the QSqlPropertyMap class to determine if it can do
 * anything with a widget.  New widgets can be added using  
 * QSqlPropertyMap::installDefaultMap(). 
 *
 * For example (note that KColorButton is already known and you don't need to
 * do this if you have a KColorButton):
 * 
 * QSqlPropertyMap *map = QSqlPropertyMap::defaultMap();
 * map.insert("KColorButton", "color");
 * QSqlPropertyMap::installDefaultMap(map);
 * 
 * If one of the widgets needs special treatment it can be specified to be
 * ignored.  
 * 
 * The name of the widget determines the name of the setting.  The initial
 * value of the widget also is the default when asked to reset.
 *
 * ---
 * 
 * This class was created to further follow the DRY principle.
 * DRY - Don't Repeat Yourself
 *
 *   Every Piece of knowledge must have a single, unambiguous, authoritative
 *   representation within a system.
 * 
 * The majority of settings files are ui files wrapped in a KDialogBase.  
 * They may have a little logic, but a lot of the code is adding the save, 
 * retrieve, and reset functions.  With every new widget many lines of code 
 * need to be added.  This leads to errors and bugs.  From simple bugs like
 * saving two objects to the same setting, to the time spent coding it this 
 * task can and should be automated.
 *   Some areas of duplication include: 
 * -Having the ui files specify default values, a function to
 *  set all the default values and readEntry specify a default value.  
 * -Naming the objects in the ui file the same as the settings.
 * -Simply coding everything over and over for every new application.
 *  
 * In KControl, when objects exit they often asks the user if they want to apply
 * the settings that the user has changed.  To add this functionality by hand,
 * code must be written comparing the old and new values.  One simpler
 * way that some have done is simply to watch for changed() signal's, but
 * this isn't always correct for a user may change a value, but then change
 * it back before hitting OK.  With KAutoConfig saveSettings() returns a bool
 * value if anything has changed and emits a signal.
 **/ 

class KAutoConfig : public QObject {

Q_OBJECT

signals:
  /**
   * One or more of the settings have been changed.
   * Is only emited by saveSettings if some setting was changed.
   */ 
  void settingsChanged();

  /**
   * One of the known options has changed.
   * @param QWidget* the widget group (pass in via addWidget) that contains
   * that setting that has changed.
   */
  void settingsChanged(QWidget*);
	
public:
  /**
   * Constructor.
   * @param kconfig - Config to use when retrieving/saving all of the widgets
   *        that it knows about.
   * @param loadAllKnown - Automaticly loads all known widgets.
   *        If false no widgets are loaded by default, widgets need to be
   *        manually added using addKAutoConfigWidget(...).  Usefull in cases
   *        where a dialog only has widgets that arn't of the default type.
   */ 
  KAutoConfig(QObject *parent=0, const char *name=0,
	      KConfig *kconfig = KGlobal::config());	
  
  /**
   * Destructor.
   */
  ~KAutoConfig();
  
  /**
   * Adds a widget to the list of widgets that should be parsed for any 
   * children that KAutoConfig might know.
   * @param widget - Pointer to the widget.
   * @param group - Name of the group from which all of the settings for this
   * widget will be located.  If a child of widget needs to be in a separate
   * group add it separately and also ignore it.
   */ 
  void addWidget(QWidget *widget, const QString &group);

  /**
   * Ignore the specified child widget when performing an action.  Doesn't
   * effect widgets that were added with addWidget only their children.
   * @param - Pointer to the widget that should be ignored.  
   **/ 
  void ignoreSubWidget(QWidget *widget);

public slots:
  /**
   * Traverse the specified widgets, retrieve the settings for all known
   * widgets that aren't being ignored.
   */ 
  void retrieveSettings();
  
  /**
   * Traverse the specified widgets, saving the settings for all known
   * widgets that aren't being ignored.  retrieveSettings() must be called
   * before this function to build the list of known widgets and defaultValues.
   * @return bool - true if any settings were changed.
   */ 
  bool saveSettings();

  /**
   * Traverse the specified widgets, reseting the settings for all known
   * widgets that aren't being ignored.  retrieveSettings() must be called
   * before this function to build the list of known widgets and defaultValues.
   */
  void resetSettings(); 

protected:
  KConfig *config;
  QSqlPropertyMap *propertyMap;
  
  /**
   * Recursive function that finds all known children.
   * Goes through the children of widget and if any are known, stores them in
   * currentGroup.
   * @param widget - Parent of the children to look at.
   * @param currentGroup - Place to store known children of widget.
   */ 
  void parseChildren(const QWidget *widget,
		  QPtrList<QWidget>&currentGroup);
 
private:
  class KAutoConfigPrivate;
  KAutoConfigPrivate *d;

};

#endif

