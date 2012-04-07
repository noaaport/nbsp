1:36 PM 4/21/2003

Early in 2003, David Gravereaux <davygrvy@pobox.com> worked on an improved
winsock channel driver for Tcl sockets.  Specifically, for tclhttpd's use as
a server.  It isn't well optimized for client sockets, and won't be easily
shoe-horned into the core at this time.

The general summary of the different paradigm is that it uses overlapped I/O
with completion ports over the older WSAAsyncSelect model of the core.
Accepts and recvs are done in a preemptive manner with back queueing so
winsock is never left waiting on buffers from user-mode.  All incoming
operations happen entirely in kernel-mode and behind the scenes.

David has a long-term goal to get it into the core, but the work to do it
will keep him busy for quite some time to make this happen.  In the meantime,
the patch to tclhttpd is small and the new channel driver is available as an
extension.

http://sourceforge.net/projects/iocpsock

It roughly has a 4 times speed-up and doesn't choke no matter how hard the
front-end is pushed.  It only runs on NT systems such as WinNT4 (sp6), Win2K
and WinXP.

It should be noted that due to the speed-up, incoming jobs can now pile-up
faster than tclhttpd can process them.  This is good in that we don't want
any connect errors, but the failure mode is now pure delays when the input
to the server is run over its maximum rate.  Maximum rate on a 500MHz pIII
with WinXP is now about 83 hits/sec.  Previous maximum was around 16
hits/sec before connect errors started getting in the way.
