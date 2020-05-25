Summary: A C interface library to i3wm
Name:    i3ipc-glib
Version: 0.6.0
Release: 1%{?dist}
Source0: %{name}-%{version}.tar.gz
License: GPL v3
URL:     https://github.com/altdesktop/i3ipc-glib

BuildRequires: libtool
BuildRequires: libX11-devel
BuildRequires: libxcb-devel
BuildRequires: gobject-introspection-devel
BuildRequires: glib2-devel
BuildRequires: json-glib-devel
BuildRequires: gtk-doc

%if 0%{?suse_version} >= 1500
Requires: libxcb1
Requires: libjson-glib-1_0-0
Requires: libglib-2_0-0
%else
Requires: libxcb
Requires: json-glib
Requires: glib2
%endif
Requires: gobject-introspection

%description
A C interface library to i3wm.


%prep
%setup -q


%build
./autogen.sh --prefix=%{_prefix} --libdir=%{_libdir}
make


%install
%make_install


%files
%doc COPYING
%doc README.md
%{_includedir}/i3ipc-glib/i3ipc*.h
%{_libdir}/girepository-1.0/i3ipc-1.0.typelib
%{_libdir}/libi3ipc-glib-1.0.*
%{_libdir}/pkgconfig/i3ipc-glib-1.0.pc
%{_datadir}/gir-1.0/i3ipc-1.0.gir


%changelog
