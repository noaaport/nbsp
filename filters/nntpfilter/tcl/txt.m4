dnl
dnl $Id$
dnl

dnl
dnl The raw text products are no longer being sent, in favor of the
dnl various groups in the da filter (Fri Dec 21 11:11:42 AST 2007).
dnl

dnl This matches everything that looks as text, with a non-blank awips,
dnl except for some products that are put in the "data" subgroup
dnl instead of the text subgroup.
match_text_except($rc(awips1), ^$, txt.raw.$rc(station), 0)

dnl This matches everything that looks as text, with a non-blank awips,
dnl except for some products that are put in the "data" subgroup
dnl instead of the text subgroup.
dnl
dnl match_text_except($rc(awips1),
dnl ^$|rob|rr1|rr2|rr3|rr4|rr5|rr6|rr7|rr8|rr9|rra|rrc|rrm|rrs|rrx|rry,
dnl txt.$rc(station), 0)

dnl This list is taken from the text-list in the development notes,
dnl except for the elimination of the same products as above, and wou added.
dnl match_text_only($rc(awips1),
dnl 18a|24a|30l|3hr|5tc|abv|ada|adm|adr|adv|adw|adx|afd|afm|afp|agf|ago|alt|apg|aqi|arp|asa|avm|awg|aws|awu|aww|boy|ccf|cem|cfw|cgr|chg|cli|clm|cmm|cod|csc|cur|cwa|cwf|cws|day|dpa|dsa|dsm|dst|efp|eol|eon|eqr|esf|esg|esp|ess|fa0|fa1|fa2|fa3|fa4|fa5|fa6|fa7|fa8|fa9|faa|fak|fan|fav|fbp|fd0|fd1|fd2|fd3|fd8|fd9|fdi|ffa|ffg|ffh|ffs|ffw|fln|fls|flw|fmr|fof|foh|frh|fsh|fss|ftj|ftm|ftp|fwa|fwc|fwd|fwe|fwf|fwl|fwm|fwn|fwo|fws|glf|glo|gls|gsm|hcm|het|hff|hls|hmd|hnd|hsf|hwd|hwo|hyd|hym|hyw|ice|ini|int|iob|law|lco|lfp|lls|lsh|lsr|ltg|man|map|mav|maw|mef|mex|mim|mis|mon|mrm|mrp|msm|mst|mtr|mtt|mvf|mws|now|npw|nsh|oav|ocd|ofa|off|omr|opu|osb|oso|osw|oua|par|pbf|pib|pir|pls|pmd|pns|prc|psh|psm|pvm|pwo|qcd|qch|qcm|qcw|qpf|qps|rdf|rdg|rec|rep|rer|rfd|rfr|rfw|rsd|rsm|rtp|rva|rvd|rvf|rvi|rvm|rvr|rvs|rws|rzf|saa|sab|sad|saf|sag|sam|saw|scc|scd|scn|scp|scs|scv|sdo|sds|sel|ses|sev|sfd|sfp|sft|sgl|sgw|shi|shn|shp|sig|sim|sls|sma|smf|smw|spe|spf|sps|srf|ssa|ssi|ssm|sta|std|sto|stp|stw|sum|svr|svs|swd|swe|swo|swr|sws|swx|taf|tap|tav|tca|tcd|tce|tcm|tcp|tcs|tcu|tid|tma|tor|tpt|tsm|tst|tsu|tvl|twb|twd|two|tws|ujx|ulg|uvi|vaa|ver|wa0|wa1|wa2|wa3|wa4|wa5|wa6|wa7|wa8|wa9|wac|war|wat|wcn|wcr|wda|wek|wpd|wrk|ws1|ws2|ws3|ws4|ws5|ws6|ws7|ws8|ws9|wst|wsv|wsw|wts|wvm|wwa|zfp|wou,
dnl txt.raw.$rc(station), 0)
