#ifndef kfm_h
#define kfm_h

#include <qobject.h>
#include <qdialog.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qstring.h>

#include "kfmclient_ipc.h"

class KDirEntry
{
public:
  QString name;
  QString access;
  QString date;
  QString owner;
  QString group;
  int size;
};

class KFM : public QObject
{
    Q_OBJECT
public:
    /**
     * This will not restart KFM if it is not running.
     * You have to call @ref #allowKFMRestart before
     * calling any other function. This will allow to
     * restart KFM on demand.
     */
    KFM();
    ~KFM();
    
    bool isOK() { return ok; }

    void refreshDesktop();
    void openURL();
    void openURL( const char *_url );
    void refreshDirectory( const char *_url );
    void openProperties( const char *_url );
    void exec( const char *_url, const char *_binding );
    void copy( const char *_src, const char *_dest );
    void move( const char *_src, const char *_dest );    

    void copyClient( const char *_src, const char *_dest );
    void moveClient( const char *_src, const char *_dest );    
    void list( const char *_url );

    // For KWM only
    void selectRootIcons( int _x, int _y, int _w, int _h, bool _add );
  
    /**
     * Allows this class to restart KFM if it is not running.
     */
    void allowKFMRestart( bool );
    /**
     * Tells wether a KFM is running.
     */
    bool isKFMRunning();
    
signals:    
    void finished();
    void error( int _kerror, const char *_text );
    void dirEntry( KDirEntry& _entry );

public slots:
    void slotFinished();
    void slotError( int _kerror, const char *_text );
    void slotDirEntry(const char* _name, const char* _access, const char* _owner,
		      const char* _group, const char* _date, int _size);
    
protected:
    void init();
    bool test();
    
    char flag;
    bool ok;
    bool allowRestart;
    
    KfmIpc *ipc;

    KDirEntry entry;
};

/// Asking for a location
/**
  This class can be used to ask for a new filename or for
  an URL.
  */
class DlgLocation : public QDialog
{
    Q_OBJECT
public:
    /// Constructor
    /**
      Create a dialog that asks for a single line of text. _value is the initial
      value of the line. _text appears as label on top of the entry box.
      */
    DlgLocation( const char *_text, const char *_value, QWidget *parent = 0L );

    /// Return the value the user entered
    const char * getText() { return edit->text(); }
    
protected:
    /// The line edit widget
    QLineEdit *edit;
};

#endif
