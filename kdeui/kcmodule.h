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
#include <qdialog.h>

class QPushButton;
class QFrame;


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
 * @author Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 */

class KCModule : public QWidget
{
  Q_OBJECT

public:

  /**
   * An enumeration type for the buttons used by this module.
   * 
   * @see KCModule::buttons
   */
  enum Button {Help=1, Default=2, Reset=4, Cancel=8, Apply=16, Ok=32};

  /*
   * Creates a new module.
   */
  KCModule(QWidget *parent=0, const char *name=0);

  /**
   * Load the configuration data into the module.
   * 
   * The load method sets the user interface elements of the
   * module to reflect the current settings stored in the
   * configuration files.
   *
   * load is called after startup, but also when the user
   * clicks the "Reset" button.
   */
  virtual void load() {};

  /**
   * Save the configuration data.
   *
   * The load module stores the config information as shown
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
   * button. It should set the display to usefull values.
   */
  virtual void defaults() {};

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
  int buttons() { return _btn; };


protected:

  /**
   * Set the buttons to display.
   *
   * The control center displays 6 buttons:
   * 
   * Help, Default, Reset, Cancel, Apply, Ok
   *
   * Not all of these make sense for all modules, so you
   * can use this method to set the buttons to be enabled when
   * your module is displayed.
   *
   * @see KCModule::buttons
   */
  void setButtons(int btn) { _btn = btn; };
  

signals:

  /**
   * Indicate the the state of the modules contents has changed.
   *
   * This signal is emitted whenever the state of the configuration
   * shown in the module changes. It allows the control center to
   * keep track of unsaved changes.
   *
   */
  void changed(bool state);


private:

  int _btn;
 
};


class KCDialog : public QDialog
{
  Q_OBJECT

public:

  KCDialog(KCModule *client, QWidget *parent=0, const char *name=0, bool modal=false, WFlags f=0);


protected:

  void resizeEvent(QResizeEvent *event);


protected slots:

  void helpClicked();
  void defaultClicked();
  void resetClicked();
  void cancelClicked();
  void applyClicked();
  void okClicked();


private:
  
  KCModule    *_client;
  QPushButton *_help;
  QPushButton *_default;
  QPushButton *_reset;
  QPushButton *_cancel;
  QPushButton *_apply;
  QPushButton *_ok;
  QWidget     *_buttons;
  QFrame      *_sep;

};


#endif
