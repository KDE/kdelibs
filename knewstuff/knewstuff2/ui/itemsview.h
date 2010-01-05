/*
    This file is part of KNewStuff2.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF2_UI_ITEMSVIEW_H
#define KNEWSTUFF2_UI_ITEMSVIEW_H

#include <QtCore/QSet>
#include <QtGui/QListView>
#include <QtGui/QLabel>
#include <QtGui/QSortFilterProxyModel>

#include <knewstuff2/dxs/dxsengine.h>
#include <knewstuff2/core/category.h>

namespace Ui
{
class DownloadDialog;
}

class QLabel;

namespace KNS
{

class EntryView;

/** GUI/CORE: HTML Widget to operate on AvailableItem::List */
class ItemsView : public QListView
{
    Q_OBJECT
public:
    ItemsView(QWidget * parentWidget);
    ~ItemsView();

    void setEngine(DxsEngine *engine);

    /** set the provider to show entries from with the feed also
     * @param provider the provider to show
     */
    void setProvider(const Provider * provider, const Feed * feed);

    /** set which feed from the current provider to show
     * @param feed the feed to use
     */
    void setFeed(const Feed*);

public slots:
    /** set the search text to filter the shown entries by
     * @param text filter text
     */
    void setSearchText(const QString & text);
    /** update the given item because it has changed
     * @param entry the entry to update
     */
    void updateItem(Entry *entry);

    // not used because the ui doesn't support it yet
    void setSorting(int sortType);

private:

    void buildContents();

    const Provider* m_currentProvider;
    const Feed* m_currentFeed;

    QWidget *m_root;
    int m_sorting;
    DxsEngine *m_engine;
    QMap<Entry*, EntryView*> m_views;
    QString m_searchText;

    QSortFilterProxyModel * m_filteredModel;
};


/** GUI/CORE: HTML Widget for exactly one AvailableItem::List */
class EntryView : public QLabel
{
    Q_OBJECT
public:
    EntryView(QWidget * parentWidget);

    void setEntry(Entry *entry);
    void updateEntry(Entry *entry);

private slots:
    void urlSelected(const QString &link);

private:
    // generate the HTML contents to be displayed by the class itself
    void buildContents();

    void setTheAaronnesqueStyle();

    Entry *m_entry;
};

}

#endif
