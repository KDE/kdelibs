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
    private final String decls = "if(!__lc){__lc=[window];__lcres;}\nfunction __lccall(i,v){var len=__lc.length;if(i>=len)return;var res=eval('__lc[i]'+v);if(res) __lc[len]=res;__lcres=len;}\nfunction __lcgetres(a){document[a].__lc_ret=''+__lcres+' '+__lc[__lcres];}";

    public JSObject(Applet a, String name, int _id) {
        /*
        try {
            throw new RuntimeException("No Exception, just debugging: a=" + a + " name=" + name + " _id=" + _id);
        } catch(Exception e) {
            e.printStackTrace();
        }
        */
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
    private JSObject evaluate(String _script) {
        Main.info("evaluate (\"" + _script + "\")");
        int oldpos = 0;
        int pos = _script.indexOf("\"");
        String script = new String("");
        while (pos > -1) {
            script += _script.substring(oldpos, pos);
            script += "\\\"";
            oldpos = pos + 1;
            pos = _script.indexOf("\"", oldpos);
        }
        if (oldpos < _script.length())
            script += _script.substring(oldpos);
        KJASAppletContext kc = (KJASAppletContext) applet.getAppletContext();
        String appletname = kc.getAppletName(appletID);
        thread = Thread.currentThread();
        kc.evaluateJavaScript("__lccall(" + id + ",\"" + script + "\")", appletID, null);
        kc.evaluateJavaScript("__lcgetres(\"" + appletname + "\")", appletID, this);
        try {
            Thread.currentThread().sleep(30000);
        } catch (InterruptedException ex) {}
        thread = null;
        Main.debug("JSObject.evaluate: __lccall(" + id + ",\"" + script + "\")");
        if (returnvalue == null)
            return null;
        String retval = returnvalue;
        returnvalue = null;
        pos = retval.indexOf(' ');
        String newName = retval.substring(pos+1);
        String newId = retval.substring(0, pos);
        Main.info("newName=" + newName + " newId=" + newId);
        JSObject ret = new JSObject(applet, newName, Integer.parseInt(newId));
        Main.info("returning " + ret);
        return ret;               
    }
    private String convertValueJ2JS(Object o) {
        /* FIXME: escape strings
         *        handle JSObect differently
         *        check for number,boolean types
         */
        if (o == null)
            return new String("null");
        return new String("\"" + o.toString() + "\"");
    }
    public Object call(String func, Object [] args) {
        Main.debug("JSObject.call: " + jsobject + "." + func);
        String script = new String("." + func + "(");
        for (int i = 0; i < args.length; i++)
            script += (i > 0 ? "," : "") + args[i];
        script += ")";
        return evaluate(script);
    }
    public Object eval(String s) {
        return evaluate("." + s);
    }
    public boolean equals(Object obj) {
        Main.debug("JSObject.equals");
        return super.equals(obj);
    }
    public Object getMember(String name) {
        Main.info("JSObject.getMember: " + jsobject + "." + name);
        Object ret = evaluate("." + name);
        try {
            Object doubleObject = new Double(ret.toString());
            ret = doubleObject;
        } catch (Exception e) {
            // this is ok. Just wasn't a number
        }
        Main.info("getMember() returns a " + ret.getClass().getName());
        return ret;
    }
    public void setMember(String name, java.lang.Object o) throws netscape.javascript.JSException {
        Main.debug("JSObject.setMember: " + jsobject + "." + name);
        Object ret = evaluate("." + name + "=" + convertValueJ2JS(o.toString()));
        if (ret == null) {
            throw new netscape.javascript.JSException("Setting javascript member " + jsobject + "." + name);
        }
    }
    public void removeMember(String name) {
        Main.debug("JSObject.removeMember: " + jsobject + "." + name);
        evaluate("." + name + "=null");
    }
    /* get array element; JS: this[index] */
    public Object getSlot(int index){
        Main.debug("JSObject.getSlot: " + jsobject + "[" + index + "]");
        return evaluate("[" + index + "]");
    }
    public void setSlot(int index, Object o) {
        Main.debug("JSObject.setSlot: " + jsobject + "[" + index + "]");
        evaluate("[" + index + "]=" + convertValueJ2JS(o));
    }
    public String toString(){
        Main.debug("JSObject.toString: " + jsobject);
        return new String(jsobject);
    }
    public static JSObject getWindow(Applet a, int dummy) {
        Main.debug("JSObject.getWindow");
        return new JSObject(a, "[WINDOW]", 0);
    }
}
