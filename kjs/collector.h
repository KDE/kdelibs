/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJSCOLLECTOR_H_
#define _KJSCOLLECTOR_H_

#include <stdlib.h>

namespace KJS {

  class CollectorBlock;

  /**
   * @short Garbage collector.
   */
  class Collector {
    CollectorBlock* root;
    CollectorBlock* currentBlock;
    int count;
    enum { BlockSize = 1000 };
    /**
     * Pointer to the current collector instance.
     */
    static Collector *curr;
    // disallow direct construction/destruction
    Collector();
  public:
    /**
     * Destructor. Will call @ref collect() before destruction.
     */
    ~Collector();
    /**
     * Create and initialize an instance of the garbage collector.
     * In case you are not interested in obtaining a handle this call may
     * be omitted as it will be called implicitly on the first @ref allocate()
     * call.
     * @return A pointer to the newly created instance.
     */
    static Collector* init();
    /**
     * @return The current collector instance.
     */
    static Collector* current() { return curr; }
    /**
     * Query the current collector and attach to the specified collector.
     * @param c Pointer to a collector created earlier with @ref init().
     */
    static void attach(Collector *c) { curr = c; }
    /**
     * Detach from the current collector. The next call to @ref allocate()
     * will create a new instance.
     */
    static void detach() { curr = 0L; }
    /**
     * Register an object with the collector. The following assumptions are
     * made:
     * @li the operator new() of the object class is overloaded.
     * @li the correct size of memory has been reserved with a malloc() call.
     * @li operator delete() has been overloaded as well and does not free
     * the memory on its own.
     *
     * @param p Pointer to the memory allocated for the object.
     * @return A pointer identical to parameter p.
     */
    static void* allocate(size_t s);
    /**
     * Run the garbage collection. This involves calling the delete operator
     * on each object and freeing the used memory.
     * In the current implemenation this will basically free all registered
     * object regardless whether they are still references by others or not.
     * 
     */
    static void collect();
    int size() const { return count; }

#ifdef KJS_DEBUG_MEM
    /**
     * @internal
     */
    bool collecting;
#endif
  private:
    void privateCollect();
  };

};

#endif
