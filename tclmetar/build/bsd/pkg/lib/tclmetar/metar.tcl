#
# $Id$
#
# Copyright (c) 2009 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

package provide metar 1.0;
package require textutil::split;

namespace eval metar {

    variable metar;

    set metar(text,weather.intensity.minus) "light";
    set metar(text,weather.intensity.plus) "heavy";
    set metar(text,weather.intensity.blank) "moderate";
    set metar(text,weather.intensity.VC) "in the vicinity";
    set metar(pat,weather.intensity) {\+|-|VC};

    set metar(pat,weather.descriptor) {MI|PI|BC|DR|BL|SH|TS|FZ};
    set metar(text,weather.MI) "shallow";
    set metar(text,weather.PI) "partial";
    set metar(text,weather.BC) "patches";
    set metar(text,weather.DR) "drizzle";
    set metar(text,weather.BL) "blowing";
    set metar(text,weather.SH) "showers";
    set metar(text,weather.TS) "thunderstorm";
    set metar(text,weather.FZ) "freezing";

    set metar(pat,weather.precipitation) {DZ|RA|SN|SG|IC|PE|GR|GS|UP};
    set metar(text,weather.DZ) "drizzle";
    set metar(text,weather.RA) "rain";
    set metar(text,weather.SN) "snow";
    set metar(text,weather.SG) "snow grains";
    set metar(text,weather.IC) "ice crystals";
    set metar(text,weather.PE) "ice pellets";
    set metar(text,weather.GR) "hail";
    set metar(text,weather.GS) "small hail/snow pellets";
    set metar(text,weather.UP) "unknown precipitation";

    set metar(pat,weather.obscuration) {BR|FG|FU|VA|DU|SA|HZ|PY};
    set metar(text,weather.BR) "mist";
    set metar(text,weather.FG) "fog";
    set metar(text,weather.FU) "smoke";
    set metar(text,weather.VA) "volcanic ash";
    set metar(text,weather.DU) "dust";
    set metar(text,weather.SA) "sand";
    set metar(text,weather.HZ) "haze";
    set metar(text,weather.PY) "spray";

    set metar(pat,weather.other) {PO|SQ|FC|SS|DS};
    set metar(text,weather.PO) "dust/sand whirls";
    set metar(text,weather.SQ) "squalls";
    set metar(text,weather.FC) "funnel cloud_tornado/waterspout";
    set metar(text,weather.SS) "sand storm";
    set metar(text,weather.DS) "dust storm";

    append metar(pat,weather.all) $metar(pat,weather.descriptor) \
	"|" $metar(pat,weather.precipitation) \
	"|" $metar(pat,weather.obscuration) \
	"|" $metar(pat,weather.other);

    set metar(text,sky.coverage.SKC) "clear";
    set metar(text,sky.coverage.CLR) "clear";
    set metar(text,sky.coverage.SCT) "scattered clouds";
    set metar(text,sky.coverage.BKN) "broken clouds";
    set metar(text,sky.coverage.FEW) "few clouds";
    set metar(text,sky.coverage.OVC) "solid overcast";
    set metar(text,sky.coverage.CAVOK) "clear skies, unlimited visibility";
    set metar(pat,sky.coverage) {SKC|CLR|SCT|BKN|FEW|OVC|CAVOK};

    set metar(text,sky.type.CU) "cumulus";
    set metar(text,sky.type.CB) "cumulonumbus";
    set metar(text,sky.type.TCU) "towering cumulus";
    set metar(text,sky.type.CI) "cirrus";
    set metar(pat,sky.type) {CU|CB|TCU|CI};

    set metar(text,report_types.metar) "Routine Weather Report";
    set metar(text,report_types.speci) "Special Weather Report";
    set metar(text,report_types.unknown) "Unknown type Weather Report";

    set metar(text,wind_calm) "Calm";
    set metar(text,wind_dir_variable) "variable";

    set metar(text,visibility_Mflag) "less than";

    set metar(text,slpno) "not available";

    set metar(text,hourlyprecip_0) "trace";

    set metar(text,presfr) "Pressure falling rapidly.";
    set metar(text,presrr) "Pressure rising rapidly.";
    set metar(text,snoincr) "Snow increasing rapidly.";
}

proc metar::init_data {} {
    #
    # Initialize all variables to empty and flags to 0.
    #

    variable metar;

    set metar(obs,TYPE) "";
    set metar(param,type) "";

    set metar(obs,STATION) "";
    set metar(param,station) "";

    set metar(obs,DATE) "";
    set metar(param,date.dd) "";
    set metar(param,date.hhmm) "";

    set metar(obs,MODIFIER) "";

    set metar(obs,WIND) "";
    set metar(flag,wind_calm) 0;
    set metar(param,wind.dir) "";
    set metar(param,wind.speed_kt) "";
    set metar(param,wind.speed_mph) "";
    set metar(param,wind.gust_kt) "";
    set metar(param,wind.gust_mph) "";

    set metar(obs,VISIBILITY) "";
    set metar(flag,visibility_Mflag) 0;
    set metar(param,visibility) "";
    
    set metar(obs,RUNWAY_VISIBILITY) "";

    set metar(obs,WEATHER) "";
    set metar(param,weather) "";

    # There can be several layers, each as a member of the list
    set metar(obs,SKY) [list];
    set metar(param,sky) [list];

    # Temp in C
    set metar(obs,TEMP_DEWP) "";
    set metar(param,temp_c) "";
    set metar(param,dewp_c) "";
    set metar(param,temp_f) "";
    set metar(param,dewp_f) "";

    set metar(obs,ALT) "";
    set metar(param,alt) "";
    set metar(flag,alt_Q) 0;   # A = Hg, Q = whole mb => flag set

    set metar(obs,REMARKS) "";

    set metar(obs,AUTO_STATIONTYPE) "";
    set metar(param,auto_stationtype) "";

    set metar(obs,SLP) "";
    set metar(param,slp) "";

    set metar(obs,SLPNO) "";
    set metar(flag,slpno) 0;

    set metar(obs,HOURLYPRECIP) "";
    set metar(param,hourlyprecip) "";
    set metar(flag,hourlyprecip_0) 0;

    set metar(obs,WEATHERLOG) [list];
    set metar(param,weatherlog) [list];

    # Temp in 0.1 C
    set metar(obs,TEMP_DEWP_01) "";
    set metar(param,temp_01_c) "";
    set metar(param,dewp_01_c) "";
    set metar(param,temp_01_f) "";
    set metar(param,dewp_01_f) "";

    set metar(obs,PRESFR) "";
    set metar(obs,PRESRR) "";       
    set metar(obs,SNOINCR) "";
    set metar(flag,presfr) 0;
    set metar(flag,presrr) 0;       
    set metar(flag,snoincr) 0;
}
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

	    set metar(obs,WIND) $tok;
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
		set metar(param,wind.speed_mph) [expr $s2 * 1.1508];
	    }
	    if {$s4 ne ""} {
		set metar(param,wind.gust_kt) $s4;
		set metar(param,wind.gust_mph) [expr $s4 * 1.1508];
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

	    if {$s1 != ""} {
		 append metar(param,temp_c) "-" $s2;
	     } else {
		 set metar(param,temp_c) $s2;
	     }

	     if {$s3 != ""} {
		 append metar(param,dewp_c) "-" $s4;
	     } else {
		 set metar(param,dewp_c) $s4;
	     }

	     set metar(param,temp_f) \
		 [expr (9.0/5.0) * $metar(param,temp_c) + 32.0];
	     set metar(param,dewp_f) \
		 [expr (9.0/5.0) * $metar(param,dewp_c) + 32.0];

	     continue;
	}


	#
	# Pressure
	#
	if {[regexp {^A([[:digit:]]{2})([[:digit:]]{2})} $tok match s1 s2]} {
	    set metar(obs,ALT) $tok;
	    set metar(param,alt) $s1.$s2;
	    continue;
	}

	if {[regexp {^Q([[:digit:]]{4})} $tok match]} {
	    set metar(obs,ALT) $tok;
	    set metar(param,alt) $s1;
	    set metar(flag,alt_Q) 1;
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
		append _weatherlog $metar(text.weather.$s1) ${_verb} $s6;
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
	    if {$s1 == 1} {
		set metar(param,temp_01_c) "-";
	    }
	    append metar(param,temp_01_c) [string trimleft $s2 "0"] "." $s3;

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

	if {[regexp {^SNOINCR} $tok]} {
	    set metar(flag,snoincr) 1;
	}
    }
}
#
# $Id$
#
proc metar::get_param_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "param,*"] {
	regexp {param,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_param {param} {

    variable metar;

    if {[info exists metar(param,$param)]} {
	return $metar(param,$param);
    }

    return -code error "$param is not a variable.";
}

proc metar::get_flag_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "flag,*"] {
	regexp {flag,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_flag {flag} {

    variable metar;

    if {[info exists metar(flag,$flag)]} {
	return $metar(flag,$flag);
    }

    return -code error "$flag is not a variable.";
}

proc metar::get_textitem_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "text,*"] {
	regexp {text,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_textitem {item} {

    variable metar;

    if {[info exists metar(text,$item)]} {
	return $metar(text,$item);
    }

    return -code error "$item is not a variable.";
}

proc metar::get_var_list {} {

    variable metar;

    set r [list];
    foreach key [array names ::metar::metar "var,*"] {
	regexp {var,(.+)} $key math s1;
	lappend r $s1;
    }
    return $r;
}

proc metar::get_var {var} {

    variable metar;

    if {[info exists metar(var,$var)]} {
	return $metar(var,$var);
    }

    return -code error "$var is not a variable.";
}

proc metar::set_var {var val} {

    variable metar;

    set metar(var,$var) $val;
}
