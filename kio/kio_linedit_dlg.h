#ifndef __kio_dlg_h__
#define __kio_dlg_h__

#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <kurlcompletion.h>

/**
 * @sort Asking for a single line of text
 * This class can be used to ask for a new filename or for
 * an URL.
 */
class KLineEditDlg : public QDialog
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
  KLineEditDlg( const QString&_text, const QString&_value, 
		QWidget *parent, bool _file_mode = FALSE );
  ~KLineEditDlg();

  /**
   * @return the value the user entered
   */
  QString text() { return edit->text(); }
    
public slots:
  /**
   * The slot for clearing the edit widget
   */
  void slotClear();

protected:
  /**
   * The line edit widget
   */
  QLineEdit *edit;

  /**
   * Completion helper ..
   */
  KURLCompletion * completion;
};

#endif
