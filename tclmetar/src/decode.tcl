#
# $Id$
#

#
# Main library function
#
proc metar::decode {line} {

    variable metar;

    init_data;

    # Find report type from first token
    set tok_list [::textutil::split::splitx $line];
    set tok [lindex $tok_list 0];

    if {($tok eq "METAR") || ($tok eq "M")} {
	set metar(obs,TYPE) "METAR";
	set metar(param,type) $metar(text,report_types.metar);
	set tok_list [lreplace $tok_list 0 0];
    } elseif {($tok eq "SPECI") || ($tok eq "S")} {
	set metar(obs,TYPE) "SPECI";
	set metar(param,type) $metar(text,report_types.speci);
	set tok_list [lreplace $tok_list 0 0];
    } else {
	set metar(obs,TYPE) "UNKNOWN";
	set metar(param,type) $metar(text,report_types.unknown);
    }

    set siteid_found 0;	
    foreach tok $tok_list {
	#
	# Station 
	#
	if {[regexp {[A-Z0-9]{4}} $tok] && ($siteid_found == 0)} {
	    set siteid_found 1;
	    set metar(obs,STATION) $tok;
	    set metar(param,station) [string tolower $tok];
	    continue;
	}

	#
	# Date-Time
	# 
	if {[regexp {(\d{2})(\d{2})(\d{2})Z} $tok match s1 s2 s3]} {
	    set metar(obs,DATE) $tok;
	    set metar(param,date.dd) $s1;
	    set metar(param,date.hhmm) "";
	    append  metar(param,date.hhmm) $s2 ":" $s3;
	    continue;
	}

	#
	# Report modifier
	#
	if {[regexp {^(AUTO|COR)} $tok]} {
	    set metar(obs,MODIFIER) $tok;
	    continue;
	}

	#
	# Wind
	#
	if {[regexp {(\d{3}|VRB)(\d{2,3})(G(\d{2,3}))?KT$} $tok match \
		s1 s2 s3 s4]} {

	    set metar(obs,WIND) $tok;

	    if {$s1 ne ""} {
		set s1 [string trimleft $s1 "0"];
		if {$s1 eq ""} {
		    set s1 0;
		}
	    }
	    if {$s2 ne ""} {
		set s2 [string trimleft $s2 "0"];
		if {$s2 eq ""} {
		    set s2 0;
		}
	    }
	    if {$s4 ne ""} {
		set s4 [string trimleft $s4 "0"];
		if {$s4 eq ""} {
		    set s4 0;
		}
	    }

	    if {$tok eq "00000KT"} {
		set metar(flag,wind_calm) 1;
	    }

	    if {$s1 eq "VRB"} {
		set metar(flag,wind_dir_variable) 1;
	    } else {
		set metar(param,wind.dir) $s1;
	    }

	    if {$s2 ne ""} {
		set metar(param,wind.speed_kt) $s2;
		set metar(param,wind.speed_mph) [expr int($s2 * 1.1508)];
	    }
	    if {$s4 ne ""} {
		set metar(param,wind.gust_kt) $s4;
		set metar(param,wind.gust_mph) [expr int($s4 * 1.1508)];
	    }

	    continue;
	}

	#
	# Visibility (statute miles)
	#
	if {[regexp {(M?)(.+)SM$} $tok match s1 s2]} {
	    set metar(obs,VISIBILITY) $tok;
	    if {$s1 ne ""} {
		set metar(flag,visibility_Mflag) 1;
	    }
	    set metar(param,visibility) $s2;
	    continue;
	}

	#
	# Runway visibility
	#
	if {[regexp {^R.*FT$} $tok]} {
	    set metar(obs,RUNWAY_VISIBILITY) $tok;
	    continue;
	}

	#
	# Weather
	#
	set _pat "";
	append _pat "($metar(pat,weather.intensity))?";
	append _pat "($metar(pat,weather.descriptor))?";
	append _pat "($metar(pat,weather.precipitation))?";
	append _pat "($metar(pat,weather.obscuration))?";
	append _pat "($metar(pat,weather.other))?";
	append _pat {$};
	#
	# NOTE: Mon Oct  8 20:20:46 AST 2012
	# The last statement avoids the problem with TSNO reported
	# by Craig Fincher for example with
        # M KJXI 081715Z AUTO 00000KT 10SM SCT031 BKN035 13/07 \
	#   A3026 RMK AO2 TSNO
	#

	# Match any combination of these, but at least one of them.
	if {[regexp ${_pat} $tok match s1 s2 s3 s4 s5] && ($match ne "")} {
	    set metar(obs,WEATHER) $tok;
	    if {$s1 eq "-"} {
		set _weather $metar(text,weather.intensity.minus)
	    } elseif {$s1 eq "+"} {
		set _weather $metar(text,weather.intensity.plus)
	    } elseif {$s1 eq "VC"} {
		set _weather $metar(text,weather.intensity.VC)
	    } else {
		set _weather $metar(text,weather.intensity.blank)
	    }

	    if {$s2 ne ""} {
		append _weather " " $metar(text,weather.$s2);
	    }

	    if {$s3 ne ""} {
		append _weather " " $metar(text,weather.$s3);
	    }

	    if {$s4 ne ""} {
		append _weather " " $metar(text,weather.$s4);
	    }

	    if {$s5 ne ""} {
		append _weather " " $metar(text,weather.$s5);
	    }

	    set metar(param,weather) ${_weather};
	    unset _weather;
	}
	unset _pat;

	#
	# Sky (Clouds)
	#
	set _pat "";
	append _pat "($metar(pat,sky.coverage))";
	append _pat {([[:digit:]]{3})?};
	append _pat "($metar(pat,sky.type))?";

	if {[regexp ${_pat} $tok match s1 s2 s3]} {
	    lappend metar(obs,SKY) $tok;
	    set _sky $metar(text,sky.coverage.$s1);
	    if {$s3 ne ""} {
		append _sky " " $metar(text,sky.type.$s3);
	    }

	    if {$s2 ne ""} {
		set s2 [string trimleft $s2 "0"];
		if {$s2 ne ""} {
		    append _sky " " [expr 100 * $s2] " ft";
		}
	    }
	    lappend metar(param,sky) ${_sky};
	    unset _sky;
	    continue;
	}
	unset _pat;

	#
	# Temp and DewP in C
	#
	if {[regexp {^(M?)([[:digit:]]{2})/(M?)([[:digit:]]{2})} $tok \
			    match s1 s2 s3 s4]} {

	    set metar(obs,TEMP_DEWP) $tok;

	    set s2 [string trimleft $s2 "0"];
	    set s4 [string trimleft $s4 "0"];

	    if {$s2 eq ""} {
		set s2 0;
	    }
	    if {$s4 eq ""} {
		set s4 0;
	    }

	    if {$s1 ne ""} {
		set metar(param,temp_c) "";
		append metar(param,temp_c) "-" $s2;
	     } else {
		 set metar(param,temp_c) $s2;
	     }

	     if {$s3 ne ""} {
		 set metar(param,dewp_c) "";
		 append metar(param,dewp_c) "-" $s4;
	     } else {
		 set metar(param,dewp_c) $s4;
	     }

	     set metar(param,temp_f) \
		[format "%.1f" [expr (9.0/5.0) * $metar(param,temp_c) + 32.0]];
	     set metar(param,dewp_f) \
		[format "%.1f" [expr (9.0/5.0) * $metar(param,dewp_c) + 32.0]];

	     continue;
	}


	#
	# Pressure
	#
	if {[regexp {^A([[:digit:]]{2})([[:digit:]]{2})} $tok match s1 s2]} {
	    set metar(obs,ALT) $tok;
	    set metar(param,alt_hg) $s1.$s2;
	    set metar(param,alt_mb) \
		[format "%.2f" [expr $metar(param,alt_hg) * 33.8639]];
	    continue;
	}

	if {[regexp {^Q([[:digit:]]{4})} $tok match s1]} {
	    set metar(flag,alt_Q) 1;
	    set metar(obs,ALT) $tok;
	    set metar(param,alt_mb) $s1;
	    set metar(param,alt_hg) \
		[format "%.2f" [expr $metar(param,alt_mb) / 33.8639]];
	    continue;
	}

	#
	# Remarks
	#
	if {$tok eq "RMK"} {
	    set metar(obs,REMARKS) $tok;
	    set rmk_found 1;
	    continue;
	}

	#
	# Automatic setting
	#
	if {[info exists rmk_found] && ($rmk_found == 1) && \
		[regexp {^A(O[[:digit:]])$} $tok match s1]} { 
	    set metar(obs,AUTO_STATIONTYPE) $tok;
	    set metar(param,auto_stationtype) $s1;
	    continue;
	}

	#
	# Sea level pressure
	#
	if {[regexp {^SLP(\d)(\d)(\d)} $tok match s1 s2 s3]} {
	    set metar(obs,SLP) $tok;
	    # When the SLP starts with a 9 or 8, always add a 9 to the
	    # front of the value, e.g., 998.1.  When the SLP starts
	    # with a 0 or greater, add a 10 to the beginning of the value,
	    # e.g., 1031.0.
	    # The decimal point is inserted between the 2nd and 3rd digits.

	    set metar(param,slp) "";
	    if {$s1 >= 8} {
		append metar(param,slp) "9" $s1 $s2 "." $s3;
	    } else {
		append metar(param,slp) "10" $s1 $s2 "." $s3;
	    }
	    continue;
	}

	#
	# Sea level pressure not available
	#
	if {$tok eq "SLPNO"} {
	    set metar(obs,SLPNO) $tok;
	    set metar(flag,slpno) 1;
	    continue;
	}

	#
	# Hourly precipitation
	#
	if {[regexp {^P([[:digit:]]{4})} $tok match s1]} {
	    set metar(obs,HOURLYPRECIP) $tok;
	    if {$s1 eq "0000"} {
		set metar(flag,hourlyprecip_0) 1;
	    } else {
		set metar(param,hourlyprecip) [string trimleft $s1 "0"];
	    }
	    continue;
	}

	#
	# Weather begin/end times
	#
	set _pat "";
	set _weatherlog "";
	set _verb "";
	append _pat {^} "($metar(pat,weather.all))" \
	       {(B|E)([[:digit:]]{2})((B|E)([[:digit:]]{2}))?};
	                   # In contrast with the Weather section,
	                   #   append _pat {$}; 
	                   # is not included here because there can be
	                   # additional BE parts.

	if {[regexp ${_pat} $tok match s1 s2 s3 s4 s5 s6]} {
	    lappend metar(obs,WEATHERLOG) $tok;
	    if {$s2 eq "B"} {
		set _verb " begin: ";
	    } else {
		set _verb " end: ";
	    }
	    append _weatherlog $metar(text,weather.$s1) ${_verb} $s3;
	    lappend metar(param,weatherlog) ${_weatherlog};

	    if {$s4 ne ""} {
		if {$s5 eq "B"} {
		    set _verb " begin: ";
		} else {
		    set _verb " end: ";
		}
		unset _weatherlog;
		append _weatherlog $metar(text,weather.$s1) ${_verb} $s6;
		lappend metar(param,weatherlog) ${_weatherlog};
	    }
	    continue;
	}
	unset _pat;
	unset _weatherlog;
	unset _verb;

	#
	# Temp/Dewp in 0.1 C
	#
	if {[regexp {^T(\d)(\d{2})(\d)(\d)(\d{2})(\d)} $tok match \
		 s1 s2 s3 s4 s5 s6]} {

	    set metar(obs,TEMP_DEWP_01) $tok;

	    set metar(param,temp_01_c) "";
	    if {$s1 == 1} {
		set metar(param,temp_01_c) "-";
	    }
	    append metar(param,temp_01_c) [string trimleft $s2 "0"] "." $s3;

	    set metar(param,dewp_01_c) "";
	    if {$s4 == 1} {
		set metar(param,dewp_01_c) "-";
	    }
	    append metar(param,dewp_01_c) [string trimleft $s5 "0"] "." $s6;

	    set metar(param,temp_01_f) \
		[expr (9.0/5.0) * $metar(param,temp_01_c) + 32.0];
	    set metar(param,dewp_01_f) \
		[expr (9.0/5.0) * $metar(param,dewp_01_c) + 32.0];
	}

	if {[regexp {^PRESFR} $tok]} {
	    set metar(flag,presfr) 1;
	}

	if {[regexp {^PRESRR} $tok]} {
	    set metar(flag,presrr) 1;
	}

	if {[regexp {^SNINCR} $tok]} {
	    set metar(flag,snincr) 1;
	}
    }
}
