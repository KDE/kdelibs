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

class QStringList;

class KAboutData;
class KConfigDialogManager;
class KConfigSkeleton;
class KCModulePrivate;
class KInstance;

/**
 * The base class for control center modules.
 *
 * Starting from KDE 2.0, control center modules are realized as shared
 * libraries that are loaded into the control center at runtime.
 *
 * The module in principle is a simple widget displaying the
 * item to be changed. The module has a very small interface.
 *
 * All the necessary glue logic and the GUI bells and whistles
 * are provided by the control center and must not concern
 * the module author.
 *
 * To write a config module, you have to create a library
 * that contains at one factory function like this:
 *
 * \code
 * #include <kgenericfactory.h>
 *
 * typedef KGenericFactory<YourKCModule, QWidget> YourKCModuleFactory;
 * K_EXPORT_COMPONENT_FACTORY( yourLibName, YourKCModuleFactory("name_of_the_po_file") );
 * \endcode
 *
 * The parameter "name_of_the_po_file" has to correspond with the messages target
 * that you created in your Makefile.am.
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
   * You should only use Help, Default and Apply. The rest is obsolete.
   *
   * @see KCModule::buttons @see KCModule::setButtons
   */
  enum Button {Help=1, Default=2, Apply=16,
               Reset=4, /* obsolete, do not use! */
               Cancel=8, /* obsolete, do not use! */
               Ok=32, /* obsolete, do not use! */
  	       SysDefault=64 /* obsolete, do not use! */ };

  /*
   * Base class for all KControlModules.
   * Make sure you have a QStringList argument in your
   * implementation.
   */
  KCModule(QWidget *parent=0, const char *name=0, const QStringList &args=QStringList() );

  KCModule(KInstance *instance, QWidget *parent=0, const QStringList &args=QStringList() );

  /*
   * Destroys the module.
   */
  ~KCModule();

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
   * settings. 
   *
   * If you use KConfigXT, loading is taken care of automatically and 
   * you do not need to do it manually. However, if you for some reason reimplement it and
   * also are using KConfigXT, you must call this function otherwise the loading of KConfigXT 
   * options will not work.
   *
   */
  virtual void load();
  // ### KDE 4: Call load() automatically through a single-shot timer
  //            from the constructor // and change documentation

  /**
   * Save the configuration data.
   *
   * The save method stores the config information as shown
   * in the user interface in the config files.
   *
   * If necessary, this method also updates the running system,
   * e.g. by restarting applications. This normally does not apply for
   * KSettings::Dialog modules where the updating is taken care of by
   * KSettings::Dispatcher.
   *
   * save is called when the user clicks "Apply" or "Ok".
   *
   * If you use KConfigXT saving is taken care off automatically and 
   * you do not need to load manually. However, if you for some reason reimplement it and
   * also are using KConfigXT, you must call this function, otherwise the saving of KConfigXT 
   * options will not work.
   */
  virtual void save();

  /**
   * Sets the configuration to sensible default values.
   *
   * This method is called when the user clicks the "Default"
   * button. It should set the display to useful values.
   *
   * If you use KConfigXT, you do not have to reimplement this function since
   * the fetching and settings of default values is done automatically. However, if you 
   * reimplement and also are using KConfigXT, remember to call the base function.
   */
  virtual void defaults();

  /**
   * Set the configuration to system default values.
   *
   * This method is called when the user clicks the "System-Default"
   * button. It should set the display to the system default values.
   *
   * @note The default behavior is to call defaults().
   */
  virtual void sysdefaults() { defaults(); };
  // KDE 4 deprecate

  /**
   * Return a quick-help text.
   *
   * This method is called when the module is docked.
   * The quick-help text should contain a short description of the module and
   * links to the module's help files. You can use QML formatting tags in the text.
   *
   * @note make sure the quick help text gets translated (use i18n()).
   */
  virtual QString quickHelp() const;

  /**
   * This is generally only called for the KBugReport.
   * If you override you should  have it return a pointer to a constant.
   *
   *
   * @returns the KAboutData for this module
   */
  virtual const KAboutData *aboutData() const;
  
  /**
   * This sets the KAboutData returned by aboutData()
   * @since 3.3
   */
   void setAboutData( KAboutData* about );

  /**
   * Indicate which buttons will be used.
   *
   * The return value is a value or'ed together from
   * the Button enumeration type.
   *
   * @see KCModule::setButtons
   */
  int buttons() const { return _btn; };

  /**
   * Get the RootOnly message for this module.
   *
   * When the module must be run as root, or acts differently
   * for root and a normal user, it is sometimes useful to
   * customize the message that appears at the top of the module
   * when used as a normal user. This function returns this
   * customized message. If none has been set, a default message
   * will be used.
   *
   * @see KCModule::setRootOnlyMsg
   */
  QString rootOnlyMsg() const;

  /**
   * Tell if KControl should show a RootOnly message when run as
   * a normal user.
   *
   * In some cases, the module don't want a RootOnly message to
   * appear (for example if it has already one). This function
   * tells KControl if a RootOnly message should be shown
   *
   * @see KCModule::setUseRootOnlyMsg
   */
  bool useRootOnlyMsg() const;

  KInstance *instance() const;

protected:
  /**
   * Adds a KConfigskeleton @p config to watch the widget @p widget
   *
   * This function is useful if you need to handle multiple configuration files.
   *
   * @return a pointer to the KConfigDialogManager in use
   * @param config the KConfigSkeleton to use
   * @param widget the widget to watch
   */
  KConfigDialogManager* addConfig( KConfigSkeleton *config, QWidget* widget );

  /**
   * Sets the quick help.
   */
  void setQuickHelp( const QString& help );

signals:

  /**
   * Indicate that the state of the modules contents has changed.
   *
   * This signal is emitted whenever the state of the configuration
   * shown in the module changes. It allows the control center to
   * keep track of unsaved changes.
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

protected slots:

  /**
   * Calling this slot is equivalent to emitting changed(true).
   * @since 3.3
   */
  void changed();

protected:

  /**
   * Sets the buttons to display.
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

  /**
   * Sets the RootOnly message.
   *
   * This message will be shown at the top of the module of the
   * corresponding desktop file contains the line X-KDE-RootOnly=true.
   * If no message is set, a default one will be used.
   *
   * @see KCModule::rootOnlyMsg
   */
  void setRootOnlyMsg(const QString& msg);

  /**
   * Change wether or not the RootOnly message should be shown.
   *
   * Following the value of @p on, the RootOnly message will be
   * shown or not.
   *
   * @see KCModule::useRootOnlyMsg
   */
  void setUseRootOnlyMsg(bool on);

private:

  int _btn;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  KCModulePrivate *d;

  /**
   * Internal function for initialization of the class.
   */
  void init();

};

#endif //__KCMODULE_H__

