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
#ifndef KCONFIGDIALOGMANAGER_H
#define KCONFIGDIALOGMANAGER_H

#include <qobject.h>
#include <qmap.h>
#include <q3ptrlist.h>
#include "kdelibs_export.h"

class KConfigSkeleton;
class KConfigSkeletonItem;
class QWidget;
class Q3SqlPropertyMap;

/**
 * @short Provides a means of automatically retrieving,
 * saving and resetting KConfigSkeleton based settings in a dialog.
 *
 * The KConfigDialogManager class provides a means of automatically
 * retrieving, saving and resetting basic settings.
 * It also can emit signals when settings have been changed
 * (settings were saved) or modified (the user changes a checkbox
 * from on to off).
 *
 * The names of the widgets to be managed have to correspond to the names of the
 * configuration entries in the KConfigSkeleton object plus an additional
 * "kcfg_" prefix. For example a widget named "kcfg_MyOption" would be
 * associated to the configuration entry "MyOption".
 *
 * KConfigDialogManager uses the QSqlPropertyMap class to determine if it can do
 * anything to a widget.  Note that KConfigDialogManager doesn't  require a
 * database, it simply uses the functionality that is built into the
 * QSqlPropertyMap class.  New widgets can be added to the map using
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
 * kapp->installKDEPropertyMap();
 * QSqlPropertyMap *map = QSqlPropertyMap::defaultMap();
 * map->insert("KColorButton", "color");
 * \endcode
 *
 * If you add a new widget to the QSqlPropertyMap and wish to be notified when
 * it is modified you should add its signal using addWidgetChangedSignal().

 * @since 3.2
 * @author Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
 * @author Waldo Bastian <bastian@kde.org>
 */
class KDEUI_EXPORT KConfigDialogManager : public QObject {

Q_OBJECT

Q_SIGNALS:
  /**
   * One or more of the settings have been saved (such as when the user
   * clicks on the Apply button).  This is only emitted by updateSettings()
   * whenever one or more setting were changed and consequently saved.
   */
  void settingsChanged();

  /**
   * TODO: Verify
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
   * @param parent  Dialog widget to manage
   * @param conf Object that contains settings
   */
   KConfigDialogManager(QWidget *parent, KConfigSkeleton *conf);

  /**
   * Destructor.
   */
  ~KConfigDialogManager();

  /**
   * Add additional widgets to manage
   * @param widget Additional widget to manage, inlcuding all its children
   */
  void addWidget(QWidget *widget);

  /**
   * Returns whether the current state of the known widgets are
   * different from the state in the config object.
   */
  bool hasChanged();

  /**
   * Returns whether the current state of the known widgets are
   * the same as the default state in the config object.
   */
  bool isDefault();

public Q_SLOTS:
  /**
   * Traverse the specified widgets, saving the settings of all known
   * widgets in the settings object.
   *
   * Example use: User clicks Ok or Apply button in a configure dialog.
   */
  void updateSettings();

  /**
   * Traverse the specified widgets, sets the state of all known
   * widgets according to the state in the settings object.
   *
   * Example use: Initialisation of dialog.
   * Example use: User clicks Reset button in a configure dialog.
   */
  void updateWidgets();

  /**
   * Traverse the specified widgets, sets the state of all known
   * widgets according to the default state in the settings object.
   *
   * Example use: User clicks Defaults button in a configure dialog.
   */
  void updateWidgetsDefault();

protected:

  /**
   * @param trackChanges - If any changes by the widgets should be tracked
   * set true.  This causes the emitting the modified() signal when
   * something changes.
   * TODO: @return bool - True if any setting was changed from the default.
   */
  void init(bool trackChanges);

  /**
   * Recursive function that finds all known children.
   * Goes through the children of widget and if any are known and not being
   * ignored, stores them in currentGroup.  Also checks if the widget
   * should be disabled because it is set immutable.
   * @param widget - Parent of the children to look at.
   * @param trackChanges - If true then tracks any changes to the children of
   * widget that are known.
   * @return bool - If a widget was set to something other then its default.
   */
  bool parseChildren(const QWidget *widget, bool trackChanges);

  /**
   * Set a property
   */
  void setProperty(QWidget *w, const QVariant &v);

  /**
   * Retrieve a property
   */
  QVariant property(QWidget *w);

  /**
   * Setup secondary widget properties
   */
  void setupWidget(QWidget *widget, KConfigSkeletonItem *item);

protected:
  /**
   * KConfigSkeleton object used to store settings
   */
  KConfigSkeleton *m_conf;

  /**
   * Dialog being managed
   */
  QWidget *m_dialog;

  /**
   * Pointer to the property map for easy access.
   */
  Q3SqlPropertyMap *propertyMap;

  /**
   * Map of the classes and the signals that they emit when changed.
   */
  QMap<QString, QByteArray> changedMap;

private:
  class Private;
  /**
   * KConfigDialogManager Private class.
   */
  Private *d;

};

#endif // KCONFIGDIALOGMANAGER_H

