#include <klistview.h>
#include "main.h"

#ifndef TEST_H
#define TEST_H

// still todo:
// stub:
//    - includes to super relationship, a bit much to test, needs multiple files?

class KDEUI_EXPORT DefaultTest : public QObject, virtual public DCOPObject
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

   void constTest( QString, QString ) const;
   QStringList writeTypeTest( const QString &, Q3PtrList<int> );

   void voidReturnType( QString filename, QString url, QString text, QString address, QString icon );
   QString nonVoidReturnType( QString filename, QString text, QString address );
   int intReturnType( QString filename, QString text, QString address );
   bool boolReturnType( QString filename, QString text, QString address );

   ASYNC asyncTest( QString filename, QString text, QString address );
};

namespace TestNamespace {
   class NamespaceTest
   {
      K_DCOP
   public:
      NamespaceTest();
   k_dcop:
      void function1( QString filename, QString url, QString text, QString address, QString icon );
   };
}

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

class NonHashingTest : public QObject, virtual public DCOPObject
{
   Q_OBJECT
   K_DCOP
public:
   NonHashingTest();
k_dcop:
   void function1( QString );
};

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

class SignalTest : virtual public DCOPObject
{
   K_DCOP
public:
   SignalTest(DCOPCString objId = "KBookmarkNotifier") : DCOPObject(objId) {}
k_dcop_signals:
   void signal1( QString filename, QString url, QString text, QString address, QString icon );
   void signal2( QString filename, QString text, QString address );
   void signal3( QString filename, QString url );
};

#endif // end
