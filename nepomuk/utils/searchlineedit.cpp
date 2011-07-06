/*
  Copyright (c) 2010 Oszkar Ambrus <aoszkar@gmail.com>
  Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "searchlineedit.h"

#include "klineedit.h"

#include <QtGui/QHBoxLayout>

#include "queryparser.h"
#include "andterm.h"
#include "orterm.h"
#include "literalterm.h"

using namespace Nepomuk::Query;


class Nepomuk::Utils::SearchLineEdit::Private {
public:
    KLineEdit* m_lineEdit;
    bool searchWhileTyping;

    QString m_currentQueryString;
    Query::Query m_currentQuery;

    void updateCurrentQuery();
    void _k_queryStringChanged(const QString&);

    SearchLineEdit* q;
};

void Nepomuk::Utils::SearchLineEdit::Private::updateCurrentQuery()
{
    // cache the current query for performance reasons
    if( m_currentQueryString != m_lineEdit->text() ) {
        m_currentQueryString = m_lineEdit->text();
        m_currentQuery = Nepomuk::Query::QueryParser::parseQuery( m_currentQueryString );
    }
}


void Nepomuk::Utils::SearchLineEdit::Private::_k_queryStringChanged( const QString& /*queryString*/ )
{
    emit q->queryChanged( q->query() );
}


Nepomuk::Utils::SearchLineEdit::SearchLineEdit(QWidget *parent)
    : QWidget(parent),
      d(new Private)
{
    d->q = this;

    d->m_lineEdit = new KLineEdit( this );
    d->m_lineEdit->setClearButtonShown(true);
    d->m_lineEdit->setClickMessage(i18n("Enter Search Terms..."));
    QHBoxLayout* lay = new QHBoxLayout( this );
    lay->setMargin(0);
    lay->addWidget( d->m_lineEdit );

    setFocusProxy( d->m_lineEdit );

    setSearchWhileTypingEnabled(false);
}

Nepomuk::Utils::SearchLineEdit::~SearchLineEdit()
{
    delete d;
}

void Nepomuk::Utils::SearchLineEdit::setSearchWhileTypingEnabled(bool enable)
{
    d->searchWhileTyping = enable;
    disconnect( d->m_lineEdit, 0, this, SLOT(_k_queryStringChanged(QString)) );

    if(d->searchWhileTyping == true) {
        connect( d->m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(_k_queryStringChanged(QString)) );
    }
    else {
        connect( d->m_lineEdit, SIGNAL(returnPressed(QString)), this, SLOT(_k_queryStringChanged(QString)) );
    }
}

Nepomuk::Query::Term Nepomuk::Utils::SearchLineEdit::extractUsableTerms( const Nepomuk::Query::Term& term )
{
    Term restTerm( term );
    QString queryString;

    // now extract as much as possible to put in the literal search line
    if(restTerm.isLiteralTerm()) {
        queryString = restTerm.toLiteralTerm().value().toString();
        restTerm = Term();
    }

    // from an AndTerm we can extract as many literal values as we like
    else if ( restTerm.isAndTerm() ) {
        QStringList searchTerms;
        foreach( const Nepomuk::Query::Term& subTerm, restTerm.toAndTerm().subTerms() ) {
            if(subTerm.isLiteralTerm()) {
                searchTerms << subTerm.toLiteralTerm().value().toString();
            }
            else {
                restTerm = restTerm && subTerm;
            }
        }
        queryString =  searchTerms.join( QLatin1String( " " ) );
    }

    // from an OrTerm we can only extract if all the sub terms are literals since
    // otherwise the final query would be different as all the parts are combined
    // via an AndTerm.
    else if ( restTerm.isOrTerm() ) {
        QStringList searchTerms;
        OrTerm restOrTerm;
        foreach( const Nepomuk::Query::Term& subTerm, restTerm.toAndTerm().subTerms() ) {
            if(subTerm.isLiteralTerm()) {
                searchTerms << subTerm.toLiteralTerm().value().toString();
            }
            else {
                restOrTerm.addSubTerm(subTerm);
            }
        }
        if ( restOrTerm.subTerms().isEmpty() ) {
            restTerm = restOrTerm.optimized();
            queryString = searchTerms.join( QLatin1String( " OR " ) );
        }
    }

    d->m_lineEdit->setText(queryString);
    return restTerm;
}

Nepomuk::Query::Query Nepomuk::Utils::SearchLineEdit::query() const
{
    d->updateCurrentQuery();
    return d->m_currentQuery;
}

#include "searchlineedit.moc"
