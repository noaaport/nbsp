#!/usr/local/bin/tclsh8.5

set header {<?datasetconfig version="1.0d1"?>

<parminfo>
    <parm id="stid" type="string" units="nada" name="site ID"/>
    <parm id="stnm" type="int" units="nada" name="site number"/>
    <parm id="time" type="float" units="min" name="time of observation"/>

    <parm id="lat" type="float" units="degr" name="latitude"/>
    <parm id="lon" type="float" units="degr" name="longitude"/>

    <parm id="pres_sfc" type="float" units="mbar"  name="__fh__`h' Surface level - Pressure"/>
}

set footer {</parminfo>}

set template {
    <parm id="hgt_${l}" type="float" units="m" name="__fh__`h' ${l} - Height"/>
    <parm id="tmp_${l}" type="float" units="kelv" name="__fh__`h' ${l} - Temperature"/>
    <parm id="rh_${l}" type="float" units="prct"  name="__fh__`h' ${l} - Relative humidity"/>
    <parm id="ws_${l}" type="float" units="mps"  name="__fh__`h' ${l} - Wind speed"/>
    <parm id="wd_${l}" type="float" units="degr"  name="__fh__`h' ${l} - Wind direction"/>
    <derivation id="wu_${l}" func="uwnd" args="wd_${l} wd_${l}"/>
    <derivation id="wv_${l}" func="vwnd" args="wd_${l} wd_${l}"/>
    <vector id="wind_${l}" name="__fh__`h' ${l} Winds" samag="ws_${l}" dir="wd_${l}" ucomp="wu_${l}" vcomp="wv_${l}"/>
}

puts -nonewline $header;
foreach l [list 1000 900 800 700 600 500 400 300 200 100] {
    set l "${l}mb";
    puts -nonewline [subst $template];
}
puts $footer;
