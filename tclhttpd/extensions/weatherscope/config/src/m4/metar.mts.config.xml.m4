<?datasetconfig version="1.0d1"?>

<datasetconfig>
  <property id="name" value="Noaaport Metar Data"/>
  <property id="type" value="mts"/>
  <property id="provider" value="__provider_value__" url="__provider_url__"/>
  <property id="source" value="__source_value__" url="__source_url__"/>
  <property id="contact" value="__contact_value__" url="__contact_url__"/>
  <property id="baseUrl" value="__baseurl__/weatherscope"/>
  <property id="configUrl" value="/conf/metar.mts.config.xml"/>
  <property id="siteInfoUrl" value="/conf/metar.siteinfo.xml"/>
  <property id="parmInfoUrl" value="/conf/metar.parminfo.xml"/>
  <property id="timeSeriesUrlFormat" value="__baseurl__/_weatherscope/get/metar/daily?date=%04Y%02m%02d&amp;icao=%siteid;"/>
  <property id="dataResolution" value="20"/>
  <property id="fileResolution" value="1440"/>
</datasetconfig>
