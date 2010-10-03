#!/usr/local/bin/tclsh8.6

set body {\[Doc_Dynamic\]

\[::html::set _imgpath "gis/data/img/rad/${awips1}_${name}.png"\]
\[::html::set _headtitle "[string toupper "$awips1 $name"]"\]
\[::html::set _title "$title($awips1)"\]
\[::html::set _legendpath "include/$legend($awips1).html"\]}

set title(n0r) {Base reflectivity, 124 nmi, 0.50 degree elevation};
set title(n1r) {Base reflectivity, 124 nmi, 1.45/1.5 degree elevation};
set title(n2r) {Base reflectivity, 124 nmi, 2.40/2.50 degree elevation};
set title(n3r) {Base reflectivity, 124 nmi, 3.35/3.50 degree elevation};
set title(n0z) {Base reflectivity, 248 nmi, 0.50 degree elevation};
set title(n0v) {Base radial velocity, 124 nmi, 0.50 degree elevation};
set title(n1v) {Base radial velocity, 124 nmi, 1.45/1.5 degree elevation};

set legend(n0r) "gis_rad_legend_bref";
set legend(n1r) "gis_rad_legend_bref";
set legend(n2r) "gis_rad_legend_bref";
set legend(n3r) "gis_rad_legend_bref";
set legend(n0z) "gis_rad_legend_bref";
set legend(n0v) "gis_rad_legend_brvel";
set legend(n1v) "gis_rad_legend_brvel";

foreach awips1 [list n0r n1r n2r n3r n0z n0v n1v] {
    foreach name [list ak hi pr west south central east conus] {
	exec echo "[subst $body]" > ${awips1}_${name}.in;
    }
}
