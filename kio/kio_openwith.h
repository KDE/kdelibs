// "$Id$"

#ifndef __open_with_h__
#define __open_with_h__

#include <qlineedit.h>
#include <qwidget.h>
#include <qfileinfo.h>
#include <qlistview.h>
#include <qdialog.h>

#include <kapp.h>
#include <kurlcompletion.h>
#include "kservices.h"
#include <ksharedptr.h>

class KApplicationTree;

class QCheckBox;
class QPushButton;
class QLabel;
class QStringList;

/* ------------------------------------------------------------------------- */

/**
 */
class OpenWithDlg : public QDialog
{
    Q_OBJECT
public:
    /**
     * Create a dialog that asks for a application to open a given URL(s) with. _
     *
     * @param _url   is the list of URLs that should be opened
     * @param _value is the initial value of the line
     * @param _text  appears as a label on top of the entry box.  
     */
    
    OpenWithDlg( const QStringList& _url, const QString&_text, const QString&_value, QWidget *parent );
    ~OpenWithDlg();
    
    /**
     * @return the value the user entered
     */
    QString text() { return edit->text(); }
    KService::Ptr service() { return m_pService; }
  
public slots:
    /**
    * The slot for clearing the edit widget
    */
    void slotClear();
    void slotSelected( const QString&_name, const QString& _exec );
    void slotHighlighted( const QString& _name, const QString& _exec );
    void slotOK();
    
protected:
    QLineEdit *edit;
    KURLCompletion * completion;

    KApplicationTree* m_pTree;
    QLabel *label;

    QString qName;
    bool  haveApp;
    QCheckBox   *terminal;
    QPushButton *ok;
    QPushButton *clear;
    QPushButton *cancel;
    
    KService::Ptr m_pService;
};


/* ------------------------------------------------------------------------- */

class KAppTreeListItem : public QListViewItem
{
    bool parsed;
    bool directory;
    QString path;
    QString exec;

protected:
    QString key(int column, bool ascending) const;

    void init(const QPixmap& pixmap, bool parse, bool dir, QString _path, QString exec);
    
public:
    KAppTreeListItem( QListView* parent, const char *name, const QPixmap& pixmap,
                      bool parse, bool dir, QString p, QString c );
    KAppTreeListItem( QListViewItem* parent, const char *name, const QPixmap& pixmap,
                      bool parse, bool dir, QString p, QString c );

    friend KApplicationTree;
};

/* ------------------------------------------------------------------------- */

class KApplicationTree : public QListView
{
    Q_OBJECT
public:
    KApplicationTree( QWidget *parent );

    bool isDesktopFile( const QString& filename );

    /**
     * Parse a single .desktop/.kdelnk file
     */
    void parseDesktopFile( QFileInfo *fi, KAppTreeListItem *item );
    /**
     * Parse a directory for .desktop/.kdelnk files
     */
    short parseDesktopDir( QDir d, KAppTreeListItem *item = 0 );
  
    KAppTreeListItem *it;
    KAppTreeListItem *it2;
    
protected:
    void resizeEvent( QResizeEvent *_ev );
  
public slots:
    void slotFolderSelected(QListViewItem* i);
    void slotItemHighlighted(QListViewItem* i);
    void slotSelectionChanged(QListViewItem* i);
    
signals:
    void selected( const QString& _name, const QString& _exec );
    void highlighted( const QString& _name, const  QString& _exec );
};

/* ------------------------------------------------------------------------- */

#endif
