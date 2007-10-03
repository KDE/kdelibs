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

#include "kfileview.h"
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

void KFileViewSignaler::activate( const KFileItem &item )
{
    if ( item.isDir() )
        dirActivated( item );
    else
        fileSelected( item );
}

void KFileViewSignaler::highlightFile( const KFileItem &item )
{
    fileHighlighted( item );
}

void KFileViewSignaler::activateMenu( const KFileItem &item, const QPoint& pos )
{
    activatedMenu( item, pos );
}

void KFileViewSignaler::changeSorting( QDir::SortFlags sorting )
{
    sortingChanged( sorting );
}

void KFileViewSignaler::dropURLs( const KFileItem &item, QDropEvent *event, const KUrl::List &urls )
{
    dropped( item, event, urls );
}

class KFileView::KFileViewPrivate
{
public:
    KFileViewPrivate()
    {
        actions = 0;
        dropOptions = 0;
    }

    ~KFileViewPrivate()
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

const QDir::SortFlags KFileView::KFileViewPrivate::defaultSortFlags = (QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);


KFileView::KFileView()
	:d(new KFileViewPrivate())
{
    d->m_sorting  = KFileView::KFileViewPrivate::defaultSortFlags;

    sig = new KFileViewSignaler();
    sig->setObjectName("view-signaller");

    d->filesNumber = 0;
    d->dirsNumber = 0;

    d->view_mode = All;
    d->selection_mode = KFile::Single;
    d->m_viewName = i18n("Unknown View");

    d->myOnlyDoubleClickSelectsFiles = false;
}

KFileView::~KFileView()
{
    delete d;
    delete sig;
}

void KFileView::setParentView(KFileView *parent)
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

bool KFileView::updateNumbers(const KFileItem &i)
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
void KFileView::addItemList(const KFileItemList& list)
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

void KFileView::insertItem( const KFileItem& )
{
}

QWidget* KFileView::widget() const
{
  return const_cast<KFileView*>(this)->widget();
}

QDir::SortFlags KFileView::sorting() const
{
    return d->m_sorting;
}

void KFileView::setSorting(QDir::SortFlags new_sort)
{
    d->m_sorting = new_sort;
}

void KFileView::clear()
{
    d->filesNumber = 0;
    d->dirsNumber = 0;
    clearView();
}

bool KFileView::isReversed() const
{
    return (d->m_sorting & QDir::Reversed);
}

void KFileView::sortReversed()
{
    int spec = sorting();

    setSorting( QDir::SortFlags( spec ^ QDir::Reversed ) );
}

uint KFileView::count() const
{
    return d->filesNumber + d->dirsNumber;
}

uint KFileView::numFiles() const
{
    return d->filesNumber;
}

uint KFileView::numDirs() const
{
    return d->dirsNumber;
}

#if 0
int KFileView::compareItems(const KFileItem *fi1, const KFileItem *fi2) const
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
            // sort = static_cast<QDir::SortFlags>(KFileView::defaultSortSpec & QDir::SortByMask);

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

void  KFileView::updateView(bool)
{
    widget()->repaint();
}

void KFileView::updateView(const KFileItem &)
{
}

void KFileView::setCurrentItem(const QString &filename )
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

KFileItemList KFileView::items() const
{
    KFileItemList list;

    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) )
        list.append( item );

    return list;
}

void KFileView::setOnlyDoubleClickSelectsFiles( bool enable ) {
    d->myOnlyDoubleClickSelectsFiles = enable;
}

bool KFileView::onlyDoubleClickSelectsFiles() const {
    return d->myOnlyDoubleClickSelectsFiles;
}


KFileItemList KFileView::selectedItems() const
{
    KFileItemList list;

    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) ) {
        if ( isSelected( item ) )
            list.append( item );
    }

    return list;
}

void KFileView::selectAll()
{
    if (d->selection_mode == KFile::NoSelection || d->selection_mode== KFile::Single)
        return;

    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) )
        setSelected( item, true );
}


void KFileView::invertSelection()
{
    for ( KFileItem item = firstFileItem(); !item.isNull(); item = nextItem( item ) )
        setSelected( item, !isSelected( item ) );
}


void KFileView::setSelectionMode( KFile::SelectionMode sm )
{
    d->selection_mode = sm;
}

KFile::SelectionMode KFileView::selectionMode() const
{
    return d->selection_mode;
}

void KFileView::setViewMode( KFileView::ViewMode vm )
{
    d->view_mode = vm;
}

KFileView::ViewMode KFileView::viewMode() const
{
    return d->view_mode;
}

QString KFileView::viewName() const
{
    return d->m_viewName;
}

void KFileView::setViewName( const QString& name )
{
    d->m_viewName = name;
}

void KFileView::removeItem( const KFileItem &item )
{
    if ( item.isNull() )
        return;

    if ( item.isDir() )
        d->dirsNumber--;
    else
        d->filesNumber--;
}

void KFileView::listingCompleted()
{
    // empty default impl.
}

KActionCollection * KFileView::actionCollection() const
{
    if ( !d->actions ) {
        d->actions = new KActionCollection( widget() );
        d->actions->setObjectName( "KFileView::d->actions" );
    }
    return d->actions;
}

KFileViewSignaler * KFileView::signaler() const
{
    return sig;
}

void KFileView::readConfig( KConfigGroup *)
{
}

void KFileView::writeConfig( KConfigGroup *)
{
}

QString KFileView::sortingKey( const QString& value, bool isDir, QDir::SortFlags SortFlags )
{
    bool reverse   = SortFlags & QDir::Reversed;
    bool dirsFirst = SortFlags & QDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    QString result = (SortFlags & QDir::IgnoreCase) ? value.toLower() : value;
    return result.prepend( QLatin1Char(start) );
}

QString KFileView::sortingKey( KIO::filesize_t value, bool isDir, QDir::SortFlags SortFlags)
{
    bool reverse = SortFlags & QDir::Reversed;
    bool dirsFirst = SortFlags & QDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    return KIO::number( value ).rightJustified( 24, '0' ).prepend( QLatin1Char(start) );
}

void KFileView::setDropOptions(int options)
{
    d->dropOptions = options;
}

int KFileView::dropOptions()
{
    return d->dropOptions;
}

int KFileView::autoOpenDelay()
{
    return (QApplication::startDragTime() * 3) / 2;
}

#include "kfileview.moc"
