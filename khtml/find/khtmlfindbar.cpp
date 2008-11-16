/* This file is part of the KDE project
 *
 * Copyright (C) 2008 Bernhard Beschow <bbeschow cs tu berlin de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "khtmlfindbar.h"

#include "khtml_part.h"

#include <kfind.h>
#include <kcolorscheme.h>

#include <QtGui/QMenu>
#include <QtGui/QLineEdit>
#include <QtGui/QShortcut>

#define d this

KHTMLFindBar::KHTMLFindBar( QWidget *parent ) :
    KHTMLViewBarWidget( true, parent ),
    m_enabled( KFind::WholeWordsOnly | KFind::FromCursor | KFind::SelectedText | KFind::CaseSensitive | KFind::FindBackwards | KFind::RegularExpression )
{
    QWidget *widget = new QWidget( this );
    setupUi( widget );
    layout()->addWidget( widget );

    m_next->setIcon( KIcon( "go-down-search" ) );
    m_previous->setIcon( KIcon( "go-up-search" ) );

    // Fill options menu
    m_incMenu = new QMenu();
    m_options->setMenu(m_incMenu);
    m_caseSensitive = m_incMenu->addAction(i18n("C&ase sensitive"));
    m_caseSensitive->setCheckable(true);
    m_wholeWordsOnly = m_incMenu->addAction(i18n("&Whole words only"));
    m_wholeWordsOnly->setCheckable(true);
    m_fromCursor = m_incMenu->addAction(i18n("From c&ursor"));
    m_fromCursor->setCheckable(true);
    m_selectedText = m_incMenu->addAction(i18n("&Selected text"));
    m_selectedText->setCheckable(true);
    m_regExp = m_incMenu->addAction(i18n("Regular e&xpression"));
    m_regExp->setCheckable(true);

    connect( m_selectedText, SIGNAL(toggled(bool)), this, SLOT(slotSelectedTextToggled(bool)) );
    connect( m_find, SIGNAL(textChanged(const QString &)), this, SIGNAL(searchChanged()) );
    connect( m_next, SIGNAL(clicked()), this, SIGNAL(findNextClicked()) );
    connect( m_previous, SIGNAL(clicked()), this, SIGNAL(findPreviousClicked()) );
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SIGNAL(hideMe()));
    connect( m_caseSensitive, SIGNAL(changed()), this, SIGNAL(searchChanged()) );
    connect( m_wholeWordsOnly, SIGNAL(changed()), this, SIGNAL(searchChanged()) );
    connect( m_fromCursor, SIGNAL(changed()), this, SIGNAL(searchChanged()) );
    connect( m_regExp, SIGNAL(changed()), this, SIGNAL(searchChanged()) );

    m_find->setFocus();
}

QStringList KHTMLFindBar::findHistory() const
{
    return d->m_find->historyItems();
}

long KHTMLFindBar::options() const
{
    long options = 0;

    if (d->m_caseSensitive->isChecked())
        options |= KFind::CaseSensitive;
    if (d->m_wholeWordsOnly->isChecked())
        options |= KFind::WholeWordsOnly;
    if (d->m_fromCursor->isChecked())
        options |= KFind::FromCursor;
    if (d->m_selectedText->isChecked())
        options |= KFind::SelectedText;
    if (d->m_regExp->isChecked())
        options |= KFind::RegularExpression;
    return options | KHTMLPart::FindNoPopups /* | KFind::FindIncremental */;
}

QString KHTMLFindBar::pattern() const
{
    return d->m_find->currentText();
}

void KHTMLFindBar::setFindHistory(const QStringList &strings)
{
    if (strings.count() > 0)
    {
        d->m_find->setHistoryItems(strings, true);
        d->m_find->lineEdit()->setText( strings.first() );
        d->m_find->lineEdit()->selectAll();
    }
    else
        d->m_find->clearHistory();
}

void KHTMLFindBar::setHasSelection(bool hasSelection)
{
    if (hasSelection) d->m_enabled |= KFind::SelectedText;
    else d->m_enabled &= ~KFind::SelectedText;
    d->m_selectedText->setEnabled( hasSelection );
    if ( !hasSelection )
    {
        d->m_selectedText->setChecked( false );
        slotSelectedTextToggled( hasSelection );
    }
}

void KHTMLFindBar::slotSelectedTextToggled(bool selec)
{
    // From cursor doesn't make sense if we have a selection
    m_fromCursor->setEnabled( !selec && (m_enabled & KFind::FromCursor) );
    if ( selec ) // uncheck if disabled
        m_fromCursor->setChecked( false );
}

void KHTMLFindBar::setHasCursor(bool hasCursor)
{
    if (hasCursor) d->m_enabled |= KFind::FromCursor;
    else d->m_enabled &= ~KFind::FromCursor;
    d->m_fromCursor->setEnabled( hasCursor );
    d->m_fromCursor->setChecked( hasCursor && (options() & KFind::FromCursor) );
}

void KHTMLFindBar::setOptions(long options)
{
    d->m_caseSensitive->setChecked((d->m_enabled & KFind::CaseSensitive) && (options & KFind::CaseSensitive));
    d->m_wholeWordsOnly->setChecked((d->m_enabled & KFind::WholeWordsOnly) && (options & KFind::WholeWordsOnly));
    d->m_fromCursor->setChecked((d->m_enabled & KFind::FromCursor) && (options & KFind::FromCursor));
    d->m_selectedText->setChecked((d->m_enabled & KFind::SelectedText) && (options & KFind::SelectedText));
    d->m_regExp->setChecked((d->m_enabled & KFind::RegularExpression) && (options & KFind::RegularExpression));
}

void KHTMLFindBar::setFoundMatch( bool match )
{
    if ( !match && !m_find->currentText().isEmpty() )
    {
        KStatefulBrush backgroundBrush( KColorScheme::View, KColorScheme::NegativeBackground );

        QString styleSheet = QString( "KHistoryComboBox{ background-color:%1 }" )
                             .arg( backgroundBrush.brush(m_find).color().name() );

        m_find->setStyleSheet( styleSheet );
    }
    else
    {
        m_find->setStyleSheet( QString() );
    }
}

void KHTMLFindBar::setAtEnd( bool atEnd )
{
    if ( atEnd )
        m_statusLabel->setText( i18n( "No more matches for this search direction." ) );
    else
        m_statusLabel->clear();
}

void KHTMLFindBar::setVisible( bool visible )
{
    KHTMLViewBarWidget::setVisible( visible );

    if ( isVisible() )
        m_find->setFocus( Qt::ActiveWindowFocusReason );
}
