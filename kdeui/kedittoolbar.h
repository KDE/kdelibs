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

class KActionCollection;
class QComboBox;
class QPushButton;
class KListView;;
class QListViewItem;

class KEditToolbarWidget;
class KEditToolbarPrivate;
class KEditToolbarWidgetPrivate;

/**
 * This is a dialog used to customize or configure toolbars.
 *
 * This dialog only works if your application uses the XML UI
 * framework for creating menus and toolbars.  It depends on the XML
 * files to describe the toolbar layouts and it requires the actions
 * to determine which buttons are active.
 *
 * Typically, you would include the @ref KStdAction::configureToolbars()
 * standard action in your application.  In your slot to this action,
 * you would have something like so:
 *
 * <pre>
 * KEditToolbar dlg(actionCollection());
 * if (dlg.exec())
 * {
 *   createGUI();
 * }
 * </pre>
 *
 * That code snippet also takes care of redrawing the menu and
 * toolbars if you have made any changes.
 *
 * If you are using KMainWindow's settings methods (either save/apply manually
 * or autoSaveSettings), you should write something like:
 * <pre>
 * void MyClass::slotConfigureToolbars()
 * {
 *   saveMainWindowSettings( KGlobal::config(), "MainWindow" );
 *   KEditToolbar dlg(actionCollection());
 *   connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
 *   dlg.exec();
 * }
 *
 * void MyClass::slotNewToolbarConfig() // This is called when OK or Apply is clicked
 * {
 *    ...if you use any action list, use plugActionList on each here...
 *    applyMainWindowSettings( KGlobal::config(), "MainWindow" );
 *    createGUI();
 * }
 * </pre>
 *
 * Note that the procedure is a bit different for KParts applications.
 * In this case, you need only pass along a pointer to your
 * application's @ref KXMLGUIFactory object.  The editor will take care of
 * finding all of the action collections and xml files.  The editor
 * aims to be semi-intelligent about where it assigns any
 * modifications.  In other words, it will not write out part specific
 * changes to your shell's xml file.
 *
 * An example would be:
 *
 * <pre>
 * saveMainWindowSettings( KGlobal::config(), "MainWindow" );
 * KEditToolbar dlg(factory());
 * connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
 * dlg.exec();
 *
 * void MyClass::slotNewToolbarConfig() // This is called when OK or Apply is clicked
 * {
 *    ...if you use any action list, use plugActionList on each here...
 *    applyMainWindowSettings( KGlobal::config(), "MainWindow" );
 *    // Do NOT call createGUI()!
 * }
 * </pre>
 *
 * @short A dialog used to customize or configure toolbars.
 * @author Kurt Granroth <granroth@kde.org>
 * @id $Id$
 */
class KEditToolbar : public KDialogBase
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
   * The second parameter, @ref xmlfile(), is the name (absolute or
   * relative) of your application's UI resource file.  If it is
   * left blank, then the resource file: share/apps/appname/appnameui.rc
   * is used.  This is the same resource file that is used by the
   * default @ref createGUI() function in @ref KMainWindow so you're usually
   * pretty safe in leaving it blank.
   *
   * The third parameter, @ref global(), controls whether or not the
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
   * @param name An internal name.
   */
  KEditToolbar(KActionCollection *collection,
               const QString& xmlfile = QString::null, bool global = true,
			   QWidget* parent = 0, const char* name = 0);

  /**
   * Constructor for KParts based apps.
   *
   * The main parameter, @ref factory(), is a pointer to the
   * XML GUI factory object for your application.  It contains a list
   * of all of the GUI clients (along with the action collections and
   * xml files) and the toolbar editor uses that.
   *
   * Use this like so:
   * <pre>
   * KEditToolbar edit(factory());
   * if ( edit.exec() )
   * ...
   * </pre>
   *
   * @param factory Your application's factory object
   * @param parent The usual parent for the dialog.
   * @param name An internal name.
   */
  KEditToolbar(KXMLGUIFactory* factory, QWidget* parent = 0, const char* name = 0);

  /// destructor
  ~KEditToolbar();

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

signals:
  /**
   * Signal emitted when 'apply' or 'ok' is clicked.
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
};

/**
 * This is a widget used to customize or configure toolbars
 *
 * This is the widget that does all of the work for the
 * @ref KEditToolbar dialog.  In most cases, you will want to use the
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
 * @short A widget used to customize or configure toolbars
 * @author Kurt Granroth <granroth@kde.org>
 * @id $Id$
 */
class KEditToolbarWidget : public QWidget, virtual public KXMLGUIClient
{
  Q_OBJECT
public:
  /**
   * Constructor.  This is the only entry point to this class.  You
   * @p must pass along your collection of actions (some of which
   * appear in your toolbars).  The other three parameters are
   * optional.
   *
   * The second parameter, @ref #xmlfile, is the name (absolute or
   * relative) of your application's UI resource file.  If it is
   * left blank, then the resource file: share/apps/appname/appnameui.rc
   * is used.  This is the same resource file that is used by the
   * default createGUI function in KMainWindow so you're usually
   * pretty safe in leaving it blank.
   *
   * The third parameter, @ref #global, controls whether or not the
   * global resource file is used.  If this is true, then you may
   * edit all of the actions in your toolbars -- global ones and
   * local one.  If it is false, then you may edit only your
   * application's entries.  The only time you should set this to
   * false is if your application does not use the global resource
   * file at all (very rare)
   *
   * The last parameter, @ref #parent, is the standard parent stuff.
   *
   * @param collection The collection of actions to work on
   * @param xmlfile The application's local resource file
   * @param global If true, then the global resource file will also
   *               be parsed
   * @param parent This widget's parent
   */
  KEditToolbarWidget(KActionCollection *collection,
                     const QString& file = QString::null,
                     bool global = true, QWidget *parent = 0L);

  /**
   * Constructor for KParts based apps.
   *
   * The first parameter, @ref #factory, is a pointer to the XML GUI
   * factory object for your application.  It contains a list of all
   * of the GUI clients (along with the action collections and xml
   * files) and the toolbar editor uses that.
   *
   * The second parameter, @ref #parent, is the standard parent
   *
   * Use this like so:
   * <pre>
   * KEditToolbar edit(factory());
   * if ( edit.exec() )
   * ...
   * </pre>
   *
   * @param factory Your application's factory object
   * @param parent This widget's parent
   */
  KEditToolbarWidget(KXMLGUIFactory* factory, QWidget *parent = 0L);

  /**
   * Destructor.  Note that any changes done in this widget will
   * @p NOT be saved in the destructor.  You @p must call @ref save()
   * to do that.
   */
  virtual ~KEditToolbarWidget();

  /**
   * This is overriden so that the KXMLGUIClient internal functions
   * have the proper action collections to deal with.  You shouldn't
   * have to do anything with this as it was your collection in the
   * first place.
   *
   * @return Your application's collection of actions
   */
  //virtual KActionCollection *actionCollection() const;

  /**
   * Save any changes the user made.  The file will be in the user's
   * local directory (usually $HOME/.kde/share/apps/<appname>).  The
   * filename will be the one specified in the constructor.. or the
   * made up one if the filename was NULL.
   *
   * @return The status of whether or not the save succeeded.
   */
  bool save();

signals:
  /**
   * Emitted whenever any modifications are made by the user.
   */
  void enableOk(bool);

protected slots:
  void slotToolbarSelected(const QString& text);

  void slotInactiveSelected(QListViewItem *item);
  void slotActiveSelected(QListViewItem *item);

  void slotInsertButton();
  void slotRemoveButton();
  void slotUpButton();
  void slotDownButton();

protected:
  void setupLayout();

  void loadToolbarCombo();
  void loadActionList(QDomElement& elem);
  void updateLocal(QDomElement& elem);

private:
  KListView *m_inactiveList;
  KListView *m_activeList;
  QComboBox *m_toolbarCombo;

  QPushButton *m_upAction;
  QPushButton *m_removeAction;
  QPushButton *m_insertAction;
  QPushButton *m_downAction;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KEditToolbarWidgetPrivate *d;
};

#endif // _KEDITTOOLBAR_H
