package org.kde.kjas.server;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;

public class KJASConsole
    extends JFrame
{
    private JTextArea txt;
    private JScrollPane scroll;

    public KJASConsole()
    {
        super("Java Console");

        txt = new JTextArea();
        txt.setEditable(false);

        scroll = new JScrollPane( txt );

        JPanel main = new JPanel(new BorderLayout());
        JPanel btns = new JPanel(new BorderLayout());

        JButton clear = new JButton("Clear");
        JButton close = new JButton("Close");
        
        btns.add(clear, "West");
        btns.add(close, "East");

        main.add(scroll, "Center");
        main.add(btns, "South");
        
        getContentPane().add( main );
        

        clear.addActionListener
        (
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    txt.setText("");
                }
            }
        );

        close.addActionListener
        (
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    setVisible(false);
                }
            }
        );

        addWindowListener
        (
            new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    setVisible(false);
                }
            }
        );

        setSize(300, 300);

        PrintStream st = new PrintStream( new KJASConsoleStream(txt) );
        System.setOut(st);
        System.setErr(st);
        
        System.out.println( "Java VM version: " +
                            System.getProperty("java.version") );
        System.out.println( "Java VM vendor:  " +
                            System.getProperty("java.vendor") );
    }
}

class KJASConsoleStream
    extends OutputStream
{
    JTextArea txt;

    public KJASConsoleStream( JTextArea _txt )
    {
        txt = _txt;
    }

    public void close() {}
    public void flush() {}
    public void write(byte[] b) {}
    public void write(int a) {}

    // Should be enough for the console
    public void write( byte[] bytes, int offset, int length )
    {
        try  // Just in case
        {
            String msg = new String( bytes, offset, length );
            synchronized( txt )
            {
                //get the caret position
                int old_pos = txt.getCaretPosition();
                txt.append(msg);
                txt.setCaretPosition( old_pos + length );
            }
        }
        catch(Throwable t) {}
    }
}

