#!/bin/sh
#
# $Id$
#
# Script for creating the netbsd BUILF_INFO file.

cat <<EOF
BUILD_DATE=`date`
BUILD_HOST=`uname -a`
CATEGORIES=misc
CC_VERSION=`cc --version | head -n 1`
CFLAGS=
CPPFLAGS=
FFLAGS=
HOMEPAGE=http://www.noaaport.net
LICENSE=
LOCALBASE=/usr/local
MACHINE_ARCH=`uname -m`
MACHINE_GNU_ARCH=`uname -m`
MAINTAINER=nieves@noaaport.net
OPSYS=`uname`
OS_VERSION=`uname -r`
PKGMANDIR=man
PKGPATH=misc/nbsp
PKGTOOLS_VERSION=`pkg_admin -V`
PKG_SYSCONFBASEDIR=/usr/local/etc
PKG_SYSCONFDIR=/usr/local/etc/nbsp
PROVIDES=/usr/local/sbin/nbspd
REQUIRES=/usr/pkg/bin/tclsh
EOF
