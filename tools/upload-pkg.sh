#!/bin/sh

. upload.conf

os=`uname`
[ $os = Linux ] && flavor=`../rpm/flavor.sh`

case $os in
    *BSD)
	osarch=`uname -m`
	osrelease=`uname -r | cut -f 1 -d "-"`
	osname=freebsd
	wildcard="nbsp-*.tbz"
	cd ../bsd
	;;
    Linux)
	osarch=`uname -m`
	if [ $flavor = fedoracore ]
	then
	    release=`cat /etc/fedora-release`
	    osrelease=`echo $release | cut -d ' ' -f 3`
	    osname=fedoracore
	    wildcard="nbsp-*.rpm"
	    cd ../rpm
	elif [ $flavor = opensuse ]
	then
	    release=`cat /etc/SuSE-release`
	    osrelease=`echo $release | cut -d ' ' -f 3`
	    osname=opensuse
	    wildcard="nbsp-*.rpm"
	    cd ../rpm
	elif [ $flavor = centos ]
	then
	    release=`cat /etc/redhat-release`
	    osrelease=`echo $release | cut -d ' ' -f 3`
	    osname=centos
	    wildcard="nbsp-*.rpm"
	    cd ../rpm
	elif [ $flavor = debian ]
	then
	    [ $osarch = "x86_64" ] && osarch="amd64"
	    osrelease=`cat /etc/debian_version`
	    osname=debian
	    wildcard="nbsp_*.deb"
	    cd ../debian
	fi
	;;
    SunOS)
	osarch=`uname -m`
	osrelase=
	osname=solaris
	wildcard="*.tgz"
	cd ../solaris
	;;
    Darwin)
	osarch=`uname -m`
	osrelease="10.5"
	osname=macosx
	wildcard="*.dmg"
	cd ../macosx
	;;
esac

ftp -n -v $uploadhost <<EOF
user $uploaduser
prompt
mkdir $uploaddir
cd $uploaddir
mkdir $pkguploadsubdir
cd $pkguploadsubdir
mkdir $osname-$osrelease
cd $osname-$osrelease
mkdir $osarch
cd $osarch
mdelete *
mput $wildcard
quit
EOF
