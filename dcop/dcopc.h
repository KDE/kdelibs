/*
 * C interface to DCOP
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 */

#ifndef __dcopc_h__
#define __dcopc_h__

#define DCOP_OK 0
#define DCOP_ERROR_SERVER_NOT_FOUND 1
#define DCOP_ERROR_CANNOT_REGISTER_WITH_ICE 2
#define DCOP_ERROR_CANNOT_CONTACT_SERVER 3
#define DCOP_ERROR_SERVER_REFUSED_CONNECTION 4
#define DCOP_ERROR_SERVER_REFUSED_DATA 5

char * dcop_write_int(char * buf, int);

char * dcop_write_string(char * buf, const char *);

  int
dcop_send_signal(
  const char * receiving_app,
  const char * object,
  const char * function,
  char * data,
  int data_length
);

#endif /* __dcopc_h__ */
