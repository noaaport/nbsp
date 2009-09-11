dnl
dnl $Id$
dnl

define(match,
	if {[regexp {$2} $1]} {
		filter $fpath "$3";
		return;	
	}
)dnl

define(matchmore,
	if {[regexp {$2} $1]} {
		filter $fpath "$3";
	}

)dnl

