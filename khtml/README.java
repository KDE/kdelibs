To get the Java support working, you will need to add the Java interpreter
(java executable) to your path, and set the following environment variable:
    CLASSPATH=$(KDEDIR)/share/apps/kjava/kjava-classes.zip:$(JDKDIR)/lib

At least this works for me (JDK-1.1.7). If your Java implementation doesn't 
support zipped class archives, you might need to unzip the kjava-classes.zip
file in $KDEDIR/share/kjava.
