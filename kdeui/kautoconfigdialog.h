/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2003 Benjamin C Meyer (ben@meyerhome.net)
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
#ifndef KAUTOCONFIGDIALOG_H
#define KAUTOCONFIGDIALOG_H

class KAutoConfig;
class KConfig;
#include <kdialogbase.h>
#include <qasciidict.h>

/**
 * @author Benjamin C Meyer <ben-devel at meyerhome.net>
 * 
 * The KAutoConfigDialog class provides an easy and uniform means of displaying
 * a settings dialog using @ref KDialogBase and @ref KAutoConfig.
 *
 * KAutoConfigDialog handles the enabling and disabling of buttons, creation
 * of the dialog, and deletion of the widgets.  Because of KAutoConfig, this
 * class also manages: restoring the settings, reseting them to the default
 * values, and saving them.
 * 
 * Here is an example usage of KAutoConfigDialog:
 *
 * <pre> 
 * void KCoolApp::showSettings(){
 *   if(KAutoConfigDialog::showDialog("settings"))
 *     return;
 *   KAutoConfigDialog *dialog = new KAutoConfigDialog(this, "settings");
 *   dialog->addPage(new General(0, "General"), i18n("General"), "General", "package_settings");
 *   dialog->addPage(new Appearance(0, "Appearance"), i18n("Appearance"), "Style", "style");
 *   connect(dialog, SIGNAL(settingsChanged()), mainWidget, SLOT(loadSettings()));
 *   connect(dialog, SIGNAL(settingsChanged()), this, SLOT(loadSettings()));
 *   dialog->show();
 * }
 * </pre>
 * 
 * Other then the above code each class that has settings in the dialog should
 * have a loadSettings() type slot to read settings and perform any
 * necessary changes.
 *
 * @see KAutoConfig
 * @since 3.2
 */ 
class KAutoConfigDialog : public QObject {
Q_OBJECT

signals:
  /** 
   * One or more of the settings have been permanently changed such as if
   * the user clicked on the Apply or Ok button.
   */
  void settingsChanged();
  /** 
   * One or more of the settings have been permanently changed such as if
   * the user clicked on the Apply or Ok button.
   * This signal is usefull when using kautoconfigdialog to configure
   * items in a list.  When emits the main class would then know what
   * item in the list was actually changed.
   * @param dialogName the name of the dialog.
   */
  void settingsChanged(const char *dialogName);

  /**
   * Helper function.
   */ 
  void okClicked();
  /**
   * Helper function.
   */ 
  void applyClicked();
  /**
   * Helper function.
   */ 
  void defaultClicked();
  
public:
  /**
   * @param parent - The parent object of this object.  Even though the class
   * deletes itself the parent should be set so the dialog can be centered
   * with the application on the screen.
   * 
   * @param name - The name of this object.  The name is used in determining if
   * there can be more then one dialog at a time.  Use names such as:
   * "Font Settings" or "Color Settings" and not just "Settings" in
   * applications where there are more then one dialog.
   * 
   * @param dialogType - Type used in creating the dialog.  @see KDialogBase
   *
   * @param kconfig - Specify specific config to use or NULL for the
   * standard KGlobal::config(); 
   *
   * @param dialogButtons - Buttons that should show up on the dialog.
   *
   * @param model - Because of the features of @ref KAutoConfig,
   * KAutoConfigDialog does not have to be modal.  To prevent more then one
   * settings dialog from showing the static function @ref showDialog() can be
   * used in determining if the settings dialog already exists before creating
   * a new KAutoConfigDialog object.
   */ 
  KAutoConfigDialog( QWidget *parent=0, const char *name=0,
		  KDialogBase::DialogType dialogType = KDialogBase::IconList,
		  KConfig *kconfig=NULL, 
		  KDialogBase::ButtonCode dialogButtons = 
		    (KDialogBase::ButtonCode) (KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Help),
		  bool modal=false );
  /**
   * Deconstructor, removes name from the list of open dialogs.
   * Deletes private class.
   * @see exists()
   */ 
  ~KAutoConfigDialog();
 
  /**
   * Adds page to the dialog and to @ref KAutoConfig.  When an application is
   * done adding pages @ref show() should be called to display the dialog.
   * Note that after you call @ref show() you can not add any more pages
   * to the dialog.
   * @param page - Pointer to the page that is to be added to the dialog.  
   * This object is reparented.
   * @param itemName - Name of the page.
   * @param groupName - Name of the group where all of the settings
   * 			for the page should be stored.
   * @param pixmap - Name of the pixmap that should be used if needed.
   * @param header - Header text use in the list modes. Ignored in Tabbed
   *        mode. If empty, the itemName text is used when needed.
   * @param manage - Whether KAutoConfig should manage the page or not.
   */ 
  void addPage( QWidget *page, const QString &itemName,
				  const QString &groupName,
		                  const QString &pixmapName,
				  const QString &header=QString::null,
				  bool manage=true); 
 
  /**
   * See if a dialog with the name 'name' already exists.
   * @see showDialog()
   * @param name - Dialog name to look for.
   * @return Pointer to widget or NULL if it does not exist.
   */ 
  static KAutoConfigDialog* exists( const char* name );
 
  /**
   * Attempts to show the dialog with the name 'name'.
   * @see exists()
   * @param name - The name of the dialog to show.
   * @return True if the dialog 'name' exists and was shown. 
   */ 
  static bool showDialog( const char* name );
  
  /**
   * Shows the dialog.  This has to be called after all of the pages
   * have been added.
   * @param track - Track all of the widgets for any changes. If false all
   * of the buttons are enabled.  This can be set to false as a temporary
   * measure, but applications in general should leave it on.  This 
   * parameter only matters the first time this function is called.
   * @see hide()
   */ 
  virtual void show( bool track=true );

  /**
   * Hides the dialog.  An application shouldn't normally need to use
   * this function.
   * @see show()
   */ 
  void hide(){ kdialogbase->hide(); };

  /**
   * KAutoConfigDialog automatically gives the dialog a common KDE
   * configuration caption. This function is provided for dialogs other
   * then the normal application configuration such as plugin configurations.
   * Do not include the applications name as it will automaticly be added
   * via the KDE rules. @ref KDialog::setCaption()
   * @param caption - The new name for the dialog's caption such as: 
   * "Configure Audio Plugin"
   **/
  virtual void setCaption( const QString &caption );
 
protected slots:
  /**
   * Some setting was modified, updates the Apply and Default buttons.
   */ 
  virtual void settingModified();
  
  /**
   * Some setting was changed. Emit the signal with the dialogs name
   */
  virtual void settingsChangedSlot();

protected:
  /**
   * Helper function that connects the three dialog buttons to kautoconfig's
   * slots and connects both settingsChanged() signals and widgetModified()
   * signal to the dialog.
   */ 
  virtual void connectKAutoConfig( KAutoConfig const* kautoconfig_object );
  
  // Pointer to the KAutoConfig object.
  KAutoConfig *kautoconfig;
  // Pointer to the KDialogBase object.
  KDialogBase *kdialogbase;
  
private:
  // The list of existing dialogs.
  static QAsciiDict<QObject> openDialogs;
 
  // Private class.
  class KAutoConfigDialogPrivate;
  KAutoConfigDialogPrivate *d;
};

#endif //KAUTOCONFIGDIALOG_H

