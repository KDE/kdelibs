/*
  This file is part of the KDE libraries
  Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>
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

#ifndef KLISTWIDGETSEARCHLINE_H
#define KLISTWIDGETSEARCHLINE_H

#include <klineedit.h>

class QListWidget;
class QListWidgetItem;

/**
 * This class makes it easy to add a search line for filtering the items in a
 * listwidget based on a simple text search.
 *
 * No changes to the application other than instantiating this class with an
 * appropriate QListWidget should be needed.
 */
class KDEUI_EXPORT KListWidgetSearchLine : public KLineEdit
{
    Q_OBJECT

public:

    /**
     * Constructs a KListWidgetSearchLine with \a listWidget being the QListWidget to
     * be filtered.
     *
     * If \a listWidget is null then the widget will be disabled until a listWidget
     * is set with setListWidget().
     */
    KListWidgetSearchLine( QWidget *parent = 0,
                           QListWidget *listWidget = 0 );

    /**
     * Destroys the KListWidgetSearchLine.
     */
    virtual ~KListWidgetSearchLine();

    /**
     * Returns if the search is case sensitive.  This defaults to Qt::CaseInsensitive.
     *
     * @see setCaseSensitive()
     */
    Qt::CaseSensitivity caseSensitive() const;

    /**
     * Returns the listWidget that is currently filtered by the search.
     *
     * @see setListWidget()
     */
    QListWidget *listWidget() const;


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
    void setCaseSensitivity( Qt::CaseSensitivity cs );

    /**
     * Sets the QListWidget that is filtered by this search line.  If \a lv is null
     * then the widget will be disabled.
     *
     * @see listWidget()
     */
    void setListWidget( QListWidget *lv );


    /**
     * Clear line edit and empty hiddenItems, returning elements to listWidget.
     */
    void clear();


protected:
    /**
     * Returns true if \a item matches the search \a s.  This will be evaluated
     * based on the value of caseSensitive().  This can be overridden in
     * subclasses to implement more complicated matching schemes.
     */
    virtual bool itemMatches( const QListWidgetItem *item,
                              const QString &s ) const;


    /**
     * Do initialization common to both constructors.
     */
    void init( QListWidget *listWidget = 0 );

    /**
     * Hide item.
     */
    void hideItem( QListWidgetItem *item );

    /**
     * Show item.
     *
     * Just unhide it, doesn't necessary show it on screen, for that use
     * listWidget->scrollToItem()
     */
    void showItem( QListWidgetItem *item );


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
     * However if there are no other pending searches this starts the list widget
     * search.
     *
     * @see queueSearch()
     */
    void activateSearch();


private:
    /**
     * Take action need when listWidget is deleted.
     */
    Q_PRIVATE_SLOT(d, void listWidgetDeleted());


private:
    class KListWidgetSearchLinePrivate;
    KListWidgetSearchLinePrivate *d;
};


#endif /* KLISTWIDGETSEARCHLINE_H */
