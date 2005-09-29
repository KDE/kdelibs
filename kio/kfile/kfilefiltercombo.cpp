/* This file is part of the KDE libraries
    Copyright (C) Stephan Kulow <coolo@kde.org>

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

#include <klocale.h>
#include <kdebug.h>
#include <kstaticdeleter.h>
#include <config-kfile.h>

#include "kfilefiltercombo.h"

class KFileFilterCombo::KFileFilterComboPrivate
{
public:
    KFileFilterComboPrivate() {
        hasAllSupportedFiles = false;
        defaultFilter = i18n("*|All Files");
        isMimeFilter = false;
    }

    // when we have more than 3 mimefilters and no default-filter,
    // we don't show the comments of all mimefilters in one line,
    // instead we show "All supported files". We have to translate
    // that back to the list of mimefilters in currentFilter() tho.
    bool hasAllSupportedFiles;
    // true when setMimeFilter was called
    bool isMimeFilter;
    QString lastFilter;
    QString defaultFilter;
};

KFileFilterCombo::KFileFilterCombo( QWidget *parent, const char *name)
    : KComboBox(true, parent, name), d( new KFileFilterComboPrivate )
{
    setTrapReturnKey( true );
    setInsertionPolicy(NoInsertion);
    connect( this, SIGNAL( activated( int )), this, SIGNAL( filterChanged() ));
    connect( this, SIGNAL( returnPressed() ), this, SIGNAL( filterChanged() ));
    connect( this, SIGNAL( filterChanged() ), SLOT( slotFilterChanged() ));
    m_allTypes = false;
}

KFileFilterCombo::~KFileFilterCombo()
{
    delete d;
}

void KFileFilterCombo::setFilter(const QString& filter)
{
    clear();
    filters.clear();
    d->hasAllSupportedFiles = false;

    if (!filter.isEmpty()) {
	QString tmp = filter;
	int index = tmp.find('\n');
	while (index > 0) {
	    filters.append(tmp.left(index));
	    tmp = tmp.mid(index + 1);
	    index = tmp.find('\n');
	}
	filters.append(tmp);
    } 
    else
	filters.append( d->defaultFilter );

    QStringList::ConstIterator it;
	QStringList::ConstIterator end(filters.end());
    for (it = filters.begin(); it != end; ++it) {
	int tab = (*it).find('|');
	insertItem((tab < 0) ? *it :
		   (*it).mid(tab + 1));
    }

    d->lastFilter = currentText();
    d->isMimeFilter = false;
}

QString KFileFilterCombo::currentFilter() const
{
    QString f = currentText();
    if (f == text(currentItem())) { // user didn't edit the text
	f = *filters.at(currentItem());
        if ( d->isMimeFilter || (currentItem() == 0 && d->hasAllSupportedFiles) ) {
            return f; // we have a mimetype as filter
        }
    }

    int tab = f.find('|');
    if (tab < 0)
	return f;
    else
	return f.left(tab);
}

void KFileFilterCombo::setCurrentFilter( const QString& filter )
{
    int pos = 0;
    for( QStringList::ConstIterator it = filters.begin();
         it != filters.end();
         ++it, ++pos ) {
        if( *it == filter ) {
            setCurrentItem( pos );
            filterChanged();
            return;
        }
    }
    setCurrentText( filter );
    filterChanged();
}

void KFileFilterCombo::setMimeFilter( const QStringList& types, 
                                      const QString& defaultType )
{
    clear();
    filters.clear();
    QString delim = QString::fromLatin1(", ");
    d->hasAllSupportedFiles = false;

    m_allTypes = defaultType.isEmpty() && (types.count() > 1);

    QString allComments, allTypes;
    int i = 0;
    for(QStringList::ConstIterator it = types.begin(); it != types.end(); ++it,  ++i)
    {
        if ( m_allTypes && it != types.begin() ) {
            allComments += delim;
            allTypes += ' ';
        }

	kdDebug(kfile_area) << *it << endl;
        KMimeType::Ptr type = KMimeType::mimeType( *it );
        filters.append( type->name() );
        if ( m_allTypes )
        {
            allTypes += type->name();
            allComments += type->comment();
        }
        insertItem( type->comment() );
        if ( type->name() == defaultType )
            setCurrentItem( i );
    }

    if ( m_allTypes )
    {
        if ( i < 3 ) // show the mime-comments of at max 3 types
            insertItem( allComments, 0 );
        else {
            insertItem( i18n("All Supported Files"), 0 );
            d->hasAllSupportedFiles = true;
        }

        filters.prepend( allTypes );
    }

    d->lastFilter = currentText();
    d->isMimeFilter = true;
}

void KFileFilterCombo::slotFilterChanged()
{
    d->lastFilter = currentText();
}

bool KFileFilterCombo::eventFilter( QObject *o, QEvent *e )
{
    if ( o == lineEdit() && e->type() == QEvent::FocusOut ) {
        if ( currentText() != d->lastFilter )
            emit filterChanged();
    }

    return KComboBox::eventFilter( o, e );
}

void KFileFilterCombo::setDefaultFilter( const QString& filter )
{
    d->defaultFilter = filter;
}

QString KFileFilterCombo::defaultFilter() const
{
    return d->defaultFilter;
}

void KFileFilterCombo::virtual_hook( int id, void* data )
{ KComboBox::virtual_hook( id, data ); }

#include "kfilefiltercombo.moc"
