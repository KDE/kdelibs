/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Koos Vriezen <koos ! vriezen () xs4all ! nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

package org.kde.kjas.server;

import java.net.*;
import java.io.*;
import java.util.*;
import java.security.*;
/**
 *
 */

class KIOConnection
{
    final static int DATA = 0;
    final static int FINISHED = 1;
    final static int ERRORCODE = 2;

    final static int STOP = 0;
    final static int HOLD = 1;
    final static int RESUME = 2;

    protected static int id = 0;
    static Hashtable jobs = new Hashtable();

    Thread thread = null;

    protected boolean connected = false;
    protected String jobid = null;
    protected LinkedList data = new LinkedList ();
    protected int errorcode = 0;
    protected boolean finished = false;
    protected boolean onhold = false;
    protected URL url;

    void checkConnected() throws IOException {
        if (!connected)
            throw new IOException("not connected");
    }
    protected boolean haveError() {
        return errorcode != 0;
    }

    class KJASOutputStream extends OutputStream {
        KJASOutputStream() {
        }
        public void write(int b) throws IOException {
            throw new IOException("not implemented");
        }
        public void close() throws IOException {
            checkConnected();
            disconnect();
        }
        public void flush() throws IOException {
            checkConnected();
            throw new IOException("not implemented");
        }
    }

    class KJASInputStream extends InputStream {
        private byte [] buf = null;
        private int bufpos = 0;
        private boolean eof = false;

        KJASInputStream() {
        }
        private boolean getData() throws IOException {
            if (haveError()) {
                //disconnect();
                eof = true;
                //throw new IOException("i/o error " + errorcode);
            }
            if (eof)
                return false;
            checkConnected();
            if (buf != null && bufpos < buf.length)
                return true;
            int datasize = 0;
            synchronized (jobs) {
                datasize = data.size();
                if (datasize > 0) {
                    buf = (byte []) data.removeFirst();
                    bufpos = 0;
                }
            }
            if (onhold) {
                Main.protocol.sendDataCmd(jobid, RESUME);
                onhold = false;
            }
            if (datasize > 0)
                return true;
            if (finished) {
                eof = true;
                synchronized (jobs) {
                    jobs.remove(jobid);
                }
                return false;
            }
            thread = Thread.currentThread();
            try {
                Thread.currentThread().sleep(10000);
            } catch (InterruptedException ie) {
                return getData();
            }
            thread = null;
            disconnect();
            throw new IOException("timeout");
        }
        public int read() throws IOException {
            if (getData())
                return 0x00ff & buf[bufpos++];
            return -1;
        }
        public int read(byte[] b, int off, int len) throws IOException {
            if (!getData())
                return -1;
            int nr = buf.length - bufpos; // FIXME add length of all buffers
            if (nr > len)
                nr = len;
            System.arraycopy(buf, bufpos, b, off, nr);
            bufpos += nr;
            return nr;
        }
        public int read(byte[] b) throws IOException {
            return read(b, 0, b.length);
        }
        public int available() throws IOException {
            if (eof)
                return 0;
            checkConnected();
            return buf == null ? 0 : buf.length - bufpos; // FIXME see read
        }
        public boolean markSupported() {
            return false;
        }
        public void close() throws IOException {
            checkConnected();
            disconnect();
        }
    }

    private KJASOutputStream out = null;
    private KJASInputStream in = null;

    KIOConnection(URL u) {
        url = u;
    }
    public void setData(int code, byte [] d) {
        // this method is synchronized on jobs in processCommand
        switch (code) {
            case FINISHED:
                if (d != null && d.length > 0)
                    data.addLast(d);
                finished = true;
                Main.debug ("FINISHED (" + jobid + ") " + data.size());
                break;
            case DATA:
                if (d.length > 0)
                    data.addLast(d);
                Main.debug ("DATA (" + jobid + ") " + data.size());
                if (!onhold && data.size() > 2) {
                    Main.protocol.sendDataCmd(jobid, HOLD);
                    onhold = true;
                }
                break;
            case ERRORCODE:
                String codestr = new String(d);
                errorcode = Integer.parseInt(codestr);
                Main.debug ("ERRORECODE(" + jobid + ") " + errorcode);
                break;
        }
    }
    synchronized void connect(boolean doInput) throws IOException {
        Main.debug ("connect " + url);
        if (connected)
            throw new IOException("already connected");
        errorcode = 0;
        synchronized (jobs) {
            jobid = String.valueOf(id++);
            thread = Thread.currentThread();
            jobs.put(jobid, this);
        }
        if (doInput)
            Main.protocol.sendGetURLDataCmd(jobid, url.toExternalForm());
        try {
            Thread.currentThread().sleep(20000);
        } catch (InterruptedException ie) {
            connected = true;
            if (doInput)
                in = new KJASInputStream();
            else
                out = new KJASOutputStream();
            if (!haveError()) {
                Main.debug ("connect(" + jobid + ") " + url);
                return;
            }
        }
        thread = null;
        synchronized (jobs) {
            jobs.remove(jobid);
        }
        if (connected) {
            connected = false;
            if (!finished)
                Main.protocol.sendDataCmd(jobid, STOP);
            Main.debug ("connect error(" + jobid + ") " + url);
            throw new IOException("connection failed (not found)");
        }
        Main.debug ("connect timeout(" + jobid + ") " + url);
        throw new IOException("connection failed (timeout)");
    }
    void disconnect() {
        if (!connected)
            return;
        Main.debug ("disconnect " + jobid);
        synchronized (jobs) {
            jobs.remove(jobid);
        }
        connected = false;
        out = null;
        in = null;
        if (!finished)
            Main.protocol.sendDataCmd(jobid, STOP);
    }
    InputStream getInputStream() throws IOException {
        Main.debug ("getInputStream(" + jobid + ") " + url);
        if (!connected)
            connect(true);
        return in;
    }
    OutputStream getOutputStream() throws IOException {
        Main.debug ("getOutputStream(" + jobid + ") " + url);
        if (!connected)
            connect(false);
        return out;
    }
}

final class KIOHttpConnection extends KIOConnection
{
    final static int HEADERS = 3;
    final static int REDIRECT = 4;
    final static int MIMETYPE = 5;

    Vector headers = new Vector();
    Hashtable headersmap = new Hashtable();
    String responseMessage = null;
    int responseCode = -1;

    KIOHttpConnection(URL u) {
        super(u);
    }
    protected boolean haveError() {
        return responseCode >= 400;
    }
    public void setData(int code, byte [] d) {
        // this method is synchronized on jobs in processCommand
        switch (code) {
            case HEADERS:
                StringTokenizer tokenizer = new StringTokenizer(new String(d), "\n");
                while (tokenizer.hasMoreTokens()) {
                    String token = tokenizer.nextToken();
                    int pos = token.indexOf(":");
                    String [] entry = {
                        token.substring(0, pos > -1 ? pos : token.length()).toLowerCase(), token.substring(pos > -1 ? pos+1: token.length()).trim()
                    };
                    headers.add(entry);
                    headersmap.put(entry[0], entry[1]);
                    Main.debug ("header " + entry[0] + "=" + entry[1]);
                }
                if (headersmap.size() > 0) {
                    String token = ((String []) headers.get(0))[0];
                    if (!token.startsWith("http/1.")) break;
                    int spos = token.indexOf(' ');
                    if (spos < 0) break;
                    int epos = token.indexOf(' ', spos + 1);
                    if (epos < 0) break;
                    responseCode = Integer.parseInt(token.substring(spos+1, epos));
                    responseMessage = token.substring(epos);
                    Main.debug ("responsecode=" + responseCode);
                }
                break;
            default:
                super.setData(code, d);
        }
        
    }
}

final class KIOSimpleConnection extends KIOConnection
{
    KIOSimpleConnection(URL u) {
        super(u);
    }
}

final class KJASHttpURLConnection extends HttpURLConnection
{
    private KIOHttpConnection kioconnection;

    KJASHttpURLConnection(URL u) {
        super(u);
        kioconnection = new KIOHttpConnection(u);
    }
    public Map getHeaderFields() {
        Main.debug ("getHeaderFields");
        return kioconnection.headersmap;
    }
    public String getHeaderField(String name) {
        Main.debug ("getHeaderField:" + name + "=" +
                    (String) kioconnection.headersmap.get(name));
        return (String) kioconnection.headersmap.get(name);
    }
    public String getHeaderField(int n) {
        if (n >= kioconnection.headersmap.size())
            return null;
        String [] entry = (String []) kioconnection.headers.get(n);
        String line = entry[0];
        if (entry[1].length() > 0)
            line += ":" + entry[1];
        Main.debug ("getHeaderField(" + n + ")=#" + line + "#");
        return line;
    }
    public String getHeaderFieldKey(int n) {
        Main.debug ("getHeaderFieldKey " + n);
        if (n >= kioconnection.headersmap.size())
            return null;
        return ((String []) kioconnection.headers.get(n))[0];
    }
    public int getResponseCode() throws IOException {
        responseMessage = kioconnection.responseMessage;
        return kioconnection.responseCode;
    }
    public boolean usingProxy() {
        return false; // FIXME
    }
    public synchronized void connect() throws IOException {
        kioconnection.connect(doInput);
    }
    public void disconnect() {
        kioconnection.disconnect();
    }
    public InputStream getInputStream() throws IOException {
        return kioconnection.getInputStream();
    }
    public OutputStream getOutputStream() throws IOException {
        return kioconnection.getOutputStream();
    }
}

final class KJASSimpleURLConnection extends URLConnection
{
    private KIOSimpleConnection kioconnection;

    KJASSimpleURLConnection(URL u) {
        super(u);
        kioconnection = new KIOSimpleConnection(u);
    }
    public boolean usingProxy() {
        return false; // FIXME
    }
    public synchronized void connect() throws IOException {
        kioconnection.connect(doInput);
    }
    public void disconnect() {
        kioconnection.disconnect();
    }
    public InputStream getInputStream() throws IOException {
        return kioconnection.getInputStream();
    }
    public OutputStream getOutputStream() throws IOException {
        return kioconnection.getOutputStream();
    }
}


final class KJASHttpURLStreamHandler extends URLStreamHandler
{
    KJASHttpURLStreamHandler(int port) {
        default_port = port;
    }
    protected URLConnection openConnection(URL u) throws IOException {
        Main.debug ("openConnection " + u);
        return new KJASHttpURLConnection(u);
    }
    protected int getDefaultPort() {
        return default_port;
    }
    private int default_port;
}

final class KJASSimpleURLStreamHandler extends URLStreamHandler
{
    KJASSimpleURLStreamHandler(int port) {
        default_port = port;
    }
    protected URLConnection openConnection(URL u) throws IOException {
        Main.debug ("KJASSimpleURLStreamHandler.openConnection " + u);
        return new KJASSimpleURLConnection(u);
    }
    protected int getDefaultPort() {
        return default_port;
    }
    private int default_port;
}

public final class KJASURLStreamHandlerFactory 
    implements URLStreamHandlerFactory
{
    public URLStreamHandler createURLStreamHandler(String protocol) {
        //outputs to early: Main.debug ("createURLStreamHandler " + protocol);
        if (protocol.equals("http"))
            return new KJASHttpURLStreamHandler(80);
        else if (protocol.equals("https"))
            return new KJASHttpURLStreamHandler(443);
        else if (protocol.equals("ftp"))
            return new KJASSimpleURLStreamHandler(21);
        else if (protocol.equals("smb"))
            return new KJASSimpleURLStreamHandler(139);
        else if (protocol.equals("fish"))
            return new KJASSimpleURLStreamHandler(22);
        return null;
    }
}
