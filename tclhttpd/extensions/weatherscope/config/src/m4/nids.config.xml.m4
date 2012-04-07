<?datasetconfig version="1.0d1"?>

<datasetconfig>
  <property id="name" value="Noaaport Nexrad Level III"/>
  <property id="type" value="nids"/>
  <property id="provider" value="__provider_value__" url="__provider_url__"/>
  <property id="source" value="__source_value__" url="__source_url__"/>
  <property id="contact" value="__contact_value__" url="__contact_url__"/>
  <property id="baseUrl" value="__baseurl__/weatherscope"/>
  <property id="configUrl" value="/conf/nids.config.xml"/>
  <property id="siteInfoUrl" value="/conf/nids.siteinfo.xml"/>
  <property id="prodInfoUrl" value="/conf/nids.prodinfo.xml"/>
  <property id="dataUrlFormat" value="__baseurl__/_weatherscope/get/nids?site=%siteid;&amp;prod=%product;&amp;fbasename=%product;%siteid;_%04Y%02m%02d_%02H%02M.nids"/>
  <property id="dataResolution" value="1"/>
  <property id="staleThreshold" value="16"/>
  <property id="minimumCheckThreshold" value="4"/>
  <property id="dataQueryUrlFormat" value="__baseurl__/_weatherscope/query/nids?site=%siteid;&amp;prod=%product;&amp;start=%startDate;&amp;end=%endDate;"/>
  <property id="dataQueryDateFormatString" value="%Y%m%d_%H%M"/>
</datasetconfig>
