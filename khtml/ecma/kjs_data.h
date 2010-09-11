/*
 * This includes methods for marshalling/demarshalling data for
 * cross-context messaging
 *
 *  Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "kjs_binding.h"
#include "xml/dom2_eventsimpl.h"

namespace KJS {

/*
 Note: unlike other JS->DOM routines, this method is expected to cross security
 domains; hence besides current ExecState (needed to set exceptions) it needs
 the *destination* domain, to make sure everything created uses prototypes
 from it. It can also return 0 if the serialization failed; in which case an
 exception will also be set.
*/
DOM::MessageEventImpl::Data* encapsulateMessageEventData(ExecState* exec, Interpreter* ctx, JSValue* data);

/*
 Warning: Unlike the above, this accessor isn't expected to cross boundaries,
 so it doesn't do any deep copying or the like
 */
JSValue* getMessageEventData(ExecState* exec, DOM::MessageEventImpl::Data* data);

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
