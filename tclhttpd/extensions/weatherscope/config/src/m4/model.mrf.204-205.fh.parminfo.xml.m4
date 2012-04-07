<?datasetconfig version="1.0d1"?>

<parminfo>
    <parm id="stid" type="string" units="nada" name="site ID"/>
    <parm id="stnm" type="int" units="nada" name="site number"/>
    <parm id="time" type="float" units="min" name="time of observation"/>

    <parm id="lat" type="float" units="degr" name="latitude"/>
    <parm id="lon" type="float" units="degr" name="longitude"/>

    <parm id="pres_msl" type="float" units="mbar"  name="__fh__`h' Mean sea level - Pressure"/>

    <parm id="hgt_1000mb" type="float" units="m" name="__fh__`h' 1000mb - Height"/>
    <parm id="tmp_1000mb" type="float" units="kelv" name="__fh__`h' 1000mb - Temperature"/>
    <parm id="rh_1000mb" type="float" units="prct"  name="__fh__`h' 1000mb - Relative humidity"/>
    <parm id="ws_1000mb" type="float" units="mps"  name="__fh__`h' 1000mb - Wind speed"/>
    <parm id="wd_1000mb" type="float" units="degr"  name="__fh__`h' 1000mb - Wind direction"/>
    <derivation id="wu_1000mb" func="uwnd" args="wd_1000mb wd_1000mb"/>
    <derivation id="wv_1000mb" func="vwnd" args="wd_1000mb wd_1000mb"/>
    <vector id="wind_1000mb" name="__fh__`h' 1000mb Winds" samag="ws_1000mb" dir="wd_1000mb" ucomp="wu_1000mb" vcomp="wv_1000mb"/>

    <parm id="hgt_850mb" type="float" units="m" name="__fh__`h' 850mb - Height"/>
    <parm id="tmp_850mb" type="float" units="kelv" name="__fh__`h' 850mb - Temperature"/>
    <parm id="rh_850mb" type="float" units="prct"  name="__fh__`h' 850mb - Relative humidity"/>
    <parm id="ws_850mb" type="float" units="mps"  name="__fh__`h' 850mb - Wind speed"/>
    <parm id="wd_850mb" type="float" units="degr"  name="__fh__`h' 850mb - Wind direction"/>
    <derivation id="wu_850mb" func="uwnd" args="wd_850mb wd_850mb"/>
    <derivation id="wv_850mb" func="vwnd" args="wd_850mb wd_850mb"/>
    <vector id="wind_850mb" name="__fh__`h' 850mb Winds" samag="ws_850mb" dir="wd_850mb" ucomp="wu_850mb" vcomp="wv_850mb"/>

    <parm id="hgt_700mb" type="float" units="m" name="__fh__`h' 700mb - Height"/>
    <parm id="tmp_700mb" type="float" units="kelv" name="__fh__`h' 700mb - Temperature"/>
    <parm id="rh_700mb" type="float" units="prct"  name="__fh__`h' 700mb - Relative humidity"/>
    <parm id="ws_700mb" type="float" units="mps"  name="__fh__`h' 700mb - Wind speed"/>
    <parm id="wd_700mb" type="float" units="degr"  name="__fh__`h' 700mb - Wind direction"/>
    <derivation id="wu_700mb" func="uwnd" args="wd_700mb wd_700mb"/>
    <derivation id="wv_700mb" func="vwnd" args="wd_700mb wd_700mb"/>
    <vector id="wind_700mb" name="__fh__`h' 700mb Winds" samag="ws_700mb" dir="wd_700mb" ucomp="wu_700mb" vcomp="wv_700mb"/>

    <parm id="hgt_500mb" type="float" units="m" name="__fh__`h' 500mb - Height"/>
    <parm id="tmp_500mb" type="float" units="kelv" name="__fh__`h' 500mb - Temperature"/>
    <parm id="rh_500mb" type="float" units="prct"  name="__fh__`h' 500mb - Relative humidity"/>
    <parm id="ws_500mb" type="float" units="mps"  name="__fh__`h' 500mb - Wind speed"/>
    <parm id="wd_500mb" type="float" units="degr"  name="__fh__`h' 500mb - Wind direction"/>
    <derivation id="wu_500mb" func="uwnd" args="wd_500mb wd_500mb"/>
    <derivation id="wv_500mb" func="vwnd" args="wd_500mb wd_500mb"/>
    <vector id="wind_500mb" name="__fh__`h' 500mb Winds" samag="ws_500mb" dir="wd_500mb" ucomp="wu_500mb" vcomp="wv_500mb"/>

    <parm id="hgt_300mb" type="float" units="m" name="__fh__`h' 300mb - Height"/>
    <parm id="tmp_300mb" type="float" units="kelv" name="__fh__`h' 300mb - Temperature"/>
    <parm id="rh_300mb" type="float" units="prct"  name="__fh__`h' 300mb - Relative humidity"/>
    <parm id="ws_300mb" type="float" units="mps"  name="__fh__`h' 300mb - Wind speed"/>
    <parm id="wd_300mb" type="float" units="degr"  name="__fh__`h' 300mb - Wind direction"/>
    <derivation id="wu_300mb" func="uwnd" args="wd_300mb wd_300mb"/>
    <derivation id="wv_300mb" func="vwnd" args="wd_300mb wd_300mb"/>
    <vector id="wind_300mb" name="__fh__`h' 300mb Winds" samag="ws_300mb" dir="wd_300mb" ucomp="wu_300mb" vcomp="wv_300mb"/>

    <parm id="hgt_250mb" type="float" units="m" name="__fh__`h' 250mb - Height"/>
    <parm id="tmp_250mb" type="float" units="kelv" name="__fh__`h' 250mb - Temperature"/>
    <parm id="rh_250mb" type="float" units="prct"  name="__fh__`h' 250mb - Relative humidity"/>
    <parm id="ws_250mb" type="float" units="mps"  name="__fh__`h' 250mb - Wind speed"/>
    <parm id="wd_250mb" type="float" units="degr"  name="__fh__`h' 250mb - Wind direction"/>
    <derivation id="wu_250mb" func="uwnd" args="wd_250mb wd_250mb"/>
    <derivation id="wv_250mb" func="vwnd" args="wd_250mb wd_250mb"/>
    <vector id="wind_250mb" name="__fh__`h' 250mb Winds" samag="ws_250mb" dir="wd_250mb" ucomp="wu_250mb" vcomp="wv_250mb"/>
</parminfo>
