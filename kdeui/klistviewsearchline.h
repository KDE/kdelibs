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
 * No changes to the application other than 
 */

class KListViewSearchLine : public KLineEdit
{
    Q_OBJECT

public:
    KListViewSearchLine(QWidget *parent = 0, KListView *listView = 0, const char *name = 0);
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
     */
    bool keepParentsVisible() const;

    /**
     * Returns the listview that is currently filtered by the search.
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
     * Sets the KListView that 
     */
    void setListView(KListView *lv);

protected:
    virtual bool itemMatches(const QListViewItem *item, const QString &s) const;

private:
    void checkItem(QListViewItem *item);

private slots:
    void itemAdded(QListViewItem *item) const;
    void listViewDeleted();

private:
    class KListViewSearchLinePrivate;
    KListViewSearchLinePrivate *d;
};

#endif
