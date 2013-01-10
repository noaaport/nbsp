#
# $Id$
#
# The STREAM parameter controls several parameters dealing with 
# the overall streamline calculation and display (gdstream)
#
proc gempak::stream_lines {lines} {

    variable gempak;

    set gempak(param,stream.lines) $lines;
}

proc gempak::stream_arrows {arrows} {

    variable gempak;

    set gempak(param,stream.arrows) $arrows;
}

proc gempak::stream_stop {stop} {

    variable gempak;

    set gempak(param,stream.stop) $stop;
}

proc gempak::stream_slow {slow} {

    variable gempak;

    set gempak(param,stream.slow) $slow;
}

proc gempak::stream_scale {scale} {

    variable gempak;

    set gempak(param,stream.scale) $scale;
}

proc gempak::set_stream {} {

    variable gempak;

    # All
    set parts [list lines arrows stop slow scale];
    ::gempak::_set_param "stream" "/" $parts;
}

proc gempak::get_stream {} {

    return [::gempak::get "stream"];
}

proc gempak::get_stream_list {} {

    return [split [::gempak::get_stream] "/"];
}
