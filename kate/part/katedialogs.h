/* This file is part of the KDE libraries
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>

   Based on work of:
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

#ifndef __KATE_DIALOGS_H__
#define __KATE_DIALOGS_H__

#include "katehighlight.h"
#include "kateattribute.h"

#include "../interfaces/document.h"

#include <klistview.h>
#include <kdialogbase.h>
#include <kmimetype.h>

#include <qstringlist.h>
#include <qcolor.h>
#include <qintdict.h>
#include <qvbox.h>
#include <qtabwidget.h>

class KatePartPluginListItem;

struct syntaxContextData;

class KateDocument;
class KateView;

namespace KIO { class Job; }

class KAccel;
class KColorButton;
class KComboBox;
class KIntNumInput;
class KKeyButton;
class KKeyChooser;
class KMainWindow;
class KPushButton;
class KRegExpDialog;
class KIntNumInput;
class KSpellConfig;

class QButtonGroup;
class QCheckBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QListBoxItem;
class QWidgetStack;
class QVBox;
class QListViewItem;
class QCheckBox;

class KateConfigPage : public Kate::ConfigPage
{
  Q_OBJECT

  public:
    KateConfigPage ( QWidget *parent=0, const char *name=0 );
    virtual ~KateConfigPage ();

  public:
    bool changed () { return m_changed; }

  private slots:
    void somethingHasChanged ();

  private:
    bool m_changed;
};

class SpellConfigPage : public KateConfigPage
{
  Q_OBJECT

  public:
    SpellConfigPage( QWidget* parent );
    ~SpellConfigPage() {};

    void apply();
    void reset () { ; };
    void defaults () { ; };

  private:
    KSpellConfig *cPage;
};

class GotoLineDialog : public KDialogBase
{
  Q_OBJECT

  public:

    GotoLineDialog(QWidget *parent, int line, int max);
    int getLine();

  protected:

    KIntNumInput *e1;
    QPushButton *btnOK;
};

class IndentConfigTab : public KateConfigPage
{
  Q_OBJECT

  public:
    IndentConfigTab(QWidget *parent);

  protected slots:
    void spacesToggled();

  protected:
    enum { numFlags = 6 };
    static const int flags[numFlags];
    QCheckBox *opt[numFlags];
    KIntNumInput *indentationWidth;
    QButtonGroup *m_tabs;
    KComboBox *m_indentMode;

  public slots:
    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};
};

class SelectConfigTab : public KateConfigPage
{
  Q_OBJECT

  public:
    SelectConfigTab(QWidget *parent);

  protected:
    QButtonGroup *m_tabs;

  public slots:
    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};
};

class EditConfigTab : public KateConfigPage
{
    Q_OBJECT

  public:
    EditConfigTab(QWidget *parent);

  protected:
    enum { numFlags = 5 };
    static const int flags[numFlags];
    QCheckBox *opt[numFlags];

    KIntNumInput *e1;
    KIntNumInput *e2;
    KIntNumInput *e3;
    KIntNumInput *e4;
    KComboBox *e5;
    QCheckBox *e6;

  public slots:
    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};
};

class ViewDefaultsConfig : public KateConfigPage
{
  Q_OBJECT

  public:
    ViewDefaultsConfig( QWidget *parent );
    ~ViewDefaultsConfig();

  private:
    QCheckBox *m_line;
    QCheckBox *m_folding;
    QCheckBox *m_collapseTopLevel;
    QCheckBox *m_icons;
    QCheckBox *m_dynwrap;
    KIntNumInput *m_dynwrapAlignLevel;
    QCheckBox *m_wwmarker;
    QLabel *m_dynwrapIndicatorsLabel;
    KComboBox *m_dynwrapIndicatorsCombo;
    QButtonGroup *m_bmSort;

  public slots:
  void apply ();
  void reload ();
  void reset ();
  void defaults ();
};

class EditKeyConfiguration: public KateConfigPage
{
  Q_OBJECT

  public:
    EditKeyConfiguration( QWidget* parent, KateDocument* doc );

  public slots:
    void apply();
    void reload()   {};
    void reset()    {};
    void defaults() {};

  protected:
    void showEvent ( QShowEvent * );

  private:
    bool m_ready;
    class KateDocument *m_doc;
    class KActionCollection *m_ac;
    KKeyChooser* m_keyChooser;
};

class SaveConfigTab : public KateConfigPage
{
  Q_OBJECT
  public:
  SaveConfigTab( QWidget *parent );

  public slots:
  void apply();
  void reload();
  void reset();
  void defaults();

  protected:
  KComboBox *m_encoding, *m_eol;
  QCheckBox *cbLocalFiles, *cbRemoteFiles;
  QCheckBox *replaceTabs, *removeSpaces;
  QLineEdit *leBuSuffix;
};

class KatePartPluginListItem;

class KatePartPluginListView : public KListView
{
  Q_OBJECT

  friend class KatePartPluginListItem;

  public:
    KatePartPluginListView (QWidget *parent = 0, const char *name = 0);

  signals:
    void stateChange(KatePartPluginListItem *, bool);

  private:
    void stateChanged(KatePartPluginListItem *, bool);
};

class PluginConfigPage : public KateConfigPage
{
  Q_OBJECT

  public:
    PluginConfigPage (QWidget *parent);
    ~PluginConfigPage ();

  public slots:
    void apply ();
    void reload () {};
    void reset () {};
    void defaults () {};

  private:
    KatePartPluginListView *listView;
    QPtrList<KatePartPluginListItem> m_items;
};

/**
   This widget provides a checkable list of all available mimetypes,
   and a list of selected ones, as well as a corresponding list of file
   extensions, an optional text and an optional edit button (not working yet).
   Mime types is presented in a list view, with name, comment and patterns columns.
   Added by anders, jan 23, 2002
*/
class KMimeTypeChooser : public QVBox
{
  Q_OBJECT

  public:
    KMimeTypeChooser( QWidget *parent=0, const QString& text=QString::null, const QStringList &selectedMimeTypes=0,
                      bool editbutton=true, bool showcomment=true, bool showpattern=true );
    ~KMimeTypeChooser() {};
    QStringList selectedMimeTypesStringList();
    QStringList patterns();

  public slots:
    void editMimeType();
    void slotCurrentChanged(QListViewItem* i);

  private:
    QListView *lvMimeTypes;
    QPushButton *btnEditMimeType;
};

/**
   @short A Dialog to choose some mimetypes.
   Provides a checkable tree list of mimetypes, with icons and optinally comments and patterns,
   and an (optional) button to display the mimetype kcontrol module.

   @param title The title of the dialog
   @param text A Text to display above the list
   @param selectedMimeTypes A list of mimetype names, theese will be checked in the list if they exist.
   @param editbutton Whether to display the a "Edit Mimetypes" button.
   @param showcomment If this is true, a column displaying the mimetype's comment will be added to the list view.
   @param showpatterns If this is true, a column displaying the mimetype's patterns will be added to the list view.
   Added by anders, dec 19, 2001
*/
class KMimeTypeChooserDlg : public KDialogBase
{
  public:
    KMimeTypeChooserDlg( QWidget *parent=0,
                         const QString &caption=QString::null, const QString& text=QString::null,
                         const QStringList &selectedMimeTypes=QStringList(),
                         bool editbutton=true, bool showcomment=true, bool showpatterns=true );
    ~KMimeTypeChooserDlg();

    QStringList mimeTypes();
    QStringList patterns();

  private:
    KMimeTypeChooser *chooser;
};

class HlConfigPage : public KateConfigPage
{
  Q_OBJECT

  public:
    HlConfigPage (QWidget *parent);
    ~HlConfigPage ();

  public slots:
    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};

  protected slots:
    void hlChanged(int);
    void hlDownload();
    void showMTDlg();

  private:
    void writeback ();

    QComboBox *hlCombo;
    QLineEdit *wildcards;
    QLineEdit *mimetypes;
    class KIntNumInput *priority;

    QIntDict<HlData> hlDataDict;
    HlData *hlData;
};

class HlDownloadDialog: public KDialogBase
{
  Q_OBJECT

  public:
    HlDownloadDialog(QWidget *parent, const char *name, bool modal);
    ~HlDownloadDialog();

  private:
    class QListView  *list;
    class QString listData;

  private slots:
    void listDataReceived(KIO::Job *, const QByteArray &data);

  public slots:
    void slotUser1();
};

#endif
