Summary: Extensible Web+Application server written in Tcl.
Name: tclhttpd
Version: 3.5.1
Release: 1
Copyright: BSD
Group: Applications/Text
Source: http://prdownloads.sourceforge.net/tclhttpd/tclhttpd-3.5.1.tar.gz
BuildRoot: /var/tmp/%{name}-%{version}
Prefix: /usr
requires: tcl

%description
TclHttpd is a Web server implemented in pure Tcl. It works out of the box as a
Web server, but is really designed to be a Tcl application server. It supports
HTML+Tcl templates, and is extensible in a variety of ways.

%prep
%setup -q

%build

./configure --prefix=%{buildroot}%{prefix}
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{prefix}/lib
%{prefix}/bin
%{prefix}/man
%{prefix}/tclhttpd
