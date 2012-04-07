// SpecTcl generated class ServerDemo, version 1.0

import java.awt.*;

public class ServerDemo extends Helper {		

// a slot to hold an arbitrary object pointer that can
// be filled in by the app. and referenced in actions
public Object arg;

public Label label_1;
public TextField password;
public Label label_2;
public TextField entry_2;
public Button button_1;
public TextField result;

//methods to support form introspection
public static String names[] = {
	"label_1","password","label_2","entry_2","button_1","result",
};
public String[] getNames() {
	return names;
}

//There should be an easier way to do this
public Object[] getWidgets() {
	Object[] list = new Object[6];
	list[0] = label_1;
	list[1] = password;
	list[2] = label_2;
	list[3] = entry_2;
	list[4] = button_1;
	list[5] = result;
	return list;
}

// Application specific widget data
private static String datatype_private[] = {
	null, null, null, null, null, null, 
};
public String[] datatype() {
	return datatype_private;
}

private static String writeAccess_private[] = {
	"", "", "", "", "", "", 
};
public String[] writeAccess() {
	return writeAccess_private;
}

private static String appletParams_private[] = {
	null, null, null, null, null, null, 
};
public String[] appletParams() {
	return appletParams_private;
}

private static String helpURL_private[] = {
	"", "", "", "", "", "", 
};
public String[] helpURL() {
	return helpURL_private;
}


public void init() {

	// main panel
	GridBagLayout grid = new GridBagLayout();
	int rowHeights[] = {0,30,30,30};
	int columnWidths[] = {0,30,30};
	double rowWeights[] = {0.0,0.0,0.0,0.0};
	double columnWeights[] = {0.0,0.0,0.0};
	grid.rowHeights = rowHeights;
	grid.columnWidths = columnWidths;
	grid.rowWeights = rowWeights;
	grid.columnWeights = columnWeights;

	label_1 = new Label();
	label_1.setText("Enter password");
	this.add(label_1);
	

	password = new TextField(20);
	password.setEchoCharacter('*');
	this.add(password);
	

	label_2 = new Label();
	label_2.setText("Sensitive data");
	this.add(label_2);
	

	entry_2 = new TextField(20);
	this.add(entry_2);
	

	button_1 = new Button();
	button_1.setLabel("compute");
	this.add(button_1);
	

	result = new TextField(64);
	result.setFont(new Font("Helvetica",Font.PLAIN , 12));
	this.add(result);
	

	// Geometry management
	GridBagConstraints con = new GridBagConstraints();
	reset(con);
	con.gridx = 1;
	con.gridy = 1;
	con.anchor = GridBagConstraints.CENTER;
	con.fill = GridBagConstraints.NONE;
	grid.setConstraints(label_1, con);

	reset(con);
	con.gridx = 2;
	con.gridy = 1;
	con.anchor = GridBagConstraints.WEST;
	con.fill = GridBagConstraints.NONE;
	grid.setConstraints(password, con);

	reset(con);
	con.gridx = 1;
	con.gridy = 2;
	con.anchor = GridBagConstraints.CENTER;
	con.fill = GridBagConstraints.NONE;
	grid.setConstraints(label_2, con);

	reset(con);
	con.gridx = 2;
	con.gridy = 2;
	con.anchor = GridBagConstraints.SOUTHWEST;
	con.fill = GridBagConstraints.NONE;
	grid.setConstraints(entry_2, con);

	reset(con);
	con.gridx = 1;
	con.gridy = 3;
	con.anchor = GridBagConstraints.CENTER;
	con.fill = GridBagConstraints.NONE;
	grid.setConstraints(button_1, con);

	reset(con);
	con.gridx = 2;
	con.gridy = 3;
	con.anchor = GridBagConstraints.CENTER;
	con.fill = GridBagConstraints.NONE;
	grid.setConstraints(result, con);


	// Resize behavior management and parent heirarchy
	setLayout(grid);

	// Give the application a chance to do its initialization
	super.init();
}

public boolean handleEvent(Event event) {
	if (event.target == button_1 && event.id == event.ACTION_EVENT) {
		result.setText(digest(password.getText()));
	} else
	if (event.target == entry_2 && event.id == event.ACTION_EVENT) {
		result.setText(js(entry_2.getText()));
	} else
	if (event.id==event.KEY_ACTION && event.key==event.F4 && event.modifiers==event.ALT_MASK) {  // Alt-F4 always exits
		System.exit(3);
	} else
		return super.handleEvent(event);
	return true;
}

public static void main(String[] args) {
    Frame f = new Frame("ServerDemo Test");
    ServerDemo win = new ServerDemo();
    win.init();
    f.add("Center", win);
    f.pack();
    f.show();
}

private void reset(GridBagConstraints con) {
    con.gridx = GridBagConstraints.RELATIVE;
    con.gridy = GridBagConstraints.RELATIVE;
    con.gridwidth = 1;
    con.gridheight = 1;
 
    con.weightx = 0;
    con.weighty = 0;
    con.anchor = GridBagConstraints.CENTER;
    con.fill = GridBagConstraints.NONE;
 
    con.insets = new Insets(0, 0, 0, 0);
    con.ipadx = 0;
    con.ipady = 0;
}

}
