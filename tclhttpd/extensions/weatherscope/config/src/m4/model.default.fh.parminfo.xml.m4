<?datasetconfig version="1.0d1"?>

<parminfo>
    <parm id="stid" type="string" units="nada" name="site ID"/>
    <parm id="stnm" type="int" units="nada" name="site number"/>
    <parm id="time" type="float" units="min" name="time of observation"/>

    <parm id="lat" type="float" units="degr" name="latitude"/>
    <parm id="lon" type="float" units="degr" name="longitude"/>

    <parm id="pres_sfc" type="float" units="mbar"  name="__fh__`h' Surface level - Pressure"/>
    <parm id="pres_trop" type="float" units="mbar"  name="__fh__`h' Tropopause level - Pressure"/>
    <parm id="tmp_trop" type="float" units="kelv"  name="__fh__`h' Tropopause level - Temperature"/>
    <parm id="ws_trop" type="float" units="mps"  name="__fh__`h' Tropopause level - Wind speed"/>
    <parm id="wd_trop" type="float" units="degr"  name="__fh__`h' Tropopause level - Wind direction"/>

    <parm id="hgt_1000mb" type="float" units="m" name="__fh__`h' 1000mb - Height"/>
    <parm id="tmp_1000mb" type="float" units="kelv" name="__fh__`h' 1000mb - Temperature"/>
    <parm id="rh_1000mb" type="float" units="prct"  name="__fh__`h' 1000mb - Relative humidity"/>
    <parm id="ws_1000mb" type="float" units="mps"  name="__fh__`h' 1000mb - Wind speed"/>
    <parm id="wd_1000mb" type="float" units="degr"  name="__fh__`h' 1000mb - Wind direction"/>
    <derivation id="wu_1000mb" func="uwnd" args="wd_1000mb wd_1000mb"/>
    <derivation id="wv_1000mb" func="vwnd" args="wd_1000mb wd_1000mb"/>
    <vector id="wind_1000mb" name="__fh__`h' 1000mb Winds" samag="ws_1000mb" dir="wd_1000mb" ucomp="wu_1000mb" vcomp="wv_1000mb"/>

    <parm id="hgt_900mb" type="float" units="m" name="__fh__`h' 900mb - Height"/>
    <parm id="tmp_900mb" type="float" units="kelv" name="__fh__`h' 900mb - Temperature"/>
    <parm id="rh_900mb" type="float" units="prct"  name="__fh__`h' 900mb - Relative humidity"/>
    <parm id="ws_900mb" type="float" units="mps"  name="__fh__`h' 900mb - Wind speed"/>
    <parm id="wd_900mb" type="float" units="degr"  name="__fh__`h' 900mb - Wind direction"/>
    <derivation id="wu_900mb" func="uwnd" args="wd_900mb wd_900mb"/>
    <derivation id="wv_900mb" func="vwnd" args="wd_900mb wd_900mb"/>
    <vector id="wind_900mb" name="__fh__`h' 900mb Winds" samag="ws_900mb" dir="wd_900mb" ucomp="wu_900mb" vcomp="wv_900mb"/>

    <parm id="hgt_800mb" type="float" units="m" name="__fh__`h' 800mb - Height"/>
    <parm id="tmp_800mb" type="float" units="kelv" name="__fh__`h' 800mb - Temperature"/>
    <parm id="rh_800mb" type="float" units="prct"  name="__fh__`h' 800mb - Relative humidity"/>
    <parm id="ws_800mb" type="float" units="mps"  name="__fh__`h' 800mb - Wind speed"/>
    <parm id="wd_800mb" type="float" units="degr"  name="__fh__`h' 800mb - Wind direction"/>
    <derivation id="wu_800mb" func="uwnd" args="wd_800mb wd_800mb"/>
    <derivation id="wv_800mb" func="vwnd" args="wd_800mb wd_800mb"/>
    <vector id="wind_800mb" name="__fh__`h' 800mb Winds" samag="ws_800mb" dir="wd_800mb" ucomp="wu_800mb" vcomp="wv_800mb"/>

    <parm id="hgt_700mb" type="float" units="m" name="__fh__`h' 700mb - Height"/>
    <parm id="tmp_700mb" type="float" units="kelv" name="__fh__`h' 700mb - Temperature"/>
    <parm id="rh_700mb" type="float" units="prct"  name="__fh__`h' 700mb - Relative humidity"/>
    <parm id="ws_700mb" type="float" units="mps"  name="__fh__`h' 700mb - Wind speed"/>
    <parm id="wd_700mb" type="float" units="degr"  name="__fh__`h' 700mb - Wind direction"/>
    <derivation id="wu_700mb" func="uwnd" args="wd_700mb wd_700mb"/>
    <derivation id="wv_700mb" func="vwnd" args="wd_700mb wd_700mb"/>
    <vector id="wind_700mb" name="__fh__`h' 700mb Winds" samag="ws_700mb" dir="wd_700mb" ucomp="wu_700mb" vcomp="wv_700mb"/>

    <parm id="hgt_600mb" type="float" units="m" name="__fh__`h' 600mb - Height"/>
    <parm id="tmp_600mb" type="float" units="kelv" name="__fh__`h' 600mb - Temperature"/>
    <parm id="rh_600mb" type="float" units="prct"  name="__fh__`h' 600mb - Relative humidity"/>
    <parm id="ws_600mb" type="float" units="mps"  name="__fh__`h' 600mb - Wind speed"/>
    <parm id="wd_600mb" type="float" units="degr"  name="__fh__`h' 600mb - Wind direction"/>
    <derivation id="wu_600mb" func="uwnd" args="wd_600mb wd_600mb"/>
    <derivation id="wv_600mb" func="vwnd" args="wd_600mb wd_600mb"/>
    <vector id="wind_600mb" name="__fh__`h' 600mb Winds" samag="ws_600mb" dir="wd_600mb" ucomp="wu_600mb" vcomp="wv_600mb"/>

    <parm id="hgt_500mb" type="float" units="m" name="__fh__`h' 500mb - Height"/>
    <parm id="tmp_500mb" type="float" units="kelv" name="__fh__`h' 500mb - Temperature"/>
    <parm id="rh_500mb" type="float" units="prct"  name="__fh__`h' 500mb - Relative humidity"/>
    <parm id="ws_500mb" type="float" units="mps"  name="__fh__`h' 500mb - Wind speed"/>
    <parm id="wd_500mb" type="float" units="degr"  name="__fh__`h' 500mb - Wind direction"/>
    <derivation id="wu_500mb" func="uwnd" args="wd_500mb wd_500mb"/>
    <derivation id="wv_500mb" func="vwnd" args="wd_500mb wd_500mb"/>
    <vector id="wind_500mb" name="__fh__`h' 500mb Winds" samag="ws_500mb" dir="wd_500mb" ucomp="wu_500mb" vcomp="wv_500mb"/>

    <parm id="hgt_400mb" type="float" units="m" name="__fh__`h' 400mb - Height"/>
    <parm id="tmp_400mb" type="float" units="kelv" name="__fh__`h' 400mb - Temperature"/>
    <parm id="rh_400mb" type="float" units="prct"  name="__fh__`h' 400mb - Relative humidity"/>
    <parm id="ws_400mb" type="float" units="mps"  name="__fh__`h' 400mb - Wind speed"/>
    <parm id="wd_400mb" type="float" units="degr"  name="__fh__`h' 400mb - Wind direction"/>
    <derivation id="wu_400mb" func="uwnd" args="wd_400mb wd_400mb"/>
    <derivation id="wv_400mb" func="vwnd" args="wd_400mb wd_400mb"/>
    <vector id="wind_400mb" name="__fh__`h' 400mb Winds" samag="ws_400mb" dir="wd_400mb" ucomp="wu_400mb" vcomp="wv_400mb"/>

    <parm id="hgt_300mb" type="float" units="m" name="__fh__`h' 300mb - Height"/>
    <parm id="tmp_300mb" type="float" units="kelv" name="__fh__`h' 300mb - Temperature"/>
    <parm id="rh_300mb" type="float" units="prct"  name="__fh__`h' 300mb - Relative humidity"/>
    <parm id="ws_300mb" type="float" units="mps"  name="__fh__`h' 300mb - Wind speed"/>
    <parm id="wd_300mb" type="float" units="degr"  name="__fh__`h' 300mb - Wind direction"/>
    <derivation id="wu_300mb" func="uwnd" args="wd_300mb wd_300mb"/>
    <derivation id="wv_300mb" func="vwnd" args="wd_300mb wd_300mb"/>
    <vector id="wind_300mb" name="__fh__`h' 300mb Winds" samag="ws_300mb" dir="wd_300mb" ucomp="wu_300mb" vcomp="wv_300mb"/>

    <parm id="hgt_200mb" type="float" units="m" name="__fh__`h' 200mb - Height"/>
    <parm id="tmp_200mb" type="float" units="kelv" name="__fh__`h' 200mb - Temperature"/>
    <parm id="rh_200mb" type="float" units="prct"  name="__fh__`h' 200mb - Relative humidity"/>
    <parm id="ws_200mb" type="float" units="mps"  name="__fh__`h' 200mb - Wind speed"/>
    <parm id="wd_200mb" type="float" units="degr"  name="__fh__`h' 200mb - Wind direction"/>
    <derivation id="wu_200mb" func="uwnd" args="wd_200mb wd_200mb"/>
    <derivation id="wv_200mb" func="vwnd" args="wd_200mb wd_200mb"/>
    <vector id="wind_200mb" name="__fh__`h' 200mb Winds" samag="ws_200mb" dir="wd_200mb" ucomp="wu_200mb" vcomp="wv_200mb"/>

    <parm id="hgt_100mb" type="float" units="m" name="__fh__`h' 100mb - Height"/>
    <parm id="tmp_100mb" type="float" units="kelv" name="__fh__`h' 100mb - Temperature"/>
    <parm id="rh_100mb" type="float" units="prct"  name="__fh__`h' 100mb - Relative humidity"/>
    <parm id="ws_100mb" type="float" units="mps"  name="__fh__`h' 100mb - Wind speed"/>
    <parm id="wd_100mb" type="float" units="degr"  name="__fh__`h' 100mb - Wind direction"/>
    <derivation id="wu_100mb" func="uwnd" args="wd_100mb wd_100mb"/>
    <derivation id="wv_100mb" func="vwnd" args="wd_100mb wd_100mb"/>
    <vector id="wind_100mb" name="__fh__`h' 100mb Winds" samag="ws_100mb" dir="wd_100mb" ucomp="wu_100mb" vcomp="wv_100mb"/>
</parminfo>
