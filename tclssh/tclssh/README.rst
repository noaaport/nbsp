Overview
========

**Tclssh** is a package of pure Tcl functions for executing Tcl scripts
in a remote host. It is used by *Nbsp* and the *WRFPak* to offload
some long-time running jobs in parallel to other computers.
The remote host must be accessible via ssh keys without a password;
that is, for example, the contents of your *./ssh/id_rsa.pub* must
be added to the *.ssh/authorized_keys* file in the remote host.
See the Wiki section for usage and examples.
