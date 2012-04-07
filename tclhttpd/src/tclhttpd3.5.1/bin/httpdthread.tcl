# httpdthread.tcl
#
# This script has the per-thread initialization for TclHttpd.
# The httpd.tcl startup script will call this for the main thread,
# and then (if applicable) each worker thread will source this
# file to initialize itself.

# Copyright (c) 1997 Sun Microsystems, Inc.
# Copyright (c) 1998-2000 Scriptics Corporation
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# RCS: @(#) $Id$

# Note about per-thread vs. per-application.  Essentially all
# the "package require" commands are needed in all the threads,
# while it might be possible to limit the various initialization
# calls to only the main thread.  However, it isn't easy to tell,
# so we initialize all threads to ensure that configuration state
# both affects the URL dispatch done by the main thread, and is
# visible to the worker threads.

# Standard Library dependencies
package require ncgi
catch {
    # Prodebug pukes on this because it defines html::foreach
    package require html
}

# Core modules
package require httpd          	;# Protocol stack
package require httpd::version	;# Version number
package require httpd::url	;# URL dispatching
package require httpd::mtype	;# Mime types

# Search for mime.types either right in Config(lib), or down
# one level in the installed tclhttpd subdirectory

foreach path [list \
    [file join $Config(lib) mime.types] \
    [glob -nocomplain [file join $Config(lib) tclhttpd* mime.types]] \
    ] {
  if {[llength $path] > 0} {
    set path [lindex $path 0]
  }
  if {[file exists $path]} {
    Mtype_ReadTypes $path
    break
  }
}
package require httpd::counter	;# Statistics
Counter_Init $Config(secs)
package require httpd::utils	;# handy stuff like "lassign"

package require httpd::redirect	;# URL redirection
package require httpd::auth	;# Basic authentication
package require httpd::log	;# Standard logging
package require httpd::digest	;# Digest authentication

if {$Config(threads) > 0} {
    package require Thread		;# C extension
    package require httpd::threadmgr	;# Tcl layer on top
}

# Image maps are done either using a Tk canvas (!) or pure Tcl.

if {[info exists tk_version]} {
    package require httpd::ismaptk
} else {
    package require httpd::ismaptcl
}
# These packages are required for "normal" web servers

# doc
# provides access to files on the local file systems.

package require httpd::doc

# Doc_Root defines the top-level directory, or folder, for
# your web-visible file structure.

Doc_Root			$Config(docRoot)

# Merge in a second file system into the URL tree.

set htdocs_2 [file join [file dirname [info script]] ../htdocs_2]
if {[file isdirectory $htdocs_2]} {
    Doc_AddRoot /addroot	$htdocs_2
}

# Template_Interp determines which interpreter to use when
# interpreting templates.

Template_Interp		{}

# Doc_IndexFile defines the name of the default index file
# in each directory.  Its value is a glob pattern.

DirList_IndexFile			index.{tml,html,shtml,thtml,htm,subst}

# Doc_PublicHtml turns on the mapping from ~user to the
# specified directory under their home directory.

Doc_PublicHtml			public_html

# Doc_CheckTemplates causes the processing of text/html files to
# first look aside at the corresponding .tml file and check if it is
# up-to-date.  If the .tml (or its dependent files) are newer than
# the HTML file, the HTML file is regenerated from the template.

Template_Check		1

# Doc_ErrorPage registers a template to be used when a page raises an
# uncaught Tcl error.  This is a crude template that is simply passed through
# subst at the global level.  In particular,  it does not have the
# full semantics of a .tml template.

Doc_ErrorPage			/error.html

# Doc_NotFoundPage registers a template to be used when a 404 not found
# error occurs.  Like Doc_ErrorPage, this page is simply subst'ed.

Doc_NotFoundPage		/notfound.html

# Doc_Webmaster returns the value last passed into it.
# Designed to be used in page templates where contact email is needed.

Httpd_Webmaster			$Config(webmaster)

# uncomment the following and commment out the package requires
# if you want to leave out dirlist support
#proc Dir_ListingIsHidden {} {
#    return 1
#}
package require httpd::dirlist		;# Directory listings
package require httpd::include		;# Server side includes

# uncomment this and comment the package requires
# if you want to leave out cgi support
#proc Cgi_Domain {virtual directory sock suffix} {
#	Doc_NotFound $sock
#	return
#}
package require httpd::cgi		;# Standard CGI
Cgi_Directory			/cgi-bin

package require httpd::direct		;# Application Direct URLs

package require httpd::status		;# Built in status counters
Status_Url			/status /images

package require httpd::mail		;# Crude email form handlers
Mail_Url			/mail

package require httpd::admin		;# Url-based administration
Admin_Url			/admin

package require httpd::session		;# Session state module (better Safe-Tcl)

package require httpd::debug		;# Debug utilites
Debug_Url			/debug

package require httpd::redirect	;# Url redirection tables
Redirect_Init			/redirect

package require httpd::doctools ;# doctool type conversions

if {[catch {
    Auth_InitCrypt			;# Probe for crypt module
} err]} {
    catch {puts "No .htaccess support: $err"}
}

# This is currently broken
if {0} {
    package require httpd::safetcl	;# External process running safetcl shells
}

#######################################
# Load Custom Code
#######################################

if {[info exist Config(library)] && [string length $Config(library)]} {
    if {![file isdirectory $Config(library)]} {
	Stderr "Code library \"$Config(library)\" does not exist"
    } else {
	if {$Config(debug)} {
	    Stderr "Loading code from $Config(library)"
	}
	foreach f [lsort -dictionary [glob -nocomplain [file join $Config(library) *.tcl]]] {
            if {[string compare [file tail $f] "pkgIndex.tcl"] == 0} {
              continue
            } elseif {[catch {source $f} err]} {
		Stderr "$f: $err"
	    } elseif {$Config(debug)} {
		Stderr "Loaded [file tail $f]: $err"
	    }
	}
    }
}


