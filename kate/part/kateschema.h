/* This file is part of the KDE libraries
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

#ifndef __KATE_SCHEMA_H__
#define __KATE_SCHEMA_H__

#include "katehighlight.h"
#include "katedialogs.h"

#include <qstringlist.h>
#include <qintdict.h>
#include <qlistview.h>

#include <kconfig.h>
#include <kaction.h>

class KateView;
class KateStyleListItem;

class KColorButton;

class QPopupMenu;

class KateSchemaManager
{
  public:
    KateSchemaManager ();
    ~KateSchemaManager ();

    /**
     * Schema Config changed, update all renderers
     */
    void update (bool readfromfile = true);

    /**
     * return kconfig with right group set or set to Normal if not there
     */
    KConfig *schema (uint number);

    void addSchema (const QString &t);

    void removeSchema (uint number);

    /**
     * is this schema valid ? (does it exist ?)
     */
    bool validSchema (uint number);

    /**
     * if not found, defaults to 0
     */
    uint number (const QString &name);

    /**
     * group names in the end, no i18n involved
     */
    QString name (uint number);

    /**
     * Don't modify, list with the names of the schemas (i18n name for the default ones)
     */
    const QStringList &list () { return m_schemas; }

    static QString normalSchema ();
    static QString printingSchema ();

  private:
    KConfig m_config;
    QStringList m_schemas;
};


class KateViewSchemaAction : public KActionMenu
{
  Q_OBJECT

  public:
    KateViewSchemaAction(const QString& text, QObject* parent = 0, const char* name = 0)
       : KActionMenu(text, parent, name) { init(); };

    ~KateViewSchemaAction(){;};

    void updateMenu (KateView *view);

  private:
    void init();

    QGuardedPtr<KateView> m_view;
    QStringList names;
    int last;

  public  slots:
    void slotAboutToShow();

  private slots:
    void setSchema (int mode);
};

//
// DIALOGS
//

/*
    QListView that automatically adds columns for KateStyleListItems and provides a
    popup menu and a slot to edit a style using the keyboard.
    Added by anders, jan 23 2002.
*/
class KateStyleListView : public QListView
{
  Q_OBJECT

  friend class KateStyleListItem;

  public:
    KateStyleListView( QWidget *parent=0, bool showUseDefaults=false);
    ~KateStyleListView() {};
    /* Display a popupmenu for item i at the specified global position, eventually with a title,
       promoting the context name of that item */
    void showPopupMenu( KateStyleListItem *i, const QPoint &globalPos, bool showtitle=false );
    void emitChanged() { emit changed(); };

    void setBgCol( const QColor &c ) { bgcol = c; }
    void setSelCol( const QColor &c ) { selcol = c; }
    void setNormalCol( const QColor &c ) { normalcol = c; }

  private slots:
    /* Display a popupmenu for item i at item position */
    void showPopupMenu( QListViewItem *i );
    /* call item to change a property, or display a menu */
    void slotMousePressed( int, QListViewItem*, const QPoint&, int );
    /* asks item to change the property in q */
    void mSlotPopupHandler( int z );

  signals:
    void changed();

  private:
    QColor bgcol, selcol, normalcol;
    QFont docfont;
};

class KateSchemaConfigColorTab : public QWidget
{
  Q_OBJECT

  public:
    KateSchemaConfigColorTab( QWidget *parent = 0, const char *name = 0 );
    ~KateSchemaConfigColorTab();

  private:
    KColorButton *m_back;
    KColorButton *m_selected;
    KColorButton *m_current;
    KColorButton *m_bracket;
    KColorButton *m_wwmarker;
    KColorButton *m_iconborder;
    KColorButton *m_tmarker;

  public:
    void readConfig (KConfig *config);
    void writeConfig (KConfig *config);
};

class KateSchemaConfigFontTab : public QWidget
{
  Q_OBJECT

  public:
    KateSchemaConfigFontTab( QWidget *parent = 0, const char *name = 0 );
    ~KateSchemaConfigFontTab();

  public:
    void readConfig (KConfig *config);
    void writeConfig (KConfig *config);

  private:
    class KFontChooser *m_fontchooser;
    QFont myFont;

  private slots:
    void slotFontSelected( const QFont &font );
};

class KateSchemaConfigFontColorTab : public QWidget
{
  Q_OBJECT

  public:
    KateSchemaConfigFontColorTab( QWidget *parent = 0, const char *name = 0 );
    ~KateSchemaConfigFontColorTab();

  public:
    void schemaChanged (uint schema);
    void reload ();
    void apply ();

    KateAttributeList *attributeList (uint schema);

  private:
    KateStyleListView *m_defaultStyles;
    QIntDict<KateAttributeList> m_defaultStyleLists;
};

class KateSchemaConfigHighlightTab : public QWidget
{
  Q_OBJECT

  public:
    KateSchemaConfigHighlightTab( QWidget *parent = 0, const char *name = 0, KateSchemaConfigFontColorTab *page = 0 );
    ~KateSchemaConfigHighlightTab();

  public:
    void schemaChanged (uint schema);
    void reload ();
    void apply ();

  protected slots:
    void hlChanged(int z);

  private:
    KateSchemaConfigFontColorTab *m_defaults;

    QComboBox *hlCombo;
    KateStyleListView *m_styles;

    uint m_schema;
    int m_hl;

    QIntDict< QIntDict<ItemDataList> > m_hlDict;
};

class KateSchemaConfigPage : public KateConfigPage
{
  Q_OBJECT

  public:
    KateSchemaConfigPage ( QWidget *parent );
    ~KateSchemaConfigPage ();

  public slots:
    void apply();
    void reload();
    void reset();
    void defaults();

  private slots:
    void update ();
    void deleteSchema ();
    void newSchema ();
    void schemaChanged (int schema);

    void newCurrentPage (QWidget *w);

  private:
    int m_lastSchema;
    class QTabWidget *m_tabWidget;
    class QPushButton *btndel;
    class QComboBox *defaultSchemaCombo;
    class QComboBox *schemaCombo;
    KateSchemaConfigColorTab *m_colorTab;
    KateSchemaConfigFontTab *m_fontTab;
    KateSchemaConfigFontColorTab *m_fontColorTab;
    KateSchemaConfigHighlightTab *m_highlightTab;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
