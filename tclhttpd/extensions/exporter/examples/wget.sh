#!/bin/sh

# The current list of n0r radar data files in jua
#
wget -O output 'http://diablo:8015/_export/query_dir?dir=digatmos/nexrad/nids/jua/n0r&select=\.nids'

wget -O output 'http://diablo:8015/_export/query_list?select=nexrad'

