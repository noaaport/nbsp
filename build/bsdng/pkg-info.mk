#
# $Id$
#
pkg_build != cat pkg-build
abi != pkg -vv | awk '$$1 == "abi:" {print $$2}'
