/*
 * warning: the aRts C API is not yet finished and WILL CHANGE - this 
 * is here for experimental purposes ...
 */
#define E_ARTS_NOSERVER -1
#define E_ARTS_NOCONNECTION -2

#ifdef __cplusplus
extern "C" {
#endif

int arts_init();
int arts_connect();
int arts_write(/* missing fd here, */ void *buffer,int count);

#ifdef __cplusplus
}
#endif
