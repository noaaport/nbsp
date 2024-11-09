#!/bin/sh

. ../../configure.inc

case $os in
    FreeBSD|NetBSD|OpenBSD|SunOS)
	cp config.h.${os} ../config.h
	;;
    Linux)
	cp config.h.${flavor} ../config.h
	;;
esac
