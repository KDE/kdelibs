#include "thread.h"
#include "debug.h"
#include "gslglib.h"
#include <pthread.h>

gpointer
gsl_arts_mutex_new ()
{
  return new Arts::Mutex;
}

void
gsl_arts_mutex_free (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  delete m;
}

void
gsl_arts_mutex_lock (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  m->lock();
}

gboolean
gsl_arts_mutex_trylock (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  return m->tryLock();
}

void
gsl_arts_mutex_unlock (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  m->unlock();
}

gpointer
gsl_arts_cond_new ()
{
  return new Arts::ThreadCondition;
}

void
gsl_arts_cond_free (gpointer cond)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  delete c;
}

void
gsl_arts_cond_signal (gpointer cond)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  c->wakeOne();
}

void
gsl_arts_cond_broadcast (gpointer cond)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  c->wakeAll();
}

void
gsl_arts_cond_wait (gpointer cond, gpointer mutex)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);

  c->wait(*m);
}

class GslArtsThread : public Arts::Thread {
protected:
  gpointer (*func)(gpointer data);
  gpointer data;
  gpointer result;

public:
  GslArtsThread(gpointer (*func)(gpointer data), gpointer data)
    : func(func), data(data)
  {
  }
  void run()
  {
    result = func(data);
  }
};


/* KCC (KAI C++) is buggy.  If we write out the type of the first argument
   to gsl_arts_thread_create(), ala
     gsl_arts_thread_create (gpointer (*func) (gpointer data2), ...)
   it becomes C++ linkage, i.e. it's name gets mangled, _despite_ declared
   extern "C" in the header.  Other sources only calling this function,
   i.e. those only seeing the prototype correctly call the unmangled
   extern "C" variant, but for the above reason it isn't defined anywhere.
   The solution is to go through a typedef for that argument, _which must
   also be declared extern "C"_.  I'm not sure, but I think it's an error
   of KCC, that it puts the invisible type of the first arg into C++ mode,
   although the whole function should be C only.  If one declares
   two equal function types, one extern "C", one not, they are even assignment
   compatible.  But somehow KCC puts that type into C++ mode, which for
   other strange reasons force the whole function to go into C++ linkage.
   It's enough, when this typedef is local to this file.  (matz)  */
   
/* Due to gcc's unhappyness with 'extern "C" typedef ...' we enclose
   it in a real extern "C" {} block.  */
extern "C" {
typedef gpointer (*t_func)(gpointer data2);
}

gpointer
gsl_arts_thread_create (t_func func,
			gpointer data,
			gboolean joinable,
			gpointer junk)
{
  return new GslArtsThread(func, data);
}

gpointer
gsl_arts_thread_self ()
{
  pthread_t id;

  Arts::SystemThreads::the()->getCurrentThread(&id);
  return (gpointer)id; /* assuming this -can- be casted */
}

void 
gsl_arts_thread_init (gpointer arg)
{
  // FIXME: what to do about apps which are not threaded but nevertheless
  // want to use the engine?
  arts_return_if_fail (Arts::SystemThreads::the()->supported() == true);
}

/* vim:set ts=8 sts=2 sw=2: */
