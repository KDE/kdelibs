/*
 * C interface to DCOP
 *
 * (C) 2000 Rik Hemsley <rik@kde.org>
 * (C) 2000 Simon Hausmann <hausmann@kde.org>
 */

#ifndef __dcopc_h__
#define __dcopc_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef Bool
#define Bool int
#define True 1
#define False 0
#endif

typedef void (*dcop_callback_t)(
  const char * object_id,
  const char * function,
  const char * data,
  unsigned int data_length
);
  
/**
 * Attach to the DCOP server.
 * This registers you as anonymous-pid - you may then register with a 'real'
 * name with dcop_register().
 */
Bool dcop_attach(void);

/**
 * Register as app 'app_name'.
 * If add_pid is true, you will be registered as app_name-pid.
 *
 * It might not be possible to give you the exact name you requested.
 * In this case, the retval will be different to what you expect, so you
 * should not rely on getting 'app_name'.
 *
 * If it was not possible to register, retval is 0.
 * @param app_name the name of the application.
 * @param add_pid the process id of the application
 * @return the registered name, or 0 when the registration failed
 */
char * dcop_register(const char * app_name, Bool add_pid);

/**
 * Detach from the DCOP server.
 * @return true if successful, false otherwise
 */
Bool dcop_detach(void);

/**
 * Register the callback function for an object id.
 * This function should have signature dcop_callback_t. The name of the
 * actual function that should be called is passed in the struct.
 * @param object_id the object id to register
 * @param callback the callback for the object id
 * @return true if successful, false otherwise
 */
Bool dcop_register_callback(const char * object_id, dcop_callback_t callback);

/**
 * Send a signal to a DCOP object.
 *
 * @param receiving_app     Name the target application is registered under.
 *                          Note that you may use wildcards here. For example,
 *                          you could send to all 'konsole' objects that are
 *                          registered using the '-pid' extension with
 *                          "konsole-*".
 *
 * @param object            Name of the remote object.
 * @param function          Name of the function to call.
 * @param data              Marshalled arguments to pass to function.
 * @param data_length       Number of octets in data.
 * @return true if successful, false otherwise
 */
Bool dcop_send_signal(
  const char * receiving_app,
  const char * object,
  const char * function,
  char * data,
  int data_length
);

/**
 * Call a function of a DCOP object.
 *
 * @param app_name          Name this application is registered under.
 * @param remote_app_name   Name the target application is registered under.
 * @param remote_object_id  Name of the remote object.
 * @param remote_function   Name of the function to call.
 * @param data              Marshalled arguments to pass to function.
 * @param data_length       Number of octets in data.
 * @param reply_type        Will be set to type of retval, represented as a string.
 * @param reply_data        Will be set to retval (marshalled).
 * @param reply_data_length Will be set to number of octets in retval.
 * @return true if successful, false otherwise
 */
Bool dcop_call(
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

#ifdef __cplusplus
}
#endif

#endif /* __dcopc_h__ */
