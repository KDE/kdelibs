/* This is a comment */

#ifndef TEST_H
#define TEST_H

// some comment
#include <dcopobject.h>
#include <dcopref.h>

/* We never use it, but it's a test after all :)
 */
class Test2;

// this has to work too
#include <qstring.h>

typedef QMap<KIO::Job *, KIODownload> KIODownloadMap;
class Test3;

// Those namespaces are opened and closed, so they should be just ignored
namespace Kpgp {
	  class Config;
}
namespace KMail {
	  class IdentityDialog;
}

/**
 * This is a stupid comment that might help 
 * geiseri at some point in the near future.
 * blah di blah
 **/
class KDEUI_EXPORT Test : public MyNamespace::MyParentClass, virtual public DCOPObject,
             public Q3ValueList<QString>
{
    K_DCOP

public:
    struct Bar {
      int i;
    };
    class SomeForwardFooPrivate;
    Test();
    Test() : DCOPObject("name") {}
    ~Test();
    int getPageOfObj( int obj );
    void setCell( KSpreadTable *table,
		  const QPoint& point );

    SomeForwardFooPrivate *d;
    static const int AUDIO_CD = 1;
    static const int AUDIO_CD = 1 << 6;
    enum { Enum1, Enum2 };
    enum NamedEnum { Enum3, Enum4 };
k_dcop:
    virtual QString url();
    virtual QString constTest() const;
    virtual DCOPRef firstView();
    virtual DCOPRef nextView();
    virtual int getNumObjects(unsigned long num);
    virtual DCOPRef getObject( int num );
    virtual int getNumPages();
    virtual DCOPRef getPage( int num );
    /** 
     * a function comment
     * @see blah di blah also
     **/
    int getPageOfObj( int obj );
    void setCell( const int& point = 3 );
    Q3ValueList<DCOPRef> getWindows();

    long int testLongInt( long int n );
    long testLong( long n );

k_dcop_signals:
    void blah_signal();

private:
    Test3 *doc();

    struct KIODownload;
    typedef QMap<KIO::Job *, KIODownload> KIODownloadMap;
};

class Test2 : public DCOPObject
{
    K_DCOP
public:
k_dcop_signals:
    void fooSignal(int arg);
};

namespace TheNameSpace {

  class Test3;

  class Test3 : public DCOPObject
  {
      K_DCOP
  public:
      void yadda();
  };

};

#endif // end of line comment

/* Test for line numbers */


