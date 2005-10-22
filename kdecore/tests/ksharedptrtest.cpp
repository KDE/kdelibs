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

