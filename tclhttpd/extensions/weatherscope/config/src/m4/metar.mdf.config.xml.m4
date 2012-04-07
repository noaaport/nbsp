<?datasetconfig version="1.0d1"?>

<datasetconfig>
  <property id="name" value="Noaaport Metar Data"/>
  <property id="type" value="mdf"/>
  <property id="provider" value="__provider_value__" url="__provider_url__"/>
  <property id="source" value="__source_value__" url="__source_url__"/>
  <property id="contact" value="__contact_value__" url="__contact_url__"/>
  <property id="baseUrl" value="__baseurl__/weatherscope"/>
  <property id="configUrl" value="/conf/metar.mdf.config.xml"/>
  <property id="siteInfoUrl" value="/conf/metar.siteinfo.xml"/>
  <property id="parmInfoUrl" value="/conf/metar.parminfo.xml"/>
  <property id="dataUrlFormat" value="/data/%04Y%02m%02d/metar/%04Y%02m%02d%02H.mdf"/>
  <property id="dataResolution" value="5"/>
  <property id="staleThreshold" value="90"/>
  <property id="minimumCheckThreshold" value="10"/>
</datasetconfig>
