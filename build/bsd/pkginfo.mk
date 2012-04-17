#
# $Id$
#

# This file is read by the (bsd package) Makefile

package_build != cat pkg-release
package_category = misc
package_ext = tbz
package_display_file = pkg-display
package_display_file_client = pkg-display-client

OS != uname

.if ${OS} == OpenBSD
package_ext = tgz	
.endif

.if ${OS} == NetBSD
package_ext = tgz
package_display_file = pkg-display-netbsd
package_display_file_client = pkg-display-client-netbsd
package_buildinfo_file = pkg-buildinfo
package_buildinfo_file_client = pkg-buildinfo-client
.endif
