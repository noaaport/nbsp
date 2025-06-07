#
# Auxiliary functions to support enabling/disabling rules individually
#

proc is_rad_rule_enabled {key} {
#
# For radar the key is the subdirectory name, e.g., nexrad/nids/jua/n0r,
# or nexrad/craft/tjua for level2.
#
    global dafilter;

    set r [is_type_enabled $dafilter(rad_enable) $dafilter(rad_regex) $key];

    return $r;
}

proc is_sat_rule_enabled {key} {
#
##R For sat the key is "sat/gini/<subdir>" (e.g., sat/gini/tigp01).
# The key is the sat subdirectory name (e.g., sat/goesr/tir/tire05).
#
    global dafilter;

    set r [is_type_enabled $dafilter(sat_enable) $dafilter(sat_regex) $key];

    return $r;
}

proc is_file_rule_enabled {key} {
#
# For file the key is like "nwx/spc/<subdir>" (e.g., nwx/spc/stahry)
#
    global dafilter;

    set r [is_type_enabled $dafilter(file_enable) $dafilter(file_regex) $key];

    return $r;
}

proc is_grib_rule_enabled {key} {
#
# The key is the subdirectory name
#
    global dafilter;

    set r [is_type_enabled $dafilter(grib_enable) $dafilter(grib_regex) $key];

    return $r;
}

proc is_archive_rad_rule_enabled {key} {
#
# For radar the key is the subdirectory name, e.g., nexrad/nids/jua
#
    global dafilter;

    if {$dafilter(archive_enable) == 0} {
	return 0;
    }    

    set r [is_type_enabled \
	$dafilter(archive_rad_enable) $dafilter(archive_rad_regex) $key];

    return $r;
}

proc is_archive_sat_rule_enabled {key} {
#
##R For sat the key is "sat/gini/<subdir>" (e.g., sat/gini/tigp01).
# The key is the sat subdirectory name (e.g., sat/goesr/tir/tire05).
#
    global dafilter;

    if {$dafilter(archive_enable) == 0} {
	return 0;
    }    

    set r [is_type_enabled \
	$dafilter(archive_sat_enable) $dafilter(archive_sat_regex) $key];

    return $r;
}

proc is_archive_file_rule_enabled {key} {
#
# For file the key is like "nwx/spc/<subdir>" (e.g., nwx/spc/stahry)
#
    global dafilter;

    if {$dafilter(archive_enable) == 0} {
	return 0;
    }    

    set r [is_type_enabled \
	$dafilter(archive_file_enable) $dafilter(archive_file_regex) $key];

    return $r;
}

proc is_type_enabled {enable regex key} {
#
# The key is the subdirectory name:
#
# For file the key is like "nwx/spc/<subdir>" (e.g., nwx/spc/stahry)
##R For sat the key is "sat/gini/<subdir>" (e.g., sat/gini/tigp01).
# For sat the key is the sat subdirectory name (e.g., sat/goesr/tir/tire05).
# For radar the key is the subdirectory name, e.g., nexrad/nids/jua/n0r.
#
    set r 0;

    # The "enable" variable can be set to 1 or 2.
    if {($enable != 0) && ([filterlib_uwildmat $regex $key] == 1)} {
	set r 1;
    }

    return $r;
}
