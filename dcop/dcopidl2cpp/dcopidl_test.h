#ifndef TEST_H
#define TEST_H

// skel:
// - non "CLASS" main elements
// - namespacing stuff in skel
// - no functions test
// - ASYNC test
// - does dispatcher cope with unnamed args?
// - DCOPParent test - superclass?
// - qual test, const?
// writeType test

// test for k_dcop 
// NOTE - non hashing test, therefore use < 7 function
class DefaultTest : public QObject, virtual public DCOPObject
{
   Q_OBJECT
   K_DCOP
public:
   DefaultTest();
k_dcop:
   void noArgsTest();
   void argsTest( QString filename, QString url );
   void unNamedArgsTest( QString, QString );

   void function1( QString filename, QString url, QString text, QString address, QString icon );
   void function2( QString filename, QString text, QString address );
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
