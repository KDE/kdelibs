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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*
 * This file holds the definitions for all classes used to
 * display a properties dialog.
 */

#ifndef __propsdlg_h
#define __propsdlg_h

#include <qstring.h>
#include <qlist.h>

#include <kurl.h>
#include <kfileitem.h>
#include <kdialogbase.h>

class QLineEdit;
class QCheckBox;
class QPushButton;
class KLineEdit;
class KURLRequester;
class QButton;
class KIconButton;
class KPropsDlgPlugin;
class QComboBox;

#define KPropsPage KPropsDlgPlugin

namespace KIO { class Job; }

/**
 * The main properties dialog class.
 * A Properties Dialog is a dialog which displays various information
 * about a particular file or URL, or several ones.
 * This main class holds various related classes, which are instantiated in
 * the form of tab entries in the tabbed dialog that this class provides.
 * The various tabs themselves will let the user view or change information
 * about the file or URL.
 *
 * This class must be created with (void)new KPropertiesDialog(...)
 * It will take care of deleting itself.
 */
class KPropertiesDialog : public KDialogBase
{
  Q_OBJECT

public:

  /**
   * @return whether there are any property pages available for the
   * given file items
   */
  static bool canDisplay( KFileItemList _items );

  /**
   * Brings up a Properties dialog. Normal constructor for
   * file-manager-like applications.  Normally you will use this
   * method rather than the one below.
   *
   * @param item file item whose properties should be displayed.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * @param autoShow tells the dialog whethr it should show itself automatically.
   */
  KPropertiesDialog( KFileItem * item,
                     QWidget* parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true);
  /**
   * Brings up a Properties dialog. Normal constructor for
   * file-manager-like applications.
   *
   * @param _items list of file items whose properties should be
   * displayed.
   *
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * @param autoShow tells the dialog whethr it should show itself automatically.
   */
  KPropertiesDialog( KFileItemList _items,
                     QWidget *parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true);

#ifndef KDE_NO_COMPAT
  /**
   * @deprecated
   *
   * Brings up a Properties dialog. Convenience constructor for
   * non-file-manager applications.
   *
   * @param _url the URL whose properties should be displayed
   * @param _mode unused.
   * @param parent is the parent of the dialog widget.
   * @param name is the internal name.
   * @param modal tells the dialog whether it should be modal.
   * @param autoShow tells the dialog whethr it should show itself automatically.  */
  KPropertiesDialog( const KURL& _url, mode_t _mode,
                     QWidget* parent = 0L, const char* name = 0L,
                     bool modal = false, bool autoShow = true);
#endif

  /**
   * Brings up a Properties dialog. Convenience constructor for
   * non-file-manager applications.
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
   * "File / New" in file-manager applications.
   *
   * @param _templUrl template used for reading only
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
   * Adds a "3rd party" properties plugin to the dialog.  Useful
   * for extending the properties mechanism.
   *
   * To create a new plugin type, inherit from the base class KPropsPlugin
   * and implement all the methods. If you define a service .desktop file
   * for your plugin, you do not need to call insertPlugin().
   *
   * @param plugin is a pointer to the PropsPlugin. The Properties
   *        dialog will do destruction for you. The KPropsPlugin MUST
   *        have been created with the KPropertiesDialog as its parent.
   * @see KPropsDlgPlugin
   */
  void insertPlugin (KPropsDlgPlugin *plugin);

  /**
   * @return a parsed URL.
   * Valid only if dialog shown for one file/url.
   */
  const KURL& kurl() const { return m_singleUrl; }

  /**
   * @return the file item for which the dialog is shown
   * Warning, it returns the first item of the list.
   * This means, use this only if you are sure the dialog is used
   * for a single item.
   */
  KFileItem *item() { return m_items.first(); }

  /**
   * @return the items for which the dialog is shown
   */
  KFileItemList items() const { return m_items; }

  /**
   * @return a pointer to the dialog
   * @deprecated
   */
  KDialogBase* dialog() { return this; }
  const KDialogBase* dialog() const { return this; }

  /**
   * If we are building this dialog from a template,
   * @return the current directory
   * QString::null means no template used
   */
  const KURL& currentDir() const { return m_currentDir; }

  /**
   * If we are building this dialog from a template,
   * @return the default name (see 3rd constructor)
   * QString::null means no template used
   */
  const QString& defaultName() const { return m_defaultName; }

  /**
   * Updates the item url (either called by rename or because
   * a global apps/mimelnk desktop file is being saved)
   * Can only be called if the dialog applies to a single file/URL.
   * @param _name new URL
   */
  void updateUrl( const KURL& _newUrl );

  /**
   * #see FilePropsPlugin::applyChanges
   * Can only be called if the dialog applies to a single file/URL.
   * @param _name new filename, encoded.
   */
  void rename( const QString& _name );

  /**
   * To abort applying changes.
   */
  void abortApplying();

  /**
   * @since 3.1
   */
  void showFileSharingPage();

public slots:
  /**
   * Called when the user presses 'Ok'.
   */
  virtual void slotOk();      // Deletes the PropertiesDialog instance
  virtual void slotCancel();     // Deletes the PropertiesDialog instance

signals:
  /**
   * Emitted when we have finished with the properties (be it Apply or Cancel)
   */
  void propertiesClosed();
  void applied();
  void canceled();

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
  QPtrList<KPropsDlgPlugin> m_pageList;

private slots:
  void slotStatResult( KIO::Job * job );
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KPropertiesDialogPrivate;
  KPropertiesDialogPrivate *d;
};

/**
 * A Plugin in the Properties dialog
 * This is an abstract class. You must inherit from this class
 * to build a new kind of page.
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
class KPropsDlgPlugin : public QObject
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
   * The hosting PropertiesDialog will call @ref applyChanges only if the
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
class KFilePropsPlugin : public KPropsDlgPlugin
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

protected slots:
  void slotCopyFinished( KIO::Job * );
  void slotFileRenamed( KIO::Job *, const KURL &, const KURL & );
  void slotDirSizeFinished( KIO::Job * );
  void slotFoundMountPoint( const QString& mp, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail );
  void slotSizeStop();
  void slotSizeDetermine();

private slots:
  // workaround for compiler bug
void slotFoundMountPoint( const unsigned long& kBSize, const unsigned long& kBUsed, const unsigned long& kBAvail, const QString& mp );
 void nameFileChanged(const QString &text );
private:

  void determineRelativePath( const QString & path );

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
class KFilePermissionsPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
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
  void slotRecursiveClicked();

private:
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

  /**
   * Changeable Permissions
   */
  static mode_t fperm[3][4];

  class KFilePermissionsPropsPluginPrivate;
  KFilePermissionsPropsPluginPrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=Application
 *
 * Such files are used to represent a program in kpanel and kfm.
 * @internal
 */
class KExecPropsPlugin : public KPropsDlgPlugin
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
    QButton *execBrowse;

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
 * URL=....
 *
 * Such files are used to represent a program in kpanel and kfm.
 * @internal
 */
class KURLPropsPlugin : public KPropsDlgPlugin
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
 * Type=Application
 *
 * Such files are used to represent a program in kpanel and kfm.
 * @internal
 */
class KApplicationPropsPlugin : public KPropsDlgPlugin
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

private:

  void addMimeType( const QString & name );
  void updateButton();

  QLineEdit *commentEdit;
  QLineEdit *genNameEdit;
  QLineEdit *nameEdit;
  QListBox  *extensionsList;
  QListBox  *availableExtensionsList;
  QPushButton *addExtensionButton;
  QPushButton *delExtensionButton;

  class KApplicationPropsPluginPrivate;
  KApplicationPropsPluginPrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=MimeType
 * @internal
 */
class KBindingPropsPlugin : public KPropsDlgPlugin
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
class KDevicePropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  KDevicePropsPlugin( KPropertiesDialog *_props );
  virtual ~KDevicePropsPlugin();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

private slots:
  void slotActivated( int );

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

#endif

