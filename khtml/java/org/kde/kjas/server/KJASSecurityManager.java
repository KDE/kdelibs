package org.kde.kjas.server;

import java.security.*;
import java.net.*;


public class KJASSecurityManager extends SecurityManager
{
    public KJASSecurityManager()
    {
    }
    /**
     * checks for an applets permission to access certain resources
     * currently, only a check for SocketPermission is done, that the
     * applet cannot connect to any other but the host, where it comes from.
     * Anything else seems to be handled automagically
     */
    public void disabled___checkPermission(Permission perm) throws SecurityException, NullPointerException
    {
        // does not seem to work as expected, Problems with proxy - and it seems that the default
        // implementation already does all that well, what I wanted to do here.
        // It is likely that this method will hence disappear soon again.
        Object context = getSecurityContext();
        Thread thread = Thread.currentThread();
        if (perm instanceof SocketPermission) {
            // check if this is a connection back to the originating host
            // if not, fall through and call super.checkPermission
            // this gives normally access denied
            Main.debug("*** checkPermission " + perm + " in context=" + context + " Thread=" + thread);
            // use the context class loader to determine if this is one
            // of our applets
            ClassLoader contextClassLoader = thread.getContextClassLoader();
            Main.debug("*   ClassLoader=" + contextClassLoader);
            try {
                // try to cast ...
                KJASAppletClassLoader loader = (KJASAppletClassLoader)contextClassLoader;
                // ok. cast succeeded. Now get the codebase of the loader
                // because it contains the host name
                URL codebase = loader.getCodeBase();
                URL docbase = loader.getDocBase();
                Main.debug("*   Class Loader docbase=" + docbase + " codebase=" + codebase);
                String hostname = perm.getName();
                // extract the hostname from the permission name
                // which is something like "some.host.domain:XX"
                // with XX as the port number
                int colonIdx = hostname.indexOf(':');
                if (colonIdx > 0) {
                    // strip of the port
                    hostname = hostname.substring(0, colonIdx);
                }
                // Main.info("Checking " + hostname + "<->" + codebase.getHost());
                
                if (hostsAreEqual(hostname, codebase.getHost())) {
                    // ok, host matches
                    String actions = perm.getActions();
                    // just check if listen is specified which we do not want
                    // to allow
                    if (actions != null && actions.indexOf("listen") >= 0) {
                        Main.debug("*   Listen is not allowed.");
                    } else {
                        // ok, just return and throw _no_ exception
                        Main.debug("*   Hostname equals. Permission granted.");
                        return;
                    }
                } else {
                    Main.info("Host mismatch: " + perm + " != " + codebase.getHost());
                }
            } catch (ClassCastException e) {
                Main.debug("*   ClassLoader is not a KJASAppletClassLoader");
            }
            Main.debug("*   Fall through to super.checkPermission()");
        }
        super.checkPermission(perm);
    }
    
    private static final boolean hostsAreEqual(String host1, String host2) {
        if (host1 == null || host2 == null) {
            return false;
        }
        if (host1.length() == 0 || host2.length() == 0) {
            return false;
        }
        if (host1.equalsIgnoreCase(host2)) {
            return true;
        }
       
        if ( Main.proxyHost != null && Main.proxyPort != 0) {
            // if we use a proxy, we certainly cannot use DNS
            return false;
        }

        InetAddress inet1=null, inet2=null;
        try {
            inet1 = InetAddress.getByName(host1);
        } catch (UnknownHostException e) {
            Main.kjas_err("Unknown host:" + host1, e);
            return false;
        }
        try {
            inet2 = InetAddress.getByName(host2);
        } catch (UnknownHostException e) {
            Main.kjas_err("Unknown host:" + host2, e);
            return false;
        }
        if (inet1.equals(inet2)) {
            return true;
        }       
        return false;
    }
}
