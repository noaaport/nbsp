#!/bin/sh

include_dir="include"
templates_dir="templates"

main_page="main.tml.in"

index_pages="index"

status_pages="summary \
    stats \
    qstate \
    qdbstats \
    mdbstats \
    missing \
    chstats \
    connections \
    slavestats \
    statplot \
    printconf \
    received_last_minute \
    received_minute \
    received_past_hour \
    received_last_hour \
    received_last_24hours \
    received_last_3hours"

#
# main
#
[ $# -ne 1 ] && { echo "Needs build dir"; exit 1; }
build_dir=$1

index_dir=${build_dir}
status_dir=${build_dir}/nbsp/status

rm -rf ${build_dir}
mkdir -p ${index_dir} ${status_dir}

for p in $index_pages
do
    sed -e "/@header@/r ${include_dir}/header.html" \
	-e "/@header@/d" \
	-e "/@topmenu@/r ${include_dir}/topmenu.html" \
	-e "/@topmenu@/d" \
	-e "/@leftmenu@/r ${include_dir}/leftmenu.html" \
	-e "/@leftmenu@/d" \
	-e "/@body@/r ${templates_dir}/index/${p}.tml.body" \
	-e "/@body@/d" \
	${templates_dir}/index/main.tml.in > ${index_dir}/${p}.tml
done

for p in $status_pages
do
    sed -e "/@header@/r ${include_dir}/header.html" \
	-e "/@header@/d" \
	-e "/@topmenu@/r ${include_dir}/topmenu.html" \
	-e "/@topmenu@/d" \
	-e "/@leftmenu@/r ${include_dir}/leftmenu.html" \
	-e "/@leftmenu@/d" \
	-e "/@body@/r ${templates_dir}/status/${p}.tml.body" \
	-e "/@body@/d" \
	${templates_dir}/status/main.tml.in > ${status_dir}/${p}.tml
done
