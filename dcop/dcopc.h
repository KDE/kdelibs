/*
 * C interface to DCOP
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 */

#ifndef __dcopc_h__
#define __dcopc_h__

#undef Bool
#undef True
#undef False
#define Bool int
#define True 1
#define False 0

typedef void (*dcop_callback_t)(
  const char * object_id,
  const char * function,
  const char * data,
  unsigned int data_length
);
  
  Bool
dcop_attach();

  char *
dcop_register(const char * app_name, Bool add_pid);

  Bool
dcop_detach();
  
  Bool
dcop_register_callback(const char * object_id, dcop_callback_t callback);

  Bool
dcop_send_signal(
  const char * receiving_app,
  const char * object,
  const char * function,
  char * data,
  int data_length
);

  Bool
dcop_call(
  const char * app_name,
  const char * remote_app_name,
  const char * remote_object_id,
  const char * remote_function,
  const char * data,
  int data_length,
  char ** reply_type,
  char ** reply_data,
  int * reply_data_length
);

#endif /* __dcopc_h__ */
