/*
 * C interface to DCOP
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string.h>

/* sometimes __STDC__ is defined, but to 0. The hateful X headers
   ask for '#if __STDC__', so they become confused. */
#ifdef __STDC__
#if !__STDC__
#undef __STDC__
#define __STDC__ 1
#endif
#endif
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/ICE/ICE.h>
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEconn.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>

#include "dcopglobal.h"
#include "dcopc.h"

static dcop_callback_t dcop_callback = 0L;
static Bool dcop_attached = False;
static IceConn dcop_ice_conn;
static int dcop_major_opcode;
static CARD32 dcop_time;

  void
dcop_process_message(
  IceConn iceConn,
  IcePointer clientObject,
  int opcode,
  unsigned long length,
  Bool swap,
  IceReplyWaitInfo * replyWait,
  Bool * replyWaitRet
);

static IcePoVersionRec DCOPVersions[] = {
  { DCOPVersionMajor, DCOPVersionMinor, dcop_process_message }
};

  Bool
dcop_call(
  const char * remApp,
  const char * remObjId,
  const char * remFun,
  const char * data,
  int dataLength,
  char * replyType,
  char * replyData,
  int replyDataLength
);
  
  char *
dcop_write_int(char * buf, int i);
  
  char *
dcop_read_int(char * buf, int * i);
  
  char *
dcop_write_string(char * buf, const char * text);
  
  char *
dcop_read_string(char * buf, char * output);

  char *
dcop_write_int(char * buf, int i)
{
  char * p = (char *)(&i);

  buf[3] = *p++;
  buf[2] = *p++;
  buf[1] = *p++;
  buf[0] = *p;

  return buf + 4;
}

  char *
dcop_read_int(char * buf, int * i)
{
  *i = *((int *)buf);
  return buf + 4;
}

  char *
dcop_write_string(char * buf, const char * text)
{
  char * pos = buf;
  int l = strlen( text ) + 1; /* we need the \0! (Simon) */
  pos = dcop_write_int(buf, l);
  memcpy(pos, text, l);
  return pos + l;
}

  char *
dcop_read_string(char * buf, char * output)
{
  int length;
  char * pos = dcop_read_int(buf, &length);
  output = (char *)malloc(length);
  memcpy(output, pos, length);
  return pos + length;
}

  int
dcop_send_signal(
  const char * receiving_app,
  const char * object,
  const char * function,
  char * data,
  int dataLength
)
{
  /* Vars *****************************************************************/

  char  * pos               = 0L;
  char  * vendor            = 0L;
  char  * release           = 0L;
  char  * newline           = 0L;
  char  * homeDir           = 0L;
  char  * dcopServer        = 0L;
  char  * header            = 0L;
  unsigned int headerLength = 0;

  int     setupstat         = 0;
  int     majorOpcode       = 0;
  int     fd                = 0;
  int     majorVersion      = 0;
  int     minorVersion      = 0;
  ssize_t bytesRead         = 0;

  char    errBuf[1024];

  IceConn     iceConn;
  IcePointer  context = 0;
  IcePointer  somethingToPassToProtoSetup = 0;

  struct DCOPMsg * pMsgPtr = 0;

  /* Find home dir ********************************************************/

  homeDir = getenv("HOME");

  if (NULL == homeDir)
    return DCOP_ERROR_SERVER_NOT_FOUND;

  /* ICE setup ************************************************************/

  majorOpcode =
    IceRegisterForProtocolSetup(
      (char *)("DCOP"),
      DCOPVendorString,
      DCOPReleaseString,
      1,
      DCOPVersions,
      DCOPAuthCount,
      (char **)DCOPAuthNames,
      DCOPClientAuthProcs,
      0L
    );

  if (majorOpcode < 0)
    return DCOP_ERROR_CANNOT_REGISTER_WITH_ICE;

  /* Find DCOP server and open connection ********************************/

  dcopServer = getenv("DCOPSERVER");

  if (NULL == dcopServer) {

    if (-1 == chdir(homeDir))
      return DCOP_ERROR_SERVER_NOT_FOUND;

    fd = open(".DCOPserver", O_RDONLY);

    if (-1 == fd)
      return DCOP_ERROR_SERVER_NOT_FOUND;

    dcopServer = (char *)malloc(1024);

    bytesRead = read(fd, dcopServer, 1024);

    if (-1 == bytesRead)
      return DCOP_ERROR_SERVER_NOT_FOUND;

    newline = strchr(dcopServer, '\n');

    if (NULL == newline)
      return DCOP_ERROR_SERVER_NOT_FOUND;

    *newline = '\0';

    iceConn =
      IceOpenConnection(
        dcopServer,
        context,
        False,
        majorOpcode,
        sizeof(errBuf),
        errBuf
      );

    free(dcopServer);

  } else {

    iceConn =
      IceOpenConnection(
        dcopServer,
        context,
        False,
        majorOpcode,
        sizeof(errBuf),
        errBuf
      );
  }

  if (iceConn == 0L)
    return DCOP_ERROR_CANNOT_CONTACT_SERVER;

  IceSetShutdownNegotiation(iceConn, False);

  /* Setup protocol ******************************************************/

  setupstat =
    IceProtocolSetup(
      iceConn,
      majorOpcode,
      somethingToPassToProtoSetup,
      True, /* must authenticate */
      &(majorVersion), &(minorVersion), &(vendor), &(release),
      1024, errBuf
    );

  if (
    setupstat == IceProtocolSetupFailure ||
    setupstat == IceProtocolSetupIOError ||
    setupstat == IceProtocolAlreadyActive)
    return DCOP_ERROR_CANNOT_REGISTER_WITH_ICE;

  if (IceConnectionStatus(iceConn) != IceConnectAccepted)
    return DCOP_ERROR_SERVER_REFUSED_CONNECTION;

  /* Send data to DCOP ***************************************************/

  /*
   * First let ICE initialize the ICE Message Header and give us a pointer
   * to it (ICE manages that buffer internally)
   */
  IceGetHeader(
    iceConn,
    majorOpcode,
    DCOPSend,
    sizeof(struct DCOPMsg),
    struct DCOPMsg,
    pMsgPtr
  );

  /* marshall the arguments for the DCOP message header (callerApp, destApp,
   * destObj, destFunc.
   * The last argument is actually part of the data part of the call, but
   * we add it to the header. It's the size of the marshalled argument data.
   * In Qt it would look like
   * QDataStream str( ... )
   * str << callerApp << destApp << destObj << destFun << argumentQByteArrayDataStuff;
   * (where as str is the complete data stream sent do the dcopserver, excluding
   *  the ICE header)
   * As the QByteArray is marshalled as [size][data] and as we (below) send the
   * data in two chunks, first the dcop msg header and the the data, we just
   * put the [size] field as last field into the dcop msg header ;-)
   */

  header = (char *)malloc(1024);
  pos = header;

  pos = dcop_write_string(pos, "kmapnotify");
  pos = dcop_write_string(pos, receiving_app);
  pos = dcop_write_string(pos, object);
  pos = dcop_write_string(pos, function);
  pos = dcop_write_int(pos, dataLength);

  headerLength = pos - header;

  pMsgPtr->time = 0;
  /* the length field tells the dcopserver how much bytes the dcop message
   * takes up. We add that size to the already by IceGetHeader initialized
   * length value, as it seems that under some circumstances (depending on the
   * DCOPMsg structure size) the length field is aligned/padded. 
   */
  pMsgPtr->length += headerLength + dataLength;

  /* first let's send the dcop message header 
   * IceSendData automatically takes care of first sending the Ice Message
   * Header (outbufptr > outbuf -> flush the connection buffer)
   */
  IceSendData(iceConn, headerLength, header);
  /* and now the function argument data */
  IceSendData(iceConn, dataLength, data);

  /* send it all ;-) */
  IceFlush( iceConn );

  free(header);

  if (IceConnectionStatus(iceConn) != IceConnectAccepted)
    return DCOP_ERROR_SERVER_REFUSED_DATA;

  IceCloseConnection( iceConn );

  /* Done ***************************************************************/

  return DCOP_OK;
}

  Bool
dcop_register(
  const char * app_name,
  const char * object_id,
  dcop_callback_t callback
)
{
  /* STUB */
  return False;
#if 0
  QCString replyType;
  QByteArray data, replyData;
  QDataStream arg( data, IO_WriteOnly );
  arg <<appId;
  if ( call( "DCOPServer", "", "registerAs(QCString)", data, replyType, replyData ) ) {
    QDataStream reply( replyData, IO_ReadOnly );
    reply >> result;
  }
  d->appId = result;
  d->registered = !result.isNull();
  return result;
#endif
}

  void
dcop_process_message(
  IceConn iceConn,
  IcePointer clientObject,
  int opcode,
  unsigned long length,
  Bool swap,
  IceReplyWaitInfo * replyWait,
  Bool * replyWaitRet
)
{
  struct DCOPMsg * pMsg = 0L;
  char * buf = 0L;
  char * senderId = 0L;
  char * app = 0L;
  char * objId = 0L;
  char * fun = 0L;
  char * pos = 0L;
  int dataLength = 0L;

  /* Avoid unused param warnings */
  (void)sizeof(clientObject);
  (void)sizeof(swap);
  (void)sizeof(replyWait);
  (void)sizeof(replyWaitRet);

  if (0 == dcop_callback) {
    fprintf(stderr, "No callback registered\n");
    return;
  }

  IceReadMessageHeader(iceConn, sizeof(struct DCOPMsg), struct DCOPMsg, pMsg);

  switch (opcode) {

    case DCOPSend:

      buf = (char *)malloc(length);
      IceReadData(iceConn, length, buf);
      /* Now read strings 'senderID', 'app', 'objId' and 'fun' from buf. */

      pos = buf;

      pos = dcop_read_string(pos, senderId);
      free(senderId); /* Don't need this right now */
      pos = dcop_read_string(pos, app);
      pos = dcop_read_string(pos, objId);
      pos = dcop_read_string(pos, fun);
      pos = dcop_read_int(pos, &dataLength);

      /* Run user-provided callback. */
      dcop_callback(app, objId, fun, pos, (unsigned)dataLength);

      free(app);
      free(objId);
      free(fun);
      free(buf);

      break;

    case DCOPCall:
      fprintf(stderr, "DCOPCall not yet implemented\n");
    default:
      fprintf(stderr, "Ignoring this opcode\n");
      break;
  }
}

  Bool
dcop_call(
  const char * remApp,
  const char * remObjId,
  const char * remFun,
  const char * data,
  int dataLength,
  char * replyType,
  char * replyData,
  int replyDataLength)
{
  char * outputData = 0L;
  int outputDataLength = 0;
  Bool success = False;
  int datalen = 0;
  IceReplyWaitInfo waitInfo;
  Bool readyRet = False;
  IceProcessMessagesStatus status;
  unsigned long replyStatus = -1;

  if (0 == dcop_attached)
    return -1;

  while (1) {

    struct DCOPMsg * pMsg;

/*    outputData << d->appId << remApp << remObjId <<
 *    normalizeFunctionSignature(remFun) << data.size(); */
/*    And set outputDataLength */

    IceGetHeader(
      dcop_ice_conn,
      dcop_major_opcode,
      DCOPCall,
      sizeof(struct DCOPMsg),
      struct DCOPMsg,
      pMsg
    );

    pMsg->time = dcop_time;
    datalen = outputDataLength + dataLength;
    pMsg->length += datalen;

    IceSendData(dcop_ice_conn, outputDataLength, outputData);
    IceSendData(dcop_ice_conn, dataLength, (char *)data);

    if (IceConnectionStatus(dcop_ice_conn) != IceConnectAccepted)
      return -1;

    IceFlush(dcop_ice_conn);

    waitInfo.sequence_of_request = IceLastSentSequenceNumber(dcop_ice_conn);
    waitInfo.major_opcode_of_request = dcop_major_opcode;
    waitInfo.minor_opcode_of_request = DCOPCall;

    replyType = &replyType;
    replyData = &replyData;

    /* Hmm... need to figure out what this is for */
/*    waitInfo.reply = (IcePointer)&tmpReplyStruct; */

    do {
      
      status = IceProcessMessages(dcop_ice_conn, &waitInfo, &readyRet);

      if (status == IceProcessMessagesIOError) {
        IceCloseConnection(dcop_ice_conn);
        return False;
      }

    } while (!readyRet);

    /* if we were rejected by the server, we try again, otherwise we return */
    if (replyStatus == 2 /* Rejected */)
      continue;

    success = (replyStatus == 1 /* Ok */) ? True : False;

    break;
  }

  return success;
}



/* vim: set ts=2:sw=2:tw=78: */
