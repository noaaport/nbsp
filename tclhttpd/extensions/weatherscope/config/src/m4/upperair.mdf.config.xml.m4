<?datasetconfig version="1.0d1"?>

<datasetconfig>
  <property id="name" value="Noaaport Upperair Data"/>
  <property id="provider" value="__provider_value__" url="__provider_url__"/>
  <property id="source" value="__source_value__" url="__source_url__"/>
  <property id="contact" value="__contact_value__" url="__contact_url__"/>
  <property id="baseUrl" value="__baseurl__/weatherscope"/>
  <property id="configUrl" value="/conf/upperair.mdf.config.xml"/>
  <property id="siteInfoUrl" value="/conf/upperair.siteinfo.xml"/>
  <property id="parmInfoUrl" value="/conf/upperair.parminfo.xml"/>
  <property id="dataUrlFormat" value="/data/%04Y%02m%02d/upperair/%04Y%02m%02d%02H.mdf"/>
  <property id="dataResolution" value="720"/>
  <property id="staleThreshold" value="1080"/>
  <property id="minimumCheckThreshold" value="60"/>
</datasetconfig>
