#include <klistview.h>
#include "main.h"

#ifndef TEST_H
#define TEST_H

// misc tests:
// - writeType
// skel:
// - non "CLASS" main elements
// - namespacing stuff in skel
// - qual / const
// - does dispatcher cope with unnamed args?
// stub:
// - includes to super relationship, a bit much to test, needs multiple files?

// test for k_dcop 
// NOTE - non hashing test, therefore use < 7 function
class DefaultTest : public QObject, virtual public DCOPObject
{
   Q_OBJECT
   K_DCOP
public:
   DefaultTest();
   void nonDcopFunction( QString filename, QString url, QString text, QString address, QString icon );
k_dcop:
   void noArgsTest();
   void argsTest( QString filename, QString url );
   void unNamedArgsTest( QString, QString );

   void voidReturnType( QString filename, QString url, QString text, QString address, QString icon );
   QString nonVoidReturnType( QString filename, QString text, QString address );
   int intReturnType( QString filename, QString text, QString address );

   ASYNC asyncTest( QString filename, QString text, QString address );
};

class NoSuper
{
   K_DCOP
public:
   NoSuper();
k_dcop:
   void function1( QString filename, QString url, QString text, QString address, QString icon );
};

class NonDCOPObject : public MyDCOPObjectBase
{
   K_DCOP
public:
   NonDCOPObject();
k_dcop:
   void function1( QString filename, QString url, QString text, QString address, QString icon );
};

class NoFunctions : public DCOPObject
{
   K_DCOP
public:
   NonDCOPObject();
   void nonDcopFunction( QString filename, QString url, QString text, QString address, QString icon );
};

// test for useHashing
class HashingTest : public QObject, virtual public DCOPObject
{
   Q_OBJECT
   K_DCOP
public:
   HashingTest();
k_dcop:
   void function1( QString );
   void function2( QString, QString );
   void function3( QString, QString, QString );
   void function4( QString, QString, QString, QString );
   void function5( QString, QString, QString, QString, QString );
   void function6( QString, QString, QString, QString, QString, QString );
   void function7( QString, QString, QString, QString, QString, QString, QString );
   void function8( QString, QString, QString, QString, QString, QString, QString, QString );
};

// test for k_dcop_signals
class SignalTest : virtual public DCOPObject
{
   K_DCOP
public:
   SignalTest(QCString objId = "KBookmarkNotifier") : DCOPObject(objId) {}
k_dcop_signals:
   void signal1( QString filename, QString url, QString text, QString address, QString icon );
   void signal2( QString filename, QString text, QString address );
   void signal3( QString filename, QString url );
};

#endif // end
