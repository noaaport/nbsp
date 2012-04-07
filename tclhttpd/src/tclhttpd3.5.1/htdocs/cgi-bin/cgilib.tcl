# cgilib.tcl

# Note that these interfaces are deprecated in favor of the new
# ncgi module that is part of the Standard Tcl Library
#
# This file	ncgi module
# Cgi_Parse	ncgi::parse
# Cgi_List	ncgi::nvlist
# Cgi_Query	ncgi::query
# Cgi_Decode	ncgi::decode
# Cgi_Encode	ncgi::encode
# Cgi_Value	ncgi::value
# Cgi_Header	ncgi::header
# Cgi_Redirect	ncgi::redirect

# There is also a new html package in the Standard Tcl Library

proc Cgi_Parse {} {
    global cgi query
    set query [Cgi_Query]
    regsub -all {\+} $query { } query
    foreach {name value} [split $query &=] {
	set name [CgiDecode $name]
	if [info exists list($name)] {
	    set cgi($name) [list $cgi($name) [CgiDecode $value]]
	    unset list($name)
	} elseif [info exists cgi($name)] {
	    lappend cgi($name) [CgiDecode $value]
	} else {
	    set cgi($name) [CgiDecode $value]
	    set list($name) 1	;# Need to listify if more values are added
	}
    }
    return [array names cgi]
} 
proc Cgi_List {} {
    set query [Cgi_Query]
    regsub -all {\+} $query { } query
    set result {}
    foreach {x} [split $query &=] {
	lappend result [CgiDecode $x]
    }
    return $result
}
proc Cgi_Query {} {
    global env
    if {![info exists env(QUERY_STRING)] ||
	    [string length $env(QUERY_STRING)] == 0} {
	if {[info exists env(CONTENT_LENGTH)] &&
		[string length $env(CONTENT_LENGTH)] != 0} {
	    set query [read stdin $env(CONTENT_LENGTH)]
	} else {
	    fconfigure stdin -blocking 0
	    if {[gets stdin query] < 0} {
		set query ""
	    }
	}
    } else {
	set query $env(QUERY_STRING)
    }
    set env(ALT_QUERY_STRING) $query
    return $query
}
proc CgiDecode {str} {
    # Protect Tcl special chars
    regsub -all {[][\\\$]} $str {\\&} str
    # Replace %xx sequences with a format command
    regsub -all {%([0-9a-fA-F][0-9a-fA-F])} $str {[format %c 0x\1]} str
    # Replace the format commands with their result
    return [subst $str]
}
# do x-www-urlencoded character mapping
# The spec says: "non-alphanumeric characters are replaced by '%HH'"
 
for {set i 1} {$i <= 256} {incr i} {
    set c [format %c $i]
    if {![string match \[a-zA-Z0-9\] $c]} {
        set UrlEncodeMap($c) %[format %.2x $i]
    }
}
 
# These are handled specially
array set UrlEncodeMap {
    " " +   \n %0d%0a
}
 
# 1 leave alphanumerics characters alone
# 2 Convert every other character to an array lookup
# 3 Escape constructs that are "special" to the tcl parser
# 4 "subst" the result, doing all the array substitutions
 
proc CgiEncode {string} {
    global UrlEncodeMap 
    regsub -all \[^a-zA-Z0-9\] $string {$UrlEncodeMap(&)} string
    regsub -all \n $string {\\n} string
    regsub -all \t $string {\\t} string
    regsub -all {[][{})\\]\)} $string {\\&} string
    return [subst $string]
}
proc Url_Encode {string} {
    CgiEncode $string
}
 
proc Cgi_Value {key} {
    global cgi
    if [info exists cgi($key)] {
	return $cgi($key)
    } else {
	return {}
    }
}
proc Cgi_Header {title {bodyparams {}}} {
    puts stdout \
"Content-Type: text/html

<HTML>
<Head>
<title>$title</title>
</Head>
<Body $bodyparams>"
}
proc Cgi_Tail {} {
    puts </Body>
}
proc Cgi_Redirect {url} {
    puts stdout "\
Content-type: text/html
Location: $url

Please go to $url
"
}
proc Cgi_CopyBits {file} {
    if {![file exists $file] ||
	[catch {open $file} in]} {
	puts "Content-Type: text/html"
	puts ""
	puts "Cannot find file [file tail $file]"
	exit 0
    }
    switch -- [file extension $file] {
	".hqx" {set type application/mac-binhex40}
	default {set type application/octet-stream}
    }
    puts stdout "Content-Type: $type\nContent-Length: [file size $file]"
    puts ""
    fconfigure stdout -translation binary -buffering full -buffersize 8192
    fconfigure $in -translation binary
    copychannel $in stdout
    close $in
}
proc H1 {str} {
    Html_Tag H1 {} $str
}
proc H2 {str} {
    Html_Tag H2 {} $str
}
proc H3 {str} {
    Html_Tag H3 {} $str
}
proc H4 {str} {
    Html_Tag H4 {} $str
}
proc H5 {str} {
    Html_Tag H5 {} $str
}
proc H6 {str} {
    Html_Tag H6 {} $str
}
proc P {} {
    puts stdout <p>
}
proc Link {text href} {
    puts "<a href=\"$href\">$text</a>"
}
proc Html_Tag {tag params str} {
    puts stdout "<[string trim "$tag $params"]>$str</$tag>"
}
proc Form {url {method POST}} {
    puts stdout "<form action=\"$url\" method=$method>"
}
proc Counter {filename} {
    if [catch {open $filename} in] {
	set number 0
    } else {
	set info [read $in]
	close $in
	if ![regexp {[0-9]+} $info number] {
	    return [clock seconds]	;# Bail - race with file access
	}
    }
    incr number
    # Cannot open $filename.new because we likely won't
    # have permission to create the temp file.
    set out [open $filename w]
    puts $out $number
    close $out
    return $number
}

# Empty --
#
#	Return true if the variable doesn't exist or is an empty string

proc Empty {varname} {
    upvar 1 $varname var
    return [expr {![info exist var] || [string length $var] == 0}]
}

# Cgi_SubstFile --
# Use a file as a template

proc Cgi_SubstFile {path} {
    if {[catch {open $path} in]} {
	puts "<pre>Cgi_SubstFile: $path: $in</pre>"
    } else {
	set X [read $in]
	close $in
	puts [uplevel 1 [list subst $X]]
    }
    flush stdout
}
