/* $XConsortium: accept.c,v 1.24 94/12/20 17:50:13 mor Exp $ */
/******************************************************************************


Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

Author: Ralph Mor, X Consortium
******************************************************************************/

#include <string.h>

#include "KDE-ICE/ICElib.h"
#include "KDE-ICE/ICElibint.h"
#include "KDE-ICE/Xtrans.h"
#include "KDE-ICE/globals.h"

IceConn
IceAcceptConnection (listenObj, statusRet)

IceListenObj 	listenObj;
IceAcceptStatus	*statusRet;

{
    IceConn    		iceConn;
    XtransConnInfo	newconn;
    iceByteOrderMsg 	*pMsg;
    int   		endian, status;

    /*
     * Accept the connection.
     */

    if ((newconn = (XtransConnInfo)_kde_IceTransAccept (listenObj->trans_conn, &status)) == 0)
    {
	if (status == TRANS_ACCEPT_BAD_MALLOC)
	    *statusRet = IceAcceptBadMalloc;
	else
	    *statusRet = IceAcceptFailure;
	return (NULL);
    }


    /*
     * Set close-on-exec so that programs that fork() don't get confused.
     */

    _kde_IceTransSetOption (newconn, TRANS_CLOSEONEXEC, 1);


    /*
     * Create an ICE object for this connection.
     */

    if ((iceConn = (IceConn) malloc (sizeof (struct _IceConn))) == NULL)
    {
	_kde_IceTransClose (newconn);
	*statusRet = IceAcceptBadMalloc;
	return (NULL);
    }

    iceConn->listen_obj = listenObj;

    iceConn->waiting_for_byteorder = True;
    iceConn->connection_status = IceConnectPending;
    iceConn->io_ok = True;
    iceConn->dispatch_level = 0;
    iceConn->context = NULL;
    iceConn->my_ice_version_index = 0;

    iceConn->trans_conn = newconn;
    iceConn->send_sequence = 0;
    iceConn->receive_sequence = 0;

    iceConn->connection_string = (char *) malloc (
	strlen (listenObj->network_id) + 1);

    if (iceConn->connection_string == NULL)
    {
	_kde_IceTransClose (newconn);
	free ((char *) iceConn);
	*statusRet = IceAcceptBadMalloc;
	return (NULL);
    }
    else
	strcpy (iceConn->connection_string, listenObj->network_id);

    iceConn->vendor = NULL;
    iceConn->release = NULL;

    if ((iceConn->inbuf = iceConn->inbufptr =
	(char *) malloc (ICE_INBUFSIZE)) != NULL)
    {
	iceConn->inbufmax = iceConn->inbuf + ICE_INBUFSIZE;
    }
    else
    {
	_kde_IceTransClose (newconn);
	free ((char *) iceConn);
	*statusRet = IceAcceptBadMalloc;
	return (NULL);
    }

    if ((iceConn->outbuf = iceConn->outbufptr =
	(char *) malloc (ICE_OUTBUFSIZE)) != NULL)
    {
	memset(iceConn->outbuf, 0, ICE_OUTBUFSIZE);
	iceConn->outbufmax = iceConn->outbuf + ICE_OUTBUFSIZE;
    }
    else
    {
	_kde_IceTransClose (newconn);
	free (iceConn->inbuf);
	free ((char *) iceConn);
	*statusRet = IceAcceptBadMalloc;
	return (NULL);
    }

    iceConn->scratch = NULL;
    iceConn->scratch_size = 0;

    iceConn->open_ref_count = 1;
    iceConn->proto_ref_count = 0;

    iceConn->skip_want_to_close = False;
    iceConn->want_to_close = False;
    iceConn->free_asap = False;

    iceConn->saved_reply_waits = NULL;
    iceConn->ping_waits = NULL;

    iceConn->process_msg_info = NULL;

    iceConn->connect_to_you = NULL;
    iceConn->protosetup_to_you = NULL;

    iceConn->connect_to_me = NULL;
    iceConn->protosetup_to_me = NULL;


    /*
     * Send our byte order.
     */

    IceGetHeader (iceConn, 0, ICE_ByteOrder,
	SIZEOF (iceByteOrderMsg), iceByteOrderMsg, pMsg);

    endian = 1;
    if (*(char *) &endian)
	pMsg->byteOrder = IceLSBfirst;
    else
	pMsg->byteOrder = IceMSBfirst;

    IceFlush (iceConn);


    if (_IceWatchProcs)
    {
	/*
	 * Notify the watch procedures that an iceConn was opened.
	 */

	_IceConnectionOpened (iceConn);
    }

    *statusRet = IceAcceptSuccess;

    return (iceConn);
}
