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

#ifndef DCOPSIGNALS_H
#define DCOPSIGNALS_H "$Id$"

#include <qdict.h>
#include <qptrlist.h>
#include <qstring.h>

class DCOPConnection;

//
// This requires:
// DCOPSignalConnectionList * DCOPConnection::signalConnectionList()
/// @internal
class DCOPSignalConnection
{
public:
   QCString sender;      // Sender client, empty means any client
   DCOPConnection *senderConn; //Sender client.
   QCString senderObj;   // Object that sends the signal.
   QCString signal;      // Signal name. Connections are sorted on signal name.

   DCOPConnection *recvConn; // Client that wants to receive the signal
   QCString recvObj;     // Object that wants to receive the signal
   QCString slot;        // Function to send to in the object.
};

/// @internal
class DCOPSignalConnectionList : public QPtrList<DCOPSignalConnection>
{
public:
   DCOPSignalConnectionList() { };
};

/**
 * @internal
 */
class DCOPSignals
{
public:
   DCOPSignals();

   /**
    * Client "conn" emits the signal "fun" with "data" as arguments.
    * conn is 0L if the dcopserver sends the signal itself
    *
    * The emitting object is encoded in "fun".
    *
    * If "excludeSelf" is true, signal is never send to "conn" itself.
    */
   void emitSignal( DCOPConnection *conn, const QCString &fun, const QByteArray &data, bool excludeSelf);

   /**
    * Connects "signal" of the client named "sender" with the "slot" of
    * "receiverObj" in the "conn" client.
    *
    * If "Volatile" is true the connection will be removed when either
    * "sender" or "conn" unregisters.
    *
    * If "Volatile" is false the connection will only be removed when
    * "conn" unregisters.
    *
    * Returns true if succesfull. False on failure.
    * An attempt to create a Volatile connection to a non-existing client
    * results in a failure.
    */
   bool connectSignal( const QCString &sender, const QCString &senderObj,
                       const QCString &signal,
                       DCOPConnection *conn, const QCString &receiverObj,
                       const QCString &slot, bool Volatile);

   /**
    * Disconnects "signal" of the client named "sender" from the "slot" of
    * "receiverObj" in the "conn" client.
    *
    * If "receiverObj" is empty, every object is disconnected.
    * If "slot" is empty, every slot is disconnected.
    *
    * Returns true if successfull, false if no connection was found.
    */
   bool disconnectSignal( const QCString &sender, const QCString &senderObj,
                       const QCString &signal,
                       DCOPConnection *conn, const QCString &receiverObj,
                       const QCString &slot);

   /**
    * Removes all connections related to the "conn" client.
    * This means:
    *   All connections for which "conn" is the receiver.
    *   All volatile connections for which "conn" is the sender.
    */
   void removeConnections(DCOPConnection *conn, const QCString &obj=0);


   /*
    * The administration.
    *
    * All connections are sorted by "signal" and then inserted in the
    * administration as a list.
    *
    * connections[signal] gives a list of all connections related to
    * a given signal. The connections in this list may specify different
    * senders and receiving clients/objects.
    */
   QAsciiDict<DCOPSignalConnectionList> connections;
};

//
// connectSignal:
//
// Check whether signal and slot match wrt arguments.
// A slot may ignore arguments from the signal.
//
// If volatile
//    then lookup senderConn...
//         If not found?
//            then return false
// Create DCOPSignalConnection.
// Add DCOPSignalConnection to "connections".
// Add DCOPSignalConnection to conn->connectionList()
// If volatile
//    then Add DCOPSignalConnection to senderConn->connectionList()
// Return true


// removeConnections:
//
// For each DCOPSignalConnection in conn->connectionList
//    if DCOPSignalConnection->sender == conn->appId
//        then remove DCOPSignalConnection from DCOPSIgnalConnection->conn
//    if DCOPSignalConnection->conn == conn
//       then.. this is ugly.. remove DCOPSignalConnection from DCOPSignalConnection->sender
//    WARNING: Take care when sender and conn are the same client!
//    Remove DCOPSignalConnection from connections[DCOPSignalConnection->signal]

#endif
