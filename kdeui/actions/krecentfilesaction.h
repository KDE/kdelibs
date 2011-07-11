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
#include <kurl.h>

class KConfigGroup;
class KRecentFilesActionPrivate;

/**
 *  @short Recent files action
 *
 *  This class is an action to handle a recent files submenu.
 *  The best way to create the action is to use KStandardAction::openRecent.
 *  Then you simply need to call loadEntries on startup, saveEntries
 *  on shutdown, addURL when your application loads/saves a file.
 *
 *  @author Michael Koch
 */
class KDEUI_EXPORT KRecentFilesAction : public KSelectAction
{
  Q_OBJECT
  Q_PROPERTY( int maxItems READ maxItems WRITE setMaxItems )
  Q_DECLARE_PRIVATE(KRecentFilesAction)

public:
    /**
     * Constructs an action with the specified parent.
     *
     * @param parent The parent of this action.
     */
    explicit KRecentFilesAction(QObject *parent);

    /**
     * Constructs an action with text; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the most common KAction used when you do not have a
     * corresponding icon (note that it won't appear in the current version
     * of the "Edit ToolBar" dialog, because an action needs an icon to be
     * plugged in a toolbar...).
     *
     * @param text The text that will be displayed.
     * @param parent The parent of this action.
     */
    KRecentFilesAction(const QString &text, QObject *parent);

    /**
     * Constructs an action with text and an icon; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the other common KAction used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The parent of this action.
     */
    KRecentFilesAction(const KIcon &icon, const QString &text, QObject *parent);

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

public Q_SLOTS:
  /**
   * Clears the recent files list.
   * Note that there is also an action shown to the user for clearing the list.
   */
  virtual void clear();

public:
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
   *  Loads the recent files entries from a given KConfigGroup object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are load from a group called 'RecentFiles'
   *
   */
  void loadEntries( const KConfigGroup &config );

  /**
   *  Saves the current recent files entries to a given KConfigGroup object.
   *  You can provide the name of the group used to load the entries.
   *  If the groupname is empty, entries are saved to a group called 'RecentFiles'
   *
   */
  void saveEntries( const KConfigGroup &config );

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

  /**
   *  Retrieve a list of all URLs in the recent files list.
   */
  KUrl::List urls() const;

Q_SIGNALS:
  /**
   *  This signal gets emitted when the user selects an URL.
   *
   *  @param url The URL thats the user selected.
   */
  void urlSelected( const KUrl& url );

  /**
   *  This signal gets emitted when the user clear list. 
   *  So when user store url in specific config file it can saveEntry.
   *  @since 4.3
   */ 
  void recentListCleared();

private:
    //Internal
    void clearEntries();
    // Don't warn about the virtual overload. As the comment of the other
    // addAction() says, addAction( QAction* ) should not be used.
    using KSelectAction::addAction;
    
    Q_PRIVATE_SLOT( d_func(), void _k_urlSelected(QAction*) )
};

#endif
