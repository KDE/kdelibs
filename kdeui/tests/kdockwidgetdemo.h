#ifndef KDOCKWIDGETDEMO_H
#define KDOCKWIDGETDEMO_H

#include <kdockwidget.h>

#include <qdialog.h>
#include <qlistview.h>
#include <qstring.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qscrollview.h>
#include <qfiledialog.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qurl.h>
#include <qpixmap.h>

class QMultiLineEdit;
class QTextView;
class QToolButton;
class QSpinBox;
class QShowEvent;
class QPopupMenu;

class DirectoryView;
class CustomFileDialog;
class Preview;
class DirectoryView;

class SFileDialog : public QDialog
{Q_OBJECT

public:
	SFileDialog( QString initially = QString::null,
                                  const QStringList& filter = "All files ( * )", const char* name = 0 );
	~SFileDialog();

  static QString getOpenFileName( QString initially = QString::null,
                                  const QStringList& filter = "All files ( * )",
                                  const QString caption = QString::null, const char* name = 0 );

  static QStringList getOpenFileNames( QString initially = QString::null,
                                  const QStringList& filter = "All files ( * )",
                                  const QString caption = QString::null, const char* name = 0 );


protected:
  void showEvent( QShowEvent *e );

protected slots:
  void dockChange();
  void setDockDefaultPos( KDockWidget* );
  void changeDir( const QString& );

private:
  DirectoryView* dirView;
  CustomFileDialog* fd;
  Preview* preview;

  KDockManager* dockManager;
  KDockWidget* d_dirView;
  KDockWidget* d_preview;
  KDockWidget* d_fd;

  QToolButton *b_tree;
  QToolButton *b_preview;
};
/******************************************************************************************************/
class Directory : public QListViewItem
{
public:
    Directory( QListView * parent, const QString& filename );
    Directory( Directory * parent, const QString& filename );

    QString text( int column ) const;

    QString fullName();

    void setOpen( bool );
    void setup();

private:
    QFile f;
    Directory * p;
    bool readable;
};

class DirectoryView : public QListView
{Q_OBJECT
public:
  DirectoryView( QWidget *parent = 0, const char *name = 0 );
  virtual void setOpen ( QListViewItem *, bool );

  QString selectedDir();

public slots:
  void setDir( const QString & );

signals:
  void folderSelected( const QString & );

protected slots:
  void slotFolderSelected( QListViewItem * );

private:
  QString fullPath(QListViewItem* item);
};
/******************************************************************************************************/
class PixmapView : public QScrollView
{Q_OBJECT
public:
  PixmapView( QWidget *parent );
  void setPixmap( const QPixmap &pix );
  void drawContents( QPainter *p, int, int, int, int );

private:
  QPixmap pixmap;
};

class Preview : public QWidgetStack
{Q_OBJECT
public:
  Preview( QWidget *parent );

public slots:
  void showPreview( const QString& );

private:
  QMultiLineEdit *normalText;
  QTextView *html;
  PixmapView *pixmap;
};

class CustomFileDialog : public QFileDialog
{Q_OBJECT
public:
  CustomFileDialog( QWidget* parent );
  ~CustomFileDialog();

  void addToolButton( QButton * b, bool separator = false ){ QFileDialog::addToolButton(b,separator); }
  void setBookmark( QStringList& );
  QStringList getBookmark(){ return bookmarkList; }

public slots:
  void setDir2( const QString & );

signals:
  void signalDone( int );

protected slots:
  void bookmarkChosen( int i );
  void goHome();
  virtual void done( int );

private:
  QPopupMenu *bookmarkMenu;
  QStringList bookmarkList;
  int addId, clearId;
};

#endif


