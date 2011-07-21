#
# $Id$
#

# This is read by the (rpm) makefile to produce the <name>-<version>.spec
# file. The defaults are for FC, and the overrides are other flavors are below.
FLAVOR = $(shell ./flavor.sh)

package_build = 1
#rpmroot = /usr/local/src/redhat
rpmroot = /usr/src/redhat

# empty variables will be filled by make
Summary = Noaaport Broadcast System Processor 
Name = ${name}
Version = ${version}
Release = ${package_build}
License = BSD
Group = Applications/Internet
Source = http://www.noaaport.net/software/${pkgsrc_name}/src/${pkgsrc_name}.tgz
BuildRoot = ${rpmroot}/BUILD/${pkgsrc_name}/build/rpm/pkg
Requires = tcp_wrappers-libs db4 db4-utils tcl tcllib tk libpng netpbm-progs zlib gnuplot gifsicle
Conflicts = ${nameclient}

ifeq (${FLAVOR}, opensuse)
rpmroot = /usr/src/packages
Requires = sharutils tcpd libdb-4_5 db-utils tcl tcllib tk expect libpng netpbm zlib bzip2 gnuplot gifsicle mapserver
endif

ifeq (${FLAVOR}, centos)
Requires =  sharutils tcp_wrappers tcl tcllib tk libpng netpbm-progs zlib bzip2 gnuplot gifsicle
endif
