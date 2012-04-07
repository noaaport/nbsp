# This implements a simple file upload and deletion facility,
# using the core Upload domain.
# based on http://mini.net/tcl/4368

package require httpd::upload

Upload_Url /fileupload [file join [Doc_Root] hup] FileUpload -totalbytes 10000000

proc FileUpload {args} {
    return [Redirect_Self /upfile]
}

Direct_Url /upfile UpFile

proc UpFile {} {
    append html {
	<form ENCtype=multipart/form-data action=/fileupload method=post>
	<input type=submit value="Upload">
	<input type=file name=the_file>
	</form>
    }

    set files [glob -nocomplain -- [file join [Doc_Root] hup *]]
    if {$files != {}} {
	append html {
	    <form action=/upfile/filedelete method=post>
	    <table border="1">
	    <th><input type=submit value="Delete"></th>
	    <th>Files</th>
	    <th>Size</th>
	    <th>Date</th>
	}
	
	set file ""
	foreach f $files {
	    set file [file tail $f]
	    append html [subst {
		<tr>
		<td align="center">
		<input type="checkbox" name="$file">
		</td>
		<td>$file</td>
		<td>[file size $f]</td>
		<td>[clock format [file mtime $f] -format "%x %X"]</td>
		</tr>
	    }]
	}
	
	append html {
	    </table>
	    </form>
	}
    }
    return $html
}

proc UpFile/filedelete {args} {
    foreach {name value} $args {
	set name [file tail [file normalize $name]]
	file delete [file join [Doc_Root] hup $name]
    }
    return [Redirect_Self /upfile]
}
