/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "lockfreequeue_p.h"
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <stdlib.h>
#include "globalstatic_p.h"

struct MemoryPool
{
    MemoryPool() : size(0) {}
    ~MemoryPool();
    // Stack structure:
    QAtomicPointer<LockFreeQueueBase::NodeBase> stack;
    QAtomicInt count;
    size_t size;
    const int *poolSize;

    void clear();
};

void MemoryPool::clear()
{
    LockFreeQueueBase::NodeBase *node = stack;
    while (node) {
        if (stack.testAndSetAcquire(node, const_cast<LockFreeQueueBase::NodeBase *>(node->next))) {
            count.deref();
            free(node);
        }
        node = stack;
    }
}

MemoryPool::~MemoryPool()
{
    LockFreeQueueBase::NodeBase *node = stack;
    while (node) {
        void *toDelete = node;
        node = const_cast<LockFreeQueueBase::NodeBase *>(node->next);
        ::free(toDelete);
    }
}

struct MemoryPoolVector
{
    static const int POOL_COUNT = 16;
    MemoryPoolVector() : next(0), poolSize(128) {}
    ~MemoryPoolVector() { delete next; }
    inline MemoryPool &operator[](size_t s)
    {
        for (int i = 0; i < POOL_COUNT; ++i) {
            if (m_pools[i].size == s) {
                return m_pools[i];
            } else if (m_pools[i].size == 0) {
                QMutexLocker lock(&m_mutex);
                if (m_pools[i].size == 0) {
                    m_pools[i].size = s;
                    m_pools[i].poolSize = &poolSize;
                    return m_pools[i];
                }
                if (m_pools[i].size == s) {
                    return m_pools[i];
                }
            }
        }
        if (!next) {
            QMutexLocker lock(&m_mutex);
            if (!next) {
                next = new MemoryPoolVector;
            }
        }
        return (*next)[s];
    }

    void clearAllPools()
    {
        for (int i = 0; i < POOL_COUNT; ++i) {
            if (0 == m_pools[i].size) {
                return;
            }
            m_pools[i].clear();
        }
        if (next) {
            next->clearAllPools();
        }
    }

    MemoryPool m_pools[POOL_COUNT];
    MemoryPoolVector *next;
    QMutex m_mutex;
    int poolSize;
};

PHONON_GLOBAL_STATIC(MemoryPoolVector, s_memoryPool)

void *LockFreeQueueBase::NodeBaseKeepNodePool::operator new(size_t s)
{
    MemoryPool &p = (*s_memoryPool)[s];
    NodeBase *node = p.stack;
    if (node) {
        if (!p.stack.testAndSetAcquire(node, const_cast<NodeBase *>(node->next))) {
            return ::malloc(s);
        }
        p.count.deref();
        return node;
    }
    return ::malloc(s);
}

void LockFreeQueueBase::NodeBaseKeepNodePool::operator delete(void *ptr, size_t s)
{
    MemoryPool &p = (*s_memoryPool)[s];
    if (p.count > *p.poolSize) {
        ::free(ptr);
        return;
    }
    NodeBase *node = static_cast<NodeBase *>(ptr);
    NodeBase *next = p.stack;
    node->next = next;
    if (!p.stack.testAndSetOrdered(next, node)) {
        ::free(ptr);
        return;
    }
    p.count.ref();
}

void LockFreeQueueBase::NodeBaseKeepNodePool::clear()
{
    s_memoryPool->clearAllPools();
}

void LockFreeQueueBase::NodeBaseKeepNodePool::setPoolSize(int s)
{
    s_memoryPool->poolSize = s;
}

int LockFreeQueueBase::NodeBaseKeepNodePool::poolSize()
{
    return s_memoryPool->poolSize;
}

class LockFreeQueueBasePrivate
{
    public:
        LockFreeQueueBasePrivate();
        ~LockFreeQueueBasePrivate();
        QMutex dataReadyHandlerMutex;
        LockFreeQueueBase::NodeBase sentinel; // end marker
        LockFreeQueueBase::NodeBase *lastHeadNode;
        QAtomicPointer<LockFreeQueueBase::NodeBasePointer> queueHead;
        QAtomicPointer<LockFreeQueueBase::NodeBasePointer> queueTail;
        QAtomicInt size;
        LockFreeQueueBase::DataReadyHandler *dataReadyHandler;
};

LockFreeQueueBasePrivate::LockFreeQueueBasePrivate()
    : lastHeadNode(new LockFreeQueueBase::NodeBase(&sentinel)),
    queueHead(&lastHeadNode->next),
    queueTail(&lastHeadNode->next),
    size(0),
    dataReadyHandler(0)
{
    // let d->sentinel point to itself so that we can use d->sentinel.next as
    // QAtomicPointer<Node> for d->queueHead and d->queueTail
    sentinel.next = &sentinel;
}

LockFreeQueueBasePrivate::~LockFreeQueueBasePrivate()
{
    Q_ASSERT(queueHead);
    LockFreeQueueBase::NodeBase *node = lastHeadNode;
    while (node != &sentinel) {
        LockFreeQueueBase::NodeBase *toDelete = node;
        node = const_cast<LockFreeQueueBase::NodeBase *>(node->next);
        delete toDelete;
    }
}

LockFreeQueueBase::LockFreeQueueBase()
    : d(new LockFreeQueueBasePrivate)
{
}

LockFreeQueueBase::~LockFreeQueueBase()
{
    delete d;
}

void LockFreeQueueBase::setDataReadyHandler(DataReadyHandler *h)
{
    QMutexLocker lock(&d->dataReadyHandlerMutex);
    d->dataReadyHandler = h;
}

void LockFreeQueueBase::_enqueue(NodeBase *newNode)
{
    newNode->next = &d->sentinel;
    NodeBasePointer &lastNextPointer = *d->queueTail.fetchAndStoreAcquire(&newNode->next);
    lastNextPointer = newNode;
    d->size.ref();

    if (d->dataReadyHandler) {
        QMutexLocker lock(&d->dataReadyHandlerMutex);
        if (d->dataReadyHandler) {
            d->dataReadyHandler->dataReady();
        }
    }
}

LockFreeQueueBase::NodeBase *LockFreeQueueBase::_acquireHeadNodeBlocking()
{
    NodeBasePointer *node = 0;
    while (d->size > 0) {
        if ((node = d->queueHead.fetchAndStoreRelaxed(&d->sentinel.next)) != &d->sentinel.next) {
            // node is a pointer to a Node::next member pointing to the first entry in the list
            if (*node != &d->sentinel) {
                d->size.deref();
                return const_cast<NodeBase *>(*node);
            }
            // empty (d->size == 0), put it back
            const bool check = d->queueHead.testAndSetRelaxed(&d->sentinel.next, node);
            Q_ASSERT(check); Q_UNUSED(check);
            // try again, with some luck d->size is > 0 again
        }
    }
    return 0;
}

LockFreeQueueBase::NodeBase *LockFreeQueueBase::_acquireHeadNode()
{
    if (*d->queueHead == &d->sentinel || d->queueHead == &d->sentinel.next) {
        return 0;
    }
    // setting d->queueHead to &d->sentinel.next makes the above check fail (i.e. all
    // other threads in a dequeue function will exit). Also enqueue will not modify
    // this as d->queueTail references d->lastHeadNode->next which != d->sentinel.next
    NodeBasePointer *node = d->queueHead.fetchAndStoreRelaxed(&d->sentinel.next);
    if (node == &d->sentinel.next) {
        // Another thread got the real node, we just got the placeholder telling us to not touch
        // anything. As we replaced &d->sentinel.next with &d->sentinel.next in
        // d->queueHead we don't have to reset anything.
        return 0;
    }
    // node is a pointer to a Node::next member pointing to the first entry in
    // the list
    if (*node == &d->sentinel) {
        //qDebug() << "empty, put it back";
        const bool check = d->queueHead.testAndSetRelaxed(&d->sentinel.next, node);
        Q_ASSERT(check); Q_UNUSED(check);
        return 0;
    }
    d->size.deref();
    return const_cast<NodeBase *>(*node);
}

void LockFreeQueueBase::_releaseHeadNode(NodeBase *node)
{
    NodeBase *toDelete = d->lastHeadNode;
    d->lastHeadNode = node;
    const bool check = d->queueHead.testAndSetRelease(&d->sentinel.next, &node->next);
    Q_ASSERT(check); Q_UNUSED(check);
    delete toDelete;
}

int LockFreeQueueBase::size() const
{
    return d->size;
}
