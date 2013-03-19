%define idmetaversion %(. ./IDMETA; echo $VERSION|tr -d '\n')
Summary: YAZ++ package (main)
Name: yazpp
Version: %{idmetaversion}
Release: 1indexdata
License: BSD
Group: Applications/Internet
Vendor: Index Data ApS <info@indexdata.dk>
Source: yazpp-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prefix: %{_prefix}
BuildRequires: pkgconfig, libyaz4-devel >= 4.2.8
Packager: Adam Dickmeiss <adam@indexdata.dk>
URL: http://www.indexdata.com/yazplusplus

%description
YAZ++ package.

%package -n libyazpp5
Summary: YAZ++ and ZOOM library
Group: Libraries
Requires: libyaz4 >= 4.2.8

%description -n libyazpp5
Libraries for the YAZ++ package.

%package -n libyazpp5-devel
Summary: Z39.50 Library - development package
Group: Development/Libraries
Requires: libyazpp5 = %{version}, libyaz4-devel
Conflicts: libyazpp4-devel

%description -n libyazpp5-devel
Development libraries and include files for the YAZ++ package.

%prep
%setup

%build

CFLAGS="$RPM_OPT_FLAGS" \
 ./configure --prefix=%{_prefix} --libdir=%{_libdir} --mandir=%{_mandir} \
	--enable-shared --with-yaz=/usr/bin
make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -fr ${RPM_BUILD_ROOT}
make install DESTDIR=${RPM_BUILD_ROOT}
rm ${RPM_BUILD_ROOT}/%{_libdir}/*.la

%clean
rm -fr ${RPM_BUILD_ROOT}

%post -n libyazpp5 -p /sbin/ldconfig 
%postun -n libyazpp5 -p /sbin/ldconfig 

%files -n libyazpp5
%doc README LICENSE NEWS
%defattr(-,root,root)
%{_libdir}/*.so.*

%files -n libyazpp5-devel
%defattr(-,root,root)
%{_bindir}/yazpp-config
%{_includedir}/yazpp
%{_libdir}/*.so
%{_libdir}/*.a
%{_datadir}/aclocal/yazpp.m4
%{_mandir}/man?/yazpp-config.*
%{_datadir}/doc/yazpp

