package org.kde.kjas.server;

import java.applet.Applet;
import java.net.*;
import java.security.*;
import java.io.*;

public class KJASSecurityManager extends SecurityManager
{
    public KJASSecurityManager()
    {
    }

    //Thread Checks...
    private static RuntimePermission threadPermission;
    private static RuntimePermission threadGroupPermission;
    private boolean inThreadGroupCheck = false;

    public synchronized void checkAccess( Thread t )
    {
        if ( !inThreadGroup( t ) )
        {
            if (threadPermission == null)
                threadPermission = new RuntimePermission( "modifyThread" );
            checkPermission( threadPermission );
        }
    }

    public synchronized void checkAccess( ThreadGroup g )
    {
        if (inThreadGroupCheck)
        {
            if (threadGroupPermission == null)
                threadGroupPermission =
                    new RuntimePermission( "modifyThreadGroup" );
            checkPermission( threadGroupPermission );
        } else
        {
            try
            {
                inThreadGroupCheck = true;
                if (!inThreadGroup(g))
                {
                    if (threadGroupPermission == null)
                        threadGroupPermission =
                            new RuntimePermission( "modifyThreadGroup" );
                    checkPermission( threadGroupPermission );
                }
            }
            finally
            {
                inThreadGroupCheck = false;
            }
        }
    }

    protected boolean inThreadGroup( ThreadGroup g )
    {
        return getThreadGroup().parentOf( g );
    }


    protected boolean inThreadGroup( Thread thread )
    {
	    return inThreadGroup( thread.getThreadGroup() );
    }

    //Package access security checks...
    public void checkPackageAccess( final String pkgname )
    {
        // first see if the VM-wide policy allows access to this package
        super.checkPackageAccess( pkgname );

        final boolean[] OkToCheck = { false };
        AccessController.doPrivileged(
            new PrivilegedAction()
            {
                public Object run()
                {
                    String pkg = pkgname;
                    int index = 0;
                    while( index != -1 )
                    {
                        if( Boolean.getBoolean( "package.restrict.access." + pkg ) )
                        {
                            OkToCheck[0] = true;
                            break;
                        }

                        index = pkg.lastIndexOf('.');
                        if( index != -1 )
                            pkg = pkg.substring( 0,index );
                    }
                    return null;
                }
            }
        );

        if ( OkToCheck[0] )
            checkPermission( new java.lang.RuntimePermission
                            ("accessClassInPackage." + pkgname) );
    }


    public void checkAwtEventQueueAccess()
    {
        //allow everything for now...
        //we've got to figure out what it's doing in the first place...
    }

}
