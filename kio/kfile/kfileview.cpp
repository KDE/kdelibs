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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <assert.h>
#include <stdlib.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "config-kfile.h"
#include "kfileview.h"

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

QDir::SortSpec KFileView::defaultSortSpec = static_cast<QDir::SortSpec>(QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);

class KFileView::KFileViewPrivate
{
public:
    KFileViewPrivate()
    {
        actions = 0;
    }

    ~KFileViewPrivate()
    {
        if( actions ) {
            // actions->clear();
            delete actions;
        }
    }

    QGuardedPtr<KActionCollection> actions;
};


KFileView::KFileView()
{
    d = new KFileViewPrivate();
    m_sorting  = KFileView::defaultSortSpec;

    sig = new KFileViewSignaler();
    sig->setName("view-signaller");

    m_selectedList = 0L;
    filesNumber = 0;
    dirsNumber = 0;

    view_mode = All;
    selection_mode = KFile::Single;
    m_viewName = i18n("Unknown View");

    myOnlyDoubleClickSelectsFiles = false;
    m_itemList.setAutoDelete( false ); // just references
}

KFileView::~KFileView()
{
    delete d;
    delete sig;
    delete m_selectedList;
}

void KFileView::setParentView(KFileView *parent)
{
    if ( parent ) { // pass all signals right to our parent
        QObject::connect(sig, SIGNAL( activatedMenu(const KFileItem *,
                                                        const QPoint& ) ),
                parent->sig, SIGNAL( activatedMenu(const KFileItem *,
                                                        const QPoint& )));
        QObject::connect(sig, SIGNAL( dirActivated(const KFileItem *)),
                parent->sig, SIGNAL( dirActivated(const KFileItem*)));
        QObject::connect(sig, SIGNAL( fileSelected(const KFileItem *)),
                parent->sig, SIGNAL( fileSelected(const KFileItem*)));
        QObject::connect(sig, SIGNAL( fileHighlighted(const KFileItem *) ),
                            parent->sig,SIGNAL(fileHighlighted(const KFileItem*)));
        QObject::connect(sig, SIGNAL( sortingChanged( QDir::SortSpec ) ),
                            parent->sig, SIGNAL(sortingChanged( QDir::SortSpec)));
    }
}

bool KFileView::updateNumbers(const KFileItem *i)
{
    if (!( viewMode() & Files ) && i->isFile())
        return false;

    if (!( viewMode() & Directories ) && i->isDir())
        return false;

    if (i->isDir())
        dirsNumber++;
    else
        filesNumber++;

    return true;
}

void qt_qstring_stats();

// filter out files if we're in directory mode and count files/directories
// and insert into the view
void KFileView::addItemList(const KFileItemList& list)
{
    KFileItem *tmp;

    for (KFileItemListIterator it(list); (tmp = it.current()); ++it)
    {
        if (!updateNumbers(tmp))
            continue;

        insertItem( tmp );
    }

#ifdef Q2HELPER
    qt_qstring_stats();
#endif
}

void KFileView::insertItem( KFileItem * )
{
}

void KFileView::setSorting(QDir::SortSpec new_sort)
{
    m_sorting = new_sort;
}

void KFileView::clear()
{
    m_itemList.clear();
    filesNumber = 0;
    dirsNumber = 0;
    clearView();
}

void KFileView::sortReversed()
{
    int spec = sorting();

    setSorting( static_cast<QDir::SortSpec>( spec ^ QDir::Reversed ) );
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

	    QDir::SortSpec sort = static_cast<QDir::SortSpec>(m_sorting & QDir::SortByMask);

	    //if (fi1->isDir() || fi2->isDir())
            // sort = static_cast<QDir::SortSpec>(KFileView::defaultSortSpec & QDir::SortByMask);

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
                    time_t t1 = fi1->time( KIO::UDS_MODIFICATION_TIME );
                    time_t t2 = fi2->time( KIO::UDS_MODIFICATION_TIME );
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

void  KFileView::updateView(bool f)
{
    widget()->repaint(f);
}

void KFileView::updateView(const KFileItem *)
{
}

void KFileView::setCurrentItem(const QString &filename )
{
    if (!filename.isNull()) {
        KFileItem *item;
        for ( (item = firstFileItem()); item; item = nextItem( item ) ) {
            if (item->name() == filename) {
                setCurrentItem( item );
                return;
            }
        }
    }

    kdDebug(kfile_area) << "setCurrentItem: no match found: " << filename << endl;
}

const KFileItemList * KFileView::items() const
{
    KFileItem *item = 0L;

    // only ever use m_itemList in this method!
    m_itemList.clear();
    for ( (item = firstFileItem()); item; item = nextItem( item ) )
        m_itemList.append( item );

    return &m_itemList;
}


const KFileItemList * KFileView::selectedItems() const
{
    if ( !m_selectedList )
	m_selectedList = new KFileItemList;

    m_selectedList->clear();

    KFileItem *item;
    for ( (item = firstFileItem()); item; item = nextItem( item ) ) {
        if ( isSelected( item ) )
            m_selectedList->append( item );
    }

    return m_selectedList;
}

void KFileView::selectAll()
{
    if (selection_mode == KFile::NoSelection || selection_mode== KFile::Single)
        return;

    KFileItem *item = 0L;
    for ( (item = firstFileItem()); item; item = nextItem( item ) )
        setSelected( item, true );
}


void KFileView::invertSelection()
{
    KFileItem *item = 0L;
    for ( (item = firstFileItem()); item; item = nextItem( item ) )
        setSelected( item, !isSelected( item ) );
}


void KFileView::setSelectionMode( KFile::SelectionMode sm )
{
    selection_mode = sm;
}

KFile::SelectionMode KFileView::selectionMode() const
{
    return selection_mode;
}

void KFileView::setViewMode( ViewMode vm )
{
    view_mode = vm;
}

void KFileView::removeItem( const KFileItem *item )
{
    if ( !item )
	return;

    if ( m_selectedList )
        m_selectedList->removeRef( item );
}

void KFileView::listingCompleted()
{
    // empty default impl.
}

KActionCollection * KFileView::actionCollection() const
{
    if ( !d->actions )
        d->actions = new KActionCollection( widget(), "KFileView::d->actions" );
    return d->actions;
}

void KFileView::readConfig( KConfig *, const QString&  )
{
}

void KFileView::writeConfig( KConfig *, const QString& )
{
}

QString KFileView::sortingKey( const QString& value, bool isDir, int sortSpec )
{
    bool reverse   = sortSpec & QDir::Reversed;
    bool dirsFirst = sortSpec & QDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    QString result = (sortSpec & QDir::IgnoreCase) ? value.lower() : value;
    return result.prepend( start );
}

QString KFileView::sortingKey( KIO::filesize_t value, bool isDir, int sortSpec)
{
    bool reverse = sortSpec & QDir::Reversed;
    bool dirsFirst = sortSpec & QDir::DirsFirst;
    char start = (isDir && dirsFirst) ? (reverse ? '2' : '0') : '1';
    return KIO::number( value ).rightJustify( 24, '0' ).prepend( start );
}

void KFileView::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kfileview.moc"
