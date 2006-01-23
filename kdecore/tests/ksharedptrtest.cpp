/* Copyright (c) 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "ksharedptrtest.h"
#include "ksharedptr.h"

#include <QString>

#include <qtest_kde.h>

QTEST_KDEMAIN( KSharedPtrTest, NoGUI )

class SharedString : public KShared
{
public:
    SharedString( const QString& data ) : mStr( data ) {}
    bool operator == (const SharedString &o) const { return mStr == o.mStr; }
    QString mStr;
};

void KSharedPtrTest::testWithStrings()
{
	SharedString s = QString::fromLatin1( "Hello" );
	SharedString s2 = QString::fromLatin1( "Foo" );
	SharedString s3 = QString::fromLatin1( "Bar" );

	KSharedPtr<SharedString>u(new SharedString( s ));
	QCOMPARE( *u, s );
	QVERIFY( u.isUnique() );

	KSharedPtr<SharedString> v;
	QVERIFY( u.isUnique() );
	QVERIFY( !v );

	v = u;
	QVERIFY( !u.isUnique() );
	QCOMPARE( *v, s );
	QVERIFY( !v.isUnique() );

	KSharedPtr<SharedString> w = v;
	QVERIFY( !u.isUnique() );
	QCOMPARE( u.count(), 3 );
	QVERIFY( !v.isUnique() );
	QCOMPARE( v.count(), 3 );
	QVERIFY( !w.isUnique() );
	QCOMPARE( v.count(), 3 );

	w.detach();
	QVERIFY( !u.isUnique() );
	QCOMPARE( *u, s );
	QVERIFY( !u.isUnique() );
	QCOMPARE( *v, s );
	QVERIFY( !v.isUnique() );
	QCOMPARE( *w, s );
	QVERIFY( w.isUnique() );

//	v->clear();
	v = 0;
	QVERIFY( u );
	QVERIFY( u.isUnique() );
	QVERIFY( !v );
	QVERIFY( !v.isUnique() );
	QCOMPARE( *w, s );
	QVERIFY( w.isUnique() );

	u = v = w;
	QCOMPARE( *u, s );
	QVERIFY( !u.isUnique() );
	QCOMPARE( *v, s );
	QVERIFY( !v.isUnique() );
	QCOMPARE( *w, s );
	QVERIFY( !w.isUnique() );

	u->mStr = s2.mStr;
	QCOMPARE( *u, s2 );
	QVERIFY( !u.isUnique() );
	QCOMPARE( *v, s2 );
	QVERIFY( !v.isUnique() );
//	QCOMPARE( *w, s2 );
//	QVERIFY( !w.isUnique() );

#if 0
	w.detach();
	*w = s3;
	QCOMPARE( *u, s2 );
	QVERIFY( !u.isUnique() );
	QCOMPARE( *v, s2 );
	QVERIFY( !v.isUnique() );
	QCOMPARE( *w, s3 );
	QVERIFY( w.isUnique() );
#endif
}

static int dtor_called = 0;
class Base : public KShared
{
public:
	virtual ~Base() { ++dtor_called; }
};

void KSharedPtrTest::testDeletion()
{
	dtor_called = 0;
	{
		Base* obj = new Base;
		KSharedPtr<Base> ptrBase(obj);
		QCOMPARE( ptrBase.data(), obj );
		QCOMPARE( dtor_called, 0 ); // no dtor called yet
	}
	QCOMPARE( dtor_called, 1 );
}

class Derived : public Base
{
public:
	Derived() { /*qDebug( "Derived created %p", (void*)this );*/ }
	virtual ~Derived() { /*qDebug( "Derived deleted %p", (void*)this );*/ }
};

void KSharedPtrTest::testDifferentTypes()
{
	dtor_called = 0;
	{
		Derived* obj = new Derived;
		KSharedPtr<Base> ptrBase(obj);
		// then we call some method that takes a KSharedPtr<Base> as argument
		// and there we downcast again:
		KSharedPtr<Derived> ptrDerived = KSharedPtr<Derived>::staticCast( ptrBase );
		QCOMPARE( dtor_called, 0 ); // no dtor called yet
		QCOMPARE( ptrDerived.data(), obj );

		// now test assignment operator
		ptrDerived = KSharedPtr<Derived>::dynamicCast( ptrBase );
		QCOMPARE( dtor_called, 0 ); // no dtor called yet
		QCOMPARE( ptrDerived.data(), obj );
	}
	QCOMPARE( dtor_called, 1 );
}

#include "ksharedptrtest.moc"
