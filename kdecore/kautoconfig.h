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
#ifndef KAUTOCONFIG_H
#define KAUTOCONFIG_H

#include <qobject.h>
#include <qptrlist.h>

class KConfig;
class QWidget;

/**
 * @author Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
 * 
 * The KAutoConfig class provides a means of automaticly retrieving,
 * saving and reseting basic settings.  It also can emit signals when
 * settings have been changed (settings were saved) or modified (the
 * user changes a checkbox from on to off).
 *
 * When told to retrieve settings ( retrieveSettings()) KAutoConfig 
 * will traverse the specified widgets building a list of all known widgets
 * that have a name and havn't been marked to be ignored.
 * If a setting is marked immutable the value is loaded and the widget is
 * disabled.
 *
 * The name of the widget determines the name of the setting.  The initial
 * value of the widget also is the default value when the widget is reset.
 * If the widget does not have a name then it is ignored.
 *
 * When saveSettings() or resetSettings() is called KAutoConfig
 * goes through the list of known widgets and performs the operation on each
 * of them.
 *
 * If one of the widgets needs special treatment it can be specified to be
 * ignored using the ignoreSubWidget() function.  
 *
 * <hr>
 * 
 * KAutoConfig uses the QSqlPropertyMap class to determine if it can do
 * anything to a widget.  Note that KAutoConfig doesn't  require a database,
 * it simply uses the functionality that is built into the QSqlPropertyMap
 * class.  New widgets can be added to the map using  
 * QSqlPropertyMap::installDefaultMap().  Note that you can't just add any
 * class.  The class must have a matching Q_PROPERTY(...) macro defined.
 * 
 * For example (note that KColorButton is already added and it doesn't need to 
 * manually added):
 *
 * kcolorbutton.h defines the following property:
 * \code
 * Q_PROPERTY( QColor color READ color WRITE setColor )
 * \endcode
 *
 * To add KColorButton the following code would be inserted in the main.
 * 
 * \code
 * QSqlPropertyMap *map = QSqlPropertyMap::defaultMap();
 * map.insert("KColorButton", "color");
 * QSqlPropertyMap::installDefaultMap(map);
 * \endcode
 *
 * If you add a new widget to the QSqlPropertyMap and wish to be notified when
 * it is modified you should add its signal using addWidgetChangedSignal().
 * If the Apply and Default buttons and enabled/disabled by KAutoConfigDialog
 * automaticly than this must be done.
 *
 * @see KAutoConfigDialog
 * @since 3.2
 */ 
class KAutoConfig : public QObject {

Q_OBJECT

signals:
  /**
   * One or more of the settings have been saved (such as when the user
   * clicks on the Apply button).  This is only emitted by saveSettings()
   * whenever one or more setting were changed and consequently saved.
   */ 
  void settingsChanged();

  /**
   * One or more of the settings have been changed.
   * @param widget - The widget group (pass in via addWidget()) that
   * contains the one or more modified setting.
   * @see settingsChanged()
   */
  void settingsChanged( QWidget *widget );
	
  /**
   * If retrieveSettings() was told to track changes then if
   * any known setting was changed this signal will be emitted.  Note
   * that a settings can be modified several times and might go back to the 
   * original saved state. hasChanged() will tell you if anything has
   * actually changed from the saved values.
   */
  void widgetModified();


public:
  /**
   * Constructor.
   * @param kconfig - KConfig to use when retrieving/saving the widgets
   * that KAutoConfig knows about.
   * @param parent - Parent object.
   * @param name - Object name.
   */ 
  KAutoConfig( KConfig *kconfig, QObject *parent=0, const char *name=0 );	
 
  /**
   * Constructor.
   * Uses KGlobal::config() when retrieving/saving the widgets that
   * KAutoConfig knows about.
   * @param parent - Parent object.
   * @param name - Object name.
   */ 
  KAutoConfig( QObject *parent=0, const char *name=0 );

  /**
   * Destructor.  Deletes private class.
   */
  ~KAutoConfig();
  
  /**
   * Adds a widget to the list of widgets that should be parsed for any 
   * children that KAutoConfig might know when retrieveSettings() is
   * called.
   * @param widget - Pointer to the widget to add.
   * @param group - Name of the group from which all of the settings for this
   * widget will be located.  If a child of 'widget' needs to be in a separate
   * group it should be added separately and also ignored.
   * @see ignoreSubWidget()
   */ 
  void addWidget( QWidget *widget, const QString &group );

  /**
   * Ignore the specified child widget when performing an action.  Doesn't
   * effect widgets that were added with addWidget() only their children.
   * @param widget - Pointer to the widget that should be ignored.
   * Note: Widgets that don't have a name are ignored automatically.
   **/ 
  void ignoreSubWidget( QWidget *widget );

  /**
   * Traverse the specified widgets to see if anything is different then the
   * current settings. retrieveSettings() must be called before this 
   * function to build the list of known widgets and default values.
   * @return bool - True if any settings are different then the stored values.
   */
  bool hasChanged() const;

  /**
   * Traverse the specified widgets to see if anything is different then the
   * default. retrieveSettings() must be called before this function to
   * build the list of known widgets and default values.
   * @return bool - True if all of the settings are their default values.
   */
  bool isDefault() const;

  /**
   * Adds a widget and its signal to the internal list so that when
   * KAutoConfig finds widgetName in retrieveSettings() it will know
   * how to connect its signal that it has changed to KAutoConfig's signal
   * widgetModified().  This function should be called before 
   *
   * Example:
   * \code
   * addWidgetChangedSignal( "QCheckbox", SIGNAL(stateChanged(int)) );
   * \endcode
   *
   * This is generally used in conjunction with the addition of a class
   * to QSqlPropertyMap so KAutoConfig can get/set its values.
   *
   * @param widgetName - The class name of the widget (className()).
   * @param signal - The signal (with "SIGNAL()" wrapper) that should be called.
   */ 
  inline void addWidgetChangedSignal( const QString &widgetName,
		  const QCString &signal){
    changedMap.insert( widgetName, signal );
  }


public slots:
  /**
   * Traverse the specified widgets, retrieving the settings for all known
   * widgets that aren't being ignored and storing the default values.
   * @param trackChanges - If any changes by the widgets should be tracked
   * set true.  This causes the emitting the modified() signal when
   * something changes.
   * @return bool - True if any setting was changed from the default.
   */ 
  bool retrieveSettings( bool trackChanges=false );
  
  /**
   * Traverse the specified widgets, saving the settings for all known
   * widgets that aren't being ignored. retrieveSettings() must be called
   * before this function to build the list of known widgets and default values.
   * @return bool - True if any settings were changed.
   */ 
  bool saveSettings();

  /**
   * Traverse the specified widgets, reseting the settings to their default
   * values for all known widgets that aren't being ignored.
   * retrieveSettings() must be called before this function to build
   * the list of known widgets and default values.
   */
  void resetSettings(); 


protected:
  /**
   * KConfigBase object used to get/save values.
   */
  KConfig *config;
  /**
   * Map of the classes and the signals that they emit when changed. 
   */
  QMap<QString, QCString> changedMap;

  /**
   * Recursive function that finds all known children.
   * Goes through the children of widget and if any are known and not being 
   * ignored, stores them in currentGroup.  Also checks if the widget
   * should be disabled because it is set immutable.
   * @param widget - Parent of the children to look at.
   * @param currentGroup - Place to store known children of widget.
   * @param trackChanges - If true then tracks any changes to the children of
   * widget that are known.
   * @return bool - If a widget was set to something other then its default.
   * @see retrieveSettings()
   */ 
  bool parseChildren( const QWidget *widget,
		  QPtrList<QWidget>&currentGroup, bool trackChanges );


private:
  class KAutoConfigPrivate;
  /**
   * KAutoConfig Private class.
   */ 
  KAutoConfigPrivate *d;

};

#endif // KAUTOCONFIG_H

