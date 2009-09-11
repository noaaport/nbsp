#!/usr/local/bin/tclsh8.4

package require textutil;

set block {
    <parm id="hgt_${level}mb" type="float" units="m"     name="${level} millibars - Height"/>
    <parm id="tmp_${level}mb" type="float" units="kelv"  name="${level} millibars - Temperature"/>
    <parm id="rh_${level}mb" type="float" units="prct"  name="${level} millibars - Relative humidity"/>
    <parm id="ws_${level}mb" type="float" units="mps"  name="${level} millibars - Wind speed"/>
    <parm id="wd_${level}mb" type="float" units="degr"  name="${level} millibars - Wind direction"/>};

set prslevels [list 1000 900 800 700 600 500 400 300 200 100];

foreach level $prslevels {
    puts [subst $block];
}

