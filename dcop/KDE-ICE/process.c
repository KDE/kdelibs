/* $Xorg: process.c,v 1.3 2000/08/17 19:44:16 cpqbld Exp $ */
/******************************************************************************


Copyright 1993, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Author: Ralph Mor, X Consortium
******************************************************************************/
/* $XFree86: xc/lib/ICE/process.c,v 3.5 2001/01/17 19:41:29 dawes Exp $ */

#include "KDE-ICE/ICElib.h"
#include "KDE-ICE/ICElibint.h"
#include "KDE-ICE/globals.h"

#include <stdio.h> /* sprintf */

#ifdef MINIX
#include <KDE-ICE/Xtrans.h>
#endif

/*
 * Check for bad length
 */

#define CHECK_SIZE_MATCH(_iceConn, _opcode, _expected_len, _actual_len, _severity) \
    if ((((_actual_len) - SIZEOF (iceMsg)) >> 3) != _expected_len) \
    { \
       _IceErrorBadLength (_iceConn, 0, _opcode, _severity); \
       return (0); \
    }

#define CHECK_AT_LEAST_SIZE(_iceConn, _opcode, _expected_len, _actual_len, _severity) \
    if ((((_actual_len) - SIZEOF (iceMsg)) >> 3) > _expected_len) \
    { \
       _IceErrorBadLength (_iceConn, 0, _opcode, _severity); \
       return (0); \
    }

#define CHECK_COMPLETE_SIZE(_iceConn, _opcode, _expected_len, _actual_len, _pStart, _severity) \
    if (((PADDED_BYTES64((_actual_len)) - SIZEOF (iceMsg)) >> 3) \
        != _expected_len) \
    { \
       _IceErrorBadLength (_iceConn, 0, _opcode, _severity); \
       IceDisposeCompleteMessage (iceConn, _pStart); \
       return (0); \
    }

#define BAIL_STRING(_iceConn, _opcode, _pStart) {\
    _IceErrorBadLength (_iceConn, 0, _opcode, IceFatalToConnection);\
    IceDisposeCompleteMessage (_iceConn, _pStart);\
    return (0);\
}

/*
 * IceProcessMessages:
 *
 * If replyWait == NULL, the client is not waiting for a reply.
 *
 * If replyWait != NULL, the client is waiting for a reply...
 *
 *    - replyWait->sequence_of_request is the sequence number of the
 *      message for which the client is waiting a reply.  This is needed
 *	to determine if an error matches a replyWait.
 *
 *    - replyWait->major_opcode_of_request is the major opcode of the
 *      message for which we are waiting a reply.
 *
 *    - replyWait->minor_opcode_of_request is the minor opcode of the
 *      message for which we are waiting a reply.
 *
 *    - replyWait->reply is a pointer to the reply message which will be
 *	filled in when the reply is ready (the protocol library should
 *      cast this IcePointer to the appropriate reply type).  In most cases,
 *      the reply will have some fixed-size part, and the sender function
 *      will have provided a pointer to a structure (e.g.) to hold this
 *      fixed-size data.  If there is variable-length data, it would be
 *      expected that the reply function will have to allocate additional
 *      memory and store pointer(s) to that memory in the fixed-size
 *      structure.  If the entire data is variable length (e.g., a single
 *      variable-length string), then the sender function would probably
 *      just pass a pointer to fixed-size space to hold a pointer, and the
 *      reply function would allocate the storage and store the pointer.
 *	It is the responsibility of the client receiving the reply to
 *	free up any memory allocated on it's behalf.
 *
 * We might be waiting for several different replies (a function can wait
 * for a reply, and while calling IceProcessMessages, a callback can be
 * invoked which will wait for another reply).  We take advantage of the
 * fact that for a given protocol, we are guaranteed that messages are
 * processed in the order we sent them.  So, everytime we have a new
 * replyWait, we add it to the END of the 'saved_reply_waits' list.  When
 * we read a message and want to see if it matches a replyWait, we use the
 * FIRST replyWait in the list with the major opcode of the message.  If the
 * reply is ready, we remove that replyWait from the list.
 *
 * If the reply/error is ready for the replyWait passed in to
 * IceProcessMessages, *replyReadyRet is set to True.
 *
 * The return value of IceProcessMessages is one of the following:
 *
 * IceProcessMessagesSuccess - the message was processed successfully.
 * IceProcessMessagesIOError - an IO error occured.  The caller should
 *			       invoked IceCloseConnection.
 * IceProcessMessagesConnectionClosed - the connection was closed as a
 *					result of shutdown negotiation.
 */

IceProcessMessagesStatus
IceProcessMessages (iceConn, replyWait, replyReadyRet)

IceConn		 iceConn;
IceReplyWaitInfo *replyWait;
Bool		 *replyReadyRet;

{
    iceMsg		*header;
    Bool		replyReady = False;
    IceReplyWaitInfo	*useThisReplyWait = NULL;
    IceProcessMessagesStatus retStatus = IceProcessMessagesSuccess;

    if (replyWait)
	*replyReadyRet = False;

    /*
     * Each time IceProcessMessages is entered, we increment the dispatch
     * level.  Each time we leave it, we decrement the dispatch level.
     */

    iceConn->dispatch_level++;


    /*
     * Read the ICE message header.
     */

    if (!_IceRead (iceConn, (unsigned long) SIZEOF (iceMsg), iceConn->inbuf))
    {
	/*
	 * If we previously sent a WantToClose and now we detected
	 * that the connection was closed, _IceRead returns status 0.
	 * Since the connection was closed, we just want to return here.
	 */

	return (IceProcessMessagesConnectionClosed);
    }

    if (!iceConn->io_ok)
    {
	/*
	 * An unexpected IO error occured.  The caller of IceProcessMessages
	 * should call IceCloseConnection which will cause the watch procedures
	 * to be invoked and the ICE connection to be freed.
	 */

	iceConn->dispatch_level--;
	iceConn->connection_status = IceConnectIOError;
	return (IceProcessMessagesIOError);
    }

    header = (iceMsg *) iceConn->inbuf;
    iceConn->inbufptr = iceConn->inbuf + SIZEOF (iceMsg);

    iceConn->receive_sequence++;

    if (iceConn->waiting_for_byteorder)
    {
	if (header->majorOpcode == 0 &&
	    header->minorOpcode == ICE_ByteOrder)
	{
	    char byteOrder = ((iceByteOrderMsg *) header)->byteOrder;
	    int endian = 1;

	    CHECK_SIZE_MATCH (iceConn, ICE_ByteOrder,
	        header->length, SIZEOF (iceByteOrderMsg),
		IceFatalToConnection);

	    if (byteOrder != IceMSBfirst && byteOrder != IceLSBfirst)
	    {
		_IceErrorBadValue (iceConn, 0,
	            ICE_ByteOrder, 2, 1, &byteOrder);

		iceConn->connection_status = IceConnectRejected;
	    }
	    else
	    {
		iceConn->swap =
	            (((*(char *) &endian) && byteOrder == IceMSBfirst) ||
	             (!(*(char *) &endian) && byteOrder == IceLSBfirst));

		iceConn->waiting_for_byteorder = 0;
	    }
	}
	else
	{
	    if (header->majorOpcode != 0)
	    {
		_IceErrorBadMajor (iceConn, header->majorOpcode,
		    header->minorOpcode, IceFatalToConnection);
	    }
	    else
	    {
		_IceErrorBadState (iceConn, 0,
		    header->minorOpcode, IceFatalToConnection);
	    }

	    iceConn->connection_status = IceConnectRejected;
	}

	iceConn->dispatch_level--;
	if (!iceConn->io_ok)
	{
	    iceConn->connection_status = IceConnectIOError;
	    retStatus = IceProcessMessagesIOError;
	}

	return (retStatus);
    }

    if (iceConn->swap)
    {
	/* swap the length field */

	header->length = lswapl (header->length);
    }

    if (replyWait)
    {
	/*
	 * Add to the list of replyWaits (only if it doesn't exist
	 * in the list already.
	 */

	_IceAddReplyWait (iceConn, replyWait);


	/*
	 * Note that there are two different replyWaits.  The first is
	 * the one passed into IceProcessMessages, and is the replyWait
	 * for the message the client is blocking on.  The second is
	 * the replyWait for the message currently being processed
	 * by IceProcessMessages.  We call it "useThisReplyWait".
	 */

	useThisReplyWait = _IceSearchReplyWaits (iceConn, header->majorOpcode);
    }

    if (header->majorOpcode == 0)
    {
	/*
	 * ICE protocol
	 */

	Bool connectionClosed;

	_IceProcessCoreMsgProc processIce =
	    _IceVersions[iceConn->my_ice_version_index].process_core_msg_proc;

	(*processIce) (iceConn, header->minorOpcode,
	    header->length, iceConn->swap,
	    useThisReplyWait, &replyReady, &connectionClosed);

	if (connectionClosed)
	{
	    /*
	     * As a result of shutdown negotiation, the connection was closed.
	     */

	    return (IceProcessMessagesConnectionClosed);
	}
    }
    else
    {
	/*
	 * Sub protocol
	 */

	if ((int) header->majorOpcode < iceConn->his_min_opcode ||
	    (int) header->majorOpcode > iceConn->his_max_opcode ||
	    !(iceConn->process_msg_info[header->majorOpcode -
	    iceConn->his_min_opcode].in_use))
	{
	    /*
	     * The protocol of the message we just read is not supported.
	     */

	    _IceErrorBadMajor (iceConn, header->majorOpcode,
		header->minorOpcode, IceCanContinue);

	    _IceReadSkip (iceConn, header->length << 3);
	}
	else
	{
	    _IceProcessMsgInfo *processMsgInfo = &iceConn->process_msg_info[
		header->majorOpcode - iceConn->his_min_opcode];

	    if (processMsgInfo->accept_flag)
	    {
		IcePaProcessMsgProc processProc =
		    processMsgInfo->process_msg_proc.accept_client;

		(*processProc) (iceConn, processMsgInfo->client_data,
		    header->minorOpcode, header->length, iceConn->swap);
	    }
	    else
	    {
		IcePoProcessMsgProc processProc =
		    processMsgInfo->process_msg_proc.orig_client;

		(*processProc) (iceConn,
		    processMsgInfo->client_data, header->minorOpcode,
		    header->length, iceConn->swap,
		    useThisReplyWait, &replyReady);
	    }
	}
    }

    if (replyReady)
    {
	_IceSetReplyReady (iceConn, useThisReplyWait);
    }


    /*
     * Now we check if the reply is ready for the replyWait passed
     * into IceProcessMessages.  The replyWait is removed from the
     * replyWait list if it is ready.
     */

    if (replyWait)
	*replyReadyRet = _IceCheckReplyReady (iceConn, replyWait);


    /*
     * Decrement the dispatch level.  If we reach level 0, and the
     * free_asap bit is set, free the connection now.  Also check for
     * possible bad IO status.
     */

    iceConn->dispatch_level--;

    if (iceConn->dispatch_level == 0 && iceConn->free_asap)
    {
	_IceFreeConnection (iceConn);
	retStatus = IceProcessMessagesConnectionClosed;
    }
    else if (!iceConn->io_ok)
    {
	iceConn->connection_status = IceConnectIOError;
	retStatus = IceProcessMessagesIOError;
    }

    return (retStatus);
}



static void
AuthRequired (iceConn, authIndex, authDataLen, authData)

IceConn		iceConn;
int  		authIndex;
int  		authDataLen;
IcePointer	authData;

{
    iceAuthRequiredMsg *pMsg;

    IceGetHeader (iceConn, 0, ICE_AuthRequired,
	SIZEOF (iceAuthRequiredMsg), iceAuthRequiredMsg, pMsg);

    pMsg->authIndex = authIndex;
    pMsg->authDataLength = authDataLen;
    pMsg->length += WORD64COUNT (authDataLen);

    IceWriteData (iceConn, authDataLen, (char *) authData);

    if (PAD64 (authDataLen))
	IceWritePad (iceConn, PAD64 (authDataLen));

    IceFlush (iceConn);
}



static void
AuthReply (iceConn, authDataLen, authData)

IceConn		iceConn;
int 		authDataLen;
IcePointer	authData;

{
    iceAuthReplyMsg *pMsg;

    IceGetHeader (iceConn, 0, ICE_AuthReply,
	SIZEOF (iceAuthReplyMsg), iceAuthReplyMsg, pMsg);

    pMsg->authDataLength = authDataLen;
    pMsg->length +=  WORD64COUNT (authDataLen);

    IceWriteData (iceConn, authDataLen, (char *) authData);

    if (PAD64 (authDataLen))
	IceWritePad (iceConn, PAD64 (authDataLen));

    IceFlush (iceConn);
}



static void
AuthNextPhase (iceConn, authDataLen, authData)

IceConn		iceConn;
int  		authDataLen;
IcePointer	authData;

{
    iceAuthNextPhaseMsg *pMsg;

    IceGetHeader (iceConn, 0, ICE_AuthNextPhase,
	SIZEOF (iceAuthNextPhaseMsg), iceAuthNextPhaseMsg, pMsg);

    pMsg->authDataLength = authDataLen;
    pMsg->length += WORD64COUNT (authDataLen);

    IceWriteData (iceConn, authDataLen, (char *) authData);

    if (PAD64 (authDataLen))
	IceWritePad (iceConn, PAD64 (authDataLen));

    IceFlush (iceConn);
}



static void
AcceptConnection (iceConn, versionIndex)

IceConn iceConn;
int 	versionIndex;

{
    iceConnectionReplyMsg	*pMsg;
    char			*pData;
    int				extra;

    extra = STRING_BYTES (IceVendorString) + STRING_BYTES (IceReleaseString);

    IceGetHeaderExtra (iceConn, 0, ICE_ConnectionReply,
	SIZEOF (iceConnectionReplyMsg), WORD64COUNT (extra),
	iceConnectionReplyMsg, pMsg, pData);

    pMsg->versionIndex = versionIndex;

    STORE_STRING (pData, IceVendorString);
    STORE_STRING (pData, IceReleaseString);

    IceFlush (iceConn);

    iceConn->connection_status = IceConnectAccepted;
}



static void
AcceptProtocol (iceConn, hisOpcode, myOpcode, versionIndex, vendor, release)

IceConn iceConn;
int  	hisOpcode;
int  	myOpcode;
int  	versionIndex;
char 	*vendor;
char 	*release;

{
    iceProtocolReplyMsg	*pMsg;
    char		*pData;
    int			extra;

    extra = STRING_BYTES (vendor) + STRING_BYTES (release);

    IceGetHeaderExtra (iceConn, 0, ICE_ProtocolReply,
	SIZEOF (iceProtocolReplyMsg), WORD64COUNT (extra),
	iceProtocolReplyMsg, pMsg, pData);

    pMsg->protocolOpcode = myOpcode;
    pMsg->versionIndex = versionIndex;

    STORE_STRING (pData, vendor);
    STORE_STRING (pData, release);

    IceFlush (iceConn);


    /*
     * We may be using a different major opcode for this protocol
     * than the other client.  Whenever we get a message, we must
     * map to our own major opcode.
     */

    _IceAddOpcodeMapping (iceConn, hisOpcode, myOpcode);
}



static void
PingReply (iceConn)

IceConn iceConn;

{
    IceSimpleMessage (iceConn, 0, ICE_PingReply);
    IceFlush (iceConn);
}



static Bool
ProcessError (iceConn, length, swap, replyWait)

IceConn		 iceConn;
unsigned long	 length;
Bool		 swap;
IceReplyWaitInfo *replyWait;

{
    int		invokeHandler;
    Bool	errorReturned = False;
    iceErrorMsg *message;
    char 	*pData, *pStart;
    char	severity;

    CHECK_AT_LEAST_SIZE (iceConn, ICE_Error,
	length, SIZEOF (iceErrorMsg),
	(iceConn->connect_to_you || iceConn->connect_to_me) ?
	IceFatalToConnection : IceFatalToProtocol);

    IceReadCompleteMessage (iceConn, SIZEOF (iceErrorMsg),
	iceErrorMsg, message, pStart);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    severity = message->severity;

    if (severity != IceCanContinue && severity != IceFatalToProtocol &&
	severity != IceFatalToConnection)
    {
	_IceErrorBadValue (iceConn, 0,
	    ICE_Error, 9, 1, &severity);
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    pData = pStart;

    if (swap)
    {
	message->errorClass = lswaps (message->errorClass);
	message->offendingSequenceNum = lswapl (message->offendingSequenceNum);
    }

    if (!replyWait ||
	message->offendingSequenceNum != replyWait->sequence_of_request)
    {
	invokeHandler = 1;
    }
    else
    {
	if (iceConn->connect_to_you &&
	    ((!iceConn->connect_to_you->auth_active &&
            message->offendingMinorOpcode == ICE_ConnectionSetup) ||
            (iceConn->connect_to_you->auth_active &&
	    message->offendingMinorOpcode == ICE_AuthReply)))
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);
	    char *errorStr = NULL;
	    char *tempstr;
	    char *prefix, *temp;

	    invokeHandler = 0;
	    errorReturned = True;

	    switch (message->errorClass)
	    {
	    case IceNoVersion:

		tempstr =
		    "None of the ICE versions specified are supported";
		errorStr = (char *) malloc (strlen (tempstr) + 1);
		strcpy (errorStr, tempstr);
		break;

	    case IceNoAuth:

		tempstr =
		    "None of the authentication protocols specified are supported";
		errorStr = (char *) malloc (strlen (tempstr) + 1);
		strcpy (errorStr, tempstr);
		break;

	    case IceSetupFailed:

		prefix = "Connection Setup Failed, reason : ";

		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceAuthRejected:

		prefix = "Authentication Rejected, reason : ";
		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceAuthFailed:

		prefix = "Authentication Failed, reason : ";
		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    default:
		invokeHandler = 1;
	    }

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = errorStr;
	}
	else if (iceConn->protosetup_to_you &&
	    ((!iceConn->protosetup_to_you->auth_active &&
            message->offendingMinorOpcode == ICE_ProtocolSetup) ||
            (iceConn->protosetup_to_you->auth_active &&
	    message->offendingMinorOpcode == ICE_AuthReply)))
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);
	    char *errorStr = "";
	    char *prefix, *temp;

	    invokeHandler = 0;
	    errorReturned = True;

	    switch (message->errorClass)
	    {
	    case IceNoVersion:

	        temp =
		    "None of the protocol versions specified are supported";
		errorStr = (char *) malloc (strlen (temp) + 1);
		strcpy (errorStr, temp);
		break;

	    case IceNoAuth:

		temp =
		    "None of the authentication protocols specified are supported";
		errorStr = (char *) malloc (strlen (temp) + 1);
		strcpy (errorStr, temp);
		break;

	    case IceSetupFailed:

		prefix = "Protocol Setup Failed, reason : ";

		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceAuthRejected:

		prefix = "Authentication Rejected, reason : ";
		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceAuthFailed:

		prefix = "Authentication Failed, reason : ";
		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceProtocolDuplicate:

		prefix = "Protocol was already registered : ";
		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    case IceMajorOpcodeDuplicate:

		prefix = "The major opcode was already used : ";
		errorStr = (char *) malloc (strlen (prefix) + 16);
		sprintf (errorStr, "%s%d", prefix, (int) *pData);
		break;

	    case IceUnknownProtocol:

		prefix = "Unknown Protocol : ";
		EXTRACT_STRING (pData, swap, temp);
		errorStr = (char *) malloc (
		    strlen (prefix) + strlen (temp) + 1);
		sprintf (errorStr, "%s%s", prefix, temp);
		free (temp);
		break;

	    default:
		invokeHandler = 1;
	    }

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = errorStr;
	}

	if (errorReturned == True)
	{
	    /*
	     * If we tried to authenticate, tell the authentication
	     * procedure to clean up.
	     */

	    IcePoAuthProc authProc;

	    if (iceConn->connect_to_you &&
		iceConn->connect_to_you->auth_active)
	    {
		authProc = _IcePoAuthProcs[(int)
		    (iceConn->connect_to_you->my_auth_index)];

		(*authProc) (iceConn, &iceConn->connect_to_you->my_auth_state,
		    True /* clean up */, False /* swap */,
		    0, NULL, NULL, NULL, NULL);
	    }
	    else if (iceConn->protosetup_to_you &&
		iceConn->protosetup_to_you->auth_active)
	    {
		_IcePoProtocol *protocol = _IceProtocols[
		    iceConn->protosetup_to_you->my_opcode - 1].orig_client;

		authProc = protocol->auth_procs[(int)(iceConn->
		    protosetup_to_you->my_auth_index)];

		(*authProc) (iceConn,
		    &iceConn->protosetup_to_you->my_auth_state,
		    True /* clean up */, False /* swap */,
		    0, NULL, NULL, NULL, NULL);
	    }
	}
    }

    if (invokeHandler)
    {
	(*_IceErrorHandler) (iceConn, swap, message->offendingMinorOpcode,
	    message->offendingSequenceNum, message->errorClass,
	    message->severity, (IcePointer) pData);
    }

    IceDisposeCompleteMessage (iceConn, pStart);

    return (errorReturned);
}



static int
ProcessConnectionSetup (iceConn, length, swap)

IceConn		iceConn;
unsigned long	length;
Bool		swap;

{
    iceConnectionSetupMsg *message;
    int  myVersionCount, hisVersionCount;
    int	 myVersionIndex, hisVersionIndex;
    int  hisMajorVersion, hisMinorVersion;
    int	 myAuthCount, hisAuthCount;
    int	 found, i, j;
    char *myAuthName, **hisAuthNames;
    char *pData, *pStart, *pEnd;
    char *vendor = NULL;
    char *release = NULL;
    int myAuthIndex = 0;
    int hisAuthIndex = 0;
    int accept_setup_now = 0;
    char mustAuthenticate;
    int	authUsableCount;
    int	authUsableFlags[MAX_ICE_AUTH_NAMES];
    int	authIndices[MAX_ICE_AUTH_NAMES];

    CHECK_AT_LEAST_SIZE (iceConn, ICE_ConnectionSetup,
	length, SIZEOF (iceConnectionSetupMsg), IceFatalToConnection);

    IceReadCompleteMessage (iceConn, SIZEOF (iceConnectionSetupMsg),
	iceConnectionSetupMsg, message, pStart);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    pData = pStart;
    pEnd = pStart + (length << 3);
    
    SKIP_STRING (pData, swap, pEnd, 
		 BAIL_STRING(iceConn, ICE_ConnectionSetup,
			     pStart));			       /* vendor */
    SKIP_STRING (pData, swap, pEnd, 
		 BAIL_STRING(iceConn, ICE_ConnectionSetup,
			    pStart));	        	       /* release */
    SKIP_LISTOF_STRING (pData, swap, (int) message->authCount, pEnd, 
			BAIL_STRING(iceConn, ICE_ConnectionSetup,
				   pStart));		       /* auth names */
    
    pData += (message->versionCount * 4);		       /* versions */

    CHECK_COMPLETE_SIZE (iceConn, ICE_ConnectionSetup,
	length, pData - pStart + SIZEOF (iceConnectionSetupMsg),
	pStart, IceFatalToConnection);

    mustAuthenticate = message->mustAuthenticate;
    if (mustAuthenticate != 0 && mustAuthenticate != 1)
    {
	_IceErrorBadValue (iceConn, 0,
	    ICE_ConnectionSetup, 8, 1, &mustAuthenticate);
	iceConn->connection_status = IceConnectRejected;
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    pData = pStart;

    EXTRACT_STRING (pData, swap, vendor);
    EXTRACT_STRING (pData, swap, release);

    if ((hisAuthCount = message->authCount) > 0)
    {
	hisAuthNames = (char **) malloc (hisAuthCount * sizeof (char *));
	EXTRACT_LISTOF_STRING (pData, swap, hisAuthCount, hisAuthNames);
    }

    hisVersionCount = message->versionCount;
    myVersionCount = _IceVersionCount;

    hisVersionIndex = myVersionIndex = found = 0;

    for (i = 0; i < hisVersionCount && !found; i++)
    {
	EXTRACT_CARD16 (pData, swap, hisMajorVersion);
	EXTRACT_CARD16 (pData, swap, hisMinorVersion);

	for (j = 0; j < myVersionCount && !found; j++)
	{
	    if (_IceVersions[j].major_version == hisMajorVersion &&
		_IceVersions[j].minor_version == hisMinorVersion)
	    {
		hisVersionIndex = i;
		myVersionIndex = j;
		found = 1;
	    }
	}
    }

    if (!found)
    {
	_IceErrorNoVersion (iceConn, ICE_ConnectionSetup);
	iceConn->connection_status = IceConnectRejected;

	free (vendor);
	free (release);

	if (hisAuthCount > 0)
	{
	    for (i = 0; i < hisAuthCount; i++)
		free (hisAuthNames[i]);
	
	    free ((char *) hisAuthNames);
	}

	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    _IceGetPaValidAuthIndices ("ICE", iceConn->connection_string,
	_IceAuthCount, _IceAuthNames, &authUsableCount, authIndices);

    for (i = 0; i < _IceAuthCount; i++)
    {
	authUsableFlags[i] = 0;
	for (j = 0; j < authUsableCount && !authUsableFlags[i]; j++)
	    authUsableFlags[i] = (authIndices[j] == i);
    }

    myAuthCount = _IceAuthCount;

    for (i = found = 0; i < myAuthCount && !found; i++)
    {
	if (authUsableFlags[i])
	{
	    myAuthName = _IceAuthNames[i];

	    for (j = 0; j < hisAuthCount && !found; j++)
		if (strcmp (myAuthName, hisAuthNames[j]) == 0)
		{
		    myAuthIndex = i;
		    hisAuthIndex = j;
		    found = 1;
		}
	}
    }

    if (!found)
    {
	/*
	 * None of the authentication methods specified by the
	 * other client is supported.  If the other client requires
	 * authentication, we must reject the connection now.
	 * Otherwise, we can invoke the host-based authentication callback
	 * to see if we can accept this connection.
	 */

	if (mustAuthenticate || !iceConn->listen_obj->host_based_auth_proc)
	{
	    _IceErrorNoAuthentication (iceConn, ICE_ConnectionSetup);
	    iceConn->connection_status = IceConnectRejected;
	}
	else
	{
	    char *hostname = _IceGetPeerName (iceConn);

	    if ((*iceConn->listen_obj->host_based_auth_proc) (hostname))
	    {
		accept_setup_now = 1;
	    }
	    else 
	    {
		_IceErrorAuthenticationRejected (iceConn,
	            ICE_ConnectionSetup, "None of the authentication protocols specified are supported and host-based authentication failed");

		iceConn->connection_status = IceConnectRejected;
	    }

	    if (hostname)
		free (hostname);
	}

	if (iceConn->connection_status == IceConnectRejected)
	{
	    free (vendor);
	    free (release);
	}
    }
    else
    {
	IcePaAuthStatus	status;
	int		authDataLen;
	IcePointer	authData = NULL;
	IcePointer	authState;
	char		*errorString = NULL;
	IcePaAuthProc	authProc = _IcePaAuthProcs[myAuthIndex];

	authState = NULL;

	status = (*authProc) (iceConn, &authState,
	    swap, 0, NULL, &authDataLen, &authData, &errorString);

	if (status == IcePaAuthContinue)
	{
	    _IceConnectToMeInfo *setupInfo;

	    AuthRequired (iceConn, hisAuthIndex, authDataLen, authData);

	    iceConn->connect_to_me = setupInfo = (_IceConnectToMeInfo *)
		malloc (sizeof (_IceConnectToMeInfo));

	    setupInfo->my_version_index = myVersionIndex;
	    setupInfo->his_version_index = hisVersionIndex;
	    setupInfo->his_vendor = vendor;
	    setupInfo->his_release = release;
	    setupInfo->my_auth_index = myAuthIndex;
	    setupInfo->my_auth_state = authState;
	    setupInfo->must_authenticate = mustAuthenticate;
	}
	else if (status == IcePaAuthAccepted)
	{
	    accept_setup_now = 1;
	}

	if (authData && authDataLen > 0)
	    free ((char *) authData);

	if (errorString)
	    free (errorString);
    }
    
    if (accept_setup_now)
    {
	AcceptConnection (iceConn, hisVersionIndex);

	iceConn->vendor = vendor;
	iceConn->release = release;
	iceConn->my_ice_version_index = myVersionIndex;
    }

    if (hisAuthCount > 0)
    {
	for (i = 0; i < hisAuthCount; i++)
	    free (hisAuthNames[i]);
	
	free ((char *) hisAuthNames);
    }

    IceDisposeCompleteMessage (iceConn, pStart);
    return (0);
}



static Bool
ProcessAuthRequired (iceConn, length, swap, replyWait)

IceConn			iceConn;
unsigned long	 	length;
Bool			swap;
IceReplyWaitInfo	*replyWait;

{
    iceAuthRequiredMsg  *message;
    int			authDataLen;
    IcePointer 		authData;
    int 		replyDataLen;
    IcePointer 		replyData = NULL;
    char		*errorString = NULL;
    IcePoAuthProc	authProc;
    IcePoAuthStatus	status;
    IcePointer 		authState;
    int			realAuthIndex;

    CHECK_AT_LEAST_SIZE (iceConn, ICE_AuthRequired,
	length, SIZEOF (iceAuthRequiredMsg),
	iceConn->connect_to_you ? IceFatalToConnection : IceFatalToProtocol);

    IceReadCompleteMessage (iceConn, SIZEOF (iceAuthRequiredMsg),
	iceAuthRequiredMsg, message, authData);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, authData);
	return (0);
    }

    if (swap)
    {
	message->authDataLength = lswaps (message->authDataLength);
    }

    CHECK_COMPLETE_SIZE (iceConn, ICE_AuthRequired, length,
	message->authDataLength + SIZEOF (iceAuthRequiredMsg), authData,
	iceConn->connect_to_you ? IceFatalToConnection : IceFatalToProtocol);

    if (iceConn->connect_to_you)
    {
	if ((int) message->authIndex >= _IceAuthCount)
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);

	    char *tempstr = "Received bad authIndex in the AuthRequired message";
	    char errIndex = (int) message->authIndex;

	    errorString = (char *) malloc (strlen (tempstr) + 1);
	    strcpy (errorString, tempstr);

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = errorString;

	    _IceErrorBadValue (iceConn, 0,
		ICE_AuthRequired, 2, 1, &errIndex);

	    IceDisposeCompleteMessage (iceConn, authData);
	    return (1);
	}
	else
	{
	    authProc = _IcePoAuthProcs[message->authIndex];

	    iceConn->connect_to_you->auth_active = 1;
	}
    }
    else if (iceConn->protosetup_to_you)
    {
	if ((int) message->authIndex >=
	    iceConn->protosetup_to_you->my_auth_count)
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);

	    char *tempstr = "Received bad authIndex in the AuthRequired message";
	    char errIndex = (int) message->authIndex;

	    errorString = (char *) malloc (strlen (tempstr) + 1);
	    strcpy (errorString, tempstr);

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = errorString;

	    _IceErrorBadValue (iceConn, 0,
		ICE_AuthRequired, 2, 1, &errIndex);

	    IceDisposeCompleteMessage (iceConn, authData);
	    return (1);
	}
	else
	{
	    _IcePoProtocol *myProtocol = _IceProtocols[
	        iceConn->protosetup_to_you->my_opcode - 1].orig_client;

	    realAuthIndex = iceConn->protosetup_to_you->
		my_auth_indices[message->authIndex];

	    authProc = myProtocol->auth_procs[realAuthIndex];

	    iceConn->protosetup_to_you->auth_active = 1;
	}
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, 0, ICE_AuthRequired, IceCanContinue);

	IceDisposeCompleteMessage (iceConn, authData);
	return (0);
    }

    authState = NULL;
    authDataLen = message->authDataLength;

    status = (*authProc) (iceConn, &authState, False /* don't clean up */,
	swap, authDataLen, authData, &replyDataLen, &replyData, &errorString);

    if (status == IcePoAuthHaveReply)
    {
	AuthReply (iceConn, replyDataLen, replyData);

	replyWait->sequence_of_request = iceConn->send_sequence;
	replyWait->minor_opcode_of_request = ICE_AuthReply;

	if (iceConn->connect_to_you)
	{
	    iceConn->connect_to_you->my_auth_state = authState;
	    iceConn->connect_to_you->my_auth_index = message->authIndex;
	}
	else if (iceConn->protosetup_to_you)
	{
	    iceConn->protosetup_to_you->my_auth_state = authState;
	    iceConn->protosetup_to_you->my_auth_index = realAuthIndex;
	}
    }
    else if (status == IcePoAuthRejected || status == IcePoAuthFailed)
    {
	char *prefix, *returnErrorString;

	if (status == IcePoAuthRejected)
	{
	    _IceErrorAuthenticationRejected (iceConn,
	        ICE_AuthRequired, errorString);

	    prefix = "Authentication Rejected, reason : ";
	}
	else
	{
	    _IceErrorAuthenticationFailed (iceConn,
	       ICE_AuthRequired, errorString);

	    prefix = "Authentication Failed, reason : ";
	}

	returnErrorString = (char *) malloc (strlen (prefix) +
	    strlen (errorString) + 1);
	sprintf (returnErrorString, "%s%s", prefix, errorString);
	free (errorString);
	
	if (iceConn->connect_to_you)
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = returnErrorString;
	}
	else
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = returnErrorString;
	}
    }

    if (replyData && replyDataLen > 0)
	free ((char *) replyData);

    IceDisposeCompleteMessage (iceConn, authData);

    return (status != IcePoAuthHaveReply);
}



static int
ProcessAuthReply (iceConn, length, swap)

IceConn		iceConn;
unsigned long	length;
Bool		swap;

{
    iceAuthReplyMsg 	*message;
    int			replyDataLen;
    IcePointer		replyData;
    int 		authDataLen;
    IcePointer 		authData = NULL;
    char		*errorString = NULL;

    CHECK_AT_LEAST_SIZE (iceConn, ICE_AuthReply,
	length, SIZEOF (iceAuthReplyMsg),
	iceConn->connect_to_me ? IceFatalToConnection : IceFatalToProtocol);

    IceReadCompleteMessage (iceConn, SIZEOF (iceAuthReplyMsg),
	iceAuthReplyMsg, message, replyData);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, replyData);
	return (0);
    }

    if (swap)
    {
	message->authDataLength = lswaps (message->authDataLength);
    }

    CHECK_COMPLETE_SIZE (iceConn, ICE_AuthReply, length,
	message->authDataLength + SIZEOF (iceAuthReplyMsg), replyData,
	iceConn->connect_to_me ? IceFatalToConnection : IceFatalToProtocol);

    replyDataLen = message->authDataLength;

    if (iceConn->connect_to_me)
    {
	IcePaAuthProc authProc = _IcePaAuthProcs[(int)
	    (iceConn->connect_to_me->my_auth_index)];
	IcePaAuthStatus status =
	    (*authProc) (iceConn, &iceConn->connect_to_me->my_auth_state, swap,
	    replyDataLen, replyData, &authDataLen, &authData, &errorString);

	if (status == IcePaAuthContinue)
	{
	    AuthNextPhase (iceConn, authDataLen, authData);
	}
	else if (status == IcePaAuthRejected || status == IcePaAuthFailed)
	{
	    /*
	     * Before we reject, invoke host-based authentication callback
	     * and give it a chance to accept the connection (only if the
	     * other client doesn't require authentication).
	     */

	    if (!iceConn->connect_to_me->must_authenticate &&
		iceConn->listen_obj->host_based_auth_proc)
	    {
		char *hostname = _IceGetPeerName (iceConn);

		if ((*iceConn->listen_obj->host_based_auth_proc) (hostname))
		{
		    status = IcePaAuthAccepted;
		}

		if (hostname)
		    free (hostname);
	    }

	    if (status != IcePaAuthAccepted)
	    {
		free (iceConn->connect_to_me->his_vendor);
		free (iceConn->connect_to_me->his_release);
		free ((char *) iceConn->connect_to_me);
		iceConn->connect_to_me = NULL;

		iceConn->connection_status = IceConnectRejected;

		if (status == IcePaAuthRejected)
		{
		    _IceErrorAuthenticationRejected (iceConn,
	                ICE_AuthReply, errorString);
		}
		else
		{
		    _IceErrorAuthenticationFailed (iceConn,
	                ICE_AuthReply, errorString);
		}
	    }
	}

	if (status == IcePaAuthAccepted)
	{
	    AcceptConnection (iceConn,
		iceConn->connect_to_me->his_version_index);

	    iceConn->vendor = iceConn->connect_to_me->his_vendor;
	    iceConn->release = iceConn->connect_to_me->his_release;
	    iceConn->my_ice_version_index =
		iceConn->connect_to_me->my_version_index;

	    free ((char *) iceConn->connect_to_me);
	    iceConn->connect_to_me = NULL;
	}
    }
    else if (iceConn->protosetup_to_me)
    {
	_IcePaProtocol *myProtocol = _IceProtocols[iceConn->protosetup_to_me->
	    my_opcode - 1].accept_client;
	IcePaAuthProc authProc = myProtocol->auth_procs[(int)
	    (iceConn->protosetup_to_me->my_auth_index)];
	IcePaAuthStatus status =
	    (*authProc) (iceConn, &iceConn->protosetup_to_me->my_auth_state,
	    swap, replyDataLen, replyData,
	    &authDataLen, &authData, &errorString);
	int free_setup_info = 1;

	if (status == IcePaAuthContinue)
	{
	    AuthNextPhase (iceConn, authDataLen, authData);
	    free_setup_info = 0;
	}
	else if (status == IcePaAuthRejected || status == IcePaAuthFailed)
	{
	    /*
	     * Before we reject, invoke host-based authentication callback
	     * and give it a chance to accept the Protocol Setup (only if the
	     * other client doesn't require authentication).
	     */

	    if (!iceConn->protosetup_to_me->must_authenticate &&
		myProtocol->host_based_auth_proc)
	    {
		char *hostname = _IceGetPeerName (iceConn);

		if ((*myProtocol->host_based_auth_proc) (hostname))
		{
		    status = IcePaAuthAccepted;
		}

		if (hostname)
		    free (hostname);
	    }

	    if (status == IcePaAuthRejected)
	    {
		_IceErrorAuthenticationRejected (iceConn,
	            ICE_AuthReply, errorString);
	    }
	    else
	    {
	        _IceErrorAuthenticationFailed (iceConn,
	            ICE_AuthReply, errorString);
	    }
	}

	if (status == IcePaAuthAccepted)
	{
	    IcePaProcessMsgProc	processMsgProc;
	    IceProtocolSetupProc protocolSetupProc;
	    IceProtocolActivateProc protocolActivateProc;
	    _IceProcessMsgInfo *process_msg_info;
	    IcePointer clientData = NULL;
	    char *failureReason = NULL;
	    Status status = 1;

	    protocolSetupProc = myProtocol->protocol_setup_proc;
	    protocolActivateProc = myProtocol->protocol_activate_proc;

	    if (protocolSetupProc)
	    {
		/*
		 * Notify the client of the Protocol Setup.
		 */

		status = (*protocolSetupProc) (iceConn,
		    myProtocol->version_recs[iceConn->protosetup_to_me->
		        my_version_index].major_version,
		    myProtocol->version_recs[iceConn->protosetup_to_me->
		        my_version_index].minor_version,
		    iceConn->protosetup_to_me->his_vendor,
		    iceConn->protosetup_to_me->his_release,
		    &clientData, &failureReason);

		/*
		 * Set vendor and release pointers to NULL, so it won't
		 * get freed below.  The ProtocolSetupProc should
		 * free it.
		 */

		iceConn->protosetup_to_me->his_vendor = NULL;
		iceConn->protosetup_to_me->his_release = NULL;
	    }

	    if (status != 0)
	    {
		/*
		 * Send the Protocol Reply
		 */

		AcceptProtocol (iceConn,
	            iceConn->protosetup_to_me->his_opcode,
	            iceConn->protosetup_to_me->my_opcode,
	            iceConn->protosetup_to_me->his_version_index,
		    myProtocol->vendor, myProtocol->release);


		/*
		 * Set info for this protocol.
		 */

		processMsgProc = myProtocol->version_recs[
	            iceConn->protosetup_to_me->
	            my_version_index].process_msg_proc;

		process_msg_info = &iceConn->process_msg_info[
	            iceConn->protosetup_to_me->
		    his_opcode -iceConn->his_min_opcode];

		process_msg_info->client_data = clientData;
		process_msg_info->accept_flag = 1;
		process_msg_info->process_msg_proc.
		    accept_client = processMsgProc;


		/*
		 * Increase the reference count for the number
		 * of active protocols.
		 */

		iceConn->proto_ref_count++;


		/*
		 * Notify the client that the protocol is active.  The reason
		 * we have this 2nd callback invoked is because the client
		 * may wish to immediately generate a message for this
		 * protocol, but it must wait until we send the Protocol Reply.
		 */

		if (protocolActivateProc)
		{
		    (*protocolActivateProc) (iceConn,
		        process_msg_info->client_data);
		}
	    }
	    else
	    {
		/*
		 * An error was encountered.
		 */

		_IceErrorSetupFailed (iceConn, ICE_ProtocolSetup,
		    failureReason);

		if (failureReason)
		    free (failureReason);
	    }
	}


	if (free_setup_info)
	{
	    if (iceConn->protosetup_to_me->his_vendor)
		free (iceConn->protosetup_to_me->his_vendor);
	    if (iceConn->protosetup_to_me->his_release)
		free (iceConn->protosetup_to_me->his_release);
	    free ((char *) iceConn->protosetup_to_me);
	    iceConn->protosetup_to_me = NULL;
	}
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, 0, ICE_AuthReply, IceCanContinue);
    }

    if (authData && authDataLen > 0)
	free ((char *) authData);

    if (errorString)
	free (errorString);

    IceDisposeCompleteMessage (iceConn, replyData);
    return (0);
}



static Bool
ProcessAuthNextPhase (iceConn, length, swap, replyWait)

IceConn		  	iceConn;
unsigned long	 	length;
Bool			swap;
IceReplyWaitInfo	*replyWait;

{
    iceAuthNextPhaseMsg *message;
    int 		authDataLen;
    IcePointer		authData;
    int 		replyDataLen;
    IcePointer		replyData = NULL;
    char		*errorString = NULL;
    IcePoAuthProc 	authProc;
    IcePoAuthStatus	status;
    IcePointer 		*authState;

    CHECK_AT_LEAST_SIZE (iceConn, ICE_AuthNextPhase,
	length, SIZEOF (iceAuthNextPhaseMsg),
	iceConn->connect_to_you ? IceFatalToConnection : IceFatalToProtocol);

    IceReadCompleteMessage (iceConn, SIZEOF (iceAuthNextPhaseMsg),
	iceAuthNextPhaseMsg, message, authData);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, authData);
	return (0);
    }

    if (swap)
    {
	message->authDataLength = lswaps (message->authDataLength);
    }

    CHECK_COMPLETE_SIZE (iceConn, ICE_AuthNextPhase, length,
	message->authDataLength + SIZEOF (iceAuthNextPhaseMsg), authData,
	iceConn->connect_to_you ? IceFatalToConnection : IceFatalToProtocol);

    if (iceConn->connect_to_you)
    {
	authProc = _IcePoAuthProcs[(int)
	    (iceConn->connect_to_you->my_auth_index)];

	authState = &iceConn->connect_to_you->my_auth_state;
    }
    else if (iceConn->protosetup_to_you)
    {
	_IcePoProtocol *myProtocol =
	  _IceProtocols[iceConn->protosetup_to_you->my_opcode - 1].orig_client;

	authProc = myProtocol->auth_procs[(int)
	    (iceConn->protosetup_to_you->my_auth_index)];

	authState = &iceConn->protosetup_to_you->my_auth_state;
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, 0, ICE_AuthNextPhase, IceCanContinue);

	IceDisposeCompleteMessage (iceConn, authData);
	return (0);
    }

    authDataLen = message->authDataLength;

    status = (*authProc) (iceConn, authState, False /* don't clean up */,
	swap, authDataLen, authData, &replyDataLen, &replyData, &errorString);

    if (status == IcePoAuthHaveReply)
    {
	AuthReply (iceConn, replyDataLen, replyData);

	replyWait->sequence_of_request = iceConn->send_sequence;
    }
    else if (status == IcePoAuthRejected || status == IcePoAuthFailed)
    {
	char *prefix, *returnErrorString;

	if (status == IcePoAuthRejected)
	{
	    _IceErrorAuthenticationRejected (iceConn,
	       ICE_AuthNextPhase, errorString);

	    prefix = "Authentication Rejected, reason : ";
	}
	else if (status == IcePoAuthFailed)
	{
	    _IceErrorAuthenticationFailed (iceConn,
	       ICE_AuthNextPhase, errorString);

	    prefix = "Authentication Failed, reason : ";
	}

	returnErrorString = (char *) malloc (strlen (prefix) +
	    strlen (errorString) + 1);
	sprintf (returnErrorString, "%s%s", prefix, errorString);
	free (errorString);

	if (iceConn->connect_to_you)
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);

	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message = returnErrorString;
	}
	else
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);

	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message = returnErrorString;
	}
    }

    if (replyData && replyDataLen > 0)
	free ((char *) replyData);

    IceDisposeCompleteMessage (iceConn, authData);

    return (status != IcePoAuthHaveReply);
}



static Bool
ProcessConnectionReply (iceConn, length, swap, replyWait)

IceConn			iceConn;
unsigned long	 	length;
Bool			swap;
IceReplyWaitInfo 	*replyWait;

{
    iceConnectionReplyMsg 	*message;
    char 			*pData, *pStart, *pEnd;
    Bool			replyReady;

    CHECK_AT_LEAST_SIZE (iceConn, ICE_ConnectionReply,
	length, SIZEOF (iceConnectionReplyMsg), IceFatalToConnection);

    IceReadCompleteMessage (iceConn, SIZEOF (iceConnectionReplyMsg),
	iceConnectionReplyMsg, message, pStart);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    pData = pStart;
    pEnd = pStart + (length << 3);

    SKIP_STRING (pData, swap, pEnd,
		 BAIL_STRING (iceConn, ICE_ConnectionReply,
			      pStart));		    	     /* vendor */
    SKIP_STRING (pData, swap, pEnd,
		 BAIL_STRING (iceConn, ICE_ConnectionReply,
			      pStart));			     /* release */

    CHECK_COMPLETE_SIZE (iceConn, ICE_ConnectionReply,
	length, pData - pStart + SIZEOF (iceConnectionReplyMsg),
	pStart, IceFatalToConnection);

    pData = pStart;

    if (iceConn->connect_to_you)
    {
	if (iceConn->connect_to_you->auth_active)
	{
	    /*
	     * Tell the authentication procedure to clean up.
	     */

	    IcePoAuthProc authProc = _IcePoAuthProcs[(int)
		(iceConn->connect_to_you->my_auth_index)];

	    (*authProc) (iceConn, &iceConn->connect_to_you->my_auth_state,
		True /* clean up */, False /* swap */,
	        0, NULL, NULL, NULL, NULL);
	}

	if ((int) message->versionIndex >= _IceVersionCount)
	{
	    _IceConnectionError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->connection_error);
	    char errIndex = message->versionIndex;

	    _IceErrorBadValue (iceConn, 0,
		ICE_ConnectionReply, 2, 1, &errIndex);
	    
	    errorReply->type = ICE_CONNECTION_ERROR;
	    errorReply->error_message =
		"Received bad version index in Connection Reply";
	}
	else
	{
	    _IceReply *reply = (_IceReply *) (replyWait->reply);

	    reply->type = ICE_CONNECTION_REPLY;
	    reply->connection_reply.version_index = message->versionIndex;

	    EXTRACT_STRING (pData, swap, reply->connection_reply.vendor);
	    EXTRACT_STRING (pData, swap, reply->connection_reply.release);
	}

	replyReady = True;
    }
    else
    {
	/*
	 * Unexpected message
	 */

	_IceErrorBadState (iceConn, 0, ICE_ConnectionReply, IceCanContinue);

	replyReady = False;
    }

    IceDisposeCompleteMessage (iceConn, pStart);

    return (replyReady);
}



static int
ProcessProtocolSetup (iceConn, length, swap)

IceConn		iceConn;
unsigned long	length;
Bool		swap;

{
    iceProtocolSetupMsg	*message;
    _IcePaProtocol 	*myProtocol;
    int  	      	myVersionCount, hisVersionCount;
    int	 	      	myVersionIndex, hisVersionIndex;
    int  	      	hisMajorVersion, hisMinorVersion;
    int	 	      	myAuthCount, hisAuthCount;
    int  	      	myOpcode, hisOpcode;
    int	 	      	found, i, j;
    char	      	*myAuthName, **hisAuthNames;
    char 	      	*protocolName;
    char 		*pData, *pStart, *pEnd;
    char 	      	*vendor = NULL;
    char 	      	*release = NULL;
    int  	      	accept_setup_now = 0;
    int			myAuthIndex = 0;
    int			hisAuthIndex = 0;
    char		mustAuthenticate;
    int			authUsableCount;
    int			authUsableFlags[MAX_ICE_AUTH_NAMES];
    int			authIndices[MAX_ICE_AUTH_NAMES];

    CHECK_AT_LEAST_SIZE (iceConn, ICE_ProtocolSetup,
	length, SIZEOF (iceProtocolSetupMsg), IceFatalToProtocol);

    if (iceConn->want_to_close)
    {
	/*
	 * If we sent a WantToClose message, but just got a ProtocolSetup,
	 * we must cancel our WantToClose.  It is the responsiblity of the
	 * other client to send a WantToClose later on.
	 */

	iceConn->want_to_close = 0;
    }

    IceReadCompleteMessage (iceConn, SIZEOF (iceProtocolSetupMsg),
	iceProtocolSetupMsg, message, pStart);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    pData = pStart;
    pEnd = pStart + (length << 3);

    SKIP_STRING (pData, swap, pEnd,
		 BAIL_STRING(iceConn, ICE_ProtocolSetup, 
			     pStart));			       /* proto name */
    SKIP_STRING (pData, swap, pEnd,
		 BAIL_STRING(iceConn, ICE_ProtocolSetup, 
			     pStart));			       /* vendor */
    SKIP_STRING (pData, swap, pEnd,
		 BAIL_STRING(iceConn, ICE_ProtocolSetup, 
			     pStart));			       /* release */
    SKIP_LISTOF_STRING (pData, swap, (int) message->authCount, pEnd,
			BAIL_STRING(iceConn, ICE_ProtocolSetup, 
				    pStart));		       /* auth names */
    pData += (message->versionCount * 4);		       /* versions */

    CHECK_COMPLETE_SIZE (iceConn, ICE_ProtocolSetup,
	length, pData - pStart + SIZEOF (iceProtocolSetupMsg),
	pStart, IceFatalToProtocol);

    mustAuthenticate = message->mustAuthenticate;

    if (mustAuthenticate != 0 && mustAuthenticate != 1)
    {
	_IceErrorBadValue (iceConn, 0,
	    ICE_ProtocolSetup, 4, 1, &mustAuthenticate);
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    pData = pStart;

    if (iceConn->process_msg_info &&
	(int) message->protocolOpcode >= iceConn->his_min_opcode &&
        (int) message->protocolOpcode <= iceConn->his_max_opcode &&
	iceConn->process_msg_info[
	message->protocolOpcode - iceConn->his_min_opcode].in_use)
    {
	_IceErrorMajorOpcodeDuplicate (iceConn, message->protocolOpcode);
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    EXTRACT_STRING (pData, swap, protocolName);

    if (iceConn->process_msg_info)
    {
	for (i = 0;
	    i <= (iceConn->his_max_opcode - iceConn->his_min_opcode); i++)
	{
	    if (iceConn->process_msg_info[i].in_use && strcmp (protocolName,
	        iceConn->process_msg_info[i].protocol->protocol_name) == 0)
	    {
		_IceErrorProtocolDuplicate (iceConn, protocolName);
		free (protocolName);
		IceDisposeCompleteMessage (iceConn, pStart);
		return (0);
	    }
	}
    }

    for (i = 0; i < _IceLastMajorOpcode; i++)
	if (strcmp (protocolName, _IceProtocols[i].protocol_name) == 0)
	    break;

    if (i < _IceLastMajorOpcode &&
        (myProtocol = _IceProtocols[i].accept_client) != NULL)
    {
	hisOpcode = message->protocolOpcode;
	myOpcode = i + 1;
	free (protocolName);
    }
    else
    {
	_IceErrorUnknownProtocol (iceConn, protocolName);
	free (protocolName);
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    EXTRACT_STRING (pData, swap, vendor);
    EXTRACT_STRING (pData, swap, release);

    if ((hisAuthCount = message->authCount) > 0)
    {
	hisAuthNames = (char **) malloc (hisAuthCount * sizeof (char *));
	EXTRACT_LISTOF_STRING (pData, swap, hisAuthCount, hisAuthNames);
    }

    hisVersionCount = message->versionCount;
    myVersionCount = myProtocol->version_count;

    hisVersionIndex = myVersionIndex = found = 0;

    for (i = 0; i < hisVersionCount && !found; i++)
    {
	EXTRACT_CARD16 (pData, swap, hisMajorVersion);
	EXTRACT_CARD16 (pData, swap, hisMinorVersion);

	for (j = 0; j < myVersionCount && !found; j++)
	{
	    if (myProtocol->version_recs[j].major_version == hisMajorVersion &&
		myProtocol->version_recs[j].minor_version == hisMinorVersion)
	    {
		hisVersionIndex = i;
		myVersionIndex = j;
		found = 1;
	    }
	}
    }

    if (!found)
    {
	_IceErrorNoVersion (iceConn, ICE_ProtocolSetup);

	free (vendor);
	free (release);

	if (hisAuthCount > 0)
	{
	    for (i = 0; i < hisAuthCount; i++)
		free (hisAuthNames[i]);
	
	    free ((char *) hisAuthNames);
	}

	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    myAuthCount = myProtocol->auth_count;

    _IceGetPaValidAuthIndices (
	_IceProtocols[myOpcode - 1].protocol_name,
	iceConn->connection_string, myAuthCount, myProtocol->auth_names,
        &authUsableCount, authIndices);

    for (i = 0; i < myAuthCount; i++)
    {
	authUsableFlags[i] = 0;
	for (j = 0; j < authUsableCount && !authUsableFlags[i]; j++)
	    authUsableFlags[i] = (authIndices[j] == i);
    }

    for (i = found = 0; i < myAuthCount && !found; i++)
    {
	if (authUsableFlags[i])
	{
	    myAuthName = myProtocol->auth_names[i];

	    for (j = 0; j < hisAuthCount && !found; j++)
		if (strcmp (myAuthName, hisAuthNames[j]) == 0)
		{
		    myAuthIndex = i;
		    hisAuthIndex = j;
		    found = 1;
		}
	}
    }

    if (!found)
    {
	/*
	 * None of the authentication methods specified by the
	 * other client is supported.  If the other client requires
	 * authentication, we must reject the Protocol Setup now.
	 * Otherwise, we can invoke the host-based authentication callback
	 * to see if we can accept this Protocol Setup.
	 */

	if (mustAuthenticate || !myProtocol->host_based_auth_proc)
	{
	    _IceErrorNoAuthentication (iceConn, ICE_ProtocolSetup);
	}
	else
	{
	    char *hostname = _IceGetPeerName (iceConn);

	    if ((*myProtocol->host_based_auth_proc) (hostname))
	    {
		accept_setup_now = 1;
	    }
	    else 
	    {
		_IceErrorAuthenticationRejected (iceConn,
	            ICE_ProtocolSetup, "None of the authentication protocols specified are supported and host-based authentication failed");
	    }

	    if (hostname)
		free (hostname);
	}
    }
    else
    {
	IcePaAuthStatus	status;
	int 		authDataLen;
	IcePointer 	authData = NULL;
	IcePointer 	authState;
	char		*errorString = NULL;
	IcePaAuthProc	authProc =
		myProtocol->auth_procs[myAuthIndex];

	authState = NULL;

	status = (*authProc) (iceConn, &authState, swap, 0, NULL,
	    &authDataLen, &authData, &errorString);

	if (status == IcePaAuthContinue)
	{
	    _IceProtoSetupToMeInfo *setupInfo;

	    AuthRequired (iceConn, hisAuthIndex, authDataLen, authData);
	 
	    iceConn->protosetup_to_me = setupInfo =
		(_IceProtoSetupToMeInfo *) malloc (
		sizeof (_IceProtoSetupToMeInfo));

	    setupInfo->his_opcode = hisOpcode;
	    setupInfo->my_opcode = myOpcode;
	    setupInfo->my_version_index = myVersionIndex;
	    setupInfo->his_version_index = hisVersionIndex;
	    setupInfo->his_vendor = vendor;
	    setupInfo->his_release = release;
	    vendor = release = NULL;   /* so we don't free it */
	    setupInfo->my_auth_index = myAuthIndex;
	    setupInfo->my_auth_state = authState;
	    setupInfo->must_authenticate = mustAuthenticate;
	}
	else if (status == IcePaAuthAccepted)
	{
	    accept_setup_now = 1;
	}

	if (authData && authDataLen > 0)
	    free ((char *) authData);

	if (errorString)
	    free (errorString);
    }

    if (accept_setup_now)
    {
	IcePaProcessMsgProc		processMsgProc;
	IceProtocolSetupProc		protocolSetupProc;
	IceProtocolActivateProc		protocolActivateProc;
	_IceProcessMsgInfo		*process_msg_info;
	IcePointer			clientData = NULL;
	char 				*failureReason = NULL;
	Status				status = 1;

	protocolSetupProc = myProtocol->protocol_setup_proc;
	protocolActivateProc = myProtocol->protocol_activate_proc;

	if (protocolSetupProc)
	{
	    /*
	     * Notify the client of the Protocol Setup.
	     */

	    status = (*protocolSetupProc) (iceConn,
		myProtocol->version_recs[myVersionIndex].major_version,
		myProtocol->version_recs[myVersionIndex].minor_version,
	        vendor, release, &clientData, &failureReason);

	    vendor = release = NULL;   /* so we don't free it */
	}

	if (status != 0)
	{
	    /*
	     * Send the Protocol Reply
	     */

	    AcceptProtocol (iceConn, hisOpcode, myOpcode, hisVersionIndex,
	        myProtocol->vendor, myProtocol->release);


	    /*
	     * Set info for this protocol.
	     */

	    processMsgProc = myProtocol->version_recs[
	        myVersionIndex].process_msg_proc;

	    process_msg_info = &iceConn->process_msg_info[hisOpcode -
	        iceConn->his_min_opcode];

	    process_msg_info->client_data = clientData;
	    process_msg_info->accept_flag = 1;
	    process_msg_info->process_msg_proc.accept_client = processMsgProc;


	    /*
	     * Increase the reference count for the number of active protocols.
	     */

	    iceConn->proto_ref_count++;


	    /*
	     * Notify the client that the protocol is active.  The reason
	     * we have this 2nd callback invoked is because the client
	     * may wish to immediately generate a message for this
	     * protocol, but it must wait until we send the Protocol Reply.
	     */

	    if (protocolActivateProc)
	    {
		(*protocolActivateProc) (iceConn,
		    process_msg_info->client_data);
	    }
	}
	else
	{
	    /*
	     * An error was encountered.
	     */

	    _IceErrorSetupFailed (iceConn, ICE_ProtocolSetup, failureReason);

	    if (failureReason)
		free (failureReason);
	}
    }

    if (vendor)
	free (vendor);

    if (release)
	free (release);

    if (hisAuthCount > 0)
    {
	for (i = 0; i < hisAuthCount; i++)
	    free (hisAuthNames[i]);

	free ((char *) hisAuthNames);
    }

    IceDisposeCompleteMessage (iceConn, pStart);
    return (0);
}



static Bool
ProcessProtocolReply (iceConn, length, swap, replyWait)

IceConn		  	iceConn;
unsigned long	 	length;
Bool			swap;
IceReplyWaitInfo 	*replyWait;

{
    iceProtocolReplyMsg *message;
    char		*pData, *pStart, *pEnd;
    Bool		replyReady;

    CHECK_AT_LEAST_SIZE (iceConn, ICE_ProtocolReply,
	length, SIZEOF (iceProtocolReplyMsg), IceFatalToProtocol);

    IceReadCompleteMessage (iceConn, SIZEOF (iceProtocolReplyMsg),
	iceProtocolReplyMsg, message, pStart);

    if (!IceValidIO (iceConn))
    {
	IceDisposeCompleteMessage (iceConn, pStart);
	return (0);
    }

    pData = pStart;
    pEnd = pStart + (length << 3);

    SKIP_STRING (pData, swap, pEnd,
		 BAIL_STRING(iceConn, ICE_ProtocolReply,
			     pStart));			     /* vendor */
    SKIP_STRING (pData, swap, pEnd,
		 BAIL_STRING(iceConn, ICE_ProtocolReply,
			     pStart));			     /* release */

    CHECK_COMPLETE_SIZE (iceConn, ICE_ProtocolReply,
	length, pData - pStart + SIZEOF (iceProtocolReplyMsg),
	pStart, IceFatalToProtocol);

    pData = pStart;

    if (iceConn->protosetup_to_you)
    {
	if (iceConn->protosetup_to_you->auth_active)
	{
	    /*
	     * Tell the authentication procedure to clean up.
	     */

	    _IcePoProtocol *myProtocol = _IceProtocols[
		iceConn->protosetup_to_you->my_opcode - 1].orig_client;

	    IcePoAuthProc authProc = myProtocol->auth_procs[(int)
		(iceConn->protosetup_to_you->my_auth_index)];

#ifdef SVR4

/*
 * authProc is never NULL, but the cc compiler on UNIX System V/386
 * Release 4.2 Version 1 screws up an optimization.  Unless there is
 * some sort of reference to authProc before the function call, the
 * function call will seg fault.
 */
	    if (authProc)
#endif
		(*authProc) (iceConn,
		&iceConn->protosetup_to_you->my_auth_state,
		True /* clean up */, False /* swap */,
	        0, NULL, NULL, NULL, NULL);
	}

	if ((int) message->versionIndex >= _IceVersionCount)
	{
	    _IceProtocolError *errorReply =
	        &(((_IceReply *) (replyWait->reply))->protocol_error);
	    char errIndex = message->versionIndex;

	    _IceErrorBadValue (iceConn, 0,
		ICE_ProtocolReply, 2, 1, &errIndex);
	    
	    errorReply->type = ICE_PROTOCOL_ERROR;
	    errorReply->error_message =
		"Received bad version index in Protocol Reply";
	}
	else
	{
	    _IceProtocolReply *reply = 
	        &(((_IceReply *) (replyWait->reply))->protocol_reply);

	    reply->type = ICE_PROTOCOL_REPLY;
	    reply->major_opcode = message->protocolOpcode;
	    reply->version_index = message->versionIndex;

	    EXTRACT_STRING (pData, swap, reply->vendor);
	    EXTRACT_STRING (pData, swap, reply->release);
	}

	replyReady = True;
    }
    else
    {
	_IceErrorBadState (iceConn, 0, ICE_ProtocolReply, IceCanContinue);

	replyReady = False;
    }

    IceDisposeCompleteMessage (iceConn, pStart);

    return (replyReady);
}



static int
ProcessPing (iceConn, length)

IceConn 	iceConn;
unsigned long	length;

{
    CHECK_SIZE_MATCH (iceConn, ICE_Ping,
	length, SIZEOF (icePingMsg), IceFatalToConnection);

    PingReply (iceConn);

    return (0);
}



static int
ProcessPingReply (iceConn, length)

IceConn 	iceConn;
unsigned long	length;

{
    CHECK_SIZE_MATCH (iceConn, ICE_PingReply,
	length, SIZEOF (icePingReplyMsg), IceFatalToConnection);

    if (iceConn->ping_waits)
    {
	_IcePingWait *next = iceConn->ping_waits->next;
	
	(*iceConn->ping_waits->ping_reply_proc) (iceConn,
	    iceConn->ping_waits->client_data);

	free ((char *) iceConn->ping_waits);
	iceConn->ping_waits = next;
    }
    else
    {
	_IceErrorBadState (iceConn, 0, ICE_PingReply, IceCanContinue);
    }

    return (0);
}



static int
ProcessWantToClose (iceConn, length, connectionClosedRet)

IceConn 	iceConn;
unsigned long	length;
Bool		*connectionClosedRet;

{
    *connectionClosedRet = False;

    CHECK_SIZE_MATCH (iceConn, ICE_WantToClose,
	length, SIZEOF (iceWantToCloseMsg), IceFatalToConnection);

    if (iceConn->want_to_close || iceConn->open_ref_count == 0)
    {
	/*
	 * We just received a WantToClose.  Either we also sent a
	 * WantToClose, so we close the connection, or the iceConn
	 * is not being used, so we close the connection.  This
	 * second case is possible if we sent a WantToClose because
	 * the iceConn->open_ref_count reached zero, but then we
	 * received a NoClose.
	 */

	_IceConnectionClosed (iceConn);		/* invoke watch procs */
	_IceFreeConnection (iceConn);
	*connectionClosedRet = True;
    }
    else if (iceConn->proto_ref_count > 0)
    {
	/*
	 * We haven't shut down all of our protocols yet.  We send a NoClose,
	 * and it's up to us to generate a WantToClose later on.
	 */

	IceSimpleMessage (iceConn, 0, ICE_NoClose);
	IceFlush (iceConn);
    }
    else
    {
	/*
	 * The reference count on this iceConn is zero.  This means that
	 * there are no active protocols, but the client didn't explicitly
	 * close the connection yet.  If we didn't just send a Protocol Setup,
	 * we send a NoClose, and it's up to us to generate a WantToClose
	 * later on.
	 */

	if (!iceConn->protosetup_to_you)
	{
	    IceSimpleMessage (iceConn, 0, ICE_NoClose);
	    IceFlush (iceConn);
	}
    }

    return (0);
}



static int
ProcessNoClose (iceConn, length)

IceConn 	iceConn;
unsigned long	length;

{
    CHECK_SIZE_MATCH (iceConn, ICE_NoClose,
	length, SIZEOF (iceNoCloseMsg), IceFatalToConnection);

    if (iceConn->want_to_close)
    {
	/*
	 * The other side can't close now.  We cancel our WantToClose,
	 * and we can expect a WantToClose from the other side.
	 */

	iceConn->want_to_close = 0;
    }
    else
    {
	_IceErrorBadState (iceConn, 0, ICE_NoClose, IceCanContinue);
    }

    return (0);
}



void
_IceProcessCoreMessage (iceConn, opcode, length, swap,
    replyWait, replyReadyRet, connectionClosedRet)

IceConn 	 iceConn;
int     	 opcode;
unsigned long	 length;
Bool    	 swap;
IceReplyWaitInfo *replyWait;
Bool		 *replyReadyRet;
Bool		 *connectionClosedRet;

{
    Bool replyReady = False;

    *connectionClosedRet = False;

    switch (opcode)
    {
    case ICE_Error:

	replyReady = ProcessError (iceConn, length, swap, replyWait);
	break;

    case ICE_ConnectionSetup:

	ProcessConnectionSetup (iceConn, length, swap);
	break;

    case ICE_AuthRequired:

	replyReady = ProcessAuthRequired (iceConn, length, swap, replyWait);
        break;

    case ICE_AuthReply:

	ProcessAuthReply (iceConn, length, swap);
	break;

    case ICE_AuthNextPhase:

	replyReady = ProcessAuthNextPhase (iceConn, length, swap, replyWait);
	break;

    case ICE_ConnectionReply:

	replyReady = ProcessConnectionReply (iceConn, length, swap, replyWait);
	break;

    case ICE_ProtocolSetup:

	ProcessProtocolSetup (iceConn, length, swap);
	break;

    case ICE_ProtocolReply:

	replyReady = ProcessProtocolReply (iceConn, length, swap, replyWait);
	break;

    case ICE_Ping:

	ProcessPing (iceConn, length);
	break;

    case ICE_PingReply:

	ProcessPingReply (iceConn, length);
	break;

    case ICE_WantToClose:

	ProcessWantToClose (iceConn, length, connectionClosedRet);
	break;

    case ICE_NoClose:

	ProcessNoClose (iceConn, length);
	break;

    default:

	_IceErrorBadMinor (iceConn, 0, opcode, IceCanContinue);
	_IceReadSkip (iceConn, length << 3);
	break;
    }

    if (replyWait)
	*replyReadyRet = replyReady;
}


#ifdef MINIX
int 
MNX_IceMessagesAvailable(iceConn)

IceConn          iceConn;
{
	BytesReadable_t bytes;

	_KDE_IceTransSetOption(iceConn->trans_conn, TRANS_NONBLOCKING, 1);
	if (_KDE_IceTransBytesReadable(iceConn->trans_conn, &bytes) < 0)
		bytes= -1;
	_KDE_IceTransSetOption(iceConn->trans_conn, TRANS_NONBLOCKING, 0);
	return (bytes != 0);
}
#endif
