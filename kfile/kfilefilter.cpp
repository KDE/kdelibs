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
#include <kstaticdeleter.h>
#include <config-kfile.h>

#include "kfilefilter.h"

class KFileFilterPrivate
{
public:
    KFileFilterPrivate() {
        hasAllSupportedFiles = false;
    }

    // when we have more than 3 mimefilters and no default-filter,
    // we don't show the comments of all mimefilters in one line,
    // instead we show "All supported files". We have to translate
    // that back to the list of mimefilters in currentFilter() tho.
    bool hasAllSupportedFiles;
};

QPtrDict<KFileFilterPrivate> * KFileFilter::s_Hack = 0L;
KStaticDeleter<QPtrDict<KFileFilterPrivate> > sd;

KFileFilter::KFileFilter( QWidget *parent, const char *name)
    : KComboBox(true, parent, name)
{
    if ( !s_Hack ) {
        s_Hack = new QPtrDict<KFileFilterPrivate>;
        s_Hack->setAutoDelete( true );
        sd.setObject( s_Hack );
    }

    KFileFilterPrivate *d = new KFileFilterPrivate;
    s_Hack->insert( this, d );

    setTrapReturnKey( true );
    setInsertionPolicy(NoInsertion);
    connect( this, SIGNAL( activated( int )), this, SIGNAL( filterChanged() ));
    connect( this, SIGNAL( returnPressed() ), this, SIGNAL( filterChanged() ));
    m_allTypes = false;
}

KFileFilter::~KFileFilter()
{
    s_Hack->remove( this );
}

void KFileFilter::setFilter(const QString& filter)
{
    clear();
    filters.clear();
    s_Hack->find( this )->hasAllSupportedFiles = false;

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
    if (f == text(currentItem())) { // user didn't edit the text
	f = *filters.at(currentItem());
        int mime = f.contains( '/' );
        KFileFilter *that = const_cast<KFileFilter*>( this );
        if ( mime > 0 || (currentItem() == 0 &&
                          s_Hack->find( that )->hasAllSupportedFiles) ) {
            return f; // we have a mimetype as filter
        }
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
    s_Hack->find( this )->hasAllSupportedFiles = false;

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
            insertItem( i18n("All supported files"), 0 );
            s_Hack->find( this )->hasAllSupportedFiles = true;
        }

        filters.prepend( allTypes );
    }
}

#include "kfilefilter.moc"
