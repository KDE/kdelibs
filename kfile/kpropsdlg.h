/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qstring.h>
#include <qlist.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qtooltip.h>

#include <kurl.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kdialogbase.h>

class QLineEdit;
class QCheckBox;
class QPushButton;
class KLineEdit;

class KIconButton;
class KPropsPage;

namespace KIO { class Job; }

/**
 * The main class.
 * This one is visible to the one who created the dialog.
 * It brings up a QTabDialog.
 * This class must be created with (void)new PropertiesDialog(...)
 * It will take care of deleting itself.
 */
class KPropertiesDialog : public QObject
{
  Q_OBJECT
public:

  /**
   * @return whether there are any property pages available for the given file items
   */
  static bool canDisplay( KFileItemList _items );

  /**
   * Bring up a Properties dialog. Normal constructor for
   * file-manager-like applications.
   *
   * @param _items file item whose properties should be displayed.
   */
  KPropertiesDialog( KFileItem * item );

  /**
   * Bring up a Properties dialog. Normal constructor for
   * file-manager-like applications.
   *
   * @param _items list of file items whose properties should be
   * displayed. NOTE : the current limitations of PropertiesDialog
   * makes it use only the FIRST item in the list
   *
   */
  KPropertiesDialog( KFileItemList _items );

  /**
   * Bring up a Properties dialog. Convenience constructor for
   * non-file-manager applications.
   *
   * @param _url the URL whose properties should be displayed
   * @param _mode the mode, as returned by stat(). Don't set if unknown.  */
  KPropertiesDialog( const KURL& _url, mode_t _mode = (mode_t) -1 );

  /**
   * Create a properties dialog for a new .desktop file (whose name
   * is not known yet), based on a template. Special constructor for
   * "File / New" in file-manager applications.
   *
   * @param _templUrl template used for reading only
   * @param _currentDir directory where the file will be written to
   * @param _defaultName something to put in the name field, like mimetype.desktop */
  KPropertiesDialog( const KURL& _tempUrl, const KURL& _currentDir,
                    const QString& _defaultName );

  /**
   * This looks very much like a destructor :)
   */
  virtual ~KPropertiesDialog();


  /**
   * Adds a "3rd party" properties page to the dialog.  Useful
   * for extending the properties mechanism.
   *
   * To create a new page type, inherit from the base class PropsPage
   * and implement all the methods.
   *
   * @param page is a pointer to the PropsPage widget.  The Properties
   *        dialog will do destruction for you.  The PropsPage MUST
   *        have been created with the Properties Dialog as its parent.
   * @see PropsPage
   */
  void addPage(KPropsPage *page);

  /**
   * @return a parsed URL.
   * Valid only if dialog shown for one file/url.
   */
  const KURL& kurl() const { return m_singleUrl; }

  /**
   * @return the file item for which the dialog is shown
   * HACK : returns the first item of the list
   */
  KFileItem *item() { return m_items.first(); }

  KFileItemList items() const { return m_items; }

  /**
   * @return a pointer to the dialog
   */
  KDialogBase* dialog() const { return tab; }

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
   * @param _name new URL
   */
  void updateUrl( const KURL& _newUrl );

  /**
   * #see FilePropsPage::applyChanges
   * @param _name new filename, encoded.
   */
  void rename( const QString& _name );

public slots:
  /**
   * Called when the user presses 'Ok'.
   */
  void slotApply();      // Deletes the PropertiesDialog instance
  void slotCancel();     // Deletes the PropertiesDialog instance

signals:
  /**
   * Notify that we have finished with the properties (be it Apply or Cancel)
   */
  void propertiesClosed();
  void applied();
  void canceled();

private:

  /**
   * Common initialization for all constructors
   */
  void init();
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
  bool m_bMustDestroyItems;

  /** For templates */
  QString m_defaultName;
  KURL m_currentDir;

  /**
   * List of all pages inserted ( first one first )
   */
  QList<KPropsPage> pageList;

  /**
   * The dialog
   */
  KDialogBase *tab;

private slots:
  void slotDeleteMyself();
private:
  class KPropertiesDialogPrivate;
  KPropertiesDialogPrivate *d;
};

/**
 * A Page in the Properties dialog
 * This is an abstract class. You must inherit from this class
 * to build a new kind of page.
 */
class KPropsPage : public QObject
{
  Q_OBJECT
public:
  /**
   * Constructor
   * To insert tabs into the properties dialog, use the add methods provided by
   * KDialogBase (via props->dialog() )
   */
  KPropsPage( KPropertiesDialog *_props );
  virtual ~KPropsPage();

  /**
   * Apply all changes to the file.
   * This function is called when the user presses 'Ok'. The last page inserted
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
   * Emit this event when the user changed anything the page's tabs.
   * The hosting PropertiesDialog will call @ref applyChanges only if the
   * PropsPage emits the changed event.
   */
  void changed();

protected:
  /**
   * Pointer to the dialog
   */
  KPropertiesDialog *properties;

  int fontHeight;
private:
  class KPropsPagePrivate;
  KPropsPagePrivate *d;
};

/**
 * 'General' page
 *  This page displays the name of the file, its size and access times.
 */
class KFilePropsPage : public KPropsPage
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KFilePropsPage( KPropertiesDialog *_props );
  virtual ~KFilePropsPage();

  /**
   * Applies all changes made.  'General' must be always the first
   * page in the dialog, since this function may rename the file which
   * may confuse other applyChanges functions. When this page is the
   * first one this means that this applyChanges function is the first
   * one called.
   */
  virtual void applyChanges();

  /**
   * Called after all pages applied their changes
   */
  void postApplyChanges();

  /**
   * Tests whether the files specified by _items need a 'General' page.
   */
  static bool supports( KFileItemList _items );

protected slots:
  void slotRenameFinished( KIO::Job * );

private:
  QWidget *iconArea;
  QWidget *nameArea;

  QString m_sRelativePath;
  bool m_bFromTemplate;

  /**
   * The initial filename
   */
  QString oldName;

  class KFilePropsPagePrivate;
  KFilePropsPagePrivate *d;
};

/**
 * 'Permissions' page
 * In this page you can modify permissions and change
 * the owner of a file.
 */
class KFilePermissionsPropsPage : public KPropsPage
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KFilePermissionsPropsPage( KPropertiesDialog *_props );
  virtual ~KFilePermissionsPropsPage();

  virtual void applyChanges();

  /**
   * Tests whether the file specified by _items needs a 'Permissions' page.
   */
  static bool supports( KFileItemList _items );

private slots:

  void slotChmodResult( KIO::Job * );

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

  class KFilePermissionsPropsPagePrivate;
  KFilePermissionsPropsPagePrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=Application
 *
 * Such files are used to represent a program in kpanel and kfm.
 */
class KExecPropsPage : public KPropsPage
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KExecPropsPage( KPropertiesDialog *_props );
  virtual ~KExecPropsPage();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

public slots:
  void slotBrowseExec();

private slots:
  void enableCheckedEdit();
  void enableSuidEdit();

private:

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

    class KExecPropsPagePrivate;
    KExecPropsPagePrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * URL=....
 *
 * Such files are used to represent a program in kpanel and kfm.
 */
class KURLPropsPage : public KPropsPage
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KURLPropsPage( KPropertiesDialog *_props );
  virtual ~KURLPropsPage();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

private:
  QLineEdit *URLEdit;
  KIconButton *iconBox;

  QString URLStr;
  QString iconStr;

  QPixmap pixmap;
  QString pixmapFile;
private:
  class KURLPropsPagePrivate;
  KURLPropsPagePrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=Application
 *
 * Such files are used to represent a program in kpanel and kfm.
 */
class KApplicationPropsPage : public KPropsPage
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KApplicationPropsPage( KPropertiesDialog *_props );
  virtual ~KApplicationPropsPage();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

public slots:
  void slotDelExtension();
  void slotAddExtension();

private:

  void addMimeType( const QString & name );

  QLineEdit *commentEdit;
  QLineEdit *nameEdit;
  QListBox  *extensionsList;
  QListBox  *availableExtensionsList;
  QPushButton *addExtensionButton;
  QPushButton *delExtensionButton;

  QString nameStr;
  QStringList extensions;
  QString commentStr;

  class KApplicationPropsPagePrivate;
  KApplicationPropsPagePrivate *d;
};

/**
 * Used to edit the files containing
 * [Desktop Entry]
 * Type=MimeType
 */
class KBindingPropsPage : public KPropsPage
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KBindingPropsPage( KPropertiesDialog *_props );
  virtual ~KBindingPropsPage();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

private:

  QLineEdit *commentEdit;
  QLineEdit *patternEdit;
  QLineEdit *mimeEdit;
  QString m_sMimeStr;

  QCheckBox * cbAutoEmbed;

  class KBindingPropsPagePrivate;
  KBindingPropsPagePrivate *d;
};

/**
 * Properties page for device .desktop files
 */
class KDevicePropsPage : public KPropsPage
{
  Q_OBJECT
public:
  KDevicePropsPage( KPropertiesDialog *_props );
  virtual ~KDevicePropsPage();

  virtual void applyChanges();

  static bool supports( KFileItemList _items );

private slots:
  void slotActivated( int );

private:
  QComboBox* device;
  QLineEdit* mountpoint;
  QCheckBox* readonly;
  QLineEdit* fstype;
  //KIconButton* mounted;
  KIconButton* unmounted;

  bool IamRoot;

  QStringList m_devicelist;
  int indexDevice;
  int indexMountPoint;
  int indexFSType;

  QPixmap pixmap;
  QString pixmapFile;

  class KDevicePropsPagePrivate;
  KDevicePropsPagePrivate *d;
};

#endif

