// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * dialog.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef KSPELL_DIALOG_H
#define KSPELL_DIALOG_H

#include <kdialogbase.h>

class Q3ListViewItem;

namespace KSpell2
{
    class Filter;
    class BackgroundChecker;
    class KSPELL2_EXPORT Dialog : public KDialogBase
    {
        Q_OBJECT
    public:
        Dialog( BackgroundChecker *checker,
                QWidget *parent, const char *name=0 );
        ~Dialog();

        QString originalBuffer() const;
        QString buffer() const;

        void show();
        void activeAutoCorrect( bool _active );

    public Q_SLOTS:
        void setBuffer( const QString& );
        void setFilter( Filter* filter );

    Q_SIGNALS:
        void done( const QString& newBuffer );
        void misspelling( const QString& word, int start );
        void replace( const QString& oldWord, int start,
                      const QString& newWord );

        void stop();
        void cancel();
        void autoCorrect( const QString & currentWord, const QString & replaceWord );
    private Q_SLOTS:
        void slotMisspelling(const QString& word, int start );
        void slotDone();

        void slotFinished();
        void slotCancel();

        void slotAddWord();
        void slotReplaceWord();
        void slotReplaceAll();
        void slotSkip();
        void slotSkipAll();
        void slotSuggest();
        void slotChangeLanguage( const QString& );
        void slotSelectionChanged( Q3ListViewItem * );
        void slotAutocorrect();

    private:
        void updateDialog( const QString& word );
        void fillSuggestions( const QStringList& suggs );
        void initConnections();
        void initGui();
        void continueChecking();

    private:
        class Private;
        Private* const d;
    };
}

#endif
