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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <klocale.h>
#include <kdebug.h>

#include "kfilefilter.h"

KFileFilter::KFileFilter( QWidget *parent, const char *name)
    : KComboBox(true, parent, name)
{
    setTrapReturnKey( true );
    setInsertionPolicy(NoInsertion);
    connect( this, SIGNAL( activated( int )), this, SIGNAL( filterChanged() ));
    connect( this, SIGNAL( returnPressed() ), this, SIGNAL( filterChanged() ));
    m_allTypes = false;
}

KFileFilter::~KFileFilter()
{
}

void KFileFilter::setFilter(const QString& filter)
{
    clear();
    filters.clear();

    if (!filter.isEmpty()) {
	QString tmp = filter;
	int index = tmp.find('\n');
	while (index > 0) {
	    filters.append(tmp.left(index));
	    tmp = tmp.mid(index + 1);
	    index = tmp.find('\n');
	}
	filters.append(tmp);
    } else
	filters.append(i18n("*|All Files"));

    QStringList::ConstIterator it;
    for (it = filters.begin(); it != filters.end(); it++) {
	int tab = (*it).find('|');
	insertItem((tab < 0) ? *it :
		   (*it).mid(tab + 1));
    }
}

QString KFileFilter::currentFilter() const
{
    QString f = currentText();
    if (f == text(currentItem())) {
	f = *filters.at(currentItem());
        int mime = f.contains( '/' );
        if ( mime > 0 ) // we have a mimetype as filter
            return f;
    }

    int tab = f.find('|');
    if (tab < 0)
	return f;
    else
	return f.left(tab);
}

void KFileFilter::setMimeFilter( const QStringList& types, const QString& defaultType )
{
    clear();
    filters.clear();
    QString delim = QString::fromLatin1(", ");

    m_allTypes = defaultType.isEmpty() && (types.count() > 1);

    QString allComments, allTypes;
    int i = 0;
    for(QStringList::ConstIterator it = types.begin(); it != types.end(); ++it,  ++i)
    {
        if ( m_allTypes && it != types.begin() ) {
            allComments += delim;
            allTypes += ' ';
        }

	kdDebug() << *it << endl;
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
        insertItem( allComments, 0 );
        filters.prepend( allTypes );
    }
}

#include "kfilefilter.moc"
