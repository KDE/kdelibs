
#ifndef __notepad_h__
#define __notepad_h__

#include <kpart.h>

class QMultiLineEdit;

/**
 * Wo said writing a part should be complex ? :-)
 * Here is a very simple kedit-like part
 */
class NotepadPart : public KReadWritePart
{
  Q_OBJECT
public:
  NotepadPart( QWidget * parentWidget );
  virtual ~NotepadPart();

  virtual bool save();

protected:
  virtual bool openFile();

protected:
  virtual QString configFile() const;
  QMultiLineEdit * m_edit;
  bool m_bModified;
};

#endif
