/*
Copyright (c) 2000 Waldo Bastian <bastian@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <dcopserver.h>
#include <dcopsignals.h>

template class QPtrList<DCOPSignalConnection>;

DCOPSignals::DCOPSignals()
{
   connections.setAutoDelete(true);
}

/**
 * Client "conn" emits the signal "fun" with "data" as arguments.
 *
 * If "excludeSelf" is true, signal is never sent to "conn" itself.
 */
void
DCOPSignals::emitSignal( DCOPConnection *conn, const QCString &_fun, const QByteArray &data, bool excludeSelf)
{
   QCString senderObj;
   QCString fun = _fun;
   int i = fun.find('#');
   if (i > -1)
   {
      senderObj = fun.left(i);
      fun = fun.mid(i+1);
   }

   DCOPSignalConnectionList *list = connections.find(fun);
   if (!list) return;
   for(DCOPSignalConnection *current = list->first(); current; current = list->next())
   {
      bool doSend = false;
      if (current->senderConn)
      {
         if (current->senderConn == conn)
	    doSend = true;
      }
      else if (!current->sender.isEmpty())
      {
         if ((conn && current->sender == conn->appId) || (current->sender == "DCOPServer"))
	    doSend = true;
      }
      else
      {
         doSend = true;
      }

      if (!current->senderObj.isEmpty() &&
          (current->senderObj != senderObj))
      {
         doSend = false;
      }

      if (excludeSelf && (conn == current->recvConn))
         doSend = false;
      if (doSend)
      {
         the_server->sendMessage(current->recvConn, conn ? conn->appId : QCString("DCOPServer"),
                                 current->recvConn->appId, current->recvObj,
                                 current->slot, data);
      }
   }
}

/**
 * Connects "signal" of the client named "sender" with the "slot" of
 * "receiverObj" in the "conn" client.
 *
 * If "Volatile" is true the connection will be removed when either
 * "sender" or "conn" unregisters.
 *
 * If "Volatile" is false the connection will only be removed when
 * "conn" unregisters.
 */
bool
DCOPSignals::connectSignal( const QCString &sender, const QCString &senderObj,
                       const QCString &signal,
                       DCOPConnection *conn, const QCString &receiverObj,
                       const QCString &slot, bool Volatile)
{
   // TODO: Check if signal and slot match
   QCString signalArgs, slotArgs;
   int i,j;
   i = signal.find('(');
   if (i < 0) return false;
   signalArgs = signal.mid(i+1);
   j = signalArgs.find(')');
   if (j < 0) return false;
   signalArgs.truncate(j);
   i = slot.find('(');
   if (i < 0) return false;
   slotArgs = slot.mid(i+1);
   j = slotArgs.find(')');
   if (j < 0) return false;
   slotArgs.truncate(j);

   if(signalArgs != slotArgs)
   {
      // Maybe the signal has more arguments than the slot...
      if (signalArgs.length() <= slotArgs.length())
         return false;
      if ((slotArgs.length() > 0) && (signalArgs[slotArgs.length()] != ','))
         return false;
      if (signalArgs.left(slotArgs.length()) != slotArgs)
         return false;
   }

   DCOPConnection *senderConn = 0;
   if (Volatile)
   {
      senderConn = the_server->findApp(sender);
      if (!senderConn)
         return false; // Sender does not exist.
   }
   DCOPSignalConnection *current = new DCOPSignalConnection;
   current->sender = sender;
   current->senderObj = senderObj;
   current->senderConn = senderConn;
   current->signal = signal;
   current->recvConn = conn;
   current->recvObj = receiverObj;
   current->slot = slot;

   DCOPSignalConnectionList *list = connections.find(signal);
   if (!list)
   {
      list = new DCOPSignalConnectionList;
      connections.insert(signal, list);
   }

   list->append( current );
   conn->signalConnectionList()->append(current);
   if (senderConn && senderConn != conn)
      senderConn->signalConnectionList()->append(current);
   return true;
}

/**
 * Disconnects "signal" of the client named "sender" from the "slot" of
 * "receiverObj" in the "conn" client.
 *
 * Special case:
 *   If sender & signal are empty all connections that involve
 *       conn & receiverObj (either as receiver or as sender)
 *   are disconnected.
 */
bool
DCOPSignals::disconnectSignal( const QCString &sender, const QCString &senderObj,
                       const QCString &signal,
                       DCOPConnection *conn, const QCString &receiverObj,
                       const QCString &slot)
{
   if (sender.isEmpty() && signal.isEmpty())
   {
      removeConnections(conn, receiverObj);
      return true;
   }

   DCOPSignalConnectionList *list = connections.find(signal);
   if (!list)
      return false; // Not found...

   DCOPSignalConnection *next = 0;
   bool result = false;

   for(DCOPSignalConnection *current = list->first(); current; current = next)
   {
      next = list->next();

      if (current->recvConn != conn)
         continue;

      if (current->senderConn)
      {
         if (current->senderConn->appId != sender)
	    continue;
      }
      else if (current->sender != sender)
	    continue;

      if (!senderObj.isEmpty() &&
          (current->senderObj != senderObj))
         continue;

      if (!receiverObj.isEmpty() &&
          (current->recvObj != receiverObj))
         continue;

      if (!slot.isEmpty() &&
          (current->slot != slot))
         continue;

      result = true;
      list->removeRef(current);
      conn->signalConnectionList()->removeRef(current);
      if (current->senderConn)
         current->senderConn->signalConnectionList()->removeRef(current);
      delete current;
   }
   return result;
}

/**
 * Removes all connections related to "obj" in the "conn" client
 * This means:
 *   All connections for which "conn"/"obj" is the receiver.
 *   All volatile connections for which "conn"/"obj" is the sender.
 */
void
DCOPSignals::removeConnections(DCOPConnection *conn, const QCString &obj)
{
   DCOPSignalConnectionList *list = conn->_signalConnectionList;
   if (!list)
      return; // Nothing to do...

   DCOPSignalConnection *next = 0;

   for(DCOPSignalConnection *current = list->first(); current; current = next)
   {
      next = list->next();

      if (!obj.isEmpty())
      {
         if ((current->senderConn == conn) && (current->senderObj != obj))
            continue;

         if ((current->recvConn == conn) && (current->recvObj != obj))
            continue;
      }

      if (current->senderConn && (current->senderConn != conn))
         current->senderConn->signalConnectionList()->removeRef(current);

      if (current->recvConn != conn)
         current->recvConn->signalConnectionList()->removeRef(current);

      DCOPSignalConnectionList *signalList = connections.find(current->signal);
      if (signalList)
      {
         signalList->removeRef(current);
         if (signalList->isEmpty())
            connections.remove(current->signal);
      }
      else
      {
         qDebug("Error: Signal Connection was not in signalList!\n");
      }
      list->removeRef(current);
      delete current;
   }
}


