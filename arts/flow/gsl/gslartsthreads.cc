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

gpointer
gsl_arts_thread_create (gpointer (*func)(gpointer data),
			gpointer data,
			gboolean joinable,
			gpointer junk)
{
  return new GslArtsThread(func, data);
}

gpointer
gsl_arts_thread_self ()
{
  gpointer self = (gpointer) pthread_self();
  return self;
}

void 
gsl_arts_thread_init (gpointer arg)
{
  // FIXME: what to do about apps which are not threaded but nevertheless
  // want to use the engine?
  arts_return_if_fail (Arts::SystemThreads::the()->supported() == true);
}

/* vim:set ts=8 sts=2 sw=2: */
