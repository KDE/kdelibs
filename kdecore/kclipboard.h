#ifndef __kclipboard_h__
#define __kclipboard_h__

#include <qclipboard.h>
#include <qobject.h>
#include <qapplication.h>
#include <qbuffer.h>
#include <qwidget.h>
#include <qstring.h>
#include <qstrlist.h>

class KClipboard : public QObject, public QBuffer
{
  Q_OBJECT
public:
  KClipboard();
  ~KClipboard();
   
  bool open( int _mode );
  bool open( int _mode, const QString& _format );
  void close();
    
  void clear();

  const QString format();

  /////////////////////////
  // Convenience functions
  /////////////////////////
  void setURLList( QStrList& _urls );
  bool urlList( QStrList& _urls );
  void setText( const QString& _text );
  const QString text();
  QByteArray octetStream();
  void setOctetStream( QByteArray& _arr );

  static KClipboard* self();
  
signals:
  void ownerChanged();
    
protected:
  virtual bool event( QEvent *e );

  void fetchData();
  void setOwner();
  bool isOwner();
  bool isEmpty();
  QWidget* makeOwner();
    
private:
  bool m_bOwner;
  QWidget *m_pOwner;
  bool m_bEmpty;
  QString m_strFormat;
  int m_mimeTypeLen;
  
  static KClipboard* s_pSelf;
};

#endif
