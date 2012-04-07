#
# $Id$
#
package provide upperair::fm35 1.0;

# The three external functions are
#
# ::upperair::fm35::decode {body}
# ::upperair::fm35::get_levels {}
# ::upperair::fm35::get_data {level}
# ::upperair::fm35::set_na {s}
# ::upperair::fm35::get_na
#
# "body" is assumed to be a line of the form
#
# USDL02 EDZW 151800 TTAA 15171 10393 99007 01106 25005 00170 01508 25008 \
# 92788 04903 30014 85449 08310 31513 70930 15710 33515 50540 30722 34523
#
# with or without the first three wmo elements. The function seeks the TTAA
# or TTCC and starts from there. The function decodes the data and stores
# the results in the array "fm35_decoded_data". The functions
#
# ::upperair::fm35::get_levels {}
# ::upperair::fm35::get_data {level}
#
# then retrieve those results from the internal data structures.
#
# The function
#
#    ::upperair::fm35::get_levels {}
#
# returns the list of levels for which data is available. The order is:
#    surface, 1000, ..., tropopause, windmax
#
# The function
#
#    ::upperair::fm35::get_data {level}
#
# returns the data for a given level. The returned list depends on the level:
#
#    p_mb temp_c dewp_c wspeed_kt wdir       (surface, tropopause)
#    height_m temp_c dewp_c wspeed_kt wdir   (1000, ... mb)
#    p_mb wspeed_kt wdir                     (windmax)
#
# When a parameter is not available it is returned as the empty string "",
# but that can be changed with function ::upperair::fm35::set_na {s}.
#
package require textutil::split;

namespace eval upperair::fm35 {} {

    variable fm35;
    variable fm35_raw_data;
    variable fm35_decoded_data;
    variable mandatory_levels;
    
    set fm35(na_symbol) "";
    array set fm35_raw_data {};
    array set fm35_decoded_data {};

    set mandatory_levels(ttaa,00) 1000;
    set mandatory_levels(ttaa,92) 925;
    set mandatory_levels(ttaa,85) 850;
    set mandatory_levels(ttaa,70) 700;
    set mandatory_levels(ttaa,50) 500;
    set mandatory_levels(ttaa,40) 400;
    set mandatory_levels(ttaa,30) 300;
    set mandatory_levels(ttaa,25) 250;
    set mandatory_levels(ttaa,20) 200;
    set mandatory_levels(ttaa,15) 150;
    set mandatory_levels(ttaa,10) 100;
    #
    set mandatory_levels(ttcc,70) 70;
    set mandatory_levels(ttcc,50) 50;
    set mandatory_levels(ttcc,30) 30;
    set mandatory_levels(ttcc,20) 20;
    set mandatory_levels(ttcc,10) 10;
}

proc ::upperair::fm35::_strip_zeros {v} {

    set r [string trimleft $v 0];
    if {$r eq ""} {
	set r 0;
    }

    return $r;
}

proc ::upperair::fm35::_decode_temp_dewp {tttdd} {
    #
    # Temp and Dewp
    #
    # TTT - Temperature (.1 C) ... ///  if missing
    # If the tenths digit is odd, the temperature is negative. Otherwise,
    # the temperature is positive.
    # For example:
    # 234 =  23.4 C 
    # 123 = -12.3 C
    #  DD - Dewpoint depression (C) ... // if missing
    # If DD is less than or equal to 50, then the dewpoint depression
    # is in tenths of a degree C.
    # Otherwise, the dewpoint depression is in degrees C plus 50.
    # For example:
    # 65 = 15 C
    # 57 =  7 C
    # 44 =  4.4 C
    # 12 =  1.2 C
    #
    # NOTE: While the raw data contains the "dewp depression"
    # this function returns the dewp temperature.

    if {[regexp {/} $tttdd]} {
	return [list [get_na] [get_na]];
    }

    if {[regexp {(\d{2})(\d)(\d{2})} $tttdd match tt tt_tenth dd] == 0} {
	return -code error "Invalid tttdd.";
    }
    set t "";
    append t [_strip_zeros $tt] "." $tt_tenth;
    if {[expr $tt_tenth % 2] == 1} {
	set t "-$t";
    }
    set temp_c $t;

    set d [_strip_zeros $dd];
    if {$d <= 50} {
	set d [expr $d / 10.0];
    } else {
	set d [expr $d - 50];
    }
    set dewp_c [expr $temp_c - $d];

    # This ensures that there is only one decimal point. Otherwise, e.g.,
    # things like 20.2 - 0.1 can result is 20.099999999999.

    set dewp_c [format "%.1f" $dewp_c];
    
    return [list $temp_c $dewp_c];
}

proc ::upperair::fm35::_decode_wind {dddff} {
    #
    # Wind
    #
    # http://weather.unisys.com/wxp/Appendices/Formats/TEMP.html
    #
    # ddd - Wind direction (to nearest 5 degrees) ... /// if missing
    # If ones digit 1 or 6,  add 100 to wind speed and direction is
    # 0 or 5 repsectively. If ones digit 2 or 7,  add 200 to wind speed
    # and direction is 0 or 5 repsectively
    # ff - Wind speed (knts) ... // if missing
    # If DDD is not an even multiple of 5, the difference between ddd and an
    # even multiple of 5 is then added to the wind speed as the
    # hundredths digit. For example:
    #
    # 31523 = 315 deg at  23 knts
    # 25612 = 255 deg at 112 knts

    if {[regexp {/} $dddff]} {
	return [list [get_na] [get_na]];
    }

    if {[regexp {(\d{3})(\d{2})} $dddff match ddd ff] == 0} {
	return -code error "Invalid dddff.";
    }

    set dir [_strip_zeros $ddd];
    set speed [_strip_zeros $ff];

    if {$dir == 1} {
	set ws_kt [expr $speed + 100];
	set wd_deg 0;
    } elseif {$dir == 6} {
	set ws_kt [expr $speed + 100];
	set wd_deg 5;
    } elseif {$dir == 2} {
	set ws_kt [expr $speed + 200];
	set wd_deg 0;
    } elseif {$dir == 7} {
	set ws_kt [expr $speed + 200];
	set wd_deg 5;
    } else {
	set wd_deg $dir;
	set ws_kt [expr ($dir % 5)*100 + $speed];
    }

    return [list $ws_kt $wd_deg];
}

proc ::upperair::fm35::_decode_height_ttaa {pphhh} {

    if {[regexp {/} $pphhh]} {
	return [get_na];
    }

    if {[regexp {(\d{2})(\d{3})} $pphhh match pp hhh] == 0} {
	return -code error "Invalid pphhh: $pphhh.";
    }

    # In this part the pp is the code (00, 92, 85, ...) coresponding
    # to a given level (1000, 925, 850 mb, ...).
    set h [_strip_zeros $hhh];
    if {$pp == 00} {
	if {$h >= 500} {
	    set h [expr 500 - $h];
	}
    } elseif {$pp == 92} {
	;
    } elseif {$pp == 85} {
	set h [expr $h + 1000];
    } elseif {$pp == 70} {
	if {$h < 500} {
	    set h [expr $h + 3000];
	} else {
	    set h [expr $h + 2000];
	}
    } elseif {($pp == 50) || ($pp == 40)} {
	set h [expr $h * 10];
    } elseif {($pp == 30) || ($pp == 25)} {
	if {$h < 500} {
	    set h [expr $h * 10 + 1000];
	} else {
	    set h [expr $h * 10];
	}
    } elseif {($pp == 20) || ($pp == 15) || ($pp == 10)} {
	set h [expr $h * 10 + 10000];
    } else {
	return -code error "Invalid pphhh: $pphhh.";
    }

    return $h;
}

proc ::upperair::fm35::_decode_height_ttcc {pphhh} {

    if {[regexp {/} $pphhh]} {
	return [get_na];
    }

    if {[regexp {(\d{2})(\d{3})} $pphhh match pp hhh] == 0} {
	return -code error "Invalid pphhh: $pphhh.";
    }

    # In this part the pp is the pressure level itself
    set h [_strip_zeros $hhh];
    if {$pp == 70} {
	set h [expr $h * 10 + 10000];
    } elseif {$pp == 50} {
	if {$h < 500} {
	    set h [expr $h * 10 + 20000];
	} else {
	    set h [expr $h * 10 + 10000];
	}
    } elseif {($pp == 30) || ($pp == 20)} {
	set h [expr $h * 10 + 20000];
    } elseif {$pp == 10} {
	if {$h < 500} {
	    set h [expr $h * 10 + 30000];
	} else {
	    set h [expr $h * 10 + 20000];
	}
    } else {
	return -code error "yy-Invalid pphhh: $pphhh.";
    }

    return $h;
}

proc ::upperair::fm35::_decode_surface_tropopause {data} {
#
# Here "data" is a list with the three elements, for the surface or
# tropopause levels. The decoded data is returned in the order
#
# $p_mb $temp_c $dewp_c $ws_kt $wd_deg
#
    set ppppp [lindex $data 0];
    set tttdd [lindex $data 1];
    set dddff [lindex $data 2];
    
    #
    # pressure
    #
    if {[regexp {(\d{2})(\d{3})} $ppppp match level p] == 0} {
	return -code error "Invalid 99ppp or 88ppp: $ppppp.";
    }
    set p [_strip_zeros $p];
    if {$level == 99} {
	if {$p < 100} {
	    set p [expr $p + 1000];
	}
    } elseif {$level != 88}  {
	return -code error "Invalid 99ppp of 88ppp: $ppppp.";
    }
    set p_mb $p;

    set temp_c [lindex [_decode_temp_dewp $tttdd] 0];
    set dewp_c [lindex [_decode_temp_dewp $tttdd] 1];
    set ws_kt [lindex [_decode_wind $dddff] 0];
    set wd_deg [lindex [_decode_wind $dddff] 1];

    return [list $p_mb $temp_c $dewp_c $ws_kt $wd_deg];
}

proc ::upperair::fm35::_decode_windmax {data} {
#
# Here "data" is a list with _two_ elements.
# The decoded data is returned in the order
#
# $p_mb $ws_kt $wd_deg
#
    set ppppp [lindex $data 0];
    set dddff [lindex $data 2];
    
    #
    # pressure
    #
    if {[regexp {77(\d{3})} $ppppp match level p] == 0} {
	return -code error "Invalid 77ppp: $ppppp.";
    }
    set p_mb [_strip_zeros $p];

    set ws_kt [lindex [_decode_wind $dddff] 0];
    set wd_deg [lindex [_decode_wind $dddff] 1];

    return [list $p_mb  $ws_kt $wd_deg];
}

proc ::upperair::fm35::_decode_upperair_level {part level_data} {
#
# Here "level_data" is a list with the three elements, and "part" is
# is "ttaa" or "ttcc".
#
# The decoded data is returned in the order
#
# $temp_c $dewp_c $height_m  $ws_kt $wd_deg
#
    set pphhh [lindex $level_data 0];
    set tttdd [lindex $level_data 1];
    set dddff [lindex $level_data 2];

    if {$part eq "ttaa"} {
	set height_m [_decode_height_ttaa $pphhh];
    } elseif {$part eq "ttcc"} {
	set height_m [_decode_height_ttcc $pphhh];
    } else {
	return -code error "Unsupported part: $part.";
    }

    set temp_c [lindex [_decode_temp_dewp $tttdd] 0];
    set dewp_c [lindex [_decode_temp_dewp $tttdd] 1];
    set ws_kt [lindex [_decode_wind $dddff] 0];
    set wd_deg [lindex [_decode_wind $dddff] 1];

    return [list $height_m $temp_c $dewp_c $ws_kt $wd_deg];
}

#
# external interface functions
#

proc ::upperair::fm35::decode {body} {
#
# "body" is assumed to be a line of the form
#
# USDL02 EDZW 151800 TTAA 15171 10393 99007 01106 25005 00170 01508 25008 \
# 92788 04903 30014 85449 08310 31513 70930 15710 33515 50540 30722 34523
#
# with or without the first three wmo elements. The function seeks the TTAA
# or TTCC and starts from there. The function decodes the data and stores
# the results in the array "fm35_decoded_data". The functions
#
# ::upperair::fm35::get_levels {}
# ::upperair::fm35::get_data {level}
#
# then retrieve those results from the internal data structures.
#
    variable mandatory_levels;
    variable fm35_raw_data;
    variable fm35_decoded_data;

    # Initialize the arrays since this function can be called repeatedly.
    unset fm35_raw_data;
    unset fm35_decoded_data;
    array set fm35_raw_data [list];
    array set fm35_decoded_data [list];

    set body_list [::textutil::split::splitx $body];

    set start [lsearch -regexp $body_list {^(tt|TT)(aa|cc|AA|CC)}];
    if {$start == -1} {
	return -code error "No ttaa or ttcc found.";
    }

    set body_list [lrange $body_list $start end];
    set part [string tolower [lindex $body_list 0]];
    set ddhhi [lindex $body_list 1];
    set wmo_station_number [lindex $body_list 2];
    set data [lrange $body_list 3 end];

    # if {[regexp {^tt(aa|cc)} $part] == 0} {
    #	return -code error "Invalid data: $body.";
    # }

    if {[regexp {(\d{2})(\d{2})(\d)} $ddhhi match dd hh i] == 0} {
	return -code error "Invalid day/hour: $ddhhi.";
    }
    if {$dd > 50} {
	set dd [expr $dd - 50];
	set wind_knots_flag 1;
    } else {
	set wind_knots_flag 0;
    }
    set wind_top_level $i;

    set fm35_decoded_data(siteid) $wmo_station_number;
    set fm35_decoded_data(time) ${dd}${hh};

    set j 0;
    set count 3;	# number of elements to read for each level
    set quit 0;		# set to 1 when the widmax (77) is encountered
    foreach d $data {
	if {$j == 0} {
	    set level_index [string range $d 0 1];
	    if {$level_index eq "99"} {
		if {[string range $d 2 5] eq "999"} {
		    continue;
		}
		set level "surface";
	    } elseif {$level_index eq "88"} {
		if {[string range $d 2 5] eq "999"} {
		    continue;
		}
		set level "tropopause";
	    } elseif {$level_index eq "77"} {
		if {[string range $d 2 5] eq "999"} {
		    break;
		}
		set level "maxwind";
		set count 2;	# This only has two elements
		set quit 1;     # Don't read anymore after this level
	    } else {
		if {[info exists mandatory_levels($part,$level_index)] == 0} {
		    break;
		}
		set level "";
		append level "level" "," $mandatory_levels($part,$level_index);
	    }
	    set fm35_raw_data($level) [list];
	}
	lappend fm35_raw_data($level) $d;
	incr j;
	if {$j == $count} {
	    # Finished reading one level; start over with the next one.
	    set j 0;
	    set count 3;
	    if {$quit == 1} {
		break;
	    }
	}
    }

    if {$j != 0} {
	return -code error "Incomplete data.";
    }

    if {[info exists fm35_raw_data(surface)]} {
	set fm35_decoded_data(surface) \
	    [_decode_surface_tropopause $fm35_raw_data(surface)];
    }

    if {[info exists fm35_raw_data(tropopause)]} {
	set fm35_decoded_data(tropopause) \
	    [_decode_surface_tropopause $fm35_raw_data(tropopause)];
    }

    foreach k [lsort -dictionary -decreasing \
		   [array names fm35_raw_data "level,*"]] {
	set fm35_decoded_data($k) \
	    [_decode_upperair_level $part $fm35_raw_data($k)];
    }
}

proc ::upperair::fm35::get_levels {} {
#
# Returns the list of levels for which data is available. The order is:
#
# surface, 1000, ..., tropopause, windmax
#
    variable fm35_decoded_data;

    set r [list];

    if {[info exists fm35_decoded_data(surface)]} {
	lappend r "surface";
    }

    foreach k [lsort -dictionary -decreasing \
		   [array names fm35_decoded_data "level,*"]] {
	set level [lindex [split $k ","] 1];
	lappend r $level;
    }

    if {[info exists fm35_decoded_data(tropopause)]} {
	lappend r "tropopause";
    }

    if {[info exists fm35_decoded_data(maxwind)]} {
	lappend r "maxwind";
    }

    return $r;
}

proc ::upperair::fm35::get_data {level} {
#
# The returned list depends on the level:
#
# p_mb temp_c dewp_c wspeed_kt wdir       (surface, tropopause)
# height_m temp_c dewp_c wspeed_kt wdir   (1000, ... mb)
# p_mb wspeed_kt wdir                     (windmax)
# 
    variable fm35_decoded_data;

    if {[regexp {^\d+$} $level]} {
	set level "level,$level";
    }

    if {[info exists fm35_decoded_data($level)]} {
	return $fm35_decoded_data($level);
    }

    return [list];
}

proc ::upperair::fm35::get_siteid {} {

    variable fm35_decoded_data;

    return $fm35_decoded_data(siteid);
}

proc ::upperair::fm35::get_time {} {

    variable fm35_decoded_data;

    return $fm35_decoded_data(time);
}

proc ::upperair::fm35::set_na {s} {

    variable fm35;

    set fm35(na_symbol) $s;
}

proc ::upperair::fm35::get_na {} {

    variable fm35;

    return $fm35(na_symbol);
}
