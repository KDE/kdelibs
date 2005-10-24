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

#include <QtTest/qttest_kde.h>

QTTEST_KDEMAIN( KSharedPtrTest, NoGUI )

void KSharedPtrTest::testWithStrings()
{
	QString s = QLatin1String( "Hello" );
	QString s2 = QLatin1String( "Foo" );
	QString s3 = QLatin1String( "Bar" );

	KSharedPtr<QString> u = new QString( s );
	COMPARE( *u, s );
	VERIFY( u.isUnique() );

	KSharedPtr<QString> v;
	VERIFY( u.isUnique() );
	VERIFY( !v );

	v = u;
	VERIFY( !u.isUnique() );
	COMPARE( *v, s );
	VERIFY( !v.isUnique() );

	KSharedPtr<QString> w = v.copy();
	COMPARE( *u, s );
	VERIFY( !u.isUnique() );
	COMPARE( *v, s );
	VERIFY( !v.isUnique() );
	COMPARE( *w, s );
	VERIFY( w.isUnique() );

	v->clear();
	VERIFY( u->isEmpty() );
	VERIFY( !u.isUnique() );
	VERIFY( v->isEmpty() );
	VERIFY( !v.isUnique() );
	COMPARE( *w, s );
	VERIFY( w.isUnique() );

	u = v = w;
	COMPARE( *u, s );
	VERIFY( !u.isUnique() );
	COMPARE( *v, s );
	VERIFY( !v.isUnique() );
	COMPARE( *w, s );
	VERIFY( !w.isUnique() );

	*u.get() = s2;
	COMPARE( *u, s2 );
	VERIFY( !u.isUnique() );
	COMPARE( *v, s2 );
	VERIFY( !v.isUnique() );
	COMPARE( *w, s2 );
	VERIFY( !w.isUnique() );

	w.detach();
	*w = s3;
	COMPARE( *u, s2 );
	VERIFY( !u.isUnique() );
	COMPARE( *v, s2 );
	VERIFY( !v.isUnique() );
	COMPARE( *w, s3 );
	VERIFY( w.isUnique() );
}

static int dtor_called = 0;
class Base
{
public:
	virtual ~Base() { ++dtor_called; }
};

void KSharedPtrTest::testDeletion()
{
	dtor_called = 0;
	{
		Base* obj = new Base;
		KSharedPtr<Base> ptrBase = obj;
		COMPARE( ptrBase.get(), obj );
		COMPARE( dtor_called, 0 ); // no dtor called yet
	}
	COMPARE( dtor_called, 1 );
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
		KSharedPtr<Base> ptrBase = obj;
		// then we call some method that takes a KSharedPtr<Base> as argument
		// and there we downcast again:
		KSharedPtr<Derived> ptrDerived = ptrBase;
		COMPARE( dtor_called, 0 ); // no dtor called yet
		COMPARE( ptrDerived.get(), obj );

		// now test assignment operator
		ptrDerived = ptrBase;
		COMPARE( dtor_called, 0 ); // no dtor called yet
		COMPARE( ptrDerived.get(), obj );
	}
	COMPARE( dtor_called, 1 );
}

#include "ksharedptrtest.moc"
