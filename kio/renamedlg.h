#ifndef __kio_rename_dlg__
#define __kio_rename_dlg__ "$Id$"

#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qstring.h>

namespace KIO {

enum RenameDlg_Mode { M_OVERWRITE = 1, M_OVERWRITE_ITSELF = 2, M_SKIP = 4, M_SINGLE = 8, M_MULTI = 16, M_RESUME = 32 };

enum RenameDlg_Result { R_RESUME = 6, R_RESUME_ALL = 7, R_OVERWRITE = 4, R_OVERWRITE_ALL = 5, R_SKIP = 2, R_AUTO_SKIP = 3, R_RENAME = 1, R_CANCEL = 0 };

class RenameDlg : public QDialog
{
  Q_OBJECT
public:
  /**
   * Construct a "rename" dialog
   * @param parent parent widget (often 0)
   * @param src the url to the file/dir we're trying to copy, as it's part of the text message
   * @param dest the path to destination file/dir, i.e. the one that already exists
   * @param mode parameters for the dialog (which buttons to show...), @see RenameDlg_Mode
   * @param srcNewer set to true if @p src is newer than @p dest
   * @param modal set to true for a modal dialog
   */
  RenameDlg( QWidget *parent, const QString & src, const QString & dest, RenameDlg_Mode mode,
                bool srcNewer, bool modal = FALSE );
  ~RenameDlg();

  /**
   * @return the new destination path
   * valid only if RENAME was chosen
   */
  QString newName() { return m_pLineEdit->text(); }

protected:
  QPushButton *b0;
  QPushButton *b1;
  QPushButton *b2;
  QPushButton *b3;
  QPushButton *b4;
  QPushButton *b5;
  QPushButton *b6;
  QPushButton *b7;
  QLineEdit* m_pLineEdit;
  QVBoxLayout* m_pLayout;

  QString src;
  QString dest;

  bool modal, srcNewer;

public slots:
  void b0Pressed();
  void b1Pressed();
  void b2Pressed();
  void b3Pressed();
  void b4Pressed();
  void b5Pressed();
  void b6Pressed();
  void b7Pressed();

protected slots:
  void enableRenameButton(const QString &);

signals:
  void result( QWidget* widget, int button, const QString & src, const QString & data );
};

  /**
   * Construct a modal, parent-less "rename" dialog, and return
   * a result code, as well as the new dest. Much easier to use than the
   * class @ref RenameDlg directly.

   * @param src the file/dir we're trying to copy, as it's part of the text message
   * @param dest the destination file/dir, i.e. the one that already exists
   * @param mode parameters for the dialog (which buttons to show...), @see RenameDlg_Mode
   * @param srcNewer set to true if @p src is newer than @p dest
   * @param newDest the new destination path, valid if R_RENAME was returned.
   */
RenameDlg_Result open_RenameDlg( const QString& src, const QString & dest, RenameDlg_Mode mode, bool srcNewer, QString& newDest );

/// ???
unsigned long getOffset( QString dest );

};

#endif

