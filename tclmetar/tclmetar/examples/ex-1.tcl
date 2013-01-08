#!%TCLSH%

package require metar;


#set line "KLAF 021915Z COR 22010KT 7SM TSRA BKN055 30/17 A2974 RMK T W MOVG NE"
#set line "LTCC 022250Z 32003KT   CAVOK  	24/10  	Q1011  	NOSIG=";
#set line "KIND  	022356Z  	26009KT  	10SM  	 	CLR  	24/20  	A2973 RMK AO2 SLP062 60000 T02440200"

set line "KFHU  	022336Z  	33011G25  	35SM  	TS VCSH  	FEW040 SCT060CB BKN100 BKN250  	31/14  	A3003  	RMK WSHFT 27 FRQ LTGICCG TS N MOV W SHRA N AND NE-SE="

metar::decode $line;

foreach p [lsort [::metar::get_param_list]] {
    puts "$p = [::metar::get_param $p]";
}
