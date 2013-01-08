#
# $Id$
#
# Copyright (c) 2009 Jose F. Nieves <nieves@ltp.uprrp.edu>
#
# ISC License
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

    set metar(text,report_types.metar) "Routine Report";
    set metar(text,report_types.speci) "Special Report";
    # set metar(text,report_types.unknown) "Unknown type Weather Report";
    set metar(text,report_types.unknown) "Report";

    set metar(text,wind_calm) "Calm";
    set metar(text,wind_dir_variable) "variable";

    set metar(text,visibility_Mflag) "less than";

    set metar(text,slpno) "not available";

    set metar(text,hourlyprecip_0) "trace";

    set metar(text,presfr) "Pressure falling rapidly.";
    set metar(text,presrr) "Pressure rising rapidly.";
    set metar(text,snincr) "Snow increasing rapidly.";
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
    set metar(flag,alt_Q) 0;	# 0 => A (hg), 1 => Q (whole mb)
    set metar(param,alt_hg) "";
    set metar(param,alt_mb) "";

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
    set metar(obs,SNINCR) "";
    set metar(flag,presfr) 0;
    set metar(flag,presrr) 0;       
    set metar(flag,snincr) 0;
}
