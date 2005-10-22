#include "ksharedptr.h"

#include <QString>

int main()
{
	KSharedPtr<QString> u = new QString( "Hello" );
	Q_ASSERT( *u == "Hello" );

	KSharedPtr<QString> v;
	Q_ASSERT( v == 0 );

	v = u;
	Q_ASSERT( *v == "Hello" );

	KSharedPtr<QString> w = v.copy();
	Q_ASSERT( *u == "Hello" );
	Q_ASSERT( *v == "Hello" );
	Q_ASSERT( *w == "Hello" );

	v->clear();
	Q_ASSERT( u->isEmpty() );
	Q_ASSERT( v->isEmpty() );
	Q_ASSERT( *w == "Hello" );

	u = v = w;
	Q_ASSERT( *u == "Hello" );
	Q_ASSERT( *v == "Hello" );
	Q_ASSERT( *w == "Hello" );

	*u.get() = "Foo";
	Q_ASSERT( *u == "Foo" );
	Q_ASSERT( *v == "Foo" );
	Q_ASSERT( *w == "Foo" );

	w.detach();
	*w = "Bar";
	Q_ASSERT( *u == "Foo" );
	Q_ASSERT( *v == "Foo" );
	Q_ASSERT( *w == "Bar" );
}

