# ------------------------------------------------------------------------------
# Metakit@Web -- Webadministration-interface for Metakit with Tclhttpd
# 
# INSTALLATION:
# * Simply copy this file to <Tclhttpd-path>/custom
# * Adapt "aConfig(databaseDir)" to your "metakit"-directory
# * Restart Tclhttpd and go to "http://your.domain/mkweb"
# * Because of security-reasons you should be sure that  nobody has 
#   access to your Tclhttpd :-) 
# 
# Stefan Vogel (stefan at vogel-nest dot de)
# 
# HISTORY:
# 11/4/2003: Version 0.5
#  * Minor bugfixes (Thanks to Stefan Finzel and Jean-Claude Wippler)
#
# 10/28/2003: Version 0.4
#  * restructured (more MVC-like ...)
#  * subviews are now supported
#  * moved configuration in extra-window
#  * added gimmick: (un)mark-images (see metakit::execute/image.gif)
#  * fixed several bugs and introduced lots of new ones
# 
# 10/22/2003: Version 0.3
#  * changed Httpd_Redirect to the correct way for redirects:
#    return -code 302 ... (see eof metakit::web::execute)
#  * fixed minor bug in AddCursor
# 
# 10/22/2003: Version 0.2 (initial)
# 
# by Stefan Vogel (stefan at vogel-nest dot de), see
# http://mini.net/tcl/10241
# ------------------------------------------------------------------------------

if {[catch {package require Mk4tcl}]} {
  # Not running in Tclkit
  if {$Config(debug)} {
    puts "Skipping [file tail [info script]] which requires tclkit"
    return
  }
}

package require html

namespace eval  metakit::web {
    variable aConfig 
    array set aConfig {
        version 0.5
        prefix mkweb
        databaseDir /metakits
        maxRows {20}
        maxSubRows {4}
        maxChars {250}
        rowColsTexts {10 20}
        displayBinary 0
        subviewsEmbedded 1
        title "Metakit@Web"
    }
    # just a mapping to display "S" as datatype even if that's default
    variable aDatatypeMap
    array set aDatatypeMap {"" S I I F F L L B B M M D D}
    ::html::init [list input.size 15]
    ::html::headTag [subst {link rel="stylesheet" href="/$aConfig(prefix)/mkweb.css"}]
    ::html::headTag [subst {script language="JavaScript" src="/$aConfig(prefix)/mkweb.js" type="text/javascript"></script}]
}

# plug the given path to Metakit@Web into Tclhttpd
Direct_Url /$::metakit::web::aConfig(prefix) metakit::web::execute

# the following links exist:
# <prefix>/help         - display help
# <prefix>/mkweb.css    - deliver stylesheet
# <prefix>/mkweb.js     - deliver javascript
# <prefix>/image.gif    - display image
# <prefix>/download.bin - download-link for binary columns
# <prefix>/saverow      - posted to, to save an edited row
# 
# all other go directly to proc "execute" with parameter "cmd"
# execute acts a bit like a "controller".

# Main-method, this acts a little bit like a controller-template
# given a "cmd" it dispatches to the "action"
proc metakit::web::execute {args} {
    variable aConfig
	array set params $args
	set cmd ""
	set redirect ""
	switch -- [Show params(cmd)] {

		db_create_new_ui {
			# show html-input-form to create a new database
			set cmd {GenerateHtml [ui_createNewDb]}
		}
		db_create_new {
			# action to really create the database
			set cmd {mk_createDb $params(filename)}
			set redirect [Href "" "" cmd db_list]
		}
		db_delete {
			# delete database, no html-form necessary
			set cmd {mk_deleteDb [file join $aConfig(databaseDir) $params(filename)]}
                        set redirect [Href "" "" cmd db_list]
		}
		db_list {
			# show overview over all file in databaseDir
			set cmd {GenerateHtml [ui_listDb $aConfig(databaseDir)]}
		}

		views_list {
			# show overview of all views in selected metakit
			if {[Show params(db)] == ""} {
				# initial case when called with "nothing"
				set cmd {GenerateHtml ""}
			} else {
				set cmd {GenerateHtml [ui_listViews $params(db)]}
			}
		}
		view_create_ui {
			# html-input-form to create a new view
			set cmd {GenerateHtml [ui_createView $params(db)]}
		}
		view_create {
			# action to really create view, redirect to the list of all views
			set cmd {mk_createView $params(db) $params(view) $params(structure)}
			set redirect [Href "" "" cmd views_list db $params(db)]
		}
		view_modify_ui {
			set cmd {GenerateHtml [ui_modifyView $params(db) $params(view)]}
		}
		view_modify {
			set cmd {mk_modifyView $params(db) $params(view) $params(structure)}
			set redirect [Href "" "" cmd views_list db $params(db)]
		}
		view_delete {
			set cmd {mk_deleteView $params(db) $params(view)} 
			set redirect [Href "" "" cmd views_list db $params(db)]
		}

		content_browse {
			set cmd {GenerateHtml [ui_browseContent $params(db) $params(view) $args]}
		}
		content_new_ui {
			set cmd {GenerateHtml [ui_newContent $params(db) $params(path) $args]}
		}
		content_edit_ui {
			set cmd {GenerateHtml [ui_editContent $params(db) $params(path) $args]}
		}
		content_delete {
			# maybe marked multiple elements to delete (via checkboxes), all beginning with delete_col_...
			set lPath [Show params(delete_col)]
			foreach elem [array names params delete_col_*] {
				lappend lPath [string range $elem 11 end]
			}
			set cmd {mk_deleteRow $params(db) $lPath}
			array unset params delete_col*
			set redirect [Href "" [array get params] cmd content_browse]
		}
		content_browse_subview {
			set cmd {GenerateHtml [ui_browseSubViewContentWrap $params(db) $params(view) $params(index) $params(subview) $args]}
		}

		"" {
			# initial call, return frameset
			set cmd {GetFrameset $aConfig(prefix)}
		}
		default {
			error "No such cmd '[Show params(cmd)]'!"
		}
	}
	set result [eval $cmd]
        if {$redirect != ""} {
		return -code 302 $redirect
        }
	return $result
}

################################################################################
# Direct-Urls for different-purposes
# All direct-urls listed here are not dispatched from the controller-proc "execute"
# but instead called directly from someplaces

# issued from edit/save or new row/create
# not called via controller because the enctype is multipart/form-data
# and therefore the args look a little bit different
proc metakit::web::execute/saverow {} {
	# decoding isn't done in multipart/form-data
        foreach {n v} [ncgi::nvlist] {
            set _args($n) [lindex $v 1]
        }
        mk_openDb [set db $_args(db)]

	if {[mk_hasIndex $_args(path)]} {
		# update-operation, action from clicking "save" in edit-form (update)
		foreach {key value} [array get _args colvalue_*] {
			set key [string range $key 9 end]
			if {[Show _args(bool_colvalue_$key)] != 1} {
				lappend lRow $key $value
			}
		}
		eval mk::set $_args(path) $lRow
	} else {
		# insert operation
		foreach {key value} [array get _args colvalue_*] {
			lappend lRow [string range $key 9 end] $value
		}
		eval mk::row append $_args(path) $lRow
	}
	mk::file commit db
	mk_closeDb $db
	# delete unnecessary (save-specific) parameters from array
	array unset _args bool_colvalue_*
	array unset _args colvalue_*
	array unset _args path
	# redirect to content-display-page
	return -code 302 [Href "" [array get _args] cmd content_browse]
}

# get data from binary-columns
# needs parameters: db, path, column
proc metakit::web::execute/download.bin {args} {
    array set _args $args
    set ::metakit::web::execute/download.bin application/octet-stream
    mk_openDb $_args(db)
    set result [mk::get $_args(path) $_args(column)]
    mk_closeDb db
    return $result
} 

# Configuration-dialog
proc metakit::web::execute/config {args} {
    variable aConfig
    array set _args $args
    if {[Show _args(mode)] == "save"} {
        # set retrieved values into config-array and go back
        foreach var {maxRows maxSubRows maxChars displayBinaries subviewsEmbedded} {
			set aConfig($var) [Show _args($var)]
        }
        set aConfig(rowColsTexts) [list $_args(rowsText) $_args(colsText)]
		# return "pop-down"-statement
        return {<html><head><script type="text/javascript">window.close();</script></head><body></body></html>}
    } else {
        # browse-mode
        return [GenerateHtml [subst {<h1>Configuration</h1>
<form action="[Href /config ""]" method="get">
[GenerateHiddenFields mode save]
<table>
<tr><td>How many rows for editable textareas:</td><td><input type="text" name="rowsText" value="[lindex $aConfig(rowColsTexts) 0]" size="2"></td></tr>
<tr><td>How man columns for editable textareas:</td><td><input type="text" name="colsText" value="[lindex $aConfig(rowColsTexts) 1]" size="2"></td></tr>
<tr><td>Display Binaries?</td><td><input type="checkbox" name="displayBinaries" value="1"[expr {[Show aConfig(displayBinaries)] == 1 ? " checked" : ""}]></td></tr>
<tr><td>Maximum rows displayed for view:<br>
(leave empty if you want to display all)</td><td><input type="text" name="maxRows" value="$aConfig(maxRows)" size="2"></td></tr>
<tr><td>Maximum rows displayed for subview:<br>
(leave empty if you want to display all)</td><td><input type="text" name="maxSubRows" value="$aConfig(maxSubRows)" size="2"></td></tr>
<tr><td>Maximum number of characters displayed in content-cell:<br>
(leave empty if you want to display all)</td><td><input type="text" name="maxChars" value="$aConfig(maxChars)" size="2"></td></tr>
<tr><td>Show subviews embedded in view?</td><td><input type="checkbox" name="subviewsEmbedded" value="1"[expr {[Show aConfig(subviewsEmbedded)] == 1 ? " checked" : ""}]></td></tr>
<tr><td colspan="2" align="middle"><input type="submit" class="button" value="Save">&nbsp;<input type="button" class="button" value="Cancel" onclick="window.close();"></td></tr>
</table>
</form>}]]
    }
}

################################################################################
# Help - to use this from other functions args contains the "keyword" for help
#        as first parameter. If no keyword is given, display the whole page
#        (usually when starting mkweb)
proc metakit::web::execute/help {args} {
    variable aConfig
	switch -- [lindex $args 0] {
		search {
			set result [subst {E.g. -glob name "Hello w*"<br>
				[::html::tableFromList {
"<em>prop value</em>" "Numeric or case-insensitive match"
"<em>prop value</em>" "Numeric or case-insensitive match"
"<em>-min prop value</em>" "Property must be greater or equal to value (case is ignored)"
"<em>-max propvalue</em>"  "Property must be less or equal to value (case is ignored)"
"<em>-exact prop value</em>" "Exact case-sensitive string match"
"<em>-glob prop pattern</em>" "Match 'glob-style' expression wildcard"
"<em>-globnc prop pattern</em>" "Match 'glob-style' expression, ignoring case"
"<em>-regexp prop pattern</em>" "Match specified regular expression"
"<em>-keyword prop word</em>" "Match word as free text or partial prefix"
}]}]
		}
		columns {
			set result {Structure: e.g. <code>id:I name:S salary:F</code><br>
where: <table><tr><td>:S</td><td>string</td><td></td><td>:F</td><td>float</td></tr>
<tr><td>:I</td><td>integer</td><td></td><td>:D</td><td>double</td></tr>
<tr><td>:L</td><td>long</td><td></td><td>:B</td><td>binary</td></tr></table>}
		}
		default {
			set result [subst {<center><h1>Welcome to Metakit@Web (Version $aConfig(version))</h1></center>
<p>Just to get myself familar with <a href="http://www.equi4.com/metakit.html" target="_blank">Metakit</a> I tried to locate a good "viewer".
But unfortunately all those Metakit-viewers around either don't work or they don't do what I want.</p>
<p>That was the time I decided to write a little web-frontend (nowadays it's popular to have a web-frontend 
for almost anything, e.g. MySql). But because the web-frontend should be as easy as possible 
I chosed Tcl and the <a href="http://www.tcl.tk/software/tclhttpd/" target="_blank">Tcl-Webserver (Tclhttpd)</a>
for this purpose.</p>
<p>And what should I say? Tclhttpd is the coolest webserver I've ever seen.</p>
Features of Metakit@Web are:
<ul><li>easy installation in Tclhttpd (only one file)</li>
<li>easy editing, manipulating Metakit-files (useful for rapid-prototyping)</li>
<li>easy browsing through content of Metakit-files</li>
</ul>
Missing features are (resp. pay ATTENTION!!):
<ul><li>no authorization, everybody can modify or delete the Metakit-files (this tool is only meant to be
used by you on your local-machine)</li>
<li>only useful for one user (no sessions)</li>
<li>Only Javascript-confirmation for deleting. Site should not be spidered, because if a spider follows
a "delete"-link ... boom!!</li>
</ul>
<h2>Installation/Configuration</h2>
<p>As you seem to be able to read this, you have successfully installed Metakit@Web.<br>
Just to mention a few things:</p>
<p>You should create a directory where you place your Metakit-files which should be editable from
Metakit@Web. This directory has to be configured in the <code>mkweb.tcl</code>-file 
(Variable: <code>aConfig(databaseDir)</code> currently set to: <code>$aConfig(databaseDir)</code>).</p>
<p>Furthermore you can set the <code>aConfig(prefix)</code> (currently: <code>$aConfig(prefix)</code>). 
This variable sets the url-prefix under which you can access Metakit@Web, e.g. now it is:<br>
<a href="http://$::env(HTTP_HOST)/$aConfig(prefix)">http://$::env(HTTP_HOST)/$aConfig(prefix)</a>.</p>
<p>You can also configure some values via the configuration-frame below. Simply enter a different value
and click "Save". Afterwards you should "reload" the pages in which you expect the changes.</p>
<p><a href="#" onclick="window.open('[Href /config ""]', 'Configuration', 'dependent=yes,resizable=yes,width=450,height=300'); return false;">Configuration-options</a> (see popup from link in the "DB-frame") are:
[::html::tableFromList {
	{How many rows/columns for editable textareas} 
	{The size of the textarea when editing (:S - string)-values. If you have stored large strings it may make sense to enlarge the size. If the page-rendering takes too long it might be a good idea to reduce this value.}
	{Display Binaries?}
	{Well, I don't know what you have stored inside the binary-column. Sometimes I'm using it to store text-files. Check this, to display the binary-column as text.}
	{Maximum rows displayed for view}
	{How many rows of a view should be displayed? If you set this to "" all rows will be shown (possibly only a good idea for small views). Keep in mind that this is a Web-interface.}
	{Maximum rows displayed for subview}
	{Subviews are displayed embedded in it's view. So when you have large subviews you may wish to reduce these values.}
	{Maximum number of characters displayed in content-cell}
	{When viewing the content of a view, all string-values will be truncated to the size given here (... is appended, if string is truncated). If you want to see the full length, set this value to "".}
	{Show subviews embedded in view?}
	{I'd like to have the first rows of subviews embedded inside the views. If you don't like it, remove the check and instead of the subview-data only a link to the subview is shown.}}]
<h2>General remarks</h2>
<p>The frames of Metakit@Web look like this:
<table border="1" width="100%">
<tr><td><em>File/DB-frame</em><br>This lists all the files from <code>aConfig(databaseDir)</code>. Be careful that you are using metakits here. Deleting a database will delete the file physically!<br>
Here you find the link to the configuration-window and the "help"-link.</td>
<td rowspan="2"><em>Main-frame</em><br>
This frame displays the content of the views (and the help).</td></tr>
<tr><td><em>View-frame</em><br>If you select (click) a file from the DB-frame, the views and the structure of the views will be shown here. You can modify the views.</td></tr>
</table></p>
<p>In general the navigation (back, refresh, ..) is left to you and your browser, 
so you won't find any "back"-buttons in Metakit@Web.<br>
Normally the names of database-files, view-names, row-indices link to the content of the item.<br>
The last column of the table-header-column (database, views or rows) contains the "New ..."-link. Click it to enter a new row, view, ...</p>
<p>Usually you go to the "File/database"-frame, select a file, go to the "View"-frame and select a view (the content
is displayed in the main-area).
<h2>Thanks</h2>
<p>To Jean-Claude Wippler for his cool <a href="http://www.equi4.com/metakit.html" target="_blank">Metakit</a>
and his motivation to refine this tool. Thanks to Stefan Finzel, Reinhard Max and W. Jeffrey Rankin for their bug-reports, patches and helping me understand Tclhttpd.
And thanks to the whole Tcl-community for their never-ending patience and helpfully
comments.</p>
<p>Did I already say that Tclhttpd and Metakit are sooooo coool? :-)</p>
You can find information about this script on the <a href="http://wiki.tcl.tk/Stefan%20Vogel">Tcl'ers Wiki</a> or download
it from my 
<a href="http://www.vogel-nest.de/tcl" target="_blank">Homepage</a>.</p>
<pre>This script is free software. Use at your own risk.
Don't blame me if something goes wrong.
But tell me, if you like it :-)
</pre>
<p>Suggestions or bug-fixes are always welcome, simply drop me an email.<br>
Stefan Vogel -- stefan at vogel-nest dot de</p>
}]
            set result [GenerateHtml $result]
        }
    }
    return $result
}

################################################################################
# CSS and JS

# deliver the styleshet for Metakit@Web
proc metakit::web::execute/mkweb.css {} {
    set ::metakit::web::execute/mkweb.css text/css
    set result {
body { font-family:Arial,sans-serif; font-size:10pt; }
h1 { font-size:14pt; color:#000080; }
h2 { font-size:12pt; color:#000080; }
td { font-family:Arial,sans-serif; font-size:10pt; vertical-align:top; }
th { font-family:Arial,sans-serif; font-size:10pt; vertical-align:top; }
.dml td,.cnt td { border:1px solid black; }
.dml th { border:1px solid black; }
.cnt th { border:1px solid black; background-color:#a4a4ff; color:#ffffff; }
.subviewcnt th { border:1px solid black; background-color:#c0c0ff; color:#ffffff; font-size:8pt; }
.subviewcnt td { border:1px solid black; font-size:8pt; }
.scnd { background-color:#d9d9ff; }
.subfirst { background-color:#ffffff; font-size:8pt; }
.subscnd { background-color:#e8e8ff; font-size:8pt; }
a.button { background-color:#a4a4ff; color:#ffffff; border:1px solid black; font-weight:bold; text-decoration:none; padding-left:3px; padding-right:3px; }
.button { background-color:#a4a4ff; color:#ffffff; border:1px solid black; font-weight:bold; }
    }
    return $result
}

proc metakit::web::execute/mkweb.js {} {
    set ::metakit::web::execute/mkweb.js application/x-javascript
    set result {
function linkClick(msg) {
  return confirm(msg);
}
function setChecks(pattern, check) {
  var form = document.browse;
  var pat = eval("/^delete_col_" +pattern+ "![0-9]+$/");
  for (var c = 0; c < form.elements.length; c++)
    if (form.elements[c].type == 'checkbox' && pat.test(form.elements[c].name))
      form.elements[c].checked = check;
  }
}
    return $result
}

proc metakit::web::execute/image.gif {args} {
	# set f [open gif r]
	# fconfigure $f -translation binary
	# set r [read $f]
	# close $f
	# binary scan $r H* result
	set ::metakit::web::execute/image.gif image/gif
	array set _args $args
	set result "474946383961"
	switch -- $_args(name) {
		mark -
		unmark {
			append result "0f000f00f70000050505fbfbfb[string repeat 01 759]\
00000021f904010000ff002c000000000f000f004008"
			if {$_args(name) == "mark"} {
				append result "4300ff091c48b0e03f00000e22443890a14283071f2a5c4891a2c08a181d324c7871234787110982\
84083123c68e24258e44a931a1c78f303b724429b1244c930bff0504003b"
			} else {
				append result "3b00ff091c48b0e03f00000c0e442890a1c2830921229c48d161c58b16232accf81062c787182f36\
d4689023c8880e37a22459d0a4ca912127fe0b08003b"
			}
		}
		pixel {
			if {[info exists _args(color)]} {
				append result "01000100910000$_args(color)0000000000000000002c00000000010001000008040001040400"
			} else {
				# transparentes pixel
				append result "02000200800000ffffff00000021f90401000000002c000000000200020040020284510035"
			}
		}
	}
	return [binary format H* [string map {" " ""} $result]]
}

################################################################################
# html-metakit-specific-functions (prefix "ui_")
# Approx. the "VIEW"-part of MVC

proc metakit::web::ui_createNewDb {} {
	return [subst {[::html::h1 "Create metakit"]
<form action="[Href "" ""]" method="get">
[GenerateHiddenFields cmd db_create_new]
[::html::textInputRow "Filename:" filename]
<input type="submit" class="button" value="Create">
</form>}]
}

proc metakit::web::ui_listDb {dbDir} {
	variable aConfig
	set result [subst {<tr><th>Filename</th><th>Size</th><th><a href="[Href "" "" cmd db_create_new_ui]" class="button">New</a></th></tr>}]
	if {![file isdirectory $aConfig(databaseDir)]} {
		error "<h1>Error</h1>Database-directory: '$aConfig(databaseDir)' does not exist. Please create it or modify <code>mkweb.tcl metakit::web::execute::aConfig(databaseDir)</code>."
	} else {
		append result [::html::foreach db [glob -nocomplain -- $dbDir/*] {<tr><td><a href="[Href "" "" cmd views_list db [set _db [file tail $db]]]" target="views">$_db</a></td><td align="right">[format "%.2f kB" [expr [file size $db]/1024.0]]</td>
<td><a href="[Href "" "" cmd db_delete filename $_db]" class="button" onclick="return linkClick('Really delete complete metakit: $_db?');">Delete</a></td></tr>}]
		return [subst {[::html::h1 "Files"]
<table width="100%">$result</table>
<div align="right"><a href="#" onclick="window.open('[Href /config ""]', 'Configuration', 'dependent=yes,resizable=yes,width=450,height=300'); return false;">Configure $aConfig(title)</a><br>
<a href="[Href /help ""]" target="main">Help</a></div>}]
	}
}

# this is unfortunately a little mix of logic and html
proc metakit::web::ui_listViews {db} {
	variable aDatatypeMap
	mk_openDb $db
	set result [::html::h1 "Views of metakit: '$db'"]
	append result [subst {<table class="dml"><tr><th>View</th><th>Columns</th><th>Datatype</th><th colspan="2"><a href="[Href "" "" cmd view_create_ui db $db]" class="button">New&nbsp;View</a></th></tr>}]
	foreach view [lsort [mk::file views db]] {
		append result [subst {<tr><td><a href="[Href "" "" cmd content_browse db $db view $view]" target="main">$view</a></td><td></td><td></td>
<td><a href="[Href "" "" cmd view_modify_ui db $db view $view]" class="button">Modify</a></td>
<td><a href="[Href "" "" cmd view_delete db $db view $view]" class="button" onclick="return linkClick('Really delete complete view: $view?');">Delete</a></tr>
}]
		foreach col [mk::view layout db.$view] {
			if {[llength $col] > 1} {
				# we have a subview here
				set lColName [lindex $col 0]
				set lDatatype "Subview"
				foreach col [lindex $col 1] {
					foreach {colName datatype} [split $col :] {break}
					lappend lColName "&nbsp;&gt;&nbsp;$colName"
					lappend lDatatype "&nbsp;&gt;&nbsp;$aDatatypeMap($datatype)"
				}
				append result [subst {<tr><td></td><td><em>[join $lColName <br>]</em></td>
<td><em>[join $lDatatype <br>]</em></td><td></td><td></td></tr>
}]
			} else {
				foreach {colName datatype} [split $col :] {break}
				append result [subst {<tr><td></td><td>$colName</td>
<td>$aDatatypeMap($datatype)</td><td></td><td></td></tr>
}]
			}
		}
		append result {<tr><td colspan="5"></td></tr>}
	}
	append result {</table>}
	mk_closeDb db
	return $result
}

proc metakit::web::ui_createView {db} {
	return [subst {<h1>New view for metakit: '$db'</h1>
<form action="[Href "" ""]" method="get">
[GenerateHiddenFields "" db $db cmd view_create]
Viewname:<br><input type="text" name="view" size="40"><br>
Structure:<br><input type="text" name="structure" size="40"><br>
<input type="submit" class="button" value="Create"><br>
[execute/help columns]</form>}]
}

proc metakit::web::ui_modifyView {db viewname} {
	mk_openDb $db
	set result [subst {<h1>Modify metakit: '$db' view: '$viewname'</h1>
<form action="[Href "" ""]" method="get">
[GenerateHiddenFields "" db $db view $viewname cmd view_create]
Structure:<br>
<input type="text" name="structure" value="[mk::view layout db.$viewname]" size="40"><br>
<input type="submit" class="button" name="mode" value="Modify"><br>
<b>Modifying a structure may delete the whole content!</b><br>[execute/help columns]
</form>}]
	mk_closeDb $db
	return $result
}


# the following parameters are used to browse through content
# first - startindex
# count - number of rows to show
# sort  - either sort or rsort
# column - sort - column
# query - user queries
proc metakit::web::ui_browseContent {db viewname paramList} {
	variable aConfig
	# set cmd to content_browse, no matter what 
	array set _args [set p [concat $paramList cmd content_browse]]

	mk_openDb $db
	set view [mk::view open db.$viewname]
	# determine the datatypes for each column and store it
	set lColumns [mk_columnDatatypes $viewname aDatatype]
	set cursor ""

	# attention, e.g for wikit-database we need to escape some html-chars
	set cmd "mk::select db.$viewname"

	# build the metakit-select command from HTTP-query-parameters:
	if {[Show _args(first)] != ""} {
		append cmd " -first $_args(first)"
	}
	if {[Show _args(sort)] != "" && [Show _args(column)] != ""} {
		append cmd " -$_args(sort) $_args(column)"
	}
	if {[Show aConfig(maxRows)] != ""} {
		append cmd " -count $aConfig(maxRows)"
	}
	append cmd " [Show _args(query)]"
	
	set sContent [subst {<tr><th rowspan="2"><a href="#" onclick="setChecks('db.$viewname',false); return false;"><img src="[Href /image.gif "" name unmark]" border="0" width="15" height="15" alt="Unmark all"></a><a href="#" onclick="setChecks('db.$viewname',true); return false;"><img src="[Href /image.gif "" name mark]" border="0" width="15" height="15" alt="Mark all"></a></th><th rowspan="2">index <a href="[Href "" $paramList sort "" column ""]">&uarr;</a></th>}]
	set secondHead ""
	foreach col $lColumns {
		if {$aDatatype($col) == "Subview"} {
			# this is a subview
			append sContent [subst {<th colspan="[llength $aDatatype(:$col)]">$col&nbsp;<em>(Subview)</em></th>}]
			append secondHead "<th>[join $aDatatype(:$col) </th><th>]</th>"

		} else {
			append sContent [subst {<th rowspan="2">${col}:$aDatatype($col) <a href="[Href "" $paramList column $col sort sort]">&uarr;</a>&nbsp;<a href="[Href "" $paramList column $col sort rsort]">&darr;</a></th>}]
		}
	}
	append sContent [subst {<th rowspan="2"><a href="[Href "" $paramList cmd content_new_ui path db.$viewname]" class="button">New&nbsp;row</a></th></tr>
<tr>$secondHead</tr>}]

	set i 0
	foreach index [eval $cmd] {

		append sContent [subst {<tr[expr {[incr i]%2 ? "":" class=\"scnd\""}]><td><input type="checkbox" name="delete_col_db.$viewname!$index" value="1"></td><td><a href="[Href "" $paramList cmd content_edit_ui path db.$viewname!$index]">$index</a></td>}]
		# store the data of this row temporarily into an array
		array unset data
		array set data [mk::get db.$viewname!$index]
		foreach col $lColumns {
			if {$aDatatype($col) == "Subview"} {
				# include subview subview
				append sContent [subst {<td colspan="[llength $aDatatype(:$col)]">
[ui_browseSubViewContent $db $viewname $index $col aDatatype 1 $paramList]</td>}]
			} else {
				append sContent [subst {<td>[PrepareData $data($col) $aDatatype($col) [Href "/download.bin" "" db $db path db.$viewname!$index column $col]]&nbsp;</td>}]
			}
		}
		append sContent [subst {<td><a href="[Href "" $paramList cmd content_delete delete_col db.$viewname!$index]" class="button" onclick="return linkClick('Really delete row: $index?');">Delete</a></tr>\n}]
	}
    # only add a cursor (0-19 20-...) if necessary, that is if there is a "block"-limit (maxRows)
    # and the number of selected rows is larger than the block-size
	if {[Show aConfig(maxRows)] != ""} {
		set cursor <p>[AddCursor [Show _args(first)] $aConfig(maxRows) [$view size] "[Href "" $paramList]"]</p>
	}
	set result [subst {[::html::h1 "Content of metakit: '$db' view: '$viewname'"]
Total view-size: [$view size] rows<br>
Query-Command: <em>$cmd</em><br>
$cursor
<form name="browse" action="[Href "" ""]" method="post">
[GenerateHiddenFields "" db $db view $viewname cmd content_delete]
<table class="cnt">$sContent</table><br>
<input type="submit" class="button" value="Delete marked" onclick="return linkClick('Really delete all marked rows?');">
</form>
<p><form name="query" action="[Href "" ""]" method="get">
[GenerateHiddenFields "" db $db view $viewname cmd content_browse]
<input type="text" name="query" value="[::html::quoteFormValue [Show _args(query)]]" size="30">
<input type="submit" class="button" name="mode" value="Query">
</form></p>
[execute/help search]
</form>}]
    mk_closeDb $db
    return $result
}

proc metakit::web::ui_browseSubViewContentWrap {db viewname index subview paramList} {
	mk_columnDatatypes $viewname aDatatype
	return [subst {<h1>Content of metakit: '$db' view: '$viewname!$index.$subview'</h1>
<form name="browse" action="[Href "" ""]" method="post">
[GenerateHiddenFields "" db $db view $viewname cmd content_delete]
[ui_browseSubViewContent $db $viewname $index $subview aDatatype 0 $paramList]
<input type="submit" class="button" value="Delete marked" onclick="return linkClick('Really delete all marked rows?');">
</form>}]
}

proc metakit::web::ui_browseSubViewContent {db viewname index subview datatype showEmbedded paramList} {
	upvar $datatype aDatatype
	variable aConfig

	set addViewComplete [subst {<a href="[Href "" $paramList cmd content_browse_subview db $db view $viewname index $index subview $subview]">Complete subview: ${viewname}!$index.$subview ...</a>}]
	if {[Show aConfig(subviewsEmbedded)] != 1
		&& $showEmbedded} {
		# subviews shouldn't be displayed embedded in view and we are in the view-view (LOL)
		return $addViewComplete
	}

	# create header-row in subview-table
	set result [subst {<table class="subviewcnt"[expr {$showEmbedded ? " width=\"100%\"" : ""}]><tr><th><a href="#" onclick="setChecks('db.$viewname!$index.$subview',false); return false;"><img src="[Href /image.gif "" name unmark]" border="0" width="15" height="15" alt="Unmark all"></a><a href="#" onclick="setChecks('db.$viewname!$index.$subview',true); return false;"><img src="[Href /image.gif "" name mark]" border="0" width="15" height="15" alt="Mark all"></a></th><th>sub-index</th>
		[::html::foreach subcol $aDatatype(:$subview) {<th>${subcol}:$aDatatype($subview:$subcol)</th>}]
		<th><a href="[Href "" $paramList cmd content_new_ui path db.$viewname!$index.$subview]" class="button">New&nbsp;row</a></tr>
	}]
	set j 0
	set subcmd "mk::select db.$viewname!$index.$subview"
	if {[Show aConfig(maxSubRows)] != "" && $showEmbedded} {
		append subcmd " -count $aConfig(maxSubRows)"
	}
	# create one row in subview
	append result [::html::foreach sIndex [eval $subcmd] {
		<tr class="[expr {[incr j]%2 ? "subfirst":"subscnd"}]"><td><input type="checkbox" name="delete_col_db.$viewname!$index.$subview!$sIndex" value="1"></td><td><a href="[Href "" $paramList cmd content_edit_ui path db.$viewname!$index.$subview!$sIndex]">$sIndex</a></td>
		[::html::foreach {subkey subvalue} [mk::get db.$viewname!$index.$subview.$sIndex] {
			<td>[PrepareData $subvalue $aDatatype($subview:$subkey) [Href "/download.bin" "" db $db path db.$viewname!$index.$subview!$sIndex column $subkey]]&nbsp;</td>
		}]
		<td><a href="[Href "" $paramList cmd content_delete delete_col db.$viewname!$index.$subview.$sIndex]" class="button" onclick="return linkClick('Really delete subrow: $index.$sIndex?');">Delete</td></tr>
	}]
	append result </table>
	if {$j == [Show aConfig(maxSubRows)] && $showEmbedded} {
		append result $addViewComplete
	}
	return $result
}

proc metakit::web::ui_createInputForm {db title lHeaderRow lDataRow paramList} {
	return [subst {<h1>$title in metakit '$db'</h1>
<form action="[Href /saverow ""]" method="post" enctype="multipart/form-data">
[GenerateHiddenFields $paramList cmd content_save]
<table class="cnt">
[eval ::html::hdrRow $lHeaderRow]
[eval ::html::row $lDataRow]
</table><br>
<input type="submit" class="button" value="Save">
</form>}]
}

proc metakit::web::ui_newContent {db path paramList} {
	lappend paramList path $path
	set lDataRow {}
	foreach col [set lHeaderRow [mk_simpleDatatypes $path dummy]] {
		foreach {column datatype} [split $col :] {break}
		lappend lDataRow [PrepareInput $column $datatype]
	}
	return [ui_createInputForm $db "New row in view '$path'" $lHeaderRow $lDataRow $paramList]
}

# 
proc metakit::web::ui_editContent {db path paramList} {
	lappend paramList path $path
	set lDataRow {}
	set lHeaderRow [mk_simpleDatatypes $path aDatatype]
	foreach {col value} [mk::get $path] {
		lappend lDataRow [PrepareInput $col $aDatatype($col) $value]
	}
	return [ui_createInputForm $db "Edit row '$path'" $lHeaderRow $lDataRow $paramList]
}

################################################################################
# mk-specific-functions (prefix: "mk_")
# The "Model"-part of MVC 

proc metakit::web::mk_openDb {filename} {
    variable aConfig
    catch {mk::file close db}
	set filename [file join $aConfig(databaseDir) $filename]
	if {![file exists $filename]} {
		error "No such file: '$filename'"
	}
    mk::file open db [file join $aConfig(databaseDir) $filename]
}

proc metakit::web::mk_closeDb {db} {
    catch {mk::file close $db}
}

proc metakit::web::mk_createDb {filename} {
	variable aConfig
	set filename [file join $aConfig(databaseDir) $filename]
	if {[file exists $filename] || $filename == ""} {
		error "Existing or empty filename, could not create '$filename'!"
	}
	# simply open the file and close it
    mk::file open db $filename
	mk::file close db
}

# this is not really metakit-specific, but anyway
proc metakit::web::mk_deleteDb {filename} {
	if {![file exists $filename]
		|| ![file isfile $filename]} {
		error "Invalid filename '$filename'. Could not delete!"
	}
	file delete -force $filename
}

proc metakit::web::mk_createView {db viewname structure} {
	mk_openDb $db
	mk::view layout db.$viewname $structure
	mk::file commit db
	mk_closeDb $db
}

proc metakit::web::mk_deleteView {db viewname} {
	mk_openDb $db
	mk::view delete db.$viewname
	mk::file commit db
	mk_closeDb $db
}

# assume db is already open
proc metakit::web::mk_simpleDatatypes {path aData} {
	upvar $aData _aData
	variable aDatatypeMap
	set lResult {}
	foreach col [mk::view info $path] {
		foreach {key datatype} [split $col :] {break}
		if {$datatype != "V"} {
			lappend lResult $key:$aDatatypeMap($datatype)
			set _aData($key) $aDatatypeMap($datatype)
		}
	}
	return $lResult
}

# returns 0 for view, 1 for subview
# if there is a pattern like "!<number>." it must be a subview
proc metakit::web::mk_isSubView {path} {
	# subview:  db.view!n.subview[!m], 
	# view: db.view[!n]
	return [regexp -- {![0-9]+\.} $path]
}

# returns 0 if no index is in path (e.g db.view!n.subview)
# used to distinguish between insert and update
proc metakit::web::mk_hasIndex {path} {
	return [regexp -- {![0-9]+$} $path]
}

# assume db is already open
proc metakit::web::mk_columnDatatypes {viewname aData} {
	variable aDatatypeMap
	upvar $aData _aData
	set lColumns {}
	foreach col [mk::view layout db.$viewname] {
		if {[llength $col] > 1} {
			# subview
			set subviewname [lindex $col 0]
			set _aData($subviewname) Subview
			# store the columns of this subview
			set _aData(:$subviewname) {}
			lappend lColumns $subviewname
			foreach col [lindex $col 1] {
				set col [split $col :]
				lappend _aData(:$subviewname) [lindex $col 0]
				# use a colon to split subview-name from subview-column
				set _aData(${subviewname}:[lindex $col 0]) $aDatatypeMap([lindex $col 1])
			}
		} else {
			set col [split $col :]
			set _aData([lindex $col 0]) $aDatatypeMap([lindex $col 1])
			lappend lColumns [lindex $col 0]
		}
	}
	return $lColumns
}

proc metakit::web::mk_deleteRow {db lPath} {
	mk_openDb $db
	# catch the delete, maybe we have deleted a row and
	# afterwards we have path for subviews (which then cannot be deleted anymore, of course)
	# sort the path before deleting, because if <view>!5 is deleted before <view>!7
	# deletion of <view>!7 will fail. dictionary-sorting avoids this (it starts with
	# the highest numbers)
	foreach path [lsort -dictionary -decreasing $lPath] {
		catch {mk::row delete $path}
	}
	mk_closeDb $db
}


#################################################################################
# some little Helperfunctions used throughout mkweb
# starting with "upper-case"-first-character

proc metakit::web::PrepareData {text datatype link} {
    variable aConfig
    set result $text
    switch -- [string tolower $datatype] {
        b {
			if {[Show aConfig(displayBinaries)] != 1
				&& [string length $text]} {
					set result {<em>not displayed</em>}
			}
        }
        default {
			# avoid html-confusion
            set result [string map {< &lt;} $result]
        }
    }
	if {[Show aConfig(maxChars)] != "" 
		&& [string length $result] > $aConfig(maxChars)} {
		set result [string range $result 0 $aConfig(maxChars)]...
	}
	if {[string tolower $datatype] == "b" && [string length $text]} {
		# append download-link for binaries
		append result [subst {<br><a href="$link" target="_blank">Download</a>}]
	}
    return $result
}

proc metakit::web::PrepareInput {col datatype {value ""}} {
    variable aConfig
    if {[info exists aConfig(displayBinaries)] && $aConfig(displayBinaries) == 1} {
        # treat binaries as string
        set datatype S
    }
    switch -- [string tolower $datatype] {
        i - l - f - d {
            set result [::html::textInput colvalue_$col $value]
        }
        b {
            set result [subst {<input type="file" name="colvalue_$col">}]
            if {$value != ""} {
                append result [subst {<br><input type="checkbox" name="bool_colvalue_$col" value="1" checked> Leave value as is.<p>There are 3 alternatives here:<ul>
<li>You want to change the binary value of this row: Upload a new file and "deselect" the checkbox.</li>
<li>Don't touch the binary value of this row: Select checkbox (default).</li>
<li>Delete the value of this row: "Deselect" the checkbox and don't upload a file.</li></ul>
The checkbox has priority, so if the checkbox is selected the binary value of this row won't be changed.</p>}]
            }
        }
        default {
            # S
            set result [::html::textarea colvalue_$col "rows=\"[lindex $aConfig(rowColsTexts) 0]\" cols=\"[lindex $aConfig(rowColsTexts) 1]\"" $value]
        }
    }
    return $result
}

proc metakit::web::GetFrameset {prefix} {
    return [GenerateHtml [subst {<frameset cols="30%,60%">
  <frameset rows="40%,60%">
    <frame src="$prefix?cmd=db_list" name="db" scrolling="yes">
    <frame src="$prefix?cmd=views_list" name="views" scrolling="yes">
  </frameset>
    <frame src="$prefix/help" name="main" scrolling="yes">
</frameset>
<noframes>
  <h2>Frame Alert</h2>
  <p>This document is designed to be viewed using the frames feature. If you see this message, you are using a non-frame-capable web client.</p>
</noframes>}] 0]
}

proc metakit::web::AddCursor {pos count maxCount link} {
    set result ""
	if {$pos == ""} {set pos 0}
    for {set i 0} {$i<$maxCount} {incr i $count} {
        set str "$i - [expr $i+$count-1]"
        if {$i>$pos || $pos>=[expr $i+$count]} {
            set str [subst {<a href="$link&first=$i">$str</a>}]
        }
        append result "&nbsp;$str"
    }
    return $result
}

# Vignette-like: return value of var even if doesn't exist
proc metakit::web::Show {var} {
	upvar $var _var
	if {[info exists _var]} {
		return $_var
	}
	return ""
}

proc metakit::web::GenerateHiddenFields {data args} {
	array set _data [concat $data $args]
	return [::html::foreach {key value} [array get _data] {<input type="hidden" name="$key" value="[::html::quoteFormValue $value]">\n}]
}

# path will be always relative to the prefix (mkweb)
proc metakit::web::Href {path data args} {
	variable aConfig
	array set _data [concat $data $args]
	set lQuery {}
	foreach {key value} [array get _data] {
		if {[string length $value]} {
			lappend lQuery ${key}=[Url_Encode $value]
		}
	}
	# set lQuery [eval http::formatQuery [array get _data]]
	if {[llength $lQuery]} {set lQuery ?$lQuery}
	return "/$aConfig(prefix)$path[join $lQuery &]"
}

# simply wrap the code with <html>-tag (don't insert body-tag, for cases
# like frames ...)
proc metakit::web::GenerateHtml {body {withBodyTag 1}} {
    variable aConfig
    if {$withBodyTag} {set body <body>$body</body>}
    return [::html::head $aConfig(title)]$body\n</html>
}
