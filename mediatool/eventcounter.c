#include "mediatool.h"

/*
 * eventcounter.c
 *
 * This is the source, which deals with event counters. Event counters
 * are used in the mediatool protocol as a high-level representation
 * of key presses.
 *
 * Everything in this source seems to be boring stuff, but please note
 * the following: The event counters are in shared memory regions. This
 * means, event counters are fragile constructions, which need some care.
 * A little thoughtlessness may lead to wierd bugs. This is one of the
 * reasons, why the event counter service functions are provided.
 *
 *
 * !!! Masters as well as clients may access eventcounters only by   !!!
 * !!! use of this functions. If you need to do something special    !!!
 * !!! with the event counters, try it as follows:                   !!!
 * !!! 1) Try to simulate your task with the existing functions      !!!
 * !!! 2) If this does not work, write a new service function.       !!!
 * !!!    Please check with the author first, perhaps work on your   !!!
 * !!!    problem is already in progress.                            !!!
 *
 *
 * Internal documentation: The event counter structure items must(!)
 * be unsigned, as I allow *carryovers*. This means, I definitely allow
 * a value of (2^32-1) to go to 0. The intrinsic integer arithmetics
 * guarantee, that my service functions work fine.
 * If signed values were used instead, I would get *overflows*, which
 * could result in serious crap.
 */


#define min(a,b) ((a<b) ? (a):(b))

/******************************************************************************
 *
 * Function:	EventCounterRaise()
 *
 * Task:	Increment the value of the event counter by "count".
 *		May only be called by writer.
 *
 * in:		count	increment value
 * 
 * out:		-
 *
 *****************************************************************************/
void  EventCounterRaise(EventCounter *evc, uint32 count)
{
  evc->current += count;
}


/******************************************************************************
 *
 * Function:	EventCounterRead()
 *
 * Task:	Read the number of events since the last visit.
 *		May only be called by reader.
 *
 * in:		evc	The event counter to be checked.
 *		max	Return a maximum of "max" events. If there are more
 *			than "max" events pending, they will be stored for
 *			future reading. 0 denotes an unrestricted "max" value.
 *
 * out:		uint32	The number of events since the last visit. This value
 *			is never bigger than "max".
 *
 *****************************************************************************/
uint32 EventCounterRead(EventCounter *evc, uint32 max)
{
  uint32 tmp;

  /* I must assure, that "current" is read before "last" */
  tmp  = evc->current;
  tmp -= evc->last;

  /*
   * Restrict number of events to a maximum of "max". Do not restrict, if
   * max==0 (max==0 denotes unrestricted reading).
   */
  if (max != 0)
    tmp = min(tmp,max);

  /*
   * Update the "last" item. This will work, even if EventCounter.current
   * has a different value than the value I read at the start of this
   * function.
   */
  evc->last += tmp;

  /* Return number of events since last visit. */
  return(tmp);
}


/******************************************************************************
 *
 * Function:	EventCounterReset()
 *
 * Task:	Reset the event counter, so that EventCounterRead() will
 *		result in a value of 0. May only be called by reader.
 *
 * in:		evc	The event counter to be reset.
 * 
 * out:		-
 *
 *****************************************************************************/
void  EventCounterReset(EventCounter *evc)
{
  /*
   * This is one of the "tricky" functions. Setting both last and current
   * to 0 is perhaps not a good idea. This would mean, the reader is
   * modifying the current value. But "current" belongs to the writer, who
   * is (perhaps) modifying it just now. This may lead to a race condition.
   *
   * So I just read current and write the value to last. This fulfills the
   * semantics of the function.
   */
  evc->last = evc->current;

}


