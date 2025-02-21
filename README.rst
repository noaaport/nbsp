Overview
========

The *Noaaport Broadcast System Processor* (**Nbsp**) is a software suite
for receiving, processing and distributing the contents of the
*NOAAPort* data stream. Nbsp comes with extensive and flexible
facilities to enable product arrival notifications, database
insertions, web site transfers and many other extensions.
The core module of **Nbsp** is written in C and the post-processing
facilities are based on a comprehensive set of
`Tcl
<http://wiki.tcl.tk/41439?redir=36636>`_
libraries and scripts.

**Nbsp** can run standalone, or in a master-slave mode combination. In
a master-slave setup, the master instance of nbsp receives the
broadcast stream in one computer, and then transmits the complete
files to a slave instance of nbsp running in a different computer
for further processing and/or distribution of the files.

Extensive and flexible capabilities exist for processing the files
further by any number of postprocessors or filters. The filters,
which can be written in any language, can be used to rewrite the
files in any format, or to save them in different directories or
using a particular naming convention, or for distributing the files
using any external program. These facilities can be used for
configuring the system for specialized tasks, such as installing a
notification system when particular files are received, or to fit
specific customized storage, backup and retrieval schemes.

Nbsp can distribute the files through the network in various formats,
including standad http and ftp channels, or the (old) QBT (Quick Block
Transfer *byte blaster*) format used by emwin (pure text bulletins
as well as jpg satellite images and gif radar images). It has the
capability to distribute the files by nntp, via an inn news gateway,
and by RSS feeds accessible from the built-in web server. **Nbsp** has
a web interface for monitoring its activities. The built-in web
server supplies information about the files received, and can report
various statistics that nbsp keeps about its internal state.

What's New
==========

Thu Jan 16 20:20:52 AST 2025
----------------------------

The processing of the satellite files has been updated,
in particular the processing of the Goes-R files.
The current version (**nbsp-2.3.9r**) produces images directly
(and "automatically") from the data files, by means of new programs
**nbspgoesr** and **nbspgoesrinfo** (and corresponding libraries
`nbspgislib
<https://bitbucket.org/noaaport/nbspgislib>`_
and
`nbspgislibmap
<https://bitbucket.org/noaaport/nbspgislibmap>`_
)
with the appropriate updates to the relevant filters
(e.g., **rstfilter, dafilter**). No external applications (e.g., Gempak or such)
are required. Some usage guidelines and easily reproducible
examples are in the *nbspgislibmap*
`wiki
<https://bitbucket.org/noaaport/nbspgislibmap/wiki>`_.
