dnl
dnl $Id$
dnl 

# These rules save all the files of a given model run in one file.
# This is done as a service for applications like Wingridds, that
# can import and convert entire files like these instead of individual files.
# After the filter processes these rules, processing continues with the
# individual files rules in grib.m4. 

# This rule processes the <model>_<grid> combinations that will be saved in the
# "grids" subdir. The variable gribfilter(gridsmodels) (in gribfilter.conf)
# sets the desired combinations.

match_file_varregexp(`"$rc(gribmodel)_$rc(gribgrid)"',
$gribfilter(gridsmodels_uwildpatt),
$gribfilter(gridsdatadir)/$rc(gribmodel)/$rc(gribgrid),
[gribfilter_make_default_name rc 0])
