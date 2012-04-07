#
# $Id$
#
Direct_Url /nbsprss nbsprss

proc nbsprss/station_catalog {} {

    return [nbsprss_station_catalog];
}

proc nbsprss/received_by_station {station ftype} {

     global Config;

    return [nbsprss_received_by_station \
		$station $Config(nbsprssdir) $Config(nbsprssfext)];
}
