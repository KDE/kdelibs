/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (c) 1999, 2000 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (c) 2000 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/*
 * This file holds the definitions for all classes used to
 * display a properties dialog.
 */

#ifndef __propsdlg_h
#define __propsdlg_h

#include <qstring.h>
#include <q3ptrlist.h>

#include <kdemacros.h>
#include <kurl.h>
#include <kfileitem.h>
#include <kdialogbase.h>

class QLineEdit;
class QCheckBox;
class QPushButton;
class KLineEdit;
class KURLRequester;
class QPushButton;
class KIconButton;
class KPropsDlgPlugin;
class QComboBox;
class Q3ListBox;

#define KPropsPage KPropsDlgPlugin

namespace KIO { class Job; }

/**
 * The main properties dialog class.
 * A Properties Dialog is a dialog which displays various information
 * about a particular file or URL, or several files or URLs.
 * This main class holds various related classes, which are instantiated in
 * the form of tab entries in the tabbed dialog that this class provides.
 * The various tabs themselves will let the user view, and sometimes change,
 * information about the file or URL.
 *
 * \image html kpropertiesdialog.png "Typical KProperties Dialog"
 *
 * This class must be created with (void)new KPropertiesDialog(...)
 * It will take care of deleting itself.
 *
 * If you are looking for more flexibility, see KFileMetaInfo and
 * KFileMetaInfoWidget.
 */
class KIO_EXPORT KPropertiesDialog : public KDialogBase
{
  Q_OBJECT

public:

  /**
   * Determine whether there are any property pages available for the
   * given file items.
   * @param _items the list of items to check.
   * @return true if there are any property pages, otherwise false.
   */
  static bool canDisplay( KFileItemList _items );

  /**
   * Brings up a Properties dialog, as shown above.
   * This is the normal constructor for
   * file-manager type applications, where you have a KFileItem instance
   * to work with.  Normally you will use this
   * method rather than the one below.
   *
   * @param item file item whose properties should be displayed.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * @param autoShow tells the dialog whether it should show itself automatically.
   */
  KPropertiesDialog( KFileItem * item,
                     QWidget* parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true);

  /**
   * \overload
   *
   * You use this constructor for cases where you have a number of items,
   * rather than a single item. Be careful which methods you use
   * when passing a list of files or URLs, since some of them will only
   * work on the first item in a list.
   *
   * @param _items list of file items whose properties should be displayed.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * @param autoShow tells the dialog whether it should show itself automatically.
   */
  KPropertiesDialog( KFileItemList _items,
                     QWidget *parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true);

#ifndef KDE_NO_COMPAT
  /**
   * @deprecated  You should use the following constructor instead of this one.
   * The only change that is required is to delete the _mode argument.
   *
   * @param _url the URL whose properties should be displayed
   * @param _mode unused.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * @param autoShow tells the dialog whether it should show itself automatically.  */
  KPropertiesDialog( const KURL& _url, mode_t _mode,
                     QWidget* parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true) KDE_DEPRECATED;
#endif

  /**
   * Brings up a Properties dialog. Convenience constructor for
   * non-file-manager applications, where you have a KURL rather than a
   * KFileItem or KFileItemList.
   *
   * @param _url the URL whose properties should be displayed
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * IMPORTANT: This constructor, together with modal=true, leads to a grave
   * display bug (due to KIO::stat() being run before the dialog has all the
   * necessary information). Do not use this combination for now.
   * For local files with a known mimetype, simply create a KFileItem and pass
   * it to the other constructor.
   *
   * @param autoShow tells the dialog whethr it should show itself automatically.
   */
  KPropertiesDialog( const KURL& _url,
                     QWidget* parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true);

  /**
   * Creates a properties dialog for a new .desktop file (whose name
   * is not known yet), based on a template. Special constructor for
   * "File / New" in file-manager type applications.
   *
   * @param _tempUrl template used for reading only
   * @param _currentDir directory where the file will be written to
   * @param _defaultName something to put in the name field,
   * like mimetype.desktop
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * @param autoShow tells the dialog whethr it should show itself automatically.
   */
  KPropertiesDialog( const KURL& _tempUrl, const KURL& _currentDir,
                     const QString& _defaultName,
                     QWidget* parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true);

  /**
   * Creates an empty properties dialog (for applications that want use
   * a standard dialog, but for things not doable via the plugin-mechanism).
   *
   * @param title is the string display as the "filename" in the caption of the dialog.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   */
  KPropertiesDialog (const QString& title,
                     QWidget* parent = 0L, const char* name = 0L, bool modal = false);

  /**
   * Cleans up the properties dialog and frees any associated resources,
   * including the dialog itself. Note that when a properties dialog is
   * closed it cleans up and deletes itself.
   */
  virtual ~KPropertiesDialog();

  /**
   * Immediately displays a Properties dialog using constructor with
   * the same parameters.
   * On MS Windows, if @p item points to a local file, native (non modal) property
   * dialog is displayed (@p parent and @p modal are ignored in this case).
   *
   * @return true on succesfull dialog displaying (can be false on win32).
   * @since 3.4
   */
  static bool showDialog(KFileItem* item, QWidget* parent = 0,
                         const char* name = 0, bool modal = false);

  /**
   * Immediately displays a Properties dialog using constructor with
   * the same parameters.
   * On MS Windows, if @p _url points to a local file, native (non modal) property
   * dialog is displayed (@p parent and @p modal are ignored in this case).
   *
   * @return true on succesfull dialog displaying (can be false on win32).
   * @since 3.4
   */
  static bool showDialog(const KURL& _url, QWidget* parent = 0,
                         const char* name = 0, bool modal = false);

  /**
   * Immediately displays a Properties dialog using constructor with
   * the same parameters.
   * On MS Windows, if @p _items has one element and this element points
   * to a local file, native (non modal) property dialog is displayed
   * (@p parent and @p modal are ignored in this case).
   *
   * @return true on succesfull dialog displaying (can be false on win32).
   * @since 3.4
   */
  static bool showDialog(const KFileItemList& _items, QWidget* parent = 0,
                         const char* name = 0, bool modal = false);

  /**
   * Adds a "3rd party" properties plugin to the dialog.  Useful
   * for extending the properties mechanism.
   *
   * To create a new plugin type, inherit from the base class KPropsDlgPlugin
   * and implement all the methods. If you define a service .desktop file
   * for your plugin, you do not need to call insertPlugin().
   *
   * @param plugin is a pointer to the KPropsDlgPlugin. The Properties
   *        dialog will do destruction for you. The KPropsDlgPlugin \b must
   *        have been created with the KPropertiesDialog as its parent.
   * @see KPropsDlgPlugin
   */
  void insertPlugin (KPropsDlgPlugin *plugin);

  /**
   * The URL of the file that has its properties being displayed.
   * This is only valid if the KPropertiesDialog was created/shown
   * for one file or URL.
   *
   * @return a parsed URL.
   */
  const KURL& kurl() const { return m_singleUrl; }

  /**
   * @return the file item for which the dialog is shown
   *
   * Warning: this method returns the first item of the list.
   * This means that you should use this only if you are sure the dialog is used
   * for a single item. Otherwise, you probably want items() instead.
   */
  KFileItem *item() { return m_items.first(); }

  /**
   * @return the items for which the dialog is shown
   */
  KFileItemList items() const { return m_items; }

  /**
   * @return a pointer to the dialog
   * @deprecated KPropertiesDialog directly inherits from KDialogBase, so use \a this instead
   */
  KDE_DEPRECATED KDialogBase* dialog() { return this; }
  /**
   * @return a pointer to the dialog
   * @deprecated KPropertiesDialog directly inherits from KDialogBase, so use \a this instead
   */
  KDE_DEPRECATED const KDialogBase* dialog() const { return this; }

  /**
   * If the dialog is being built from a template, this method
   * returns the current directory. If no template, it returns QString::null.
   * See the template form of the constructor.
   *
   * @return the current directory or QString::null
   */
  const KURL& currentDir() const { return m_currentDir; }

  /**
   * If the dialog is being built from a template, this method
   * returns the default name. If no template, it returns QString::null.
   * See the template form of the constructor.
   * @return the default name or QString::null
   */
  const QString& defaultName() const { return m_defaultName; }

  /**
   * Updates the item URL (either called by rename or because
   * a global apps/mimelnk desktop file is being saved)
   * Can only be called if the dialog applies to a single file or URL.
   * @param _newUrl the new URL
   */
  void updateUrl( const KURL& _newUrl );

  /**
   * Renames the item to the specified name. This can only be called if
   * the dialog applies to a single file or URL.
   * @param _name new filename, encoded.
   * \see FilePropsDlgPlugin::applyChanges
   */
  void rename( const QString& _name );

  /**
   * To abort applying changes.
   */
  void abortApplying();

  /**
   * Shows the page that was previously set by
   * setFileSharingPage(), or does nothing if no page
   * was set yet.
   * \see setFileSharingPage
   * @since 3.1
   */
  void showFileSharingPage();

  /**
   * Sets the file sharing page.
   * This page is shown when calling showFileSharingPage().
   *
   * @param page the page to set
   * \see showFileSharingPage
   * @since 3.3
   */
  void setFileSharingPage(QWidget* page);

   /**
    * Call this to make the filename lineedit readonly, to prevent the user
    * from renaming the file.
    * \param ro true if the lineedit should be read only
    * @since 3.2
    */
  void setFileNameReadOnly( bool ro );

public slots:
  /**
   * Called when the user presses 'Ok'.
   */
  virtual void slotOk();      // Deletes the PropertiesDialog instance
  /**
   * Called when the user presses 'Cancel'.
   */
  virtual void slotCancel();     // Deletes the PropertiesDialog instance

signals:
  /**
   * This signal is emitted when the Properties Dialog is closed (for
   * example, with OK or Cancel buttons)
   */
  void propertiesClosed();

  /**
   * This signal is emitted when the properties changes are applied (for
   * example, with the OK button)
   */
  void applied();

  /**
   * This signal is emitted when the properties changes are aborted (for
   * example, with the Cancel button)
   */

  void canceled();
  /**
   * Emitted before changes to @p oldUrl are saved as @p newUrl.
   * The receiver may change @p newUrl to point to an alternative
   * save location.
   */
  void saveAs(const KURL &oldUrl, KURL &newUrl);

private:

  /**
   * Common initialization for all constructors
   */
  void init (bool modal = false, bool autoShow = true);

  /**
   * Inserts all pages in the dialog.
   */
  void insertPages();

  /**
   * The URL of the props dialog (when shown for only one file)
   */
  KURL m_singleUrl;

  /**
   * List of items this props dialog is shown for
   */
  KFileItemList m_items;

  /**
   * For templates
   */
  QString m_defaultName;
  KURL m_currentDir;

  /**
   * List of all plugins inserted ( first one first )
   */
  Q3PtrList<KPropsDlgPlugin> m_pageList;

private slots:
  void slotStatResult( KIO::Job * ); // No longer used
signals:
  void leaveModality();
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KPropertiesDialogPrivate;
  KPropertiesDialogPrivate *d;
};

/**
 * A Plugin in the Properties dialog
 * This is an abstract class. You must inherit from this class
 * to build a new kind of tabbed page for the KPropertiesDialog.
 * A plugin in itself is just a library containing code, not a dialog's page.
 * It's up to the plugin to insert pages into the parent dialog.
 *
 * To make a plugin available, define a service that implements the KPropsDlg/Plugin
 * servicetype, as well as the mimetypes for which the plugin should be created.
 * For instance, ServiceTypes=KPropsDlg/Plugin,text/html,application/x-mymimetype.
 *
 * You can also include X-KDE-Protocol=file if you want that plugin
 * to be loaded only for local files, for instance.
 */
class KIO_EXPORT KPropsDlgPlugin : public QObject
{
  Q_OBJECT
public:
  /**
   * Constructor
   * To insert tabs into the properties dialog, use the add methods provided by
   * KDialogBase (the properties dialog is a KDialogBase).
   */
  KPropsDlgPlugin( KPropertiesDialog *_props );
  virtual ~KPropsDlgPlugin();

  /**
   * Applies all changes to the file.
   * This function is called when the user presses 'Ok'. The last plugin inserted
   * is called first.
   */
  virtual void applyChanges();

  /**
   * Convenience method for most ::supports methods
   * @return true if the file is a local, regular, readable, desktop file
   */
  static bool isDesktopFile( KFileItem * _item );

  void setDirty( bool b );
  bool isDirty() const;

public slots:
  void setDirty(); // same as setDirty( true )

signals:
  /**
   * Emit this signal when the user changed anything in the plugin's tabs.
   * The hosting PropertiesDialog will call applyChanges only if the
   * PropsPlugin has emitted this signal before.
   */
  void changed();

protected:
  /**
   * Pointer to the dialog
   */
  KPropertiesDialog *properties;

  int fontHeight;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KPropsDlgPluginPrivate;
  KPropsDlgPluginPrivate *d;
};

/**
 * 'General' plugin
 *  This plugin displays the name of the file, its size and access times.
 * @internal
 */
class KIO_EXPORT KFilePropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KFilePropsPlugin( KPropertiesDialog *_props );
  virtual ~KFilePropsPlugin();

  /**
   * Applies all changes made.  This plugin must be always the first
   * plugin in the dialog, since this function may rename the file which
   * may confuse other applyChanges functions.
   */
  virtual void applyChanges();

  /**
   * Tests whether the files specified by _items need a 'General' plugin.
   */
  static bool supports( KFileItemList _items );

  /**
   * Called after all plugins applied their changes
   */
  void postApplyChanges();

  void setFileNameReadOnly( bool ro );

protected slots:
  void slotEditFileType();
  void slotCopyFinished( KIO::Job * );
  void slotFileRenamed( KIO::Job *, const KURL &, const KURL & );
  void slotDirSizeUpdate();
  void slotDirSizeFinished( KIO::Job * );
  void slotFoundMountPoint( const QString& mp, unsigned long kBSize,
			    unsigned long kBUsed, unsigned long kBAvail );
  void slotSizeStop();
  void slotSizeDetermine();

signals:
  void leaveModality();
private slots:
  // workaround for compiler bug
  void slotFoundMountPoint( const unsigned long& kBSize, const unsigned long&
			  kBUsed, const unsigned long& kBAvail, const QString& mp );
  void nameFileChanged(const QString &text );
  void slotIconChanged();

private:
  void determineRelativePath( const QString & path );
  void applyIconChanges();

  QWidget *iconArea;
  QWidget *nameArea;

  QLabel *m_sizeLabel;
  QPushButton *m_sizeDetermineButton;
  QPushButton *m_sizeStopButton;

  QString m_sRelativePath;
  bool m_bFromTemplate;

  /**
   * The initial filename
   */
  QString oldName;

  class KFilePropsPluginPrivate;
  KFilePropsPluginPrivate *d;
};

/**
 * 'Permissions' plugin
 * In this plugin you can modify permissions and change
 * the owner of a file.
 * @internal
 */
class KIO_EXPORT KFilePermissionsPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  enum PermissionsMode {
    PermissionsOnlyFiles = 0,
    PermissionsOnlyDirs = 1,
    PermissionsOnlyLinks = 2,
    PermissionsMixed = 3
  };

  enum PermissionsTarget {
    PermissionsOwner  = 0,
    PermissionsGroup  = 1,
    PermissionsOthers = 2
  };

  /**
   * Constructor
   */
  KFilePermissionsPropsPlugin( KPropertiesDialog *_props );
  virtual ~KFilePermissionsPropsPlugin();

  virtual void applyChanges();

  /**
   * Tests whether the file specified by _items needs a 'Permissions' plugin.
   */
  static bool supports( KFileItemList _items );

private slots:

  void slotChmodResult( KIO::Job * );
  void slotShowAdvancedPermissions();

signals:
  void leaveModality();

private:
  void setComboContent(QComboBox *combo, PermissionsTarget target,
		       mode_t permissions, mode_t partial);
  bool isIrregular(mode_t permissions, bool isDir, bool isLink);
  void enableAccessControls(bool enable);
  void updateAccessControls();
  void getPermissionMasks(mode_t &andFilePermissions,
			  mode_t &andDirPermissions,
			  mode_t &orFilePermissions,
			  mode_t &orDirPermissions);

  static const mode_t permissionsMasks[3];
  static const mode_t standardPermissions[4];
  static const char *permissionsTexts[4][4];

  // unused, for binary compatibility!
  QCheckBox *permBox[3][4];

  QComboBox *grpCombo;

  KLineEdit *usrEdit, *grpEdit;

  /**
   * Old permissions
   */
  mode_t permissions;
  /**
   * Old group
   */
  QString strGroup;
  /**
   * Old owner
   */
  QString strOwner;

  // unused, for compatibility
  static mode_t fperm[3][4];

  class KFilePermissionsPropsPluginPrivate;
  KFilePermissionsPropsPluginPrivate *d;
};


/**
 * Used to edit the files containing
 * [Desktop Entry]
 * URL=....
 *
 * Such files are used to represent a program in kicker and konqueror.
 * @internal
 */
class KIO_EXPORT KURLPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KURLPropsPlugin( KPropertiesDialog *_props );
  virtual ~KURLPropsPlugin();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

private:
  KURLRequester *URLEdit;
  KIconButton *iconBox;

  QString URLStr;
  QString iconStr;

  QPixmap pixmap;
  QString pixmapFile;
private:
  class KURLPropsPluginPrivate;
  KURLPropsPluginPrivate *d;
};


/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=MimeType
 * @internal
 */
class KIO_EXPORT KBindingPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KBindingPropsPlugin( KPropertiesDialog *_props );
  virtual ~KBindingPropsPlugin();

  virtual void applyChanges();
  static bool supports( KFileItemList _items );

private:

  QLineEdit *commentEdit;
  QLineEdit *patternEdit;
  QLineEdit *mimeEdit;
  QString m_sMimeStr;

  QCheckBox * cbAutoEmbed;

  class KBindingPropsPluginPrivate;
  KBindingPropsPluginPrivate *d;
};

/**
 * Properties plugin for device .desktop files
 * @internal
 */
class KIO_EXPORT KDevicePropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  KDevicePropsPlugin( KPropertiesDialog *_props );
  virtual ~KDevicePropsPlugin();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

private slots:
  void slotActivated( int );
  void slotDeviceChanged();
  void slotFoundMountPoint( const unsigned long& kBSize,
                            const unsigned long& /*kBUsed*/,
                            const unsigned long& kBAvail,
                            const QString& );

private:
  void updateInfo();

private:
  QComboBox* device;
  QLabel* mountpoint;
  QCheckBox* readonly;
  void* unused;
  //KIconButton* mounted;
  KIconButton* unmounted;

  QStringList m_devicelist;
  int indexDevice;
  int indexMountPoint;
  int indexFSType;

  QPixmap pixmap;
  QString pixmapFile;

  class KDevicePropsPluginPrivate;
  KDevicePropsPluginPrivate *d;
};

class KPropertiesDesktopBase;

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=Application
 *
 * Such files are used to represent a program in kicker and konqueror.
 * @internal
 */
class KIO_EXPORT KDesktopPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KDesktopPropsPlugin( KPropertiesDialog *_props );
  virtual ~KDesktopPropsPlugin();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

public slots:
  void slotAddFiletype();
  void slotDelFiletype();
  void slotBrowseExec();
  void slotAdvanced();
  void slotSelectMimetype();

private:
  void checkCommandChanged();

private:
  KPropertiesDesktopBase* w;

  QString m_origCommandStr;
  QString m_terminalOptionStr;
  QString m_suidUserStr;
  QString m_dcopServiceType;
  bool m_terminalBool;
  bool m_terminalCloseBool;
  bool m_suidBool;
  bool m_startupBool;
  bool m_systrayBool;

  class KDesktopPropsPluginPrivate;
  KDesktopPropsPluginPrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=Application
 *
 * Such files are used to represent a program in kicker and konqueror.
 * @internal
 * @deprecated replaced with KDesktopPropsPlugin
 */
 /// Remove in KDE4
class KIO_EXPORT_DEPRECATED KExecPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KExecPropsPlugin( KPropertiesDialog *_props );
  virtual ~KExecPropsPlugin();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

public slots:
  void slotBrowseExec();

private slots:
  void enableCheckedEdit();
  void enableSuidEdit();

private:

    QLabel *terminalLabel;
    QLabel *suidLabel;
    KLineEdit *execEdit;
    QCheckBox *terminalCheck;
    QCheckBox *suidCheck;
    KLineEdit *terminalEdit;
    KLineEdit *suidEdit;
    KLineEdit *swallowExecEdit;
    KLineEdit *swallowTitleEdit;
    QPushButton *execBrowse;

    QString execStr;
    QString swallowExecStr;
    QString swallowTitleStr;
    QString termOptionsStr;
    bool termBool;
    bool suidBool;
    QString suidUserStr;

    class KExecPropsPluginPrivate;
    KExecPropsPluginPrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=Application
 *
 * Such files are used to represent a program in kicker and konqueror.
 * @internal
 * @deprecated replaced with KDesktopPropsPlugin
 */
 /// Remove in KDE4
class KIO_EXPORT_DEPRECATED KApplicationPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KApplicationPropsPlugin( KPropertiesDialog *_props );
  virtual ~KApplicationPropsPlugin();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

public slots:
  void slotDelExtension();
  void slotAddExtension();

private slots:
  void updateButton();

private:
  void addMimeType( const QString & name );

  QLineEdit *commentEdit;
  QLineEdit *genNameEdit;
  QLineEdit *nameEdit;
  Q3ListBox  *extensionsList;
  Q3ListBox  *availableExtensionsList;
  QPushButton *addExtensionButton;
  QPushButton *delExtensionButton;

  class KApplicationPropsPluginPrivate;
  KApplicationPropsPluginPrivate *d;
};

#endif

