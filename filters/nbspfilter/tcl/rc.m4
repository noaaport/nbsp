dnl
dnl $Id$
dnl
matchmore_mail($rc(body), URGENT,
nbsp-test@cariberesearch.com, nbsp@noaaport.net, URGENT $rc(wmoid))

match_pipe($rc(fname),
^tjsj_(f[glnopxz]ca[457]2)|(a[sw]ca[468]2)|(c[dsx]ca[456]2)|(w[gh]ca[578]2),
mail -s $rc(wmoid) nieves)

match_mail($rc(wmoid),
^(admn[0-68]|admn9[^9]|admn7[^5]|noxx|nous[^46789]|nous9[^7]),
nbsp-test@cariberesearch.com, nbsp@noaaport.net, ADM)

match_mail($rc(fname), (kwno|kwbc|kncf)_nous[4678],
nbsp-test@cariberesearch.com, nbsp@noaaport.net, ADM)
