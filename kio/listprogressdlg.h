#ifndef __kio_listprogress_dlg_h__
#define __kio_listprogress_dlg_h__

#include <qwidget.h>

#include <ktmainwindow.h>
#include <ktablistbox.h>

class QTimer;

// Field constants
#define TB_OPERATION       0
#define TB_LOCAL_FILENAME  1
#define TB_RESUME          2
#define TB_COUNT           3
#define TB_PROGRESS        4
#define TB_TOTAL           5
#define TB_SPEED           6
#define TB_REMAINING_TIME  7
#define TB_ADDRESS         8


class KIOListBox : public KTabListBox
{
  Q_OBJECT

public:

  KIOListBox (QWidget *parent=0, const char *name=0, 
	      int columns=1, WFlags f=0);
  
  virtual ~KIOListBox();

  virtual void readConfig(void);
  virtual void writeConfig(void);
  
};


class KIOListProgressDlg : public KTMainWindow
{
  Q_OBJECT

public:

  KIOListProgressDlg();
  ~KIOListProgressDlg();

  // ToolBar field IDs
  enum { TOOL_CANCEL, TOOL_DOCK };

  // StatusBar field IDs
  enum { ID_TOTAL_FILES = 1, ID_TOTAL_SIZE, ID_TOTAL_TIME, ID_TOTAL_SPEED };

protected:

//   void closeEvent( QCloseEvent * );

  QTimer* updateTimer;
  KIOListBox *myTabListBox;

  KToolBar::BarPosition toolbarPos;
  QString properties;

  bool b_dockWindow;

  map<int,KIOJob*>* jobmap;

  void readSettings();
  void writeSettings();

  void updateToolBar();

protected slots:

  void slotUpdate();
  void slotSelected( int id );
  void slotUnselected( int id );

  void cancelCurrent();
  void toggleDocking();

};

#endif
