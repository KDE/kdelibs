#ifndef __open_with_h__
#define __open_with_h__

#include <qwidget.h>
#include <qfileinf.h>
#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qlabel.h>

#include <kapp.h>
#include <ktreelist.h>
#include <kurlcompletion.h>

class KApplicationTree;
class KService;

/**
 */
class OpenWithDlg : public QDialog
{
  Q_OBJECT
public:
  /**
   * Create a dialog that asks for a single line of text. _value is the initial
   * value of the line. _text appears as label on top of the entry box.
   * 
   * @param _file_mode if set to TRUE, the editor widget will provide command
   *                   completion ( Ctrl-S and Ctrl-D )
   */
  OpenWithDlg( const char *_text, const char *_value, QWidget *parent, bool _file_mode = FALSE );
  ~OpenWithDlg();
  
  /**
   * @return the value the user entered
   */
  const char * text() { return edit->text(); }
  KService* service() { return m_pService; }
  
public slots:
  /**
   * The slot for clearing the edit widget
   */
  void slotClear();
  void slotBrowse();
  void slotSelected( const char* _name, const char* _exec );
  void slotHighlighted( const char* _name, const char* _exec );
  void slotOK();

protected:
  void resizeEvent(QResizeEvent *);   
  
protected:
  /**
   * The line edit widget
   */
  QLineEdit *edit;
  
  /**
   * Completion helper ..
   */
  KURLCompletion * completion;

  KApplicationTree* m_pTree;
  QLabel *label;

  // QString qExec;
  QString qName;
  bool  haveApp;
  QPushButton *ok;
  QPushButton *clear;
  QPushButton *cancel;
  QPushButton* browse;

  KService *m_pService;
};

class KAppTreeListItem : public KTreeListItem
{	
public:
  KAppTreeListItem( const char *name, QPixmap *pixmap, bool d, bool parse, bool dir, 
		    QString p, QString c );
  bool dummy;
  bool parsed;
  bool directory;
  QString path;
  QString appname;
  QString exec;
};

class KApplicationTree : public QWidget
{
  Q_OBJECT
public:
  KApplicationTree( QWidget *parent );

  bool isKdelnkFile( const char *filename );
  void parseKdelnkFile( QFileInfo *fi, KTreeList *tree, KAppTreeListItem *item );
  short parseKdelnkDir( QDir d, KTreeList *tree, KAppTreeListItem *item = 0 );
  
  KTreeList *tree;
  KAppTreeListItem *it, *it2, *dummy;
  
protected:
  virtual void resizeEvent( QResizeEvent *_ev );
  
public slots:
  void expanded(int index);
  void selected(int index);
  void highlighted(int index);
signals:
  void selected( const char *_name, const char *_exec );
  void highlighted( const char *_name, const char *_exec );
};

#endif
