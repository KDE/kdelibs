/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2001 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "k3fileview.h"
#include <config-kfile.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <QtCore/QPointer>

#include <assert.h>
#include <stdlib.h>

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

void K3FileViewSignaler::activate( const KFileItem &item )
{
    if ( item.isDir() )
        dirActivated( item );
    else
        fileSelected( item );
}

void K3FileViewSignaler::highlightFile( const KFileItem &item )
{
    fileHighlighted( item );
}

void K3FileViewSignaler::activateMenu( const KFileItem &item, const QPoint& pos )
{
    activatedMenu( item, pos );
}

void K3FileViewSignaler::changeSorting( QDir::SortFlags sorting )
{
    sortingChanged( sorting );
}

void K3FileViewSignaler::dropURLs( const KFileItem &item, QDropEvent *event, const KUrl::List &urls )
{
    dropped( item, event, urls );
}

class K3FileView::K3FileViewPrivate
{
public:
    K3FileViewPrivate()
    {
        actions = 0;
        dropOptions = 0;
    }

    ~K3FileViewPrivate()
    {
        if( actions ) {
            actions->clear(); // so that the removed() signal is emitted!
            delete actions;
        }
    }

    QPointer<KActionCollection> actions;
    int dropOptions;

    const static QDir::SortFlags defaultSortFlags;
    QDir::SortFlags m_sorting;
    QString m_viewName;

    /**
     * counters
     **/
    uint filesNumber;
    uint dirsNumber;

    ViewMode view_mode;
    KFile::SelectionMode selection_mode;

    bool myOnlyDoubleClickSelectsFiles;

};

const QDir::SortFlags K3FileView::K3FileViewPrivate::defaultSortFlags = (QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);


K3FileView::K3FileView()
	:d(new K3FileViewPrivate())
{
    d->m_sorting  = K3FileView::K3FileViewPrivate::defaultSortFlags;

    sig = new K3FileViewSignaler();
    sig->setObjectName("view-signaller");

    d->filesNumber = 0;
    d->dirsNumber = 0;

    d->view_mode = All;
    d->selection_mode = KFile::Single;
    d->m_viewName = i18n("Unknown View");

    d->myOnlyDoubleClickSelectsFiles = false;
}

K3FileView::~K3FileView()
{
    delete d;
    delete sig;
}

void K3FileView::setParentView(K3FileView *parent)
{
    if ( parent ) { // pass all signals right to our parent
        QObject::connect(sig, SIGNAL( activatedMenu(const KFileItem &,
                                                        const QPoint& ) ),
                parent->sig, SIGNAL( activatedMenu(const KFileItem &,
                                                        const QPoint& )));
        QObject::connect(sig, SIGNAL( dirActivated(const KFileItem &)),
                parent->sig, SIGNAL( dirActivated(const KFileItem&)));
        QObject::connect(sig, SIGNAL( fileSelected(const KFileItem &)),
                parent->sig, SIGNAL( fileSelected(const KFileItem&)));
        QObject::connect(sig, SIGNAL( fileHighlighted(const KFileItem &) ),
                            parent->sig,SIGNAL(fileHighlighted(const KFileItem&)));
        QObject::connect(sig, SIGNAL( sortingChanged( QDir::SortFlags ) ),
                            parent->sig, SIGNAL(sortingChanged( QDir::SortFlags)));
        QObject::connect(sig, SIGNAL( dropped(const KFileItem &, QDropEvent*, const KUrl::List&) ),
                            parent->sig, SIGNAL(dropped(const KFileItem &, QDropEvent*, const KUrl::List&)));
    }
}

bool K3FileView::updateNumbers(const KFileItem &i)
{
    if (!( viewMode() & Files ) && i.isFile())
        return false;

    if (!( viewMode() & Directories ) && i.isDir())
        return false;

    if (i.isDir())
        d->dirsNumber++;
    else
        d->filesNumber++;

    return true;
}

// filter out files if we're in directory mode and count files/directories
// and insert into the view
void K3FileView::addItemList(const KFileItemList& list)
{
    KFileItemList::const_iterator kit = list.begin();
    const KFileItemList::const_iterator kend = list.end();
    for ( ; kit != kend; ++kit ) {
        const KFileItem item = *kit;
        if (!updateNumbers(item))
            continue;
        insertItem(item);
    }
}

void K3FileView::insertItem( const KFileItem& )
{
}

QWidget* K3FileView::widget() const
{
  return const_cast<K3FileView*>(this)->widget();
}

QDir::SortFlags K3FileView::sorting() const
{
    return d->m_sorting;
}

void K3FileView::setSorting(QDir::SortFlags new_sort)
{
    d->m_sorting = new_sort;
}

void K3FileView::clear()
{
    d->filesNumber = 0;
    d->dirsNumber = 0;
    clearView();
}

bool K3FileView::isReversed() const
{
    return (d->m_sorting & QDir::Reversed);
}

void K3FileView::sortReversed()
{
    int spec = sorting();

    setSorting( QDir::SortFlags( spec ^ QDir::Reversed ) );
}

uint K3FileView::count() const
{
    return d->filesNumber + d->dirsNumber;
}

uint K3FileView::numFiles() const
{
    return d->filesNumber;
}

uint K3FileView::numDirs() const
{
    return d->dirsNumber;
}

#if 0
int K3FileView::compareItems(const KFileItem *fi1, const KFileItem *fi2) const
{
    static const QString &dirup = KGlobal::staticQString("..");
    bool bigger = true;
    bool keepFirst = false;
    bool dirsFirst = ((m_sorting & QDir::DirsFirst) == QDir::DirsFirst);

    if (fi1 == fi2)
	return 0;

    // .. is always bigger, independent of the sort criteria
    if ( fi1->name() == dirup ) {
	bigger = false;
	keepFirst = dirsFirst;
    }
    else if ( fi2->name() == dirup ) {
	bigger = true;
	keepFirst = dirsFirst;
    }

    else {
	if ( fi1->isDir() != fi2->isDir() && dirsFirst ) {
	    bigger = fi2->isDir();
	    keepFirst = true;
	}
	else {

	    QDir::SortFlags sort = static_cast<QDir::SortFlags>(m_sorting & QDir::SortByMask);

	    //if (fi1->isDir() || fi2->isDir())
            // sort = static_cast<QDir::SortFlags>(K3FileView::defaultSortSpec & QDir::SortByMask);

            switch (sort) {
                case QDir::Name:
                default:
sort_by_name:
                    if ( (m_sorting & QDir::IgnoreCase) == QDir::IgnoreCase )
                        bigger = (fi1->name( true ) > fi2->name( true ));
                    else
                        bigger = (fi1->name() > fi2->name());
                    break;
                case QDir::Time:
                {
                    time_t t1 = fi1->time( KIO::UDSEntry::UDS_MODIFICATION_TIME );
                    time_t t2 = fi2->time( KIO::UDSEntry::UDS_MODIFICATION_TIME );
                    if ( t1 != t2 ) {
                        bigger = (t1 > t2);
                        break;
                    }

                    // Sort by name if both items have the same timestamp.
                    // Don't honor the reverse flag tho.
                    else {
                        keepFirst = true;
                        goto sort_by_name;
                    }
                }
                case QDir::Size:
                {
                    KIO::filesize_t s1 = fi1->size();
                    KIO::filesize_t s2 = fi2->size();
                    if ( s1 != s2 ) {
                        bigger = (s1 > s2);
                        break;
                    }

                    // Sort by name if both items have the same size.
                    // Don't honor the reverse flag tho.
                    else {
                        keepFirst = true;
                        goto sort_by_name;
                    }
                }
                case QDir::Unsorted:
                    bigger = true;  // nothing
                    break;
	    }
	}
    }

    if (reversed && !keepFirst ) // don't reverse dirs to the end!
      bigger = !bigger;

    return (bigger ? 1 : -1);
}
#endif

void  K3FileView::updateView(bool)
{
    widget()->repaint();
}

void K3FileView::updateView(const KFileItem &)
{
}

void K3FileView::setCurrentItem(const QString &filename )
{
    if (!filename.isNull()) {
        KFileItem item;
        for ( (item = firstFileItem()); !item.isNull(); item = nextItem( item ) ) {
            if (item.name() == filename) {
                setCurrentItem( item );
                return;
            }
        }
    }

    kDebug(kfile_area) << "setCurrentItem: no match found: " << filename;
}

KFileItemList K3FileView::items() const
{
    KFileItemList list;

    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) )
        list.append( item );

    return list;
}

void K3FileView::setOnlyDoubleClickSelectsFiles( bool enable ) {
    d->myOnlyDoubleClickSelectsFiles = enable;
}

bool K3FileView::onlyDoubleClickSelectsFiles() const {
    return d->myOnlyDoubleClickSelectsFiles;
}


KFileItemList K3FileView::selectedItems() const
{
    KFileItemList list;

    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) ) {
        if ( isSelected( item ) )
            list.append( item );
    }

    return list;
}

void K3FileView::selectAll()
{
    if (d->selection_mode == KFile::NoSelection || d->selection_mode== KFile::Single)
        return;

    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) )
        setSelected( item, true );
}


void K3FileView::invertSelection()
{
    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) )
        setSelected( item, !isSelected( item ) );
}


void K3FileView::setSelectionMode( KFile::SelectionMode sm )
{
    d->selection_mode = sm;
}

KFile::SelectionMode K3FileView::selectionMode() const
{
    return d->selection_mode;
}

void K3FileView::setViewMode( K3FileView::ViewMode vm )
{
    d->view_mode = vm;
}

K3FileView::ViewMode K3FileView::viewMode() const
{
    return d->view_mode;
}

QString K3FileView::viewName() const
{
    return d->m_viewName;
}

void K3FileView::setViewName( const QString& name )
{
    d->m_viewName = name;
}

void K3FileView::removeItem( const KFileItem &item )
{
    if ( item.isNull() )
        return;

    if ( item.isDir() )
        d->dirsNumber--;
    else
        d->filesNumber--;
}

void K3FileView::listingCompleted()
{
    // empty default impl.
}

KActionCollection * K3FileView::actionCollection() const
{
    if ( !d->actions ) {
        d->actions = new KActionCollection( widget() );
        d->actions->setObjectName( "K3FileView::d->actions" );
    }
    return d->actions;
}

K3FileViewSignaler * K3FileView::signaler() const
{
    return sig;
}

void K3FileView::readConfig( KConfigGroup *)
{
}

void K3FileView::writeConfig( KConfigGroup *)
{
}

QString K3FileView::sortingKey( const QString& value, bool isDir, QDir::SortFlags SortFlags )
{
    bool reverse   = SortFlags & QDir::Reversed;
    bool dirsFirst = SortFlags & QDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    QString result = (SortFlags & QDir::IgnoreCase) ? value.toLower() : value;
    return result.prepend( QLatin1Char(start) );
}

QString K3FileView::sortingKey( KIO::filesize_t value, bool isDir, QDir::SortFlags SortFlags)
{
    bool reverse = SortFlags & QDir::Reversed;
    bool dirsFirst = SortFlags & QDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    return KIO::number( value ).rightJustified( 24, '0' ).prepend( QLatin1Char(start) );
}

void K3FileView::setDropOptions(int options)
{
    d->dropOptions = options;
}

int K3FileView::dropOptions()
{
    return d->dropOptions;
}

int K3FileView::autoOpenDelay()
{
    return (QApplication::startDragTime() * 3) / 2;
}

#include "k3fileview.moc"
