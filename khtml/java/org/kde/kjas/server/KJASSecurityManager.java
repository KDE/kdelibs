package org.kde.kjas.server;

import java.security.*;
import java.security.cert.*;
import java.net.*;
import java.util.*;


public class KJASSecurityManager extends SecurityManager
{
    static Hashtable confirmRequests = new Hashtable();
    static int confirmId = 0;
    Hashtable grantedPermissions = new Hashtable();
    HashSet grantAllPermissions = new HashSet();
    HashSet rejectAllPermissions = new HashSet();

    public KJASSecurityManager()
    {
    }
    /**
     * checks for an applets permission to access certain resources
     * currently, only a check for SocketPermission is done, that the
     * applet cannot connect to any other but the host, where it comes from.
     * Anything else seems to be handled automagically
     */
    public void checkPermission(Permission perm) throws SecurityException, NullPointerException {
        // ClassLoader cl = Thread.currentThread().getContextClassLoader();
        try {
            super.checkPermission(perm);
        } catch (SecurityException se) {
            // Don't annoy users with these
            if (perm instanceof java.lang.RuntimePermission ||
                    perm instanceof java.awt.AWTPermission)
                throw se;

            // Collect certificates
            HashSet signers = new HashSet();
            Class [] cls = getClassContext();
            for (int i = 1; i < cls.length; i++) {
                Object[] objs = cls[i].getSigners();
                if (objs != null && objs.length > 0)
                    for (int j = 0; j < objs.length; j++)
                        signers.add(objs[j]);
            }
            Main.debug("Certificates " + signers.size() + " for " + perm);
            if (signers.size() == 0)
                throw se;

            // Check granted/denied permission
            String text = new String();
            for (Iterator i = signers.iterator(); i.hasNext(); ) {
                Object cert = i.next();
                if ( grantAllPermissions.contains(cert) )
                    return;
                if ( rejectAllPermissions.contains(cert) )
                    throw se;
                Permissions permissions = (Permissions) grantedPermissions.get(cert);
                if (permissions != null && permissions.implies(perm))
                    return;
                if (cert instanceof X509Certificate)
                    text += ((X509Certificate) cert).getIssuerDN().getName();
                else
                    text += cert.toString();
                text += "\n";
            }
            String id = "" + confirmId++;
            confirmRequests.put(id, Thread.currentThread());
            Main.protocol.sendSecurityConfirm(text + perm, id);
            boolean granted = false;
            try {
                Thread.currentThread().sleep(300000);
            } catch (InterruptedException ie) {
                if (((String) confirmRequests.get(id)).equals("yes")) {
                    granted = true;
                    for (Iterator it = signers.iterator(); it.hasNext(); ) {
                        Object cert = it.next();
                        Permissions permissions = (Permissions) grantedPermissions.get(cert);
                        if (permissions == null) {
                            permissions = new Permissions();
                            grantedPermissions.put(cert, permissions);
                        }
                        permissions.add(perm);
                    }
                } else if (((String) confirmRequests.get(id)).equals("grant")) {
                    grantAllPermissions.addAll( signers );
                    granted = true;
                } else if (((String) confirmRequests.get(id)).equals("reject")) {
                    rejectAllPermissions.addAll( signers );
                } // else "no"
            } finally {
                confirmRequests.remove(id);
            }
            if (!granted) {
                Main.debug("Permission denied" + perm);
                throw se;
            }
        }
    }

    // keytool -genkey -keystore mystore -alias myalias
    // keytool -export -keystore mystore -alias myalias -file mycert
    // keytool -printcert -file mycert
    // keytool -import -keystore myotherstore -alias myalias -file mycert
    // jarsigner -keystore mystore myjar.jar myalias
    // jarsigner -verify -keystore myotherstore myjar.jar
    //
    // policy file (use policytool and check java.security):
    // keystore "file:myotherstore", "JKS"
    // grant signedBy "myalias"
    // {
    //     permission java.io.FilePermission "<<ALL FILES>>", "read"
    // }
    // 
    // java code:
    // KeyStore store = KeyStore.getInstance("JKS", "SUN");
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
