#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define g_mutex_lock	  gsl_arts_mutex_lock
#define g_mutex_trylock	  gsl_arts_mutex_trylock
#define g_mutex_unlock	  gsl_arts_mutex_unlock
#define g_mutex_new	  gsl_arts_mutex_new
#define g_mutex_free	  gsl_arts_mutex_free

#define g_cond_new	  gsl_arts_cond_new
#define g_cond_free	  gsl_arts_cond_free
#define g_cond_signal	  gsl_arts_cond_signal
#define g_cond_broadcast  gsl_arts_cond_broadcast
#define g_cond_wait	  gsl_arts_cond_wait

#define g_thread_create	  gsl_arts_thread_create
#define g_thread_self	  gsl_arts_thread_self

#define	g_thread_init	  gsl_arts_thread_init

gpointer  gsl_arts_mutex_new ();
void	  gsl_arts_mutex_free (gpointer mutex);
void	  gsl_arts_mutex_lock (gpointer mutex);
gboolean  gsl_arts_mutex_trylock (gpointer mutex);
void	  gsl_arts_mutex_unlock (gpointer mutex);

gpointer  gsl_arts_cond_new ();
void	  gsl_arts_cond_free (gpointer cond);
void	  gsl_arts_cond_signal (gpointer cond);
void	  gsl_arts_cond_broadcast (gpointer cond);
void	  gsl_arts_cond_wait (gpointer cond, gpointer mutex);

gpointer  gsl_arts_thread_create (gpointer (*func)(gpointer data),
				  gpointer data,
				  gboolean joinable,
				  gpointer junk);
gpointer  gsl_arts_thread_self ();

void      gsl_arts_thread_init (gpointer arg);


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* vim:set ts=8 sts=2 sw=2: */
