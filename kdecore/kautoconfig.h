#ifndef KAUTOCONFIG_H
#define KAUTOCONFIG_H

#include <qwidget.h>
#include <qstring.h>
#include <kglobal.h>

class KConfig;

/**
 * @author Benjamin C Meyer <ben@meyerhome.net>
 * 
 * The KAutoConfig class provides a means of automaticly retrieving,
 * saving and reseting basic settings.
 *
 * When told to retrieve/save/reset settings KAutoConfig will traverse the
 * specified widgets performing the requested task on all known widgets that
 * haven't been marked as ignore.
 *
 * At this time KAutoConfig supports:
 * QCheckBox, QSpinBox, QLineEdit, QComboBox, QRadioButton*, KColorButton
 * If one of the above widgets needs special treatment it can be specified
 * to be ignored.
 * 
 * *QRadioButton objects will only get saved if it is checked.  The name of the
 * settings will not be the object, but the parent's name (i.e. the 
 * QButtonGroup's name)
 *
 * The name of the widget determines the name of the setting.  The initial
 * value of the widget also is the default when asked to reset.
 *
 * This class was created to further follow the DRY principle.
 * DRY - Don't Repeat Yourself
 *
 *   Every Piece of knowledge must have a single, unambiguous, authoritative
 *   representation within a system.
 * 
 * The majority of settings files are ui files wrapped in a KJanusWidget.  
 * They may have a little logic, but a lot of the code is adding the save, 
 * retrieve, and reset functions.  With every new widget many lines of code 
 * need to be added.  This leads to errors and bugs.  From simple bugs like
 * saving two objects to the same setting, to the time spent coding it this 
 * task can and should be automated.
 *   Some areas of duplication include: 
 * -Having the ui files specify default values, a function to
 *  set all the default values and readEntry specify a default value.  
 * -Naming the objects in the ui file the same as the settings.
 * -Simply doing it over and over for every new application.
 * Using KAutoConfig also leads to a nice side effect of smaller binaries. 
 * 
 * In KControl, objects when exiting often asks the user if they want to apply
 * the settings that the user has changed.  To add this functionality by hand
 * quite a bit of code must be added comparing the old and new values.  One
 * simpler way that some have done is simply to watch for change signal's, but
 * this isn't always correct for a user may change a value, but then change
 * it back.  With KAutoConfig saveSettings() returns a bool value if anything
 * has changed.  The best part is that adding the return took less then ten
 * lines of code.
 *
 * The only major downside to this class is that it is almost guaranteed (?)
 * to be slower then the normal implimentaions.  Thus the design of the class
 * is geared towards improving the speed of its operation.
 *
 **/ 

class KAutoConfig {

public:
  /**
   * Constructor.  Note that delete must be called because this is not a QObject
   */ 
  KAutoConfig(KConfig *kconfig = KGlobal::config());	
  
  /**
   * Destructor.
   */
  ~KAutoConfig();
  
  /**
   * Adds a widget to the list of widgets that should be parsed when an action
   * is called.
   * @param widget - Pointer to the widget.
   * @param group - Name of the group from which all of the  settings for this
   * widget will be located.  If a child of widget needs to be in a separate
   * group add it separately and ignore it.
   */ 
  void addWidget(QWidget *widget, QString group);

  /**
   * Ignore the specified child widget when performing an action.  Doesn't
   * effect widgets that were added with addWidget only their children.
   * @param - Pointer to the widget that should be ignored.  
   **/ 
  void ignoreSubWidget(QWidget *widget);


  /**
   * Traverse the specified widgets, retrieve the settings for all known
   * widgets that aren't being ignored.
   */ 
  void retrieveSettings();
  
  /**
   * Traverse the specified widgets, saving the settings for all known
   * widgets that aren't being ignored.  retrieveSettings() must be called
   * before this function to build the list of defaults.
   * @return bool - true if any settings were changed.
   */ 
  bool saveSettings();
  
  /**
   * Traverse the specified widgets, reseting the settings for all known
   * widgets that aren't being ignored.  retrieveSettings() must be called
   * before this function to build the list of defaults.
   */
  void resetSettings(); 

private:
  KConfig *config;
  
  /**
   * Recursive function that does all the grunt work.
   * Goes through all of the children to widget and if any of them are known
   * performs the opteration on that child otherwise calls
   * parseWidget(child, op) with the child if it is a QWidget.  The first
   * time this function is called op _must_ be retrieving the data or else
   * there will be a segfault do to pointer errors.
   * @param widget - parent of the children to look at.
   * @param op - operation to perform on the children of widget.
   * @return bool - returns true if values have been changed, value is only
   * computed if op = SAVE otherwise always return false.
   */ 
  bool parseWidget(QWidget *widget, int op );

  class KAutoConfigPrivate;
  KAutoConfigPrivate *d;
};

#endif

