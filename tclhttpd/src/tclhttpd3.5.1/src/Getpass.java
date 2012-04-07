// SpecTcl generated class Getpass, version 1.0

import java.awt.*;

public class Getpass extends Auth {		

public TextField password;
public Label label_3;

public void init() {
	GridBagLayout grid = new GridBagLayout();
	int rowHeights[] = {0,3,2,6};
	int columnWidths[] = {0,30,30};
	double rowWeights[] = {0.0,0.0,1.0,0.0};
	double columnWeights[] = {0.0,0.0,1.0};
	grid.rowHeights = rowHeights;
	grid.columnWidths = columnWidths;
	grid.rowWeights = rowWeights;
	grid.columnWeights = columnWeights;

	label_3 = new Label();
	label_3.setText("password:");
	this.add(label_3);
	
	password = new TextField(8);
	password.setEchoCharacter('*');
	this.add(password);
	

	// Geometry management
	GridBagConstraints con = new GridBagConstraints();

	reset(con);
	con.gridx = 1;
	con.gridy = 3;
	con.anchor = GridBagConstraints.CENTER;
	con.fill = GridBagConstraints.NONE;
	grid.setConstraints(label_3, con);

	reset(con);
	con.gridx = 2;
	con.gridy = 3;
	con.anchor = GridBagConstraints.CENTER;
	con.fill = GridBagConstraints.HORIZONTAL;
	grid.setConstraints(password, con);


	// Resize behavior management and parent heirarchy
	setLayout(grid);

	// Give the application a chance to do its initialization
	super.init();
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
