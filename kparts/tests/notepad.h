
#ifndef __notepad_h__
#define __notepad_h__

#include <kpart.h>

class QMultiLineEdit;

/**
 * Who said writing a part should be complex ? :-)
 * Here is a very simple kedit-like part
 */
class NotepadPart : public KParts::ReadWritePart
{
  Q_OBJECT
public:
  NotepadPart( QObject *parent, QWidget * parentWidget );
  virtual ~NotepadPart();

  virtual bool save();

protected:
  virtual bool openFile();

protected slots:
  void slotSearchReplace();

protected:
  QMultiLineEdit * m_edit;
  KInstance *m_instance;
};

#endif
