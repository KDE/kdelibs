#ifndef __kio_listprogress_dlg_h__
#define __kio_listprogress_dlg_h__

#include <qwidget.h>

#include <ktmainwindow.h>
#include <ktablistbox.h>

class QTimer;

class KIOListBox : public KTabListBox
{
  Q_OBJECT

public:

  KIOListBox (QWidget *parent=0, const char *name=0, 
	      int columns=1, WFlags f=0);
  
  virtual ~KIOListBox();

  virtual void readConfig(void);
  virtual void writeConfig(void);

  /**
   * Field constants
   */
  enum KIOListBoxFields {
    TB_OPERATION = 0,
    TB_LOCAL_FILENAME = 1,
    TB_RESUME = 2,
    TB_COUNT = 3,
    TB_PROGRESS = 4,
    TB_TOTAL = 5,
    TB_SPEED = 6,
    TB_REMAINING_TIME = 7,
    TB_ADDRESS = 8
  };
};


class KIOListProgressDlg : public KTMainWindow, KIO
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

  QMap<int,KIOJob*>* jobmap;

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
