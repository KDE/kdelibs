/* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _katesearch_h_
#define _katesearch_h_

#include "katecursor.h"

#include <kdialogbase.h>

#include <qstring.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class KateView;
class KateDocument;
class KateSuperRangeList;

class KActionCollection;

class KateSearch : public QObject
{
  Q_OBJECT

  friend class KateDocument;

  private:
    class SearchFlags
    {
      public:
        bool caseSensitive     :1;
        bool wholeWords        :1;
        bool fromBeginning     :1;
        bool backward          :1;
        bool selected          :1;
        bool prompt            :1;
        bool replace           :1;
        bool finished          :1;
        bool regExp            :1;
    };
  
    class SConfig
    {
      public:
        SearchFlags flags;
        KateTextCursor cursor;
        KateTextCursor wrappedEnd; // after wraping around, search/replace until here
        bool wrapped; // have we allready wrapped around ?
        uint matchedLength;
        KateTextCursor selBegin;
        KateTextCursor selEnd;
    };
  
  public:
    enum Dialog_results {
      srCancel = KDialogBase::Cancel,
      srAll = KDialogBase::User1,
      srLast = KDialogBase::User2,
      srNo = KDialogBase::User3,
      srYes = KDialogBase::Ok
    };

  public:
    KateSearch( KateView* );
    ~KateSearch();

    void createActions( KActionCollection* );

  public slots:
    void find();
    void replace();
    void findAgain( bool back );

  private slots:
    void replaceSlot();
    void slotFindNext() { findAgain( false ); }
    void slotFindPrev() { findAgain( true );  }

  private:
    static void addToList( QStringList&, const QString& );
    static void addToSearchList( const QString& s )  { addToList( s_searchList, s ); }
    static void addToReplaceList( const QString& s ) { addToList( s_replaceList, s ); }
    static QStringList s_searchList;
    static QStringList s_replaceList;

    void search( SearchFlags flags );
    void wrapSearch();
    bool askContinue();

    void findAgain();
    void promptReplace();
    void replaceAll();
    void replaceOne();
    void skipOne();

    QString getSearchText();
    KateTextCursor getCursor();
    bool doSearch( const QString& text );
    void exposeFound( KateTextCursor &cursor, int slen );

    inline KateView* view()    { return m_view; }
    inline KateDocument* doc() { return m_doc;  }

    KateView*     m_view;
    KateDocument* m_doc;

    KateSuperRangeList* m_arbitraryHLList;

    SConfig s;

    QValueList<SConfig> m_searchResults;
    int                 m_resultIndex;

    int           replaces;
    QDialog*      replacePrompt;
    QString m_replacement;
    QRegExp m_re;
};

class ReplacePrompt : public KDialogBase
{
  Q_OBJECT
    
  public:
    ReplacePrompt(QWidget *parent);

  signals:
    void clicked();

  protected slots:
    void slotOk( void );
    void slotClose( void );
    void slotUser1( void ); // All
    void slotUser2( void ); // Last
    void slotUser3( void ); // Yes
    virtual void done(int);
};

#endif
