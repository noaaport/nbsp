#
# Sunscript web site template definitions
#
# SCCS: @(#) sunscript.tcl 1.5 97/12/23 21:11:10
#

package provide sunscript 1.2
package require Tcl 8.0
package require opt 0.1

# Menus items

catch {unset sunscript}
array set sunscript {

    Order {home about download plugin java techcorner misc} 
    home	{""		"Home"}

    about	{about/ 	"Overview"}

    download	{TclTkCore/	"Download"}

    plugin	{plugin/ 	"Tcl Plug-in"}
    java	{java/ 	  "Tcl & Java"}

    techcorner	{techcorner/ 	"Tech Corner"}
    misc	{misc/		"More Links"}

}

# Page dimensions infos

array set sunscript {
    imgWidth    110
    imgHeight   22
    borderWidth 12
    bevelWidth 3
    verticalPadding 5
    fontSize 15
}

proc SunscriptHead {title {author "Sunscript"} {location {}}} {
    global page
    if {[string length $location]} {
	set page(location) $location
    }
    set page(nextAlign) left
    return [Head][Title $title][Author $author]
}

proc SunscriptItem {link label} {
    global page
    set html "<p align=$page(nextAlign)>\n"
    if {[string compare $page(nextAlign) "left"]==0} {
	set page(nextAlign) "right"
    } else {
	set page(nextAlign) "left"
    }

    append html "<a href=\"$link\" style=\"color: #cc3300\"><font color=\"#cc3300\">$label</font></a><br>\n"
    return $html
}

proc SunscriptBody {} {
    global page
    # Don't use color names because of NS1
    Body bgcolor="#ffffff" text="#000000"
}

proc SunscriptSpacer {width height {alt ""}} {
    global page
    append html "<img src=\"$page(root)images/s.gif\" height=$height width=$width alt=\"$alt\" border=0>"
}

proc SunscriptMenu {{subtitle ""} {menulist {}} {morehtml {}}} {
    global sunscript page

    set html [SunscriptBody]
    set page(sunscriptmenu) 1

    # The page is a 1-row table with three columns.
    # The left column is the menu
    # The middle column is a spacer
    # The right column is for the main body

    foreach varName {imgWidth imgHeight borderWidth 
                     verticalPadding fontSize} {
	set $varName $sunscript($varName)
    }
    append html "<!-- SCCS: @(#) sunscript.tcl 1.5 97/12/23 21:11:10 -->\n"
    append html "<!-- Generated too look ok in Lynx as well as the\
	    latest Netscape/IE... -->\n"
    append html "<table border=0 cellspacing=0 cellpadding=0>\n"
    append html "<tr>\n<td valign=top><!-- bgcolor=\"#CC9999\" -->\n"
    append html "<table width=$imgWidth cellpadding=0 cellspacing=0 border=0>\n"

    set item home
    ::tcl::Lassign $sunscript($item) url label
    set href $page(root)$url
#    if {[string compare $page(location) $item] == 0} {
#	set img logo128_on.gif
#    } else {
#	set img logo128.gif
#    }
    set img logo125.gif

append html "<tr><td align=center><a href=\"$href\" target=\"_top\"><img src=\"$page(root)images/$img\" width=82 height=125 alt=\"Tcl/Tk Home\" border=0></a></td></tr>\n"

    # Skip Home
    foreach item [lrange $sunscript(Order) 1 end] {
	::tcl::Lassign $sunscript($item) url label
	set href $page(root)$url
	if {[string compare $page(location) $item] == 0} {
	    set img ${item}_on.gif
	} else {
	    set img $item.gif
	}
	append html "<tr><td>[SunscriptSpacer 1 $verticalPadding |]</td></tr>"
	append html "\n<tr><td align=center><a href=\"$href\" target=\"_top\"><img src=\"$page(root)images/menu/$img\" width=$imgWidth height=$imgHeight alt=\"$label\" border=0></a></td></tr>\n"
    }

    # NOTE: The html code generated above and below is VERY TRICKY
    # to get right on 'all' browsers. Please consider twice then
    # test on at least  NS2,NS4, and lynx on Unix, and IE3, NS3 and NS4 on
    # Windows (they are all very different) before putting back

    if {[string length $subtitle]} {
	append html "<tr><td>[SunscriptSpacer 1 $verticalPadding |]</td></tr>"
	append html "<tr><td align=left><font style=\"font-size: ${fontSize}px; font-family: Helvetica\">$subtitle:</font></td></tr>\n"
    }

    set halfPadding [expr {int($verticalPadding/2+1)}]

    foreach {href label} $menulist {
	append html "<tr><td>[SunscriptSpacer 1 $halfPadding]</td></tr>"
	append html "<tr><td align=left><img src=\"$page(root)images/cross.gif\" border=0 height=5 width=5 alt=\"+\"> <a href=\"$href\" style=\"font-size: ${fontSize}px; font-family: Helvetica\" target=\"_top\">$label</a></td></tr>\n"
    }
    append html "</table></td>\n"
    append html "<td><img src=\"$page(root)images/s.gif\" height=1 width=$borderWidth alt=\"\"><p></td>\n"
    append html "<td valign=top>"

    append html $morehtml

    return $html
}

# Use [SunscriptEnd 0] for a page end without footer and [SunscriptEnd] for
# the footer
proc SunscriptEnd {{footer 1}} {
    global page
    set html ""
    if {$footer} {
	append html [_SunscriptFooter]
    }
    append html "<p align=right><font size=\"1\">Last modifed: [LastChange]</font></p>"
    if {[info exists page(sunscriptmenu)]} {
	append html [_SunscriptMenuEnd]
    }
    append html [End]
}

# SunscriptFooter is private too, you should use SunscriptEnd instead
proc _SunscriptFooter {} {
    global sunscript page
    set html "<br clear=all>\n<hr>\n<p>\n<center><font size=-1>\n"

    set sep ""
    foreach item $sunscript(Order) {
	::tcl::Lassign $sunscript($item) url label
	regsub -all & $label {\&amp;} label	;# GENERALIZE
	append html "$sep<a href=\"$page(root)$url\" target=\"_top\">$label</a>"
	set sep " | "
    }
    append html "\n</font></center>"
}

# Private function, SunscriptEnd should be called, not this one directly
proc _SunscriptMenuEnd {} {
    return "</td></tr>\n</table>\n"
}

proc SunscriptHeading {width size text {link ""}} {
    set html {}
    append html "<center><table width=\"$width\"><tr><td width=\"$width\" align=center bgcolor=\"#003399\">"
    if {[string length $link]} {
	append html "<a href=\"$link\" style=\"color: #ffffff\">"
	set more "</a>"
    } else {
	set more ""
    }
    # We use style: we need to avoid white/white result for
    # browsers which don't support bgcolor in tables...
    append html "<font size=\"$size\" face=\"Utopia, Helvetica\" color=\"#ffffff\">$text</font>$more"
    append html "</td></tr></table></center>"
    return $html
}


proc SunscriptH1 {{text ""} {link ""}} {
    global page
    if {[string length $text] == 0} {
	set text $page(title)
    }
    # The trailing <p> is a workaround IE3 bug.
    return "<h1>[SunscriptHeading 100% 6 $text $link]</h1>\n"
}

proc SunscriptH2 {text {link ""}} {
    return "<h2>[SunscriptHeading 67% 5 $text $link]</h2>\n"
}

