
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

  virtual bool isModified() const { return true; /* TODO */ }

  // Why is this one public - why does it exist at all ?
  // Shouldn't the part be the only one able to set itself as modified
  // (directly accessing a m_bModified, or implementing isModified its own way)
  virtual void setModified( bool modified = true ) {}

  virtual bool save( const QString &/*url*/ ) { return false; /* TODO !! */ }

  // should we have save() which saves into m_file ? probably

protected:
  virtual bool openFile();

protected:
  virtual QString configFile() const;
  QMultiLineEdit * m_edit;
};

#endif
