
#ifndef __notepad_h__
#define __notepad_h__

#include <kparts/part.h>

class QMultiLineEdit;

/**
 * Who said writing a part should be complex ? :-)
 * Here is a very simple kedit-like part
 * @internal
 */
class NotepadPart : public KParts::ReadWritePart
{
  Q_OBJECT
public:
  NotepadPart( QWidget * parent, const char * name = 0L );
  virtual ~NotepadPart();

  virtual void setReadWrite( bool rw );

protected:
  virtual bool openFile();
  virtual bool saveFile();

protected slots:
  void slotSearchReplace();

protected:
  QMultiLineEdit * m_edit;
  KInstance *m_instance;
};

#endif
