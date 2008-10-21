/* This file is part of the KDE libraries
   Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>
   Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "k3listviewsearchline.h"

#include <k3listview.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <kdebug.h>
#include <klocale.h>

#include <QApplication>
#include <QTimer>
#include <QMenu>
#include <QLabel>
#include <QContextMenuEvent>
#include <QList>
#include <Q3Header>
#include <QToolButton>

class K3ListViewSearchLine::K3ListViewSearchLinePrivate
{
public:
    K3ListViewSearchLinePrivate() :
        caseSensitive(Qt::CaseInsensitive),
        activeSearch(false),
        keepParentsVisible(true),
        canChooseColumns(true),
        queuedSearches(0),
        allVisibleColumnsAction(0)
    {}

    QList<K3ListView *> listViews;
    Qt::CaseSensitivity caseSensitive;
    bool activeSearch;
    bool keepParentsVisible;
    bool canChooseColumns;
    QString search;
    int queuedSearches;
    QList<int> searchColumns;
    QAction *allVisibleColumnsAction;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

K3ListViewSearchLine::K3ListViewSearchLine(QWidget *parent, K3ListView *listView) :
    KLineEdit(parent)
{
    d = new K3ListViewSearchLinePrivate;
    setClearButtonShown( true );

    connect(this, SIGNAL(textChanged(const QString &)),
            this, SLOT(queueSearch(const QString &)));

    setListView( listView );
    if( !listView)
        setEnabled(false);
}

K3ListViewSearchLine::K3ListViewSearchLine(QWidget *parent,
                                       const QList<K3ListView *> &listViews) :
     KLineEdit(parent)
{
    d = new K3ListViewSearchLinePrivate;
    setClearButtonShown( true );

    connect(this, SIGNAL(textChanged(const QString &)),
            this, SLOT(queueSearch(const QString &)));

    setListViews( listViews );
}


K3ListViewSearchLine::~K3ListViewSearchLine()
{
    delete d;
}

bool K3ListViewSearchLine::caseSensitive() const
{
    return (d->caseSensitive ==Qt::CaseSensitive);
}

QList<int> K3ListViewSearchLine::searchColumns() const
{
    if (d->canChooseColumns)
        return d->searchColumns;
    else
        return QList<int>();
}

bool K3ListViewSearchLine::keepParentsVisible() const
{
    return d->keepParentsVisible;
}

K3ListView *K3ListViewSearchLine::listView() const
{
    if ( d->listViews.count() == 1 )
        return d->listViews.first();
    else
        return 0;
}

const QList<K3ListView *> &K3ListViewSearchLine::listViews() const
{
    return d->listViews;
}


////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::addListView(K3ListView *lv)
{
    if (lv) {
        connectListView(lv);

        d->listViews.append(lv);
        setEnabled(!d->listViews.isEmpty());

        checkColumns();
    }
}

void K3ListViewSearchLine::removeListView(K3ListView *lv)
{
    if (lv) {
        int idx = d->listViews.indexOf(lv);

        if ( idx != -1 ) {
            d->listViews.removeAt( idx );
            checkColumns();

            disconnectListView(lv);

            setEnabled(!d->listViews.isEmpty());
        }
    }
}

void K3ListViewSearchLine::updateSearch(const QString &s)
{
    d->search = s.isNull() ? text() : s;

    for (QList<K3ListView *>::Iterator it = d->listViews.begin();
         it != d->listViews.end(); ++it)
        updateSearch( *it );
}

void K3ListViewSearchLine::updateSearch(K3ListView *listView)
{
    if(!listView)
        return;


    // If there's a selected item that is visible, make sure that it's visible
    // when the search changes too (assuming that it still matches).

    Q3ListViewItem *currentItem = 0;

    switch(listView->selectionMode())
    {
    case K3ListView::NoSelection:
        break;
    case K3ListView::Single:
        currentItem = listView->selectedItem();
        break;
    default:
    {
        int flags = Q3ListViewItemIterator::Selected | Q3ListViewItemIterator::Visible;
        for(Q3ListViewItemIterator it(listView, flags);
            it.current() && !currentItem;
            ++it)
        {
            if(listView->itemRect(it.current()).isValid())
                currentItem = it.current();
        }
    }
    }

    if(d->keepParentsVisible)
        checkItemParentsVisible(listView->firstChild());
    else
        checkItemParentsNotVisible(listView);

    if(currentItem)
        listView->ensureItemVisible(currentItem);
}

void K3ListViewSearchLine::setCaseSensitive(bool cs)
{
	d->caseSensitive = cs?Qt::CaseSensitive:Qt::CaseInsensitive;
}

void K3ListViewSearchLine::setKeepParentsVisible(bool v)
{
    d->keepParentsVisible = v;
}

void K3ListViewSearchLine::setSearchColumns(const QList<int> &columns)
{
    if (d->canChooseColumns)
        d->searchColumns = columns;
}

void K3ListViewSearchLine::setListView(K3ListView *lv)
{
    setListViews(QList<K3ListView *>());
    addListView(lv);
}

void K3ListViewSearchLine::setListViews(const QList<K3ListView *> &lv)
{
    for (QList<K3ListView *>::Iterator it = d->listViews.begin();
         it != d->listViews.end(); ++it)
             disconnectListView(*it);

    d->listViews = lv;

    for (QList<K3ListView *>::Iterator it = d->listViews.begin();
         it != d->listViews.end(); ++it)
        connectListView(*it);

    checkColumns();
    setEnabled(!d->listViews.isEmpty());
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

bool K3ListViewSearchLine::itemMatches(const Q3ListViewItem *item, const QString &s) const
{
    if(s.isEmpty())
        return true;

    // If the search column list is populated, search just the columns
    // specifified.  If it is empty default to searching all of the columns.

    if(!d->searchColumns.isEmpty()) {
        QList<int>::ConstIterator it = d->searchColumns.constBegin();
        for(; it != d->searchColumns.constEnd(); ++it) {
            if(*it < item->listView()->columns() &&
               item->text(*it).indexOf(s, 0, d->caseSensitive) >= 0)
                return true;
        }
    }
    else {
        for(int i = 0; i < item->listView()->columns(); i++) {
            if(item->listView()->columnWidth(i) > 0 &&
               item->text(i).indexOf(s, 0, d->caseSensitive) >= 0)
            {
                return true;
            }
        }
    }

    return false;
}

void K3ListViewSearchLine::contextMenuEvent( QContextMenuEvent*e )
{
    QMenu *popup = KLineEdit::createStandardContextMenu();

    if (d->canChooseColumns) {
        popup->addSeparator();
        QMenu *subMenu = popup->addMenu(i18n("Search Columns"));
        connect(subMenu, SIGNAL(triggered(QAction*)), this, SLOT(searchColumnsMenuActivated(QAction*)));

        d->allVisibleColumnsAction = subMenu->addAction(i18n("All Visible Columns"));
        d->allVisibleColumnsAction->setCheckable( true );
        subMenu->addSeparator();

        bool allColumnsAreSearchColumns = true;
        // TODO Make the entry order match the actual column order
        Q3Header* const header = d->listViews.first()->header();
        int visibleColumns = 0;
        for(int i = 0; i < d->listViews.first()->columns(); i++) {
          if(d->listViews.first()->columnWidth(i)>0) {
            QString columnText = d->listViews.first()->columnText(i);
            if(columnText.isEmpty()) {
              int visiblePosition=1;
              for(int j = 0; j < header->mapToIndex(i); j++)
                if(d->listViews.first()->columnWidth(header->mapToSection(j))>0)
                  visiblePosition++;

              columnText = i18nc("Column number %1","Column No. %1", visiblePosition);
            }
            QAction *action = subMenu->addAction(columnText);
            action->setData( visibleColumns );
            action->setCheckable( true );

            if(d->searchColumns.isEmpty() || d->searchColumns.indexOf(i) != -1)
              action->setChecked(true);
            else
              allColumnsAreSearchColumns = false;

            visibleColumns++;
          }
        }
        d->allVisibleColumnsAction->setChecked( allColumnsAreSearchColumns );

        // searchColumnsMenuActivated() relies on one possible "all" representation
        if(allColumnsAreSearchColumns && !d->searchColumns.isEmpty())
            d->searchColumns.clear();
    }

    popup->exec( e->globalPos() );

    delete popup;
}

void K3ListViewSearchLine::connectListView(K3ListView *lv)
{
    connect(lv, SIGNAL(destroyed( QObject * )),
            this, SLOT(listViewDeleted( QObject *)));
    connect(lv, SIGNAL(itemAdded(Q3ListViewItem *)),
            this, SLOT(itemAdded(Q3ListViewItem *)));
}

void K3ListViewSearchLine::disconnectListView(K3ListView *lv)
{
    disconnect(lv, SIGNAL(destroyed( QObject * )),
              this, SLOT(listViewDeleted( QObject *)));
    disconnect(lv, SIGNAL(itemAdded(Q3ListViewItem *)),
              this, SLOT(itemAdded(Q3ListViewItem *)));
}

bool K3ListViewSearchLine::canChooseColumnsCheck()
{
    // This is true if either of the following is true:

    // there are no listviews connected
    if (d->listViews.isEmpty())
        return false;

    const K3ListView *first = d->listViews.first();

    const unsigned int numcols = first->columns();
    // the listviews have only one column,
    if (numcols < 2)
        return false;

    QStringList headers;
    for (unsigned int i = 0; i < numcols; ++i)
        headers.append(first->columnText(i));

    QList<K3ListView *>::ConstIterator it = d->listViews.constBegin();
    for (++it /* skip the first one */; it !=d->listViews.constEnd(); ++it) {
        // the listviews have different numbers of columns,
        if ((unsigned int) (*it)->columns() != numcols)
            return false;

        // the listviews differ in column labels.
        QStringList::ConstIterator jt;
        unsigned int i;
        for (i = 0, jt = headers.constBegin(); i < numcols; ++i, ++jt) {
                Q_ASSERT(jt != headers.constEnd());
                if ((*it)->columnText(i) != *jt)
                    return false;
            }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// protected slots
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::queueSearch(const QString &search)
{
    d->queuedSearches++;
    d->search = search;
    QTimer::singleShot(200, this, SLOT(activateSearch()));
}

void K3ListViewSearchLine::activateSearch()
{
    --(d->queuedSearches);

    if(d->queuedSearches == 0)
        updateSearch(d->search);
}

////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::itemAdded(Q3ListViewItem *item) const
{
    item->setVisible(itemMatches(item, text()));
}

void K3ListViewSearchLine::listViewDeleted(QObject *o)
{
    d->listViews.removeAll(static_cast<K3ListView *>(o));
    setEnabled(d->listViews.isEmpty());
}

void K3ListViewSearchLine::searchColumnsMenuActivated(QAction *action)
{
    int id = action->data().toInt();

    if(action == d->allVisibleColumnsAction) {
        if(d->searchColumns.isEmpty())
            d->searchColumns.append(0);
        else
            d->searchColumns.clear();
    }
    else {
        if(d->searchColumns.indexOf(id) != -1)
            d->searchColumns.removeAll(id);
        else {
            if(d->searchColumns.isEmpty()) {
                for(int i = 0; i < d->listViews.first()->columns(); i++) {
                    if(i != id)
                        d->searchColumns.append(i);
                }
            }
            else
                d->searchColumns.append(id);
        }
    }
    updateSearch();
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::checkColumns()
{
    d->canChooseColumns = canChooseColumnsCheck();
}

void K3ListViewSearchLine::checkItemParentsNotVisible(K3ListView *listView)
{
    Q3ListViewItemIterator it(listView);
    for(; it.current(); ++it)
    {
        Q3ListViewItem *item = it.current();
        if(itemMatches(item, d->search))
            item->setVisible(true);
        else
            item->setVisible(false);
    }
}

#include <kvbox.h>

/** Check whether \p item, its siblings and their descendents should be shown. Show or hide the items as necessary.
 *
 *  \p item  The list view item to start showing / hiding items at. Typically, this is the first child of another item, or the
 *              the first child of the list view.
 *  \p highestHiddenParent  The highest (closest to root) ancestor of \p item which is hidden. If 0, all parents of
 *                           \p item must be visible.
 *  \return \c true if an item which should be visible is found, \c false if all items found should be hidden. If this function
 *             returns true and \p highestHiddenParent was not 0, highestHiddenParent will have been shown.
 */
bool K3ListViewSearchLine::checkItemParentsVisible(Q3ListViewItem *item, Q3ListViewItem *highestHiddenParent)
{
    bool visible = false;
    Q3ListViewItem * first = item;
    for(; item; item = item->nextSibling())
    {
        //What we pass to our children as highestHiddenParent:
        Q3ListViewItem * hhp = highestHiddenParent ? highestHiddenParent : item->isVisible() ? 0L : item;
        bool childMatch = false;
        if(item->firstChild() && checkItemParentsVisible(item->firstChild(), hhp))
            childMatch = true;
        // Should this item be shown? It should if any children should be, or if it matches.
        if(childMatch || itemMatches(item, d->search))
        {
            visible = true;
            if (highestHiddenParent)
            {
                highestHiddenParent->setVisible(true);
                // Calling setVisible on our ancestor will unhide all its descendents. Hide the ones
                // before us that should not be shown.
                for(Q3ListViewItem *hide = first; hide != item; hide = hide->nextSibling())
                    hide->setVisible(false);
                highestHiddenParent = 0;
                // If we matched, than none of our children matched, yet the setVisible() call on our
                // ancestor unhid them, undo the damage:
                if(!childMatch)
                    for(Q3ListViewItem *hide = item->firstChild(); hide; hide = hide->nextSibling())
                        hide->setVisible(false);
            }
            else
                item->setVisible(true);
        }
        else
            item->setVisible(false);
    }
    return visible;
}

////////////////////////////////////////////////////////////////////////////////
// K3ListViewSearchLineWidget
////////////////////////////////////////////////////////////////////////////////

class K3ListViewSearchLineWidget::K3ListViewSearchLineWidgetPrivate
{
public:
    K3ListViewSearchLineWidgetPrivate() : listView(0), searchLine(0) {}
    K3ListView *listView;
    K3ListViewSearchLine *searchLine;
};

K3ListViewSearchLineWidget::K3ListViewSearchLineWidget(K3ListView *listView,
                                                     QWidget *parent) :
    KHBox(parent)
{
    d = new K3ListViewSearchLineWidgetPrivate;
    d->listView = listView;

    setSpacing(5);

    QTimer::singleShot(0, this, SLOT(createWidgets()));
}

K3ListViewSearchLineWidget::~K3ListViewSearchLineWidget()
{
    delete d;
}

K3ListViewSearchLine *K3ListViewSearchLineWidget::createSearchLine(K3ListView *listView)
{
    if(!d->searchLine)
        d->searchLine = new K3ListViewSearchLine(this, listView);
    return d->searchLine;
}

void K3ListViewSearchLineWidget::createWidgets()
{
    QLabel *label = new QLabel(i18n("S&earch:"), this);
    label->setObjectName(QLatin1String("kde toolbar widget"));

    d->searchLine = createSearchLine(d->listView);
    d->searchLine->show();

    label->setBuddy(d->searchLine);
    label->show();
}

K3ListViewSearchLine *K3ListViewSearchLineWidget::searchLine() const
{
    return d->searchLine;
}

#include "k3listviewsearchline.moc"
