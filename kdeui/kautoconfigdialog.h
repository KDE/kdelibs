#ifndef KAUTOCONFIGDIALOG_H
#define KAUTOCONFIGDIALOG_H

class KAutoConfig;
#include <kdialogbase.h>
#include <qasciidict.h>

/**
 * @author Benjamin C Meyer <ben at meyerhome.net>
 * 
 * The KAutoConfigDialog class provides an easy and uniform means of displaying
 * a settings dialog use KDialogBase and KAutoConfig.
 *
 * KAutoConfigDialog handles the enabling and disabling of buttons, creation
 * of the dialog, and deletion of the widgets,
 *
 * Because of KAutoConfig it also manages: restoring the settings,
 * reseting them to the default values, and saving the values.
 * 
 * Here is an example usage of KAutoConfigDialog:
 *
 * <pre> 
 * void KCoolApp::showSettings(){
 *   if(KAutoConfigDialog::showDialog("settings"))
 *     return;
 *   KAutoConfigDialog *dialog = new KAutoConfigDialog(this, "settings");
 *   dialog->addPage(new General(0, "General"), i18n("General"), "General", "package_settings");
 *   dialog->addPage(new Ai(0, "Ai"), i18n("Ai"), "Game", "package_system");
 *   dialog->addPage(new Appearance(0, "Appearance"), i18n("Appearance"), "Game", "style");
 *   connect(dialog, SIGNAL(settingsChanged()), mainWidget, SLOT(readSettings()));
 *   connect(dialog, SIGNAL(settingsChanged()), this, SLOT(readSettings()));
 *   dialog->show();
 * }
 * </pre>
 * 
 * Other then the above code each class that has settings in the dialog should
 * have a readSettings() type slot to read its settings and perform any necessary
 * changes.
 * 
 */ 
class KAutoConfigDialog : public QObject {
Q_OBJECT

signals:
  /** 
   * One or more of the settings have been permanently changed.
   * Such as if the user clicked on the Apply button.
   */
  void settingsChanged();
	
public:
  /**
   * @param parent - The parent object of this object.  Even though the class
   * deletes itself the parent should be set the the dialog can be centered
   * with the application on the screen.
   * 
   * @param name - The name of this object.  The name is used in determining if
   * there can be more then one dialog at a time so name these for example
   * "Font Settings" or "Color Settings" and not just "Settings".
   * 
   * @param type - Used in creating the dialog.  @Ref KDialogBase
   * 
   * @param model - Because of the features of KAutoConfig, KAutoConfigDialog
   * does not have to be modal.  To prevent more then one settings dialog
   * from showing the static function @ref showDialog can be called to 
   * determine if the settings dialog already exists before creating
   * a KAutoConfigDialog object.
   */ 
  KAutoConfigDialog(QWidget *parent=0, const char *name=0,
		  KDialogBase::DialogType dialogType = KDialogBase::IconList,
		  bool modal=false);
  /**
   * Deconstructor, removes name from openDialogs list.
   * @ref exists()
   */ 
  ~KAutoConfigDialog();
  
  /**
   * Adds page to the dialog and to KAutoConfig.  When
   * all done adding pages @ref show() should be called to display
   * the dialog.  Note that after you call show you can
   * not add any more pages to the dialog.
   * @param page - The page that is to be added to the dialog.
   * @param itemName - QString used in the page's name.
   * @param groupName - Name of the group where all of the settings
   * 			for the page should be stored.
   * @param pixmap - Name of the pixmap that should be used if needed.
   * @param header - Header text use in the list modes. Ignored in Tabbed
   *        mode. If empty, the itemName text is used when needed.
   */ 
  void addPage(QWidget *page, const QString &itemName,
				  const QString &groupName,
		                  const QString &pixmapName,
				  const QString &header=QString::null); 
 
  /**
   * See if a dialog already exists.
   * @ref showDialog()
   * @param name dialog name to look for.
   * @return pointer to widget or NULL if does not exist.
   */ 
  static KAutoConfigDialog* exists(const char* name);
 
  /**
   * Attempts to show a dialog specified by 'name'.
   * @ref exists()
   * @param name - The name of the dialog to show.
   * @return true if the dialog 'name' exists and was shown. 
   */ 
  static bool showDialog(const char* name);
  
  /**
   * Shows the dialog.  This should be called after all of the pages
   * have been added.
   * @ref hide()
   */ 
  virtual void show();
  
  /**
   * Hides the dialog.  A program shouldn't normally need to use this function.
   * @ref show()
   */ 
  inline void hide(){ kdialogbase->hide(); };


protected slots:
  /**
   * Some setting were modified, updates the Apply and Default buttons.
   */ 
  virtual void settingModified();

protected:
  // Pointer to the KAutoConfig object.
  KAutoConfig *kautoconfig;
  // Pointer to the KDialogBase object.
  KDialogBase *kdialogbase;
  
private:
  bool shown; 
  KDialogBase::DialogType type;
  static QAsciiDict<QObject> openDialogs;
};

#endif //KAUTOCONFIGDIALOG_H

