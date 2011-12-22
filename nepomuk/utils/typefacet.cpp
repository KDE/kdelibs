/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "typefacet.h"
#include "filequery.h"
#include "term.h"
#include "class.h"
#include "andterm.h"
#include "orterm.h"
#include "resourcetypeterm.h"
#include "negationterm.h"
#include "literalterm.h"
#include "comparisonterm.h"
#include "nfo.h"
#include "nie.h"
#include "tmo.h"
#include "nco.h"
#include "nmo.h"

#include "kguiitem.h"
#include "kdebug.h"

#include <QtCore/QMap>
#include <QtCore/QSet>

#include <Soprano/Vocabulary/NAO>


namespace {
enum RootSelection {
    None,
    File,
    Other
};
}

class Nepomuk::Utils::TypeFacet::Private
{
public:
    Private()
        : m_rootSelection(None),
          m_usedTypes(0) {
        //
        // Default file types
        //
        m_fileTypes.append( qMakePair<QString, Query::Term>(i18nc("@option:check A filter on file type", "Documents"),
                           Query::ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Document())) );

        // need to check the mimetype as well since strigi is still not perfect
        m_fileTypes.append( qMakePair<QString, Query::Term>(i18nc("@option:check A filter on file type - audio files", "Audio"),
                           Query::ComparisonTerm(Nepomuk::Vocabulary::NIE::mimeType(), Query::LiteralTerm(QLatin1String("audio")))) );
        m_fileTypes.append( qMakePair<QString, Query::Term>(i18nc("@option:check A filter on file type - media video", "Video"),
                           Query::ComparisonTerm(Nepomuk::Vocabulary::NIE::mimeType(), Query::LiteralTerm(QLatin1String("video")))) );

        m_fileTypes.append( qMakePair<QString, Query::Term>(i18nc("@option:check A filter on file type", "Images"),
                           Query::ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Image())) );


        //
        // Default other types
        //
        m_otherTypes.append(qMakePair<QString, Query::Term>(i18nc("@option:check A filter on resource type", "Contacts"),
                            Query::ResourceTypeTerm(Nepomuk::Vocabulary::NCO::Contact())));
        m_otherTypes.append(qMakePair<QString, Query::Term>(i18nc("@option:check A filter on resource type", "Emails"),
                            Query::ResourceTypeTerm(Nepomuk::Vocabulary::NMO::Email())));
        m_otherTypes.append(qMakePair<QString, Query::Term>(i18nc("@option:check A filter on resource type", "Tasks"),
                            Query::ResourceTypeTerm(Nepomuk::Vocabulary::TMO::Task())));
        m_otherTypes.append(qMakePair<QString, Query::Term>(i18nc("@option:check A filter on resource type", "Tags"),
                            Query::ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag())));
    }

    bool findTerm( const Query::Term& term, int* index, RootSelection* rootSelection ) const;
    void setRootSelection( RootSelection selection );

    RootSelection m_rootSelection;
    QList<QPair<QString, Query::Term> > m_fileTypes;
    QList<QPair<QString, Query::Term> > m_otherTypes;

    /// points to m_fileTypes or m_otherTypes based on m_rootSelection
    QList<QPair<QString, Query::Term> >* m_usedTypes;

    /// a list of custom types which is filled by selectFromTerm()
    QList<Types::Class> m_customTypes;

    /// indexes into m_fileTypes or m_otherTypes depending on the value of m_rootSelection
    /// only used to avoid code duplication
    QSet<int> m_selectedTypes;

    /// indexes into m_customTypes
    QSet<int> m_selectedCustomTypes;
};


bool Nepomuk::Utils::TypeFacet::Private::findTerm( const Query::Term& term, int* index, RootSelection* rootSelection ) const
{
    for( int i = 0; i < m_fileTypes.count(); ++i ) {
        if( term == m_fileTypes[i].second ) {
            *index = i;
            *rootSelection = File;
            return true;
        }
    }
    for( int i = 0; i < m_otherTypes.count(); ++i ) {
        if( term == m_otherTypes[i].second ) {
            *index = i;
            *rootSelection = Other;
            return true;
        }
    }
    return false;
}


void Nepomuk::Utils::TypeFacet::Private::setRootSelection( RootSelection selection )
{
    m_rootSelection = selection;
    if( m_rootSelection == File )
        m_usedTypes = &m_fileTypes;
    else if( m_rootSelection == Other )
        m_usedTypes = &m_otherTypes;
    else
        m_usedTypes = 0;
}

Nepomuk::Utils::TypeFacet::TypeFacet(QObject *parent)
    : Nepomuk::Utils::Facet(parent),
      d(new Private())
{
}


Nepomuk::Utils::TypeFacet::~TypeFacet()
{
    delete d;
}

Nepomuk::Utils::Facet::SelectionMode Nepomuk::Utils::TypeFacet::selectionMode() const
{
    return MatchAny;
}

// while we do return file/other query terms even if there is no sub-selection we do not notify this change since
// to the user it should just be a preselection
Nepomuk::Query::Term Nepomuk::Utils::TypeFacet::queryTerm() const
{
    switch(d->m_rootSelection) {
    case None:
        return Query::Term();

    case File:
        if( d->m_selectedTypes.isEmpty() && d->m_selectedCustomTypes.isEmpty() ) {
            return Query::ResourceTypeTerm(Nepomuk::Vocabulary::NFO::FileDataObject());
        }
        else {
            Query::OrTerm term;
            Q_FOREACH( int i, d->m_selectedTypes ) {
                term.addSubTerm( d->m_fileTypes[i].second );
            }
            Q_FOREACH( int i, d->m_selectedCustomTypes ) {
                term.addSubTerm( Query::ResourceTypeTerm( d->m_customTypes[i] ) );
            }
            return term.optimized();
        }

    case Other:
        if( d->m_selectedTypes.isEmpty() && d->m_selectedCustomTypes.isEmpty() ) {
            return Query::NegationTerm::negateTerm(Query::ResourceTypeTerm(Nepomuk::Vocabulary::NFO::FileDataObject()));
        }
        else {
            Query::OrTerm term;
            Q_FOREACH( int i, d->m_selectedTypes ) {
                term.addSubTerm( d->m_otherTypes[i].second );
            }
            Q_FOREACH( int i, d->m_selectedCustomTypes ) {
                term.addSubTerm( Query::ResourceTypeTerm( d->m_customTypes[i] ) );
            }
            return term.optimized();
        }
    }

    // make gcc shut up
    return Query::Term();
}

int Nepomuk::Utils::TypeFacet::count() const
{
    if( d->m_rootSelection == None ) {
        // File and Other
        return 2;
    }
    else {
        // File/Other selection + fixed types + custom types
        return 1 + d->m_usedTypes->count() + d->m_customTypes.count();
    }
}

KGuiItem Nepomuk::Utils::TypeFacet::guiItem(int index) const
{
    if( d->m_rootSelection == None ) {
        switch( index ) {
        case 0:
            return KGuiItem(i18nc("@option:check Do filter on type - show only files", "Files"));
        case 1:
            return KGuiItem(i18nc("@option:check Do filter on type - show everything but files", "Other"));
        break;
        }
    }
    else {
        if( index == 0 ) {
            if( d->m_rootSelection == File )
                return KGuiItem(i18nc("@option:check Do filter on type - show only files", "Files"));
            else
                return KGuiItem(i18nc("@option:check Do filter on type - show everything but files", "Other"));
        }
        else {
            --index;
        }
        if( index < d->m_usedTypes->count() ) {
            return KGuiItem(d->m_usedTypes->at(index).first);
        }
        else if( index - d->m_usedTypes->count() < d->m_customTypes.count() ) {
            return KGuiItem(d->m_customTypes[index - d->m_usedTypes->count()].label());
        }
    }

    return KGuiItem();
}

bool Nepomuk::Utils::TypeFacet::isSelected(int index) const
{
    if( d->m_rootSelection == None ) {
        return false;
    }
    else if( index == 0 ) {
        return true;
    }
    else {
        --index;
        if( index < d->m_usedTypes->count() ) {
            return d->m_selectedTypes.contains(index);
        }
        else {
            index -= d->m_usedTypes->count();
            return d->m_selectedCustomTypes.contains(index);
        }
    }
}

void Nepomuk::Utils::TypeFacet::clearSelection()
{
    kDebug();
    d->m_selectedTypes.clear();
    d->m_selectedCustomTypes.clear();
    d->m_customTypes.clear();
    d->setRootSelection(None);
    setLayoutChanged();
    setSelectionChanged();
    setQueryTermChanged();
}

void Nepomuk::Utils::TypeFacet::setSelected(int index, bool selected)
{
    kDebug() << index << selected;
    if( d->m_rootSelection == None ) {
        if( selected ) {
            if( index == 0 ) {
                d->setRootSelection(File);
                setLayoutChanged();
                setSelectionChanged();
            }
            else if( index == 1 ) {
                d->setRootSelection(Other);
                setLayoutChanged();
                setSelectionChanged();
            }
        }
    }
    else {
        if( selected ) {
            if( index > 0 ) {
                --index;
                if( index < d->m_usedTypes->count() ) {
                    kDebug() << "Checking fixed type" << index;
                    if( !d->m_selectedTypes.contains(index) ) {
                        d->m_selectedTypes.insert(index);
                        setSelectionChanged();
                        setQueryTermChanged();
                    }
                }
                else {
                    index -= d->m_usedTypes->count();
                    kDebug() << "Checking custom type" << index;
                    if( index < d->m_customTypes.count() ) {
                        if( !d->m_selectedCustomTypes.contains(index) ) {
                            d->m_selectedCustomTypes.insert(index);
                            setSelectionChanged();
                            setQueryTermChanged();
                        }
                    }
                }
            }
        }
        else {
            if( index == 0 ) {
                clearSelection();
            }
            else {
                --index;
                if( index < d->m_usedTypes->count() ) {
                    if( d->m_selectedTypes.contains(index) ) {
                        d->m_selectedTypes.remove(index);
                        setSelectionChanged();
                        setQueryTermChanged();
                    }
                }
                else {
                    index -= d->m_usedTypes->count();
                    if( index < d->m_customTypes.count() ) {
                        if( d->m_selectedCustomTypes.contains(index) ) {
                            d->m_selectedCustomTypes.remove(index);
                            setSelectionChanged();
                            setQueryTermChanged();
                        }
                    }
                }
            }
        }
    }
}

bool Nepomuk::Utils::TypeFacet::selectFromTerm(const Nepomuk::Query::Term &queryTerm)
{
    // 1. compare to all the terms in d->m_fileTypes and d->m_otherTypes
    // 2. check if it is a ResourceTypeTerm
    // 3. check if it is a negation of the file term
    // 4. check if it is an OrTerm and do the above with all terms in it
    // 5. do not add nfo:FileDataObject as a custom type!

    int index = -1;
    RootSelection rootSel = None;
    if( d->findTerm( queryTerm, &index, &rootSel ) ) {
        if( d->m_rootSelection == None ||
                d->m_rootSelection == rootSel ) {
            d->m_selectedTypes.insert(index);
            setSelectionChanged();
            return true;
        }
    }
    else if( queryTerm.isResourceTypeTerm() ) {
        const Types::Class type = queryTerm.toResourceTypeTerm().type();
        if( type == Nepomuk::Vocabulary::NFO::FileDataObject() ) {
            if( d->m_rootSelection == None ) {
                setSelected(0);
                return true;
            }
            else {
                return d->m_rootSelection == File;
            }
        }
        else {
            const RootSelection reqSel = type.isSubClassOf(Nepomuk::Vocabulary::NFO::FileDataObject()) ? File : Other;
            if( d->m_rootSelection == None ) {
                d->setRootSelection(reqSel);
                d->m_customTypes.append(type);
                d->m_selectedCustomTypes.insert(d->m_customTypes.count()-1);
                setLayoutChanged();
                setSelectionChanged();
                setQueryTermChanged();
                return true;
            }
            else if( d->m_rootSelection == reqSel ) {
                if( !d->m_customTypes.contains(type) ) {
                    d->m_customTypes.append(type);
                    setLayoutChanged();
                }
                d->m_selectedCustomTypes.insert(d->m_customTypes.indexOf(type));
                setSelectionChanged();
                setQueryTermChanged();
                return true;
            }
        }
    }
    else if( queryTerm.isNegationTerm() &&
            queryTerm.toNegationTerm().subTerm().isResourceTypeTerm() &&
            queryTerm.toNegationTerm().subTerm().toResourceTypeTerm().type() == Nepomuk::Vocabulary::NFO::FileDataObject() ) {
        if( d->m_rootSelection == None ) {
            setSelected(1);
            return true;
        }
        else {
            return d->m_rootSelection == Other;
        }
    }

    // fallback
    return false;
}

#include "typefacet.moc"
