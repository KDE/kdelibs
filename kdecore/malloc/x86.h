#include <sched.h>
#include <time.h>

typedef struct {
  volatile unsigned int lock;
  int pad0_;
} mutex_t;

#define MUTEX_INITIALIZER          { 0, 0 }

static __inline__ int lock(mutex_t *m) {
  int cnt = 0, r;
  struct timespec tm;

  for(;;) {
    __asm__ __volatile__
      ("xchgl %0, %1"
       : "=r"(r), "=m"(m->lock)
       : "0"(1), "m"(m->lock)
       : "memory");
    if(!r)
      return 0;
#ifdef _POSIX_PRIORITY_SCHEDULING
    if(cnt < 50) {
      sched_yield();
      cnt++;
    } else
#endif
        {
      tm.tv_sec = 0;
      tm.tv_nsec = 2000001;
      nanosleep(&tm, NULL);
      cnt = 0;
    }
  }
}

static __inline__ int unlock(mutex_t *m) {
  __asm __volatile ("movl $0,%0" : "=m" (m->lock));
  return 0;
}
