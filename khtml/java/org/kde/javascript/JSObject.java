package org.kde.javascript;

import java.applet.Applet;
import org.kde.kjas.server.*;

public class JSObject extends netscape.javascript.JSObject {
    public String returnvalue = null;
    public Thread thread;

    private String jsobject;
    private int id;
    private Applet applet;
    private String appletID = null;

    /* JavaScript code:
     * __lc=[[JS objects],call func, get result func, last result]
     */
    private final String decls = "if(!__lc) __lc=[[window],function(i,v,glob){if(i>=__lc[0].length)return;__lc[3]='E ';__lc[3]=eval((glob?'':'__lc[0][i]')+v);},function(a){var t=typeof __lc[3];if(t=='undefined')v='V ';else if(t=='number')v='N '+__lc[3];else if(t=='string')if(__lc[3]=='E ')v='E ';else v='S '+__lc[3];else{var len=__lc[0].length;__lc[0][len]=__lc[3];v=''+len+' '+(__lc[3]==__lc?'[array]':__lc[3]);}document[a].__lc_ret=v},0]";

    public JSObject(Applet a, String name, int _id) {
        Main.info("JSObject.ctor: " + name);
        jsobject = new String(name);
        applet = a;
        id = _id;
        KJASAppletContext kc = (KJASAppletContext)applet.getAppletContext();
        appletID = kc.getAppletID(a);
        if (id == 0) {
            kc.evaluateJavaScript(decls, appletID, null);
        } 
    }

    int getId() {
        return id;
    }

    private Object evaluate(String _script, boolean global) throws netscape.javascript.JSException {
        Main.info("evaluate (\"" + _script + "\")");
        
        String script = _script.replaceAll("\\\\", "\\\\\\\\\\\\\\\\").replaceAll("\"", "\\\\\\\\\\\\\\\"");

        KJASAppletContext kc = (KJASAppletContext) applet.getAppletContext();
        String appletname = kc.getAppletName(appletID);
        thread = Thread.currentThread();

        kc.evaluateJavaScript("__lc[1](" + id + ",\\\"" + script + (global ? "\\\",true)" : "\\\")"), appletID, null);
        kc.evaluateJavaScript("__lc[2]('" + appletname + "')", appletID, this);
        boolean timedout = true;
        try {
            Thread.currentThread().sleep(30000);
        } catch (InterruptedException ex) {
            timedout = false;
        }
        thread = null;
        if (timedout || returnvalue == null)
            return null;

        /* lets see what we've got */
        String retval = returnvalue;
        int pos = retval.indexOf(' ');
        String type = retval.substring(0, pos);
        if (type.equals("E")) // Error
            throw new netscape.javascript.JSException("Script error");
        if (type.equals("V")) // Void
            return null;
        String value = retval.substring(pos+1);
        Main.info("value=" + value + " (type=" + type + ")");
        if (type.equals("N")) // Number
            return new Double(value);
        if (type.equals("S")) // String
            return value;

        /* Is it an applet? */
        if (value.startsWith("[object APPLET ref=")) {
            int p1 = value.indexOf('=');
            int p2 = value.indexOf(',', p1+1);
            int p3 = value.indexOf(']', p2+1);
            String appletid = value.substring(p2+1, p3);
            //FIXME: get the correct context, not our own
            //int contextid = Integer.parseInt(value.substring(p1+1, p2));
            return kc.getAppletById(appletid);
        }
        /* Is it a Java object then? */
        if (value.startsWith("[[embed ")) {
            int p1 = value.indexOf("ref=");
            int p2 = value.indexOf(',', p1+1);
            int p3 = value.indexOf(',', p2+1);
            int p4 = value.indexOf(']', p3+1);
            //int contextid = Integer.parseInt(value.substring(p1+1, p2));
            String appletid = value.substring(p2+1, p3);
            int objindex = Integer.parseInt(value.substring(p3+1, p4));
            return kc.getJSReferencedObject(objindex);
        }
        /* Ok, make it a JSObject */
        return new JSObject(applet, value, Integer.parseInt(type));
    }
    private String convertValueJ2JS(Object o) {
        /* FIXME: escape strings: ' -> \'
         */
        if (o == null)
            return new String("null");
        if (o instanceof java.lang.Number || o instanceof java.lang.Boolean)
            return o.toString();
        if (o instanceof netscape.javascript.JSObject)
            return new String("__lc[0][" + ((JSObject)o).getId() + "]");
        return new String("'" + o.toString() + "'");
    }
    public Object call(String func, Object [] args) throws netscape.javascript.JSException {
        Main.info("JSObject.call: " + jsobject + "." + func);
        String script = new String("." + func + "(");
        for (int i = 0; i < args.length; i++)
            script += (i > 0 ? "," : "") + args[i];
        script += ")";
        return evaluate(script, false);
    }
    public Object eval(String s) throws netscape.javascript.JSException {
        return evaluate(s, true);
    }
    public boolean equals(Object obj) {
        Main.info("JSObject.equals");
        return super.equals(obj);
    }
    public Object getMember(String name) throws netscape.javascript.JSException {
        Main.info("JSObject.getMember: " + jsobject + "." + name);
        return evaluate("." + name, false);
    }
    public void setMember(String name, java.lang.Object o) throws netscape.javascript.JSException {
        Main.info("JSObject.setMember: " + jsobject + "." + name);
        evaluate("." + name + "=" + convertValueJ2JS(o), false);
    }
    public void removeMember(String name) throws netscape.javascript.JSException {
        Main.info("JSObject.removeMember: " + jsobject + "." + name);
        evaluate("." + name + "=null", false);
    }
    /* get array element; JS: this[index] */
    public Object getSlot(int index)throws netscape.javascript.JSException {
        Main.info("JSObject.getSlot: " + jsobject + "[" + index + "]");
        return evaluate("[" + index + "]", false);
    }
    public void setSlot(int index, Object o) throws netscape.javascript.JSException {
        Main.info("JSObject.setSlot: " + jsobject + "[" + index + "]");
        evaluate("[" + index + "]=" + convertValueJ2JS(o), false);
    }
    public String toString(){
        Main.info("JSObject.toString: " + jsobject);
        return new String(jsobject);
    }
    public static JSObject getWindow(Applet a, int dummy) {
        Main.info("JSObject.getWindow");
        return new JSObject(a, "[WINDOW]", 0);
    }
}
