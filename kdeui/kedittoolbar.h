// -*- mode: c++; c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KEDITTOOLBAR_H
#define _KEDITTOOLBAR_H

#include <qwidget.h>
#include <kxmlguiclient.h>
#include <kdialogbase.h>

class KProcess;
class KActionCollection;
class QComboBox;
class QToolButton;
class KListView;
class Q3ListViewItem;

class KEditToolbarWidget;
class KEditToolbarPrivate;
class KEditToolbarWidgetPrivate;
namespace
{
  class ToolbarItem;
  class ToolbarListView;
}
/**
 * @short A dialog used to customize or configure toolbars.
 *
 * This dialog only works if your application uses the XML UI
 * framework for creating menus and toolbars.  It depends on the XML
 * files to describe the toolbar layouts and it requires the actions
 * to determine which buttons are active.
 *
 * Typically, you would include the KStdAction::configureToolbars()
 * standard action in your application.  In your slot to this action,
 * you would have something like so:
 *
 * \code
 * KEditToolbar dlg(actionCollection());
 * if (dlg.exec())
 * {
 *   createGUI();
 * }
 * \endcode
 *
 * That code snippet also takes care of redrawing the menu and
 * toolbars if you have made any changes.
 *
 * If you are using KMainWindow's settings methods (either save/apply manually
 * or autoSaveSettings), you should write something like:
 * \code
 * void MyClass::slotConfigureToolbars()
 * {
 *   saveMainWindowSettings( KGlobal::config(), "MainWindow" );
 *   KEditToolbar dlg(actionCollection());
 *   connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
 *   dlg.exec();
 * }
 *
 * void MyClass::slotNewToolbarConfig() // This is called when OK, Apply or Defaults is clicked
 * {
 *    ...if you use any action list, use plugActionList on each here...
 *    createGUI();
 *    applyMainWindowSettings( KGlobal::config(), "MainWindow" );
 * }
 * \endcode
 *
 * Note that the procedure is a bit different for KParts applications.
 * In this case, you need only pass along a pointer to your
 * application's KXMLGUIFactory object.  The editor will take care of
 * finding all of the action collections and XML files.  The editor
 * aims to be semi-intelligent about where it assigns any
 * modifications.  In other words, it will not write out part specific
 * changes to your shell's XML file.
 *
 * An example would be:
 *
 * \code
 * saveMainWindowSettings( KGlobal::config(), "MainWindow" );
 * KEditToolbar dlg(factory());
 * connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
 * dlg.exec();
 *
 * void MyClass::slotNewToolbarConfig() // This is called when OK, Apply or Defaults is clicked
 * {
 *    ...if you use any action list, use plugActionList on each here...
 *    // Do NOT call createGUI()!
 *    applyMainWindowSettings( KGlobal::config(), "MainWindow" );
 * }
 * \endcode
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KDEUI_EXPORT KEditToolbar : public KDialogBase
{
    Q_OBJECT
public:
  /**
   * Constructor for apps that do not use components.
   *
   * This is the
   * only entry point to this class.  You @em must pass along your
   * collection of actions (some of which appear in your toolbars).
   * The other two parameters are optional.
   *
   * The second parameter, xmlfile(), is the name (absolute or
   * relative) of your application's UI resource file.  If it is
   * left blank, then the resource file: share/apps/appname/appnameui.rc
   * is used.  This is the same resource file that is used by the
   * default createGUI() function in KMainWindow so you're usually
   * pretty safe in leaving it blank.
   *
   * The third parameter, global(), controls whether or not the
   * global resource file is used.  If this is @p true, then you may
   * edit all of the actions in your toolbars -- global ones and
   * local one.  If it is @p false, then you may edit only your
   * application's entries.  The only time you should set this to
   * false is if your application does not use the global resource
   * file at all (very rare).
   *
   * @param collection The collection of actions to work on.
   * @param xmlfile The application's local resource file.
   * @param global If @p true, then the global resource file will also
   *               be parsed.
   * @param parent The parent of the dialog.
   */
  KEditToolbar(KActionCollection *collection,
               const QString& xmlfile = QString::null, bool global = true,
               QWidget* parent = 0);

  //KDE 4.0: merge the two constructors
  /* Constructor for apps that do not use components, which has an extra argument
   * specifying the toolbar to be shown.
   * @param defaultToolbar The toolbar with this name will appear for editing.
   * @param collection The collection of actions to work on.
   * @param xmlfile The application's local resource file.
   * @param global If @p true, then the global resource file will also
   *               be parsed.
   * @param parent The parent of the dialog.
   * @param name An internal name.
   * @since 3.2
   */
  KEditToolbar(const QString& defaultToolbar, KActionCollection *collection,
               const QString& xmlfile = QString::null, bool global = true,
               QWidget* parent = 0);
  /**
   * Constructor for KParts based apps.
   *
   * The main parameter, factory(), is a pointer to the
   * XML GUI factory object for your application.  It contains a list
   * of all of the GUI clients (along with the action collections and
   * xml files) and the toolbar editor uses that.
   *
   * Use this like so:
   * \code
   * KEditToolbar edit(factory());
   * if ( edit.exec() )
   * ...
   * \endcode
   *
   * @param factory Your application's factory object
   * @param parent The usual parent for the dialog.
   *
   * Some people seem tempted to use this also in non-KParts apps, using KMainWindow::guiFactory().
   * This works, but only _if_ setting conserveMemory to false when calling
   * KMainWindow::createGUI()! If not, use the other KEditToolbar constructor.
   */
  KEditToolbar(KXMLGUIFactory* factory, QWidget* parent = 0);

  //KDE 4.0: merge the two constructors
  /** Constructor for KParts based apps, which has an extra argument
   * specifying the toolbar to be shown.
   *
   * @param defaultToolbar The toolbar with this name will appear for editing.
   * @param factory Your application's factory object
   * @param parent The usual parent for the dialog.
   * @since 3.2
   */
  KEditToolbar(const QString& defaultToolbar, KXMLGUIFactory* factory,
               QWidget* parent = 0);

  /// destructor
  ~KEditToolbar();

  /** Sets the default toolbar, which will be auto-selected when the constructor without the
  *    defaultToolbar argument is used.
  *   @param  toolbarName  the name of the toolbar
  *   @since 3.3
  */
  static void setDefaultToolbar(const char *toolbarName);

protected slots:
  /**
   * Overridden in order to save any changes made to the toolbars
   */
  virtual void slotOk();
  /**
   * idem
   */
  virtual void slotApply();

  /** should OK really save?
  * @internal
  **/
  void acceptOK(bool b);

  /**
   * Set toolbars to default value
  **/
  void slotDefault();

signals:
  /**
   * Signal emitted when 'apply' or 'ok' is clicked or toolbars were resetted.
   * Connect to it, to plug action lists and to call applyMainWindowSettings
   * (see sample code in this class's documentation)
   */
  void newToolbarConfig();

private:
  void init();
  KEditToolbarWidget *m_widget;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  KEditToolbarPrivate *d;

  static const char *s_defaultToolbar;
};


/**
 * @short A widget used to customize or configure toolbars
 *
 * This is the widget that does all of the work for the
 * KEditToolbar dialog.  In most cases, you will want to use the
 * dialog instead of this widget directly.
 *
 * Typically, you would use this widget only if you wanted to embed
 * the toolbar editing directly into your existing configure or
 * preferences dialog.
 *
 * This widget only works if your application uses the XML UI
 * framework for creating menus and toolbars.  It depends on the XML
 * files to describe the toolbar layouts and it requires the actions
 * to determine which buttons are active.
 *
 * @author Kurt Granroth <granroth@kde.org>
 */
class KDEUI_EXPORT KEditToolbarWidget : public QWidget, virtual public KXMLGUIClient
{
  Q_OBJECT
public:
  /**
   * Constructor.  This is the only entry point to this class.  You
   * @p must pass along your collection of actions (some of which
   * appear in your toolbars).  The other three parameters are
   * optional.
   *
   * The second parameter, xmlfile, is the name (absolute or
   * relative) of your application's UI resource file.  If it is
   * left blank, then the resource file: share/apps/appname/appnameui.rc
   * is used.  This is the same resource file that is used by the
   * default createGUI function in KMainWindow so you're usually
   * pretty safe in leaving it blank.
   *
   * The third parameter, global, controls whether or not the
   * global resource file is used.  If this is true, then you may
   * edit all of the actions in your toolbars -- global ones and
   * local one.  If it is false, then you may edit only your
   * application's entries.  The only time you should set this to
   * false is if your application does not use the global resource
   * file at all (very rare)
   *
   * The last parameter, parent, is the standard parent stuff.
   *
   * @param collection The collection of actions to work on
   * @param xmlfile The application's local resource file
   * @param global If true, then the global resource file will also
   *               be parsed
   * @param parent This widget's parent
   */
  KEditToolbarWidget(KActionCollection *collection,
                     const QString& xmlfile = QString::null,
                     bool global = true, QWidget *parent = 0L);

   //KDE 4.0: merge the two constructors
   /* Same as above, with an extra agrument specifying the toolbar to be shown.
   *
   * @param defaultToolbar The toolbar with this name will appear for editing.
   * @param collection The collection of actions to work on
   * @param xmlfile The application's local resource file
   * @param global If true, then the global resource file will also
   *               be parsed
   * @param parent This widget's parent
   * @since 3.2
   */
  KEditToolbarWidget(const QString& defaultToolbar,
                     KActionCollection *collection,
                     const QString& file = QString::null,
                     bool global = true,
                     QWidget *parent = 0L);

  /**
   * Constructor for KParts based apps.
   *
   * The first parameter, factory, is a pointer to the XML GUI
   * factory object for your application.  It contains a list of all
   * of the GUI clients (along with the action collections and xml
   * files) and the toolbar editor uses that.
   *
   * The second parameter, parent, is the standard parent
   *
   * Use this like so:
   * \code
   * KEditToolbar edit(factory());
   * if ( edit.exec() )
   * ...
   * \endcode
   *
   * @param factory Your application's factory object
   * @param parent This widget's parent
   */
  KEditToolbarWidget(KXMLGUIFactory* factory, QWidget *parent = 0L);

   //KDE 4.0: merge the two constructors
   /* Same as above, with an extra agrument specifying the toolbar to be shown.
   *
   *
   * @param defaultToolbar The toolbar with this name will appear for editing.
   * @param factory Your application's factory object
   * @param parent This widget's parent
   * @since 3.2
   */
  KEditToolbarWidget(const QString& defaultToolbar,
                     KXMLGUIFactory* factory,
                     QWidget *parent = 0L);

  /**
   * Destructor.  Note that any changes done in this widget will
   * @p NOT be saved in the destructor.  You @p must call save()
   * to do that.
   */
  virtual ~KEditToolbarWidget();

  /**
   * @internal Reimplemented for internal purposes.
   */
  virtual KActionCollection *actionCollection() const;

  /**
   * Save any changes the user made.  The file will be in the user's
   * local directory (usually $HOME/.kde/share/apps/\<appname\>).  The
   * filename will be the one specified in the constructor.. or the
   * made up one if the filename was NULL.
   *
   * @return The status of whether or not the save succeeded.
   */
  bool save();

  /**
   * Remove and readd all KMXLGUIClients to update the GUI
   * @since 3.5
   */
  void rebuildKXMLGUIClients();

signals:
  /**
   * Emitted whenever any modifications are made by the user.
   */
  void enableOk(bool);

protected slots:
  void slotToolbarSelected(const QString& text);

  void slotInactiveSelected(Q3ListViewItem *item);
  void slotActiveSelected(Q3ListViewItem *item);

  void slotDropped(KListView *list, QDropEvent *e, Q3ListViewItem *after);

  void slotInsertButton();
  void slotRemoveButton();
  void slotUpButton();
  void slotDownButton();

  void slotChangeIcon();

private slots:
  void slotProcessExited( KProcess* );

private:
  void setupLayout();

  void insertActive(ToolbarItem *item, Q3ListViewItem *before, bool prepend = false);
  void removeActive(ToolbarItem *item);
  void moveActive(ToolbarItem *item, Q3ListViewItem *before);
  void initNonKPart(KActionCollection *collection, const QString& file, bool global);
  void initKPart(KXMLGUIFactory* factory);
  void loadToolbarCombo(const QString& defaultToolbar = QString::null);
  void loadActionList(QDomElement& elem);
  void updateLocal(QDomElement& elem);

private:
  ToolbarListView *m_inactiveList;
  ToolbarListView *m_activeList;
  QComboBox *m_toolbarCombo;

  QToolButton *m_upAction;
  QToolButton *m_removeAction;
  QToolButton *m_insertAction;
  QToolButton *m_downAction;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KEditToolbarWidgetPrivate *d;
};

#endif // _KEDITTOOLBAR_H
