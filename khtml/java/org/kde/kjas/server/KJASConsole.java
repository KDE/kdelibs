package org.kde.kjas.server;

import java.awt.*;
import java.awt.event.*;
import java.io.*;

public class KJASConsole
    extends Frame 
{
    TextArea txt;
    
    public KJASConsole() {
        super("Java Console");
        Panel main = new Panel(new BorderLayout());
        Panel btns = new Panel(new BorderLayout());
        txt = new TextArea();
        Button clear = new Button("Clear");
        Button close = new Button("Close");
        
        btns.add(clear, "West");
        btns.add(close, "East");
        main.add(txt, "Center");
        main.add(btns, "South");
        
        add(main); 
        
        txt.setEditable(false);
        clear.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    txt.setText("");
                }
            });
        close.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    setVisible(false);
                }
            });
        addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    setVisible(false);
                }
            });
        
        setSize(300, 300); 
        
        PrintStream st = new PrintStream(new KJASConsoleStream(txt));
        System.setOut(st);
        System.setErr(st);
        
        System.out.println("Java VM version: " + 
                           System.getProperty("java.version"));
        System.out.println("Java VM vendor:  " + 
                           System.getProperty("java.vendor"));
    }
}

class KJASConsoleStream 
    extends OutputStream 
{
    TextArea txt;
    
    public KJASConsoleStream(TextArea _txt) {
        txt = _txt;
    }
    
    public void close() {}
    public void flush() {}
    public void write(byte[] b) {}
    public void write(int a) {}
    
    // Should be enought for the console
    public void write(byte[] bytes, int offset, int length) {
	try { // Just in case
	    String msg = new String(bytes, offset, length);
	    synchronized(txt) {
		txt.append(msg);
		// Attempt to move carret beyond text length
		// results carret to be placed at the end
		// Just what we need without calculating text length
		txt.setCaretPosition(100000);
	    }
	}
	catch(Throwable t) {}
    }
}

