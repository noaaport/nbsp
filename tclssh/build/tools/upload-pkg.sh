#!/bin/sh

. upload.conf

os=`uname`
[ $os = Linux ] && flavor=`../rpm/flavor.sh`

case $os in
    *BSD)
	osrelease=`uname -r | cut -f 1 -d "-"`
	osname=freebsd
	wildcard="${name}-*.tbz"
	cd ../bsd
	;;
    Linux)
	if [ $flavor = fedoracore ]
	then
	    release=`cat /etc/fedora-release`
	    osrelease=`echo $release | cut -d ' ' -f 3`
	    osname=fedoracore
	    wildcard="${name}-*.rpm"
	    cd ../rpm
	elif [ $flavor = opensuse ]
	then
	    release=`cat /etc/SuSE-release`
	    osrelease=`echo $release | cut -d ' ' -f 3`
	    osname=opensuse
	    wildcard="${name}-*.rpm"
	    cd ../rpm
	elif [ $flavor = centos ]
	then
	    release=`cat /etc/redhat-release`
	    osrelease=`echo $release | cut -d ' ' -f 3`
	    osname=centos
	    wildcard="${name}-*.rpm"
	    cd ../rpm
        elif [ $flavor = debian ]
        then
            osrelease=`cat /etc/debian_version`
            osname=debian
            wildcard="${name}_*.deb"
            cd ../debian
	fi
	;;
    SunOS)
	osrelase=
	osname=solaris
	osrelease=
	wildcard="{$name}*.tgz"
	cd ../solaris
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
mdelete *
mput $wildcard
quit
EOF
