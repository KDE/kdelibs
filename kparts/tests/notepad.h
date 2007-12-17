
#ifndef notepad_h
#define notepad_h

#include <kparts/part.h>

class KAboutData;
class QTextEdit;

/**
 * Who said writing a part should be complex ? :-)
 * Here is a very simple kedit-like part
 * @internal
 */
class NotepadPart : public KParts::ReadWritePart
{
  Q_OBJECT
public:
  NotepadPart( QWidget* parentWidget,
               QObject* parent,
               const QVariantList& args = QVariantList() );
  virtual ~NotepadPart();

  virtual void setReadWrite( bool rw );

  static KAboutData* createAboutData();

protected:
  virtual bool openFile();
  virtual bool saveFile();

protected Q_SLOTS:
  void slotSearchReplace();

protected:
  QTextEdit * m_edit;
};

#endif
