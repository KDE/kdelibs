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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */
#ifndef KCONFIGDIALOG_H
#define KCONFIGDIALOG_H

class KConfig;
class KConfigSkeleton;
#include <kdialogbase.h>
#include <qasciidict.h>

/**
 * @author Waldo Bastian <bastian@kde.org>
 *
 * The KConfigDialog class provides an easy and uniform means of displaying
 * a settings dialog using KDialogBase, KConfigDialogManager and a 
 * KConfigSkeleton derived settings class.
 *
 * KConfigDialog handles the enabling and disabling of buttons, creation
 * of the dialog, and deletion of the widgets.  Because of 
 * KConfigDialogManager, this class also manages: restoring 
 * the settings, reseting them to the default values, and saving them. This
 * requires that the names of the widgets corresponding to configuration entries
 * have to have the same name plus an additional "kcfg_" prefix. For example the
 * widget named "kcfg_MyOption" would be associated with the configuration entry
 * "MyOption".
 *
 * Here is an example usage of KConfigDialog:
 *
 * \code
 * void KCoolApp::showSettings(){
 *   if(KConfigDialog::showDialog("settings"))
 *     return;
 *   KConfigDialog *dialog = new KConfigDialog(this, "settings", MySettings::self(), KDialogBase::IconList);
 *   dialog->addPage(new General(0, "General"), i18n("General") );
 *   dialog->addPage(new Appearance(0, "Style"), i18n("Appearance") );
 *   connect(dialog, SIGNAL(settingsChanged()), mainWidget, SLOT(loadSettings()));
 *   connect(dialog, SIGNAL(settingsChanged()), this, SLOT(loadSettings()));
 *   dialog->show();
 * }
 * \endcode
 *
 * Other then the above code each class that has settings in the dialog should
 * have a loadSettings() type slot to read settings and perform any
 * necessary changes.
 *
 * Please note that using the setMainWidgetmethod inherited from KDialogBase
 * currently yields broken behaviour at runtime; use addPage instead.
 *
 * @see KConfigSkeleton
 * @since 3.2
 */
class KConfigDialog : public KDialogBase {
Q_OBJECT

signals:
  /**
   * A widget in the dialog was modified.
   */
  void widgetModified();

  /**
   * One or more of the settings have been permanently changed such as if
   * the user clicked on the Apply or Ok button.
   */
  void settingsChanged();

  /**
   * One or more of the settings have been permanently changed such as if
   * the user clicked on the Apply or Ok button.
   * This signal is useful when using KConfigDialog to configure
   * items in a list.  When emits the main class would then know what
   * item in the list was actually changed.
   * @param dialogName the name of the dialog.
   */
  void settingsChanged(const char *dialogName);

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
   * @param config - Config object containing settings.
   *
   * @param dialogButtons - Buttons that should show up on the dialog.
   *
   * @param modal - Whether the dialog should be modal. To prevent more than one
   * non-modal settings dialog from showing the static function showDialog() can be
   * used in determining if the settings dialog already exists before creating
   * a new KConfigDialog object.
   */
  KConfigDialog( QWidget *parent, const char *name,
                 KConfigSkeleton *config,
		 KDialogBase::DialogType dialogType = KDialogBase::IconList,
		 int dialogButtons = KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Help,
		 KDialogBase::ButtonCode defaultButton = Ok,
		 bool modal=false );

  /**
   * Deconstructor, removes name from the list of open dialogs.
   * Deletes private class.
   * @see exists()
   */
  ~KConfigDialog();

  /**
   * Adds page to the dialog and to KConfigDialogManager.  When an 
   * application is done adding pages show() should be called to 
   * display the dialog.
   * Note that after you call show() you can not add any more pages
   * to the dialog.
   * @param page - Pointer to the page that is to be added to the dialog.
   * This object is reparented.
   * @param itemName - Name of the page.
   * @param pixmapName - Name of the pixmap that should be used if needed.
   * @param header - Header text use in the list modes. Ignored in Tabbed
   *        mode. If empty, the itemName text is used when needed.
   * @param manage - Whether KConfigDialogManager should manage the page or not.
   */
  void addPage( QWidget *page, const QString &itemName,
		                  const QString &pixmapName,
				  const QString &header=QString::null,
				  bool manage=true);

  /**
   * See if a dialog with the name 'name' already exists.
   * @see showDialog()
   * @param name - Dialog name to look for.
   * @return Pointer to widget or NULL if it does not exist.
   */
  static KConfigDialog* exists( const char* name );

  /**
   * Attempts to show the dialog with the name 'name'.
   * @see exists()
   * @param name - The name of the dialog to show.
   * @return True if the dialog 'name' exists and was shown.
   */
  static bool showDialog( const char* name );

  /**
   * Show the dialog.
   */
  virtual void show();

protected slots:
  /**
   * Update the settings from the dialog.
   * Virtual function for custom additions.
   *
   * Example use: User clicks Ok or Apply button in a configure dialog.
   */ 
  virtual void updateSettings();

  /**
   * Update the dialog based on the settings.
   * Virtual function for custom additions.
   *
   * Example use: Initialisation of dialog.
   * Example use: User clicks Reset button in a configure dialog.
   */
  virtual void updateWidgets();

  /**
   * Update the dialog based on the default settings.
   * Virtual function for custom additions.
   *
   * Example use: User clicks Defaults button in a configure dialog.
   */
  virtual void updateWidgetsDefault();

protected:

  /**
   * Returns whether the current state of the dialog is
   * different from the current configutation.
   * Virtual function for custom additions.
   */
  virtual bool hasChanged() { return false; }

  /**
   * Returns whether the current state of the dialog is
   * the same as the default configuration.
   */
  virtual bool isDefault() { return true; }
  

protected slots:
  /**
   * Updates the Apply and Default buttons.
   */
  void updateButtons();

  /**
   * Some setting was changed. Emit the signal with the dialogs name
   */
  void settingsChangedSlot();

private:
  /**
   * The list of existing dialogs.
   */
  static QAsciiDict<KConfigDialog> openDialogs;

  class KConfigDialogPrivate;
  /**
   * Private class.
   */
  KConfigDialogPrivate *d;
};

#endif //KCONFIGDIALOG_H

