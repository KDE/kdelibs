/* This file is part of the KDE libraries
   Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>

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

#ifndef KLISTVIEWSEARCHLINE_H
#define KLISTVIEWSEARCHLINE_H

#include <klineedit.h>

class KListView;
class QListViewItem;

/**
 * This class makes it easy to add a search line for filtering the items in a
 * listview based on a simple text search.
 *
 * No changes to the application other than instantiating this class with an
 * appropriate KListView should be needed.
 */

class KListViewSearchLine : public KLineEdit
{
    Q_OBJECT

public:

    /**
     * Constructs a KListViewSearchLine with \a listView being the KListView to
     * be filtered.
     *
     * If \a listView is null then the widget will be disabled until a listview
     * is set with setListView().
     */
    KListViewSearchLine(QWidget *parent = 0, KListView *listView = 0, const char *name = 0);

    /**
     * Destroys the KListViewSearchLine.
     */
    virtual ~KListViewSearchLine();

    /**
     * Returns true if the search is case sensitive.  This defaults to false.
     *
     * @see setCaseSensitive()
     */
    bool caseSensitive() const;

    /**
     * Returns the current list of columns that will be searched.  If the
     * returned list is empty all columns will be searched.
     *
     * @see setSearchColumns
     */
    QValueList<int> searchColumns() const;

    /**
     * If this is true (the default) then the parents of matched items will also
     * be shown.
     *
     * @see setKeepParentsVisible()
     */
    bool keepParentsVisible() const;

    /**
     * Returns the listview that is currently filtered by the search.
     *
     * @see setListView()
     */
    KListView *listView() const;

public slots:
    /**
     * Updates search to only make visible the items that match \a s.  If
     * \a s is null then the line edit's text will be used.
     */
    virtual void updateSearch(const QString &s = QString::null);

    /**
     * Make the search case sensitive or case insensitive.
     *
     * @see caseSenstive()
     */
    void setCaseSensitive(bool cs);

    /**
     * When a search is active on a list that's organized into a tree view if
     * a parent or ancesestor of an item is does not match the search then it
     * will be hidden and as such so too will any children that match.
     *
     * If this is set to true (the default) then the parents of matching items
     * will be shown.
     *
     * @see keepParentsVisible
     */
    void setKeepParentsVisible(bool v);

    /**
     * Sets the list of columns to be searched.  The default is to search all,
     * which can be restored by passing \a columns as an empty list.
     *
     * @see searchColumns
     */
    void setSearchColumns(const QValueList<int> &columns);

    /**
     * Sets the KListView that is filtered by this search line.  If \a lv is null
     * then the widget will be disabled.
     *
     * @see listView()
     */
    void setListView(KListView *lv);

protected:

    /**
     * Returns true if \a item matches the search \a s.  This will be evaluated
     * based on the value of caseSensitive().  This can be overridden in
     * subclasses to implement more complicated matching schemes.
     */
    virtual bool itemMatches(const QListViewItem *item, const QString &s) const;

protected slots:
    void queueSearch(const QString &search);
    void activateSearch();

private:

    /**
     * This is used recursively to evalute matching items.  It makes a recusive
     * call to each sibling and also to all children.  In the case of an item
     * with children it first places the current item on the "parent stack" before
     * the recursive calls and pops it off after the call returns to build a stack
     * of parents for each matched item.
     */
    void checkItem(QListViewItem *item);

private slots:
    void itemAdded(QListViewItem *item) const;
    void listViewDeleted();

private:
    class KListViewSearchLinePrivate;
    KListViewSearchLinePrivate *d;
};

#endif
