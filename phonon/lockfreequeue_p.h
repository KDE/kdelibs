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

#ifndef LOCKFREEQUEUE_P_H
#define LOCKFREEQUEUE_P_H

#include <QtCore/QVector>

class LockFreeQueueBasePrivate;
struct MemoryPool;
class LockFreeQueueBase
{
    friend class LockFreeQueueBasePrivate;
    public:
        struct DataReadyHandler
        {
            virtual ~DataReadyHandler() {}
            virtual void dataReady() = 0;
        };

        void setDataReadyHandler(DataReadyHandler *);

        int size() const;
        bool isEmpty() const { return 0 == size(); }

    protected:
        friend struct MemoryPool;
        LockFreeQueueBase();
        ~LockFreeQueueBase();

    public:
        struct NodeBase;
        typedef volatile NodeBase * NodeBasePointer;
        struct NodeBase
        {
            inline NodeBase() {}
            inline NodeBase(NodeBase *n) : next(n) {}
            virtual ~NodeBase() {}
            NodeBasePointer next;
        };

        struct NodeBaseKeepNodePool : public NodeBase
        {
            // allocation is a bottleneck in _enqueue
            void *operator new(size_t s);
            void operator delete(void *p, size_t s);

            static void clear();
            static void setPoolSize(int);
            static int poolSize();
        };

        typedef NodeBase             StdNewDeleteMemoryManagement;
        typedef NodeBaseKeepNodePool KeepNodePoolMemoryManagement;
    protected:

        void _enqueue(NodeBase *);
        NodeBase *_acquireHeadNode();
        NodeBase *_acquireHeadNodeBlocking();
        void _releaseHeadNode(NodeBase *node);

        LockFreeQueueBasePrivate *const d;

};

template<class T, class MemoryManagementNodeBase = LockFreeQueueBase::StdNewDeleteMemoryManagement>
class LockFreeQueue : public LockFreeQueueBase
{
    public:
        enum BlockingSwitch {
            BlockUnlessEmpty,
            NeverBlock
        };

        struct Node : public MemoryManagementNodeBase
        {
            inline Node(const T &d) : data(d) {}
            T data;
        };

        inline void enqueue(const T &data)
        {
            _enqueue(new Node(data));
        }

        inline void dequeue(QVector<T> &data, BlockingSwitch block = BlockUnlessEmpty)
        {
            int count = 0;
            while (count < data.capacity()) {
                NodeBase *node = (block == NeverBlock) ? _acquireHeadNode() : _acquireHeadNodeBlocking();
                if (!node) {
                    break;
                }
                if (count < data.size()) {
                    data[count] = static_cast<Node *>(node)->data;
                } else {
                    data << static_cast<Node *>(node)->data;
                }
                ++count;
                _releaseHeadNode(node);
            }
            data.resize(count);
        }

        inline bool dequeue(T *data, BlockingSwitch block = BlockUnlessEmpty)
        {
            NodeBase *node = (block == NeverBlock) ? _acquireHeadNode() : _acquireHeadNodeBlocking();
            if (node) {
                *data = static_cast<Node *>(node)->data;
                _releaseHeadNode(node);
                return true;
            }
            return false;
        }

        inline LockFreeQueue<T> &operator<<(const T &data) { enqueue(data); return *this; }
        inline LockFreeQueue<T> &operator>>(T &data)
        {
            NodeBase *node = _acquireHeadNodeBlocking();
            if (node) {
                data = static_cast<Node *>(node)->data;
                _releaseHeadNode(node);
            }
            return *this;
        }
};

#endif // LOCKFREEQUEUE_P_H
