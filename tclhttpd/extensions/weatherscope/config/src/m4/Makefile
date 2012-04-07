#
# $Id$
#

GFS211_CONFIG_FH = 0 6 12 18 24 30 36 42 48 54 60
RUC236_CONFIG_FH = 1 2 3
NAM207_CONFIG_FH = 0 6 12 18 24 30 36 42 48 54 60
MRF204_CONFIG_FH = 0 12 24 36 48 60
MRF205_CONFIG_FH = 0 12 24 36 48 60

PACK_CLIENT_DIR = ${PACK_CLIENT_NAME}
PACK_CLIENT_FILE = ${PACK_CLIENT_DIR}.zip

PACK_SERVER_DIR = ${PACK_SERVER_NAME}
PACK_SERVER_FILE = ${PACK_SERVER_DIR}.tgz

TARGET = metar.mdf.config.xml metar.mts.config.xml upperair.mdf.config.xml \
	nids.config.xml \
	stage.build.gfs211 \
	stage.build.ruc236 \
	stage.build.nam207 \
	stage.build.mrf204 \
	stage.build.mrf205

all: ${TARGET}

metar.mdf.config.xml: defs.m4 metar.mdf.config.xml.m4
	m4 defs.m4 metar.mdf.config.xml.m4 > metar.mdf.config.xml

metar.mts.config.xml: metar.mts.config.xml.m4
	m4 defs.m4 metar.mts.config.xml.m4 > metar.mts.config.xml

upperair.mdf.config.xml: defs.m4 upperair.mdf.config.xml.m4
	m4 defs.m4 upperair.mdf.config.xml.m4 > upperair.mdf.config.xml

nids.config.xml: defs.m4 nids.config.xml.m4
	m4 defs.m4 nids.config.xml.m4 > nids.config.xml

stage.build.gfs211: defs.m4 model.gfs.211.fh.config.xml.m4 \
	model.default.fh.parminfo.xml.m4
	for FH in ${GFS211_CONFIG_FH}; \
	do \
	m4 -D __fh__=$${FH} defs.m4 model.gfs.211.fh.config.xml.m4 \
		> model.gfs.211.$${FH}h.config.xml; \
	m4 -D __fh__=$${FH} defs.m4 model.default.fh.parminfo.xml.m4 \
		> model.gfs.211.$${FH}h.parminfo.xml; \
	done
	touch stage.build.gfs211

stage.build.ruc236: defs.m4 model.ruc.236.fh.config.xml.m4 \
	model.default.fh.parminfo.xml.m4
	for FH in ${RUC236_CONFIG_FH}; \
	do \
	m4 -D __fh__=$${FH} defs.m4 model.ruc.236.fh.config.xml.m4 \
		> model.ruc.236.$${FH}h.config.xml; \
	m4 -D __fh__=$${FH} defs.m4 model.default.fh.parminfo.xml.m4 \
		> model.ruc.236.$${FH}h.parminfo.xml; \
	done
	touch stage.build.ruc236

stage.build.nam207: defs.m4 model.nam.207.fh.config.xml.m4 \
	model.nam.207.fh.parminfo.xml.m4
	for FH in ${NAM207_CONFIG_FH}; \
	do \
	m4 -D __fh__=$${FH} defs.m4 model.nam.207.fh.config.xml.m4 \
		> model.nam.207.$${FH}h.config.xml; \
	m4 -D __fh__=$${FH} defs.m4 model.nam.207.fh.parminfo.xml.m4 \
		> model.nam.207.$${FH}h.parminfo.xml; \
	done
	touch stage.build.nam207

stage.build.mrf204: defs.m4 model.mrf.204.fh.config.xml.m4 \
		model.mrf.204-205.fh.parminfo.xml.m4
	for FH in ${MRF204_CONFIG_FH}; \
	do \
	m4 -D __fh__=$${FH} defs.m4 model.mrf.204.fh.config.xml.m4 \
		> model.mrf.204.$${FH}h.config.xml; \
	m4 -D __fh__=$${FH} defs.m4 model.mrf.204-205.fh.parminfo.xml.m4 \
		> model.mrf.204.$${FH}h.parminfo.xml; \
	done
	touch stage.build.mrf204

stage.build.mrf205: defs.m4 model.mrf.205.fh.config.xml.m4 \
		model.mrf.204-205.fh.parminfo.xml.m4
	for FH in ${MRF205_CONFIG_FH}; \
	do \
	m4 -D __fh__=$${FH} defs.m4 model.mrf.205.fh.config.xml.m4 \
		> model.mrf.205.$${FH}h.config.xml; \
	m4 -D __fh__=$${FH} defs.m4 model.mrf.204-205.fh.parminfo.xml.m4 \
		> model.mrf.205.$${FH}h.parminfo.xml; \
	done
	touch stage.build.mrf205

install-client: all
	rm -rf ${PACK_CLIENT_DIR}
	install -d ${PACK_CLIENT_DIR}
	for f in *.config.xml; \
	do \
	name=$${f%.config.xml}; \
	setname=noaaport.$${name}; \
	install -d ${PACK_CLIENT_DIR}/$${setname}/info; \
	install -m 0644 $${f} ${PACK_CLIENT_DIR}/$${setname}/info/config.xml; \
	done

pack-client: install-client
	zip -r ${PACK_CLIENT_FILE} ${PACK_CLIENT_DIR}

client: pack-client
	mv ${PACK_CLIENT_FILE} ..

install-server: all
	rm -rf ${PACK_SERVER_DIR}
	install -d ${PACK_SERVER_DIR}
	for f in *.xml ../xml/*.xml; \
	do \
	install -m 0644 $${f} ${PACK_SERVER_DIR}; \
	done

pack-server: install-server
	tar -czf ${PACK_SERVER_FILE} ${PACK_SERVER_DIR}

server: pack-server
	mv ${PACK_SERVER_FILE} ..

clean:
	rm -rf *~ *.xml ${TARGET} ${PACK_CLIENT_DIR} ${PACK_SERVER_DIR}

distclean: clean
	rm -f ${PACK_CLIENT_FILE} ${PACK_SERVER_FILE}
