/*
   This file is part of the KDE libraries
   Copyright (c) 2004 Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>

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

#ifndef KICONVIEWSEARCHLINE_H
#define KICONVIEWSEARCHLINE_H

#include <kde3support_export.h>
#include <klineedit.h>

class Q3IconView;
class Q3IconViewItem;

/**
 * This class makes it easy to add a search line for filtering the items in a
 * iconview based on a simple text search.
 *
 * No changes to the application other than instantiating this class with an
 * appropriate QIconView should be needed.
 */
class KDE3SUPPORT_EXPORT K3IconViewSearchLine : public KLineEdit
{
  Q_OBJECT

public:

  /**
   * Constructs a K3IconViewSearchLine with \a iconView being the QIconView to
   * be filtered.
   *
   * If \a iconView is null then the widget will be disabled until a iconview
   * is set with setIconView().
   */
  K3IconViewSearchLine( QWidget *parent = 0,
		       Q3IconView *iconView = 0 );

  /**
   * Destroys the K3IconViewSearchLine.
   */
  virtual ~K3IconViewSearchLine();

  /**
   * Returns true if the search is case sensitive.  This defaults to false.
   *
   * @see setCaseSensitive()
   */
  bool caseSensitive() const;

  /**
   * Returns the iconview that is currently filtered by the search.
   *
   * @see setIconView()
   */
  Q3IconView *iconView() const;


public Q_SLOTS:
  /**
   * Updates search to only make visible the items that match \a s.  If
   * \a s is null then the line edit's text will be used.
   */
  virtual void updateSearch( const QString &s = QString() );

  /**
   * Make the search case sensitive or case insensitive.
   *
   * @see caseSenstive()
   */
  void setCaseSensitive( bool cs );

  /**
   * Sets the QIconView that is filtered by this search line.  If \a lv is null
   * then the widget will be disabled.
   *
   * @see iconView()
   */
  void setIconView( Q3IconView *iv );


  /**
   * Clear line edit and empty hiddenItems, returning elements to iconView.
   */
  void clear();


protected:
  /**
   * Returns true if \a item matches the search \a s.  This will be evaluated
   * based on the value of caseSensitive().  This can be overridden in
   * subclasses to implement more complicated matching schemes.
   */
  virtual bool itemMatches( const Q3IconViewItem *item,
			    const QString &s ) const;


  /**
   * Do initialization common to both constructors.
   */
  void init( Q3IconView *iconView = 0 );

  /**
   * Hide item.
   */
  void hideItem( Q3IconViewItem *item );

  /**
   * Show item.
   *
   * Just unhide it, doesn't necessary show it on screen, for that use
   * iconView->ensureItemVisible()
   */
  void showItem( Q3IconViewItem *item );


protected Q_SLOTS:
  /**
   * When keys are pressed a new search string is created and a timer is
   * activated.  The most recent search is activated when this timer runs out
   * if another key has not yet been pressed.
   *
   * This method makes @param s the most recent search and starts the
   * timer.
   *
   * Together with activateSearch() this makes it such that searches are not
   * started until there is a short break in the users typing.
   *
   * @see activateSearch()
   */
  void queueSearch( const QString &s );

  /**
   * When the timer started with queueSearch() expires this slot is called.
   * If there has been another timer started then this slot does nothing.
   * However if there are no other pending searches this starts the icon view
   * search.
   *
   * @see queueSearch()
   */
  void activateSearch();


private Q_SLOTS:
  /**
   * Take action need when iconView is deleted.
   */
  void iconViewDeleted();


private:
  class K3IconViewSearchLinePrivate;
  K3IconViewSearchLinePrivate *d;
};


#endif /* KICONVIEWSEARCHLINE_H */
