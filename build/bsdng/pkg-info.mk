#
# $Id$
#
pkg_build != cat pkg-build
abi != pkg -vv | awk '$$1 == "ABI" {print $$NF}'
