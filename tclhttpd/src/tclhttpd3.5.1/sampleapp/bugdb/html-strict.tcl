# html-strict.tcl --
#
#    Procedures that help output strict XHTML.
#
# RCS:
#
#    $Author: nieves $
#    $RCSfile: html-strict.tcl,v $
#    $Date: 2007/01/06 16:44:04 $
#    $Revision: 1.1.1.1 $

package require html

package provide html-strict 1.0

namespace eval html-strict {
    namespace export *
}

# html-strict::head --
#
#    Produce the head entities and attributes of an XHTML file
#    including the opening body entity.
#
# Arguments:
#    title    The string that is displayed in the title tags.
#
#    css      List of pair values containing the URI to the stylesheet
#             and it's media type (e.g. screen, print, etc.).
#
#    js       List of javascript URIs.
#
# Results:
#    A string containing XHTML.
#

proc html-strict::head {title css js} {

    # Initial HTML    
    set html {<?xml version="1.0" encoding="iso-8859-1"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
      "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
  <title>}

    append html $title</title>

    # Stylesheets
    foreach {ss media} $css {
        append html "\n  <link href=\"$ss\" type=\"text/css\" rel=\"stylesheet\" media=\"$media\" />"
    }
    
    # Javascripts
    foreach script $js {
        append html "\n  <script type=\"text/javascript\" src=\"$script\"></script>"
    }
    
    append html </head><body>
    return $html
}

# html-strict::foot --
#
#    Produce the closing XHTML entities (body and html).
#
# Arguments:
#    None.
#
# Results:
#    A string containing XHTML.
#
proc html-strict::foot {} {
    return </body></html>
}

# html-strict::th --
#
#    Produce a the entities for a table header.
#
# Arguments:
#    th        A list containing table header titles.
#
# Results:
#    A string containing XHTML.
#
proc html-strict::th {th} {
    
    set html <tr>
    
    foreach cell $th {
        append html <th>$cell</th>
    }
    
    append html </tr>
    return $html
}
