/* vi: set ts=2 sw=2 tw=78:
 *
 * C interface to DCOP
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* sometimes __STDC__ is defined, but to 0. The hateful X headers
   ask for '#if __STDC__', so they become confused. */
#if defined(__STDC__)
#if !__STDC__
#undef __STDC__
#define __STDC__ 1
#endif
#endif

/* We really don't want to require X11 headers...  */
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xmd.h>

#include <X11/ICE/ICE.h>
#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEconn.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>

#include "dcopglobal.h"
#include "dcopc.h"


#define BUFFER_SIZE 1024

enum {
  DCOP_REPLY_PENDING,
  DCOP_REPLY_OK,
  DCOP_REPLY_FAILED
};

struct dcop_reply_struct
{
  unsigned long status;
  char       ** replyType;
  char       ** replyData;
  int         * replyDataLength;
  int           replyId;
};

  void
dcop_process_message(
  IceConn             iceConn,
  IcePointer          clientObject,
  int                 opcode,
  unsigned long       length,
  Bool                swap,
  IceReplyWaitInfo  * replyWait,
  Bool              * replyWaitRet
);

Bool dcop_attach_internal(Bool register_as_anonymous);
Bool dcop_ice_register(void);
Bool dcop_connect(void);
Bool dcop_protocol_setup(void);

char * dcop_write_int     (char * buf, int i);
char * dcop_read_int      (char * buf, int * i);
char * dcop_write_string  (char * buf, const char * text);
char * dcop_read_string   (char * buf, char ** output);

static char           * dcop_requested_name = 0;
static char           * dcop_app_name       = 0;
static int              dcop_major_opcode   = 0;
static IceConn          dcop_ice_conn       = 0L;
static CARD32           dcop_key           = 0;
static int              dcop_reply_id       = 0;

static IcePoVersionRec DCOPVersions[] = {
  { DCOPVersionMajor, DCOPVersionMinor, dcop_process_message }
};

/***************************************************************************/

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

/***************************************************************************/

  char *
dcop_read_int(char * buf, int * i)
{
  char *p = (char *)i;

  *p++ = buf[3];
  *p++ = buf[2];
  *p++ = buf[1];
  *p   = buf[0];

  return buf + 4;
}

/***************************************************************************/

  char *
dcop_write_string(char * buf, const char * text)
{
  char * pos = buf;
  int l = strlen( text ) + 1; /* we need the \0! (Simon) */
  pos = dcop_write_int(buf, l);
  memcpy(pos, text, l);
  return pos + l;
}

/***************************************************************************/

  char *
dcop_read_string(char * buf, char ** output)
{
  int length;
  char * pos = dcop_read_int(buf, &length);
  fprintf(stderr, "dcop_read_string: length == %d\n", length);

  *output = (char *)malloc(length);
  if (*output == NULL)
    return pos;

  memcpy(*output, pos, length);
  return pos + length;
}

/***************************************************************************/

  Bool
dcop_register_callback(const char * object_id, dcop_callback_t callback)
{
  /* STUB */

  /* Avoid unused param warnings */
  object_id = object_id;
  callback = callback;
  return False;

  /*
   * TODO:
   * Map the given object id to the given callback internally, so that when we
   * receive a message, we know where to send it. Or... should we just allow
   * one callback only, and pass that all calls ? Depends whether I can be
   * bothered to figure out how to do a 'map' in C.
   */
}

/***************************************************************************/

  void
dcop_process_message(
  IceConn             iceConn,
  IcePointer          clientObject,
  int                 opcode,
  unsigned long       length,
  Bool                swap,
  IceReplyWaitInfo  * replyWait,
  Bool              * replyWaitRet
)
{
  struct DCOPMsg * pMsg = 0L;
  Status status = False;

  char  * buf         = 0L;
  char  * senderId    = 0L;
  char  * app         = 0L;
  char  * objId       = 0L;
  char  * fun         = 0L;
  char  * pos         = 0L;
  char  * replyType   = 0L;

  int     dataLength  = 0;

  unsigned long replyVal  = 0L;

  /* Avoid unused param warnings */
  clientObject = clientObject;
  swap = swap;

  if (0 == replyWait) {
    fprintf(stderr, "dcop_process_message(): replyWait is 0\n");
  }

  if (iceConn != dcop_ice_conn) {
    fprintf(stderr, "dcop_process_message(): ICE connection does not match\n");
    return;
  }

  IceReadMessageHeader(
    dcop_ice_conn,
    sizeof(struct DCOPMsg),
    struct DCOPMsg,
    pMsg
  );


  switch (opcode) {

    case DCOPReply:
      fprintf(stderr, "dcop_process_message(): DCOPReply received\n");

      fprintf(stderr, "dcop_process_message(): length == %ld\n", length);
      buf = (char *)malloc(length);
      if (buf == NULL)
        return;
      status = IceReadData(dcop_ice_conn, length, buf);
      if (False == status) {
        fprintf(stderr, "dcop_process_message(): IceReadData failed\n");
      }

      fprintf(stderr, "dcop_process_message(): Reading data\n");
      pos = buf;
      pos = dcop_read_string(pos, &replyType);
      fprintf(stderr, "dcop_process_message(): replyType : `%s'\n", replyType);

      /* TODO: Run user-provided callback. */

      free(replyType);

      replyVal = DCOP_REPLY_OK;
      break;

    case DCOPReplyFailed:
      fprintf(stderr, "dcop_process_message(): DCOPReplyFailed received\n");
      break;

    case DCOPReplyWait:
      fprintf(stderr, "dcop_process_message(): DCOPReplyWait received\n");
      break;

    case DCOPReplyDelayed:
      fprintf(stderr, "dcop_process_message(): DCOPReplyDelayed received\n");
      break;

    case DCOPFind:
      fprintf(stderr, "dcop_process_message(): DCOPFind received\n");
      break;

    case DCOPSend:

      fprintf(stderr, "dcop_process_message(): DCOPSend received\n");

      buf = (char *)malloc(length);
      if (buf == NULL)
        return;

      IceReadData(dcop_ice_conn, length, buf);

      pos = buf;
      pos = dcop_read_string(pos, &senderId);
      pos = dcop_read_string(pos, &app);
      pos = dcop_read_string(pos, &objId);
      pos = dcop_read_string(pos, &fun);
      pos = dcop_read_int(pos, &dataLength);

      /* TODO: Run user-provided callback. */

      free(senderId);
      free(app);
      free(objId);
      free(fun);
      free(buf);

      replyVal = DCOP_REPLY_OK;
      break;

    case DCOPCall:
      fprintf(stderr, "dcop_process_message(): DCOPCall not yet implemented\n");
      break;

    default:
      fprintf(stderr, "dcop_process_message(): Invalid opcode %d\n", opcode);
      break;
  }

  if (0 != replyWait)
    ((struct dcop_reply_struct *)replyWait->reply)->status = replyVal;

  fprintf(stderr, "dcop_process_message(): Setting replyWaitRet = True\n");
  *replyWaitRet = True;
  fprintf(stderr, "dcop_process_message(): Returning\n");
}

/***************************************************************************/

  Bool
dcop_send_signal(
  const char  * receiving_app,
  const char  * object,
  const char  * function,
  char        * data,
  int           dataLength
)
{
  char        * pos           = 0L;
  char        * header        = 0L;
  unsigned int  headerLength  = 0;

  struct DCOPMsg * pMsgPtr = 0;

  static const char sAnonymous = "anonymous";

  if (0 == dcop_ice_conn) {
    fprintf(stderr, "Try running dcop_attach(), moron\n");
    return False;
  }

  /*
   * First let ICE initialize the ICE Message Header and give us a pointer to
   * it (ICE manages that buffer internally)
   */
  IceGetHeader(
    dcop_ice_conn,
    dcop_major_opcode,
    DCOPSend,
    sizeof(struct DCOPMsg),
    struct DCOPMsg,
    pMsgPtr
  );

  /*
   * Marshall the arguments for the DCOP message header (callerApp, destApp,
   * destObj, destFunc. The last argument is actually part of the data part of
   * the call, but we add it to the header. It's the size of the marshalled
   * argument data. In Qt it would look like QDataStream str( ... ) str <<
   * callerApp << destApp << destObj << destFun <<
   * argumentQByteArrayDataStuff; (where as str is the complete data stream
   * sent do the dcopserver, excluding the ICE header) As the QByteArray is
   * marshalled as [size][data] and as we (below) send the data in two chunks,
   * first the dcop msg header and the the data, we just put the [size] field
   * as last field into the dcop msg header ;-)
   */

  headerLength = strlen(sAnonymous) + 1 +
                 strlen(receiving_app) + 1 +
                 strlen(object) + 1 +
                 strlen(function) + 1 +
                 4*5;  /* 4 string lengths + 1 int */

  header = (char *)malloc(headerLength);
  if (header == NULL)
    return False;

  pos = header;

  pos = dcop_write_string(pos, sAnonymous);
  pos = dcop_write_string(pos, receiving_app);
  pos = dcop_write_string(pos, object);
  pos = dcop_write_string(pos, function);
  pos = dcop_write_int(pos, dataLength);

  headerLength = pos - header;

  pMsgPtr->key = dcop_key;
  /*
   * The length field tells the dcopserver how much bytes the dcop message
   * takes up. We add that size to the already by IceGetHeader initialized
   * length value, as it seems that under some circumstances (depending on the
   * DCOPMsg structure size) the length field is aligned/padded.
   */
  pMsgPtr->length += headerLength + dataLength;

  /* First let's send the dcop message header.
   * IceSendData automatically takes care of first sending the Ice Message
   * Header (outbufptr > outbuf -> flush the connection buffer)
   */
  IceSendData(dcop_ice_conn, headerLength, header);

  /* Now the function argument data */
  IceSendData(dcop_ice_conn, dataLength, data);

  /* Send it all ;-) */
  IceFlush(dcop_ice_conn);

  free(header);

  if (IceConnectionStatus(dcop_ice_conn) != IceConnectAccepted)
    return False;
	
  return True;
}

/***************************************************************************/

  Bool
dcop_call(
  const char  * appId,
  const char  * remApp,
  const char  * remObjId,
  const char  * remFun,
  const char  * data,
  int           dataLength,
  char       ** replyType,
  char       ** replyData,
  int         * replyDataLength
)
{
  IceReplyWaitInfo          waitInfo;
  IceProcessMessagesStatus  status;
  struct dcop_reply_struct  replyStruct;

  char  * pos               = 0L;
  char  * outputData        = 0L;
  int     outputDataLength  = 0;
  int     temp              = 0;
  Bool    success           = False;
  Bool    readyRet          = False;

  struct DCOPMsg * pMsg;


  fprintf(stderr, "dcop_call() ...\n");

  if (0 == dcop_ice_conn) {
    fprintf(stderr, "Try running dcop_register(), moron\n");
    return False;
  }


  temp += strlen(appId);
  temp += strlen(remApp);
  temp += strlen(remObjId);
  temp += strlen(remFun);
  temp += dataLength;
  temp += 1024; /* Extra space for marshalling overhead */

  outputData = (char *)malloc(temp);
  if (outputData == NULL)
    return False;

  temp = 0;

  pos = outputData;
  pos = dcop_write_string(pos, appId);
  pos = dcop_write_string(pos, remApp);
  pos = dcop_write_string(pos, remObjId);
  pos = dcop_write_string(pos, remFun);
  pos = dcop_write_int(pos, dataLength);

  outputDataLength = pos - outputData;

  IceGetHeader(
	       dcop_ice_conn,
	       dcop_major_opcode,
	       DCOPCall,
	       sizeof(struct DCOPMsg),
	       struct DCOPMsg,
	       pMsg
	       );

  pMsg->length += outputDataLength + dataLength;

  IceSendData(dcop_ice_conn, outputDataLength, outputData);
  IceSendData(dcop_ice_conn, dataLength, (char *)data);

  IceFlush(dcop_ice_conn);

  free(outputData);

  if (IceConnectionStatus(dcop_ice_conn) != IceConnectAccepted) {
      fprintf(stderr, "dcop_call(): Connection not accepted\n");
      return False;
  }

  waitInfo.sequence_of_request = IceLastSentSequenceNumber(dcop_ice_conn);
  waitInfo.major_opcode_of_request = dcop_major_opcode;
  waitInfo.minor_opcode_of_request = DCOPCall;

  replyStruct.status = DCOP_REPLY_PENDING;
  replyStruct.replyId = dcop_reply_id++;
  replyStruct.replyType = replyType;
  replyStruct.replyData = replyData;
  replyStruct.replyDataLength = replyDataLength;

  waitInfo.reply = (IcePointer)(&replyStruct);

  readyRet = False;

  do {
      fprintf(stderr, "dcop_call(): Doing IceProcessMessages\n");
      status = IceProcessMessages(dcop_ice_conn, &waitInfo, &readyRet);

      if (status == IceProcessMessagesIOError) {
	  fprintf(stderr, "dcop_call(): IceProcessMessagesIOError\n");
	  IceCloseConnection(dcop_ice_conn);
	  free(outputData);
	  return False;
      }

      fprintf(stderr, "dcop_call(): readyRet == %s\n", readyRet ? "True" : "False");
  } while (!readyRet);

  fprintf(stderr, "dcop_call(): Finished\n");
  return (replyStruct.status == DCOP_REPLY_OK) ? True : False;
}

/***************************************************************************/

  Bool
dcop_attach()
{
  fprintf(stderr, "dcop_attach()\n");
  return dcop_attach_internal(True);
}

  Bool
dcop_attach_internal(Bool register_as_anonymous)
{
  fprintf(stderr,
    "dcop_attach_internal(%s)\n", register_as_anonymous ? "True" : "False");

  if (False == dcop_ice_register())   return False;
  if (False == dcop_connect())        return False;
  if (False == dcop_protocol_setup()) return False;

  if (register_as_anonymous)
    return (0L != dcop_register("anonymous", True)) ? True : False;

  return True;
}

/***************************************************************************/

  char *
dcop_register(const char * app_name, Bool add_pid)
{
  char * replyType  = 0L;
  char * replyData  = 0L;
  int    replyLen   = 0;
  char * data       = 0L;
  char * pos        = 0L;
  int    dataLength = 0;
  Bool   callStatus = False;

  fprintf(stderr, "dcop_register(`%s')\n", app_name);

  if (0 == dcop_app_name) {

    if (0 == dcop_ice_conn)
      if (False == dcop_attach_internal(False))
        return 0L;

  } else {

    fprintf(stderr, "dcop_init(): Reregistering as `%s'\n", app_name);

    callStatus = dcop_detach();

    if (False == callStatus) {
      fprintf(stderr, "dcop_init(): Could not detach before reregistering\n");
      return 0L;
    }
  }

  if (False == add_pid)
    dcop_requested_name = strdup(app_name);

  else {

    /* Leave room for "-pid" */
    int len = strlen(app_name) + 64;
    dcop_requested_name = (char *)malloc(len);
    if (dcop_requested_name == NULL)
      return NULL;

    snprintf(dcop_requested_name, len, "%s-%ld", app_name, (long)getpid());
  }

  data = (char *)malloc(strlen(dcop_requested_name) + 42);
  if (data == NULL)
    return NULL;

  pos = data;
  pos = dcop_write_string(pos, dcop_requested_name);
  dataLength = pos - data;

  callStatus =
    dcop_call(
      dcop_requested_name,
      "DCOPServer",
      "", /* Object irrelevant */
      "registerAs(QCString)",
      data,
      dataLength,
      &replyType,
      &replyData,
      &replyLen
    );

  free(dcop_requested_name);
  free(data);

  if (False == callStatus) {
    fprintf(stderr, "dcop_register(): dcop_call() failed\n");
    return 0L;
  }

  fprintf(stderr, "dcop_register(): Reply length is %d\n", replyLen);

  if (replyLen == 0)
    return 0L;

  dcop_read_string(replyData, &dcop_app_name);

  return dcop_app_name;
}

/***************************************************************************/

  Bool
dcop_ice_register()
{
  dcop_major_opcode =
    IceRegisterForProtocolSetup(
      (char *)("DCOP"),
      (char *)DCOPVendorString,
      (char *)DCOPReleaseString,
      1, /* What's this ? */
      DCOPVersions,
      DCOPAuthCount,
      (char **)DCOPAuthNames,
      DCOPClientAuthProcs,
      0L /* What's this ? */
    );

  return (dcop_major_opcode >= 0) ? True : False;
}


/***************************************************************************/

  Bool
dcop_connect()
{
  size_t      bytesRead     = 0;
  IcePointer  context       = 0;
  FILE *      f             = 0L;
  char      * newline       = 0L;
  char      * homeDir       = 0L;
  char      * display       = 0L;
  char      * dcopServer    = 0L;
  char        errBuf[BUFFER_SIZE];
  char        fileName[BUFFER_SIZE];
  char        hostName[BUFFER_SIZE];
  char        displayName[BUFFER_SIZE];
  char      * i;

  homeDir = getenv("HOME");

  if (NULL == homeDir)
    return False;

  display = getenv("DISPLAY");
  
  if (NULL == display)
    display = "NODISPLAY";

  strncpy(displayName, display, sizeof(displayName));
  displayName[sizeof(displayName) - 1] = 0;

  if((i = strrchr(displayName, '.')) > strrchr(displayName, ':') && i)
      *i = '\0';

  while((i = strchr(displayName, ':')) != NULL)
     *i = '_';

  dcopServer = getenv("DCOPSERVER");

  if (NULL == dcopServer) {

    if (-1 == chdir(homeDir)) {
      fprintf(stderr, "Cannot cd ~\n");
      return False;
    }

    if (gethostname(hostName, 255))
	    strcpy(hostName, "localhost");

    snprintf(fileName, sizeof(fileName), ".DCOPserver_%s_%s", hostName, displayName);
    f = fopen(fileName, "r");

    if (NULL == f) {
      fprintf(stderr, "Cannot open ~/%s\n", fileName);
      return False;
    }

    dcopServer = (char *)malloc(BUFFER_SIZE);
    if (dcopServer == NULL)
      return False;

    bytesRead = fread((void *)dcopServer, sizeof(char), BUFFER_SIZE, f);
    dcopServer[BUFFER_SIZE - 1] = 0;

    if (0 == bytesRead)
      return False;

    newline = strchr(dcopServer, '\n');

    if (NULL == newline) {
      fprintf(stderr, "dcop server file format invalid\n");
      return False;
    }

    *newline = '\0';
  }

  dcop_ice_conn =
    IceOpenConnection(
      dcopServer,
      context,
      False,
      dcop_major_opcode,
      sizeof(errBuf),
      errBuf
    );

  if (NULL != dcopServer)
    free(dcopServer);

  if (0 == dcop_ice_conn) {
    fprintf(stderr, "dcop_ice_conn is 0 :(\n");
    return False;
  }

  IceSetShutdownNegotiation(dcop_ice_conn, False);

  return True;
}

/***************************************************************************/

  Bool
dcop_protocol_setup()
{
  char        * vendor        = 0L;
  char        * release       = 0L;
  IcePointer    clientData    = 0;
  int           majorVersion  = 0;
  int           minorVersion  = 0;
  int           status        = 0;
  char          errBuf[BUFFER_SIZE];

  status =
    IceProtocolSetup(
      dcop_ice_conn,
      dcop_major_opcode,
      clientData,
      True,
      &(majorVersion),
      &(minorVersion),
      &(vendor),
      &(release),
      BUFFER_SIZE,
      errBuf
    );

  return (
    (status == IceProtocolSetupSuccess) &&
    (IceConnectionStatus(dcop_ice_conn) == IceConnectAccepted)
  );
}

/***************************************************************************/

  Bool
dcop_detach()
{
  int status;
  IceProtocolShutdown(dcop_ice_conn, dcop_major_opcode);
  status = IceCloseConnection(dcop_ice_conn);

  if (status == IceClosedNow)
    dcop_ice_conn = 0L;
  else
    fprintf(stderr, "dcop_detach(): Could not detach\n");

  return status == IceClosedNow;
}

