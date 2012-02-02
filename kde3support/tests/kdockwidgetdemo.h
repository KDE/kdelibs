#ifndef KDOCKWIDGETDEMO_H
#define KDOCKWIDGETDEMO_H

#include <k3dockwidget.h>

#include <QDialog>
#include <Qt3Support/Q3ListView>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <Qt3Support/Q3ScrollView>
#include <Qt3Support/Q3FileDialog>
#include <Qt3Support/Q3WidgetStack>
#include <QPixmap>

class Q3MultiLineEdit;
class Q3TextView;
class QToolButton;
class QShowEvent;
class Q3PopupMenu;

class DirectoryView;
class CustomFileDialog;
class Preview;
class DirectoryView;

class SFileDialog : public QDialog
{Q_OBJECT

public:
	SFileDialog( QString initially = QString(),
                                  const QStringList& filter = QStringList("All Files ( * )"), const char* name = 0 );
	~SFileDialog();

  static QString getOpenFileName( QString initially = QString(),
                                  const QStringList& filter = QStringList("All Files ( * )"),
                                  const QString caption = QString(), const char* name = 0 );

  static QStringList getOpenFileNames( QString initially = QString(),
                                  const QStringList& filter = QStringList("All Files ( * )"),
                                  const QString caption = QString(), const char* name = 0 );


protected:
  void showEvent( QShowEvent *e );

protected Q_SLOTS:
  void dockChange();
  void setDockDefaultPos( K3DockWidget* );
  void changeDir( const QString& );

private:
  DirectoryView* dirView;
  CustomFileDialog* fd;
  Preview* preview;

  K3DockManager* dockManager;
  K3DockWidget* d_dirView;
  K3DockWidget* d_preview;
  K3DockWidget* d_fd;

  QToolButton *b_tree;
  QToolButton *b_preview;
};
/******************************************************************************************************/
class Directory : public Q3ListViewItem
{
public:
    Directory( Q3ListView * parent, const QString& filename );
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

class DirectoryView : public Q3ListView
{Q_OBJECT
public:
  DirectoryView( QWidget *parent = 0, const char *name = 0 );
  virtual void setOpen ( Q3ListViewItem *, bool );

  QString selectedDir();

public Q_SLOTS:
  void setDir( const QString & );

Q_SIGNALS:
  void folderSelected( const QString & );

protected Q_SLOTS:
  void slotFolderSelected( Q3ListViewItem * );

private:
  QString fullPath(Q3ListViewItem* item);
};
/******************************************************************************************************/
class PixmapView : public Q3ScrollView
{Q_OBJECT
public:
  PixmapView( QWidget *parent );
  void setPixmap( const QPixmap &pix );
  void drawContents( QPainter *p, int, int, int, int );

private:
  QPixmap pixmap;
};

class Preview : public Q3WidgetStack
{Q_OBJECT
public:
  Preview( QWidget *parent );

public Q_SLOTS:
  void showPreview( const QString& );

private:
  Q3MultiLineEdit *normalText;
  Q3TextView *html;
  PixmapView *pixmap;
};

class CustomFileDialog : public Q3FileDialog
{Q_OBJECT
public:
  CustomFileDialog( QWidget* parent );
  ~CustomFileDialog();

  void addToolButton( QAbstractButton * b, bool separator = false ){ Q3FileDialog::addToolButton(b,separator); }
  void setBookmark( QStringList& );
  QStringList getBookmark(){ return bookmarkList; }

public Q_SLOTS:
  void setDir2( const QString & );

Q_SIGNALS:
  void signalDone( int );

protected Q_SLOTS:
  void bookmarkChosen( int i );
  void goHome();
  virtual void done( int );

private:
  Q3PopupMenu *bookmarkMenu;
  QStringList bookmarkList;
  int addId, clearId;
};

#endif


