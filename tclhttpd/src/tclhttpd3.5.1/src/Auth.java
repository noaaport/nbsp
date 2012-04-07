/*
 * Helper class for simple Server authentication applet
 */

import netscape.javascript.*;
import java.awt.*;
public class Auth extends java.applet.Applet {		

public TextField password;
public void init() {
	GridBagLayout grid = new GridBagLayout();
	int rowHeights[] = {0};
	int columnWidths[] = {0};
	double rowWeights[] = {0.0};
	double columnWeights[] = {0.0};
	grid.rowHeights = rowHeights;
	grid.columnWidths = columnWidths;
	grid.rowWeights = rowWeights;
	grid.columnWeights = columnWeights;

	password = new TextField(8);
	password.setEchoCharacter('*');
	this.add(password);

	GridBagConstraints con = new GridBagConstraints();
	con.gridx = 0;
	con.gridy = 0;
	con.anchor = GridBagConstraints.CENTER;
	con.fill = GridBagConstraints.HORIZONTAL;
	grid.setConstraints(password, con);
	setLayout(grid);
}

/* compute the MD5 digest of the input string with the password on the front */

public String digest(String value) {
    System.out.println(value + " + " + password.getText());
    MD5 digest = new MD5();
    digest.init();
    digest.updateASCII(password.getText());
    digest.updateASCII(value);
    digest.finish();
    System.out.println(digest.toString());
    return digest.toString();
}

/* eval an arbitrary javascript command, return a string value */
/* just testing now */

public String js(String script) {
    JSObject win;
    win = JSObject.getWindow(this);
    String result = (String)win.eval(script);
    /* JSObject.alert(result); */
    return result;
}
}
