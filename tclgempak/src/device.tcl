#
# $Id$
#
# Device functions
#
proc gempak::device_device {device} {

    variable gempak;

    set gempak(param,device.device) $device;
}

proc gempak::device_name {name} {

    variable gempak;

    set gempak(param,device.name) $name;
}

proc gempak::device_size {xsize ysize} {

    ::gempak::_join_param_parts "device.size" ";" $xsize $ysize;
}

proc gempak::device_color {color} {

    variable gempak;

    set gempak(param,device.color) $color;
}

proc gempak::set_device {} {

    set parts [list device name size color];
    ::gempak::_set_param "device" "|" $parts;
}

proc gempak::get_device {} {

    return [::gempak::get "device"];
}

proc gempak::get_device_list {} {

    return [split [::gempak::get_device] "|"];
}
