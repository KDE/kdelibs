// David Faure <faure@kde.org>
// Let's test KRun
#ifndef _kruntest_h
#define _kruntest_h
#include <krun.h>

class testKRun : public KRun
{
  Q_OBJECT
public:
  
  testKRun( const QString& _url, mode_t _mode = 0, 
            bool _is_local_file = false, bool _auto_delete = true )
    : KRun( _url, _mode, _is_local_file, _auto_delete ) {}
  
  virtual ~testKRun() {}

  virtual void foundMimeType( const char *_type );

};


class Receiver : public QWidget
{
  Q_OBJECT
public:
  Receiver();
  ~Receiver() {}
public slots:
 void slotStart();
 void slotStop();
private:
 QPushButton * start;
 QPushButton * stop;

};

#endif
