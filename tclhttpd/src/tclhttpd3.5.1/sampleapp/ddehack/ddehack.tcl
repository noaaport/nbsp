# ddehack.tcl
#
# This is little utility to turn URL requests into
# DDE operations on Netscape.  The idea is to link
# an exmh email reader running on Unix (but displaying
# on Windows via Exceed) to a Netscape running native
# on the Windows box.  The user clicks the link in
# exmh.  Exmh pokes the URL implemented by TclHttpd.
# TclHttpd uses DDE to get Netscape to load a page.

if {[catch {
    package require dde
}]} {
    # Fail silently if DDE is not available.
    return
}

package provide ddehack 1.0

proc Ddehack_Init {} {
    Direct_Url /dde Ddehack
}

proc Ddehack/hello {args} {
    return hello
}
proc Ddehack/request {server topic argument} {
    dde request $server $topic $argument
    return [list $server $topic $argument]
}

proc Ddehack/openurl {url {windowID 0xFFFFFFFF}} {
    # Try netscape first, then dde

    if {![catch {
	dde request netscape WWW_OpenUrl $url,,$windowID,0x0
    }]} {
	set browser netscape
    } elseif {![catch {
	dde request iexplore WWW_OpenUrl $url,,$windowID,0x0
    }]} {
	set browser iexplore
    } else {
	return
    }
    dde request $browser WWW_Activate $windowID,0x0
    return [list WWW_OpenUrl $url]
}
