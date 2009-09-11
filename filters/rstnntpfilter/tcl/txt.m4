dnl
dnl $Id$
dnl

dnl
dnl Only the regular buletins, without the "data" ones. 
dnl 
match_txt_only($rc(awips1),
18a|24a|30l|3hr|5tc|abv|ada|adm|adr|adv|adw|adx|afd|afm|afp|agf|ago|alt|apg|aqi|arp|asa|avm|awg|aws|awu|aww|boy|ccf|cem|cfw|cgr|chg|cli|clm|cmm|cod|csc|cur|cwa|cwf|cws|day|dpa|dsa|dsm|dst|efp|eol|eon|eqr|esf|esg|esp|ess|fa0|fa1|fa2|fa3|fa4|fa5|fa6|fa7|fa8|fa9|faa|fak|fan|fav|fbp|fd0|fd1|fd2|fd3|fd8|fd9|fdi|ffa|ffg|ffh|ffs|ffw|fln|fls|flw|fmr|fof|foh|frh|fsh|fss|ftj|ftm|ftp|fwa|fwc|fwd|fwe|fwf|fwl|fwm|fwn|fwo|fws|glf|glo|gls|gsm|hcm|het|hff|hls|hmd|hnd|hsf|hwd|hwo|hyd|hym|hyw|ice|ini|int|iob|law|lco|lfp|lls|lsh|lsr|ltg|man|map|mav|maw|mef|mex|mim|mis|mon|mrm|mrp|msm|mst|mtr|mtt|mvf|mws|now|npw|nsh|oav|ocd|ofa|off|omr|opu|osb|oso|osw|oua|par|pbf|pib|pir|pls|pmd|pns|prc|psh|psm|pvm|pwo|qcd|qch|qcm|qcw|qpf|qps|rdf|rdg|rec|rep|rer|rfd|rfr|rfw|rsd|rsm|rtp|rva|rvd|rvf|rvi|rvm|rvr|rvs|rws|rzf|saa|sab|sad|saf|sag|sam|saw|scc|scd|scn|scp|scs|scv|sdo|sds|sel|ses|sev|sfd|sfp|sft|sgl|sgw|shi|shn|shp|sig|sim|sls|sma|smf|smw|spe|spf|sps|srf|ssa|ssi|ssm|sta|std|sto|stp|stw|sum|svr|svs|swd|swe|swo|swr|sws|swx|taf|tap|tav|tca|tcd|tce|tcm|tcp|tcs|tcu|tid|tma|tor|tpt|tsm|tst|tsu|tvl|twb|twd|two|tws|ujx|ulg|uvi|vaa|ver|wa0|wa1|wa2|wa3|wa4|wa5|wa6|wa7|wa8|wa9|wac|war|wat|wcn|wcr|wda|wek|wpd|wrk|ws1|ws2|ws3|ws4|ws5|ws6|ws7|ws8|ws9|wst|wsv|wsw|wts|wvm|wwa|zfp|wou,
txt.$rc(station), $rc(awips))

match_txt_only_except($rc(station), knhc, $rc(awips), ^$,
txt.$rc(station), $rc(awips))

dnl
dnl urgent
dnl
match_txt_only($rc(body), URGENT,
emwin.urgent.$rc(station), $rc(awips)-URGENT)
match_txt_only($rc(body), URGENT,
emwin.urgent, $rc(station)-$rc(awips)-URGENT)

dnl
dnl eas activation (sent to "urgent" group)
dnl
match_txt_only($rc(body), EAS ACTIVATION,
emwin.urgent.$rc(station), $rc(awips)-EAS_ACTIVATION)
match_txt_only($rc(body), EAS ACTIVATION,
emwin.urgent, $rc(station)-$rc(awips)-EAS_ACTIVATION)

dnl
dnl warnings 
dnl
dnl match_txt_only($rc(awips1),
dnl cem|cfw|ffw|fls|flw|hls|hwo|npw|rfw|sps|svr|svs|tor|wsw,
dnl emwin.warnings.$rc(station), $rc(awips))
dnl 
dnl match_txt_only($rc(awips1),
dnl cem|cfw|ffw|fls|flw|hls|hwo|npw|rfw|sps|svr|svs|tor|wsw,
dnl emwin.warnings, $rc(station)-$rc(awips))

dnl
dnl warnings, data,forecast,summary only to station groups
dnl
match_txt_only_except($rc(wmoid), ^w, $rc(awips), ^$,
emwin.warnings.$rc(station), $rc(awips))

match_txt_only_except($rc(wmoid), ^a, $rc(awips), ^$,
emwin.summary.$rc(station), $rc(awips))

match_txt_only_except($rc(wmoid), ^f, $rc(awips), ^$,
emwin.forecast.$rc(station), $rc(awips))

match_txt_only_except($rc(wmoid), ^[crstu], $rc(awips), ^$,
emwin.data.$rc(station), $rc(awips))

dnl
dnl administrative messages
dnl
match_txt_only($rc(wmoid),
^(admn[0-68]|admn9[^9]|admn7[^5]|noxx|nous[^46789]|nous9[^7]),
misc.adm, ADM)
match_txt_only($rc(fname), (kwno|kwbc|kncf)_nous[4678], misc.adm, ADM)

stopmatch
