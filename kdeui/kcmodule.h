/*
   This file is part of the KDE libraries

   Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/
#ifndef __KCMODULE_H__
#define __KCMODULE_H__

#include <qwidget.h>
class KAboutData;
class KCModulePrivate;

/**
 * The base class for control center modules.
 *
 * In KDE 2.0, control center modules are realized as shared
 * libraries that are loaded into the control center at
 * runtime.
 *
 * The module in principle is a simple widget displaying the
 * item to be changed. The module has a very small interface.
 *
 * All the necessary glue logic and the GUI bells and whistles
 * are provided by the control center and must not concern
 * the module author.
 *
 * To write a config module, you have to create a library
 * that contains at least one factory function like this:
 *
 * <pre>
 *   extern "C" {
 *     KCModule *create_xyz(QWidget *parent, const char *name)
 *     {
 *       return new XYZ(parent, name);
 *     }
 *   }
 * </pre>
 *
 * See kdebase/kcontrol/HOWTO for more detailed documentation.
 *
 * @author Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 */

class KCModule : public QWidget
{
  Q_OBJECT

public:

  /**
   * An enumeration type for the buttons used by this module.
   *
   * @see KCModule::buttons @see KCModule::setButtons
   */
  enum Button {Help=1, Default=2, Apply=16,
               Reset=4, /* obsolete, do not use! */
               Cancel=8, /* obsolete, do not use! */
               Ok=32, /* obsolete, do not use! */
  	       SysDefault=64 /* obsolete, do not use! */ };

  /*
   * Creates a new module.
   */
  KCModule(QWidget *parent=0, const char *name=0)
	: QWidget(parent, name), _btn(Help|Default|Apply) {}
  /**
   * Load the configuration data into the module.
   *
   * The load method sets the user interface elements of the
   * module to reflect the current settings stored in the
   * configuration files.
   *
   * This method is invoked whenever the module should read its configuration
   * (most of the times from a config file) and update the user interface.
   * This happens when the user clicks the "Reset" button in the control
   * center, to undo all of his changes and restore the currently valid
   * settings. NOTE that this is not called after the modules is loaded,
   * so you probably want to call this method in the constructor.
   */
  virtual void load() {};

  /**
   * Save the configuration data.
   *
   * The save method stores the config information as shown
   * in the user interface in the config files.
   *
   * If necessary, this method also updates the running system,
   * e.g. by restarting applications.
   *
   * save is called when the user clicks "Apply" or "Ok".
   */
  virtual void save() {};

  /**
   * Set the configuration to sensible default values.
   *
   * This method is called when the user clicks the "Default"
   * button. It should set the display to useful values.
   */
  virtual void defaults() {};

  /**
   * Set the configuration to system default values.
   *
   * This method is called when the user clicks the "System-Default"
   * button. It should set the display to the system default values.
   *
   * NOTE: The default behaviour is to call defaults().
   */
  virtual void sysdefaults() { defaults(); };

  /**
   * Return a quick-help text.
   *
   * This method is called when the module is docked.
   * The quick-help text should contain a short description of the module and
   * links to the module's help files. You can use QML formating tags in the text.
   *
   * NOTE: Please make sure the quick help text gets translated (use i18n()).
   */
  virtual QString quickHelp() const { return QString::null; };

  /**
   * Returns a the KAboutData for this module
   * This is generally only called for the KBugReport.
   * Override and have it return a pointer to a constant
   */
  virtual const KAboutData *aboutData() const { return 0; }

  /**
   * Realizes the settings in the config files.
   *
   * This method may be called during system startup to apply the
   * information in the config files to the running system.
   *
   * Note that this method is static, so it is not necessary to
   * create an instance of the module at system startup.
   * Generally, it should be avoided to construct GUI elements
   * in this method.
   */
  static void init() {};

  /**
   * Indicate which buttons will be used.
   *
   * The return value is a value or'ed together from
   * the Button enumeration type.
   *
   * @see KCModule::setButtons
   */
  int buttons() const { return _btn; };


signals:

  /**
   * Indicate that the state of the modules contents has changed.
   *
   * This signal is emitted whenever the state of the configuration
   * shown in the module changes. It allows the control center to
   * keep track of unsaved changes.
   *
   */
  void changed(bool state);

  /**
   * Indicate that the module's quickhelp has changed.
   *
   * Emit this signal whenever the module's quickhelp changes.
   * Modules implemented as tabbed dialogs might want to implement
   * per-tab quickhelp for example.
   *
   */
  void quickHelpChanged();

protected:

  /**
   * Set the buttons to display.
   *
   * Help: shows a "Help" button.
   * Default: shows a "Use Defaults" button
   * Apply: in kcontrol this will show an "Apply" and "Reset" button
   *        in kcmshell this will show an "Ok", "Apply" and "Cancel" button
   *
   * If Apply is not specified, kcmshell will show a "Close" button.
   *
   * @see KCModule::buttons
   */
  void setButtons(int btn) { _btn = btn; };

private:

  int _btn;
  KCModulePrivate *d;

};

#endif
