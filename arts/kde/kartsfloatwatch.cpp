    /*

    Copyright (C) 2001 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    */

#include "artskde.h"
#include "kartsfloatwatch.moc"
#include "connect.h"

using namespace Arts;

class KArtsFloatWatchPrivate {
public:
	KFloatWatchProxy proxy;
};

namespace Arts {

class KFloatWatchProxy_impl : public KFloatWatchProxy_skel {
protected:
	KArtsFloatWatch *watch;
public:
	KFloatWatchProxy_impl(KArtsFloatWatch *watch) : watch(watch) { };

	float value() { return 0.0; /* dummy */ }
	void value(float newValue) { watch->change(newValue); }
};

}

KArtsFloatWatch::KArtsFloatWatch(Arts::Object object, const char *stream,
	QObject *parent, const char *name) : QObject(parent, name)
{
	d = new KArtsFloatWatchPrivate();
	d->proxy = KFloatWatchProxy::_from_base(new KFloatWatchProxy_impl(this));
	Arts::connect(object, stream, d->proxy, "value");
}

KArtsFloatWatch::~KArtsFloatWatch()
{
	delete d;
}

void KArtsFloatWatch::change(float newValue)
{
	emit valueChanged(newValue);
}
