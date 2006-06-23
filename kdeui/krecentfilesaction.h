/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KRECENTFILESACTION_H
#define KRECENTFILESACTION_H

#include <kselectaction.h>

class KConfig;
class KUrl;

/**
 *  @short Recent files action
 *
 *  This class is an action to handle a recent files submenu.
 *  The best way to create the action is to use KStdAction::openRecent.
 *  Then you simply need to call loadEntries on startup, saveEntries
 *  on shutdown, addURL when your application loads/saves a file.
 *
 *  @author Michael Koch
 */
class KDEUI_EXPORT KRecentFilesAction : public KSelectAction
{
  Q_OBJECT
  Q_PROPERTY( int maxItems READ maxItems WRITE setMaxItems )

public:
    /**
     * Constructs an action in the specified KActionCollection.
     *
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     */
    KRecentFilesAction(KActionCollection* parent, const QString& name);

    /**
     * Constructs an action with text; a shortcut may be specified by
     * the ampersand character (e.g. "&amp;Option" creates a shortcut with key \e O )
     *
     * This is the most common KAction used when you do not have a
     * corresponding icon (note that it won't appear in the current version
     * of the "Edit ToolBar" dialog, because an action needs an icon to be
     * plugged in a toolbar...).
     *
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     */
    KRecentFilesAction(const QString& text, KActionCollection* parent, const QString& name);

    /**
     * Constructs an action with text and an icon; a shortcut may be specified by
     * the ampersand character (e.g. "&amp;Option" creates a shortcut with key \e O )
     *
     * This is the other common KAction used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     * @param name An internal name for this action.
     */
    KRecentFilesAction(const KIcon& icon, const QString& text, KActionCollection* parent, const QString& name);

    /**
     * \overload KRecentFilesAction(const QIcon&, const QString&, KActionCollection*)
     *
     * This constructor differs from the above in that the icon is specified as
     * a icon name which can be loaded by KIconLoader.
     *
     * @param icon The name of the icon to load via KIconLoader.
     * @param text The text that will be displayed.
     * @param parent The action collection to contain this action.
     */
    KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction(const QString& icon, const QString& text, KActionCollection* parent, const QString& name);

  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const KShortcut& cut,
                      KActionCollection* parent, const QString& name = QString(),
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KUrl & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const QString& name = QString(),
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                      KActionCollection* parent, const QString& name = QString(),
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QString& pix, const KShortcut& cut,
                      KActionCollection* parent, const QString& name = QString(),
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The icons that go with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KUrl & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QIcon& pix, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const QString& name = QString(),
                      int maxItems = 10 );

  /**
   *  @param text The text that will be displayed.
   *  @param pix The dynamically loaded icon that goes with this action.
   *  @param cut The corresponding keyboard accelerator (shortcut).
   *  @param receiver The SLOT's parent.
   *  @param slot The SLOT to invoke when a URL is selected.
   *  Its signature is of the form slotURLSelected( const KUrl & ).
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( const QString& text, const QString& pix, const KShortcut& cut,
                      const QObject* receiver, const char* slot,
                      KActionCollection* parent, const QString& name = QString(),
                      int maxItems = 10 );

  /**
   *  @param parent This action's parent.
   *  @param name An internal name for this action.
   *  @param maxItems The maximum number of files to display
   */
  KDE_CONSTRUCTOR_DEPRECATED KRecentFilesAction( KActionCollection* parent = 0, const QString& name = QString(),
                      int maxItems = 10 );

  /**
   *  Destructor.
   */
  virtual ~KRecentFilesAction();

  /**
   * Adds \a action to the list of URLs, with \a url and title \a name.
   *
   * Do not use addAction(QAction*), as no url will be associated, and
   * consequently urlSelected() will not be emitted when \a action is selected.
   */
  void addAction(QAction* action, const KUrl& url, const QString& name);

  /**
   * Reimplemented for internal reasons.
   */
  virtual QAction* removeAction(QAction* action);

  /**
   * Reimplemented for internal reasons.
   */
  virtual void clear();

  /**
   *  Returns the maximum of items in the recent files list.
   */
  int maxItems() const;

  /**
   *  Sets the maximum of items in the recent files list.
   *  The default for this value is 10 set in the constructor.
   *
   *  If this value is lesser than the number of items currently
   *  in the recent files list the last items are deleted until
   *  the number of items are equal to the new maximum.
   */
  void setMaxItems( int maxItems );

  /**
   *  Loads the recent files entries from a given KConfig object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are load from a group called 'RecentFiles'
   *
   *  This method does not effect the active group of KConfig.
   */
  void loadEntries( KConfig* config, const QString &groupname=QString() );

  /**
   *  Saves the current recent files entries to a given KConfig object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are saved to a group called 'RecentFiles'
   *
   *  This method does not effect the active group of KConfig.
   */
  void saveEntries( KConfig* config, const QString &groupname=QString() );

  /**
   *  Add URL to recent files list.
   *
   *  @param url The URL of the file
   *  @param name The user visible pretty name that appears before the URL
   */
  void addUrl( const KUrl& url, const QString& name = QString() );

  /**
   *  Remove an URL from the recent files list.
   *
   *  @param url The URL of the file
   */
  void removeUrl( const KUrl& url );

Q_SIGNALS:
  /**
   *  This signal gets emited when the user selects an URL.
   *
   *  @param url The URL thats the user selected.
   */
  void urlSelected( const KUrl& url );

private Q_SLOTS:
  void urlSelected( QAction* action );

private:
  void init();

  class KRecentFilesActionPrivate* const d;
};

#endif
