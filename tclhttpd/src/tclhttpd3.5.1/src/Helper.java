/*
 * Helper class for simple Server authentication applet
 */

import netscape.javascript.*;
import java.awt.*;
public abstract class Helper extends java.applet.Applet {		

public void init() {
}

/* compute the MD5 digest of the input string */

public String digest(String value) {
    MD5 digest = new MD5();
    digest.init();
    digest.updateASCII(value);
    digest.finish();
    return digest.toString();
    }

/* eval an arbitrary javascript command, return a string value */

public String js(String script) {
    JSObject win;
    win = JSObject.getWindow(this);
    String result = (String)win.eval(script);
    System.out.println(result);
    return  result;
}
}
