#
# Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
#
# See LICENSE
#

Usage -

  npmcast [options] fpath1 fpath2 ...
  npmcast [options] < filelist

The options are

  -b              => run in backgroudn mode (use syslog for error reports)
  -C              => print configuration and exit
  -r              => file is radar (nids) - default is nwstg (txt)
  -a mcast_addr   => multicast address (default is 224.0.1.1)
  -p mcas_port    => multicast port (default is 1201)
  -i ifname       => interface name to use (conflicts with -I)
  -I ifip         => interface ip to use (conflicts with -i)
  -s prod_seq_num => product sequence number (default is (UINT32_MAX/4 - 2))
  -f sbn_seq_num] => starting sbn sequence number (same default as above)
  -t mcast_ttl	  => set mcast ttl (otherwise us default)
  -m		  => mcast multiloop flag off (default is on)
