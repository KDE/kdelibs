// $Id$

#ifndef LOADER_CLIENT_H
#define LOADER_CLIENT_H

#include <qpixmap.h>
#include "dom/dom_string.h"

namespace khtml {
    class CachedObject;
    
    /**
     * @internal
     *
     * a client who wants to load stylesheets, images or scripts from the web has to
     * inherit from this class and overload one of the 3 functions
     */
    class CachedObjectClient
    {
    public:
	virtual void setPixmap(const QPixmap &, CachedObject *, bool * = 0) {}
	virtual void setStyleSheet(const DOM::DOMString &/*url*/, const DOM::DOMString &/*sheet*/) {}
	virtual void notifyFinished(CachedObject */*finishedObj*/) {}
    };
};

#endif
