# Available build options, you will need rpm-build >= 4.0.3 for this to work.
# Example: rpmbuild -ba --with email aware.spec
#
#  Storage Options
#  ===============
#  --with email
#  --with files
#  --with snmp
#  --with spread
#  --with stomp
#  --with tokyo
#  --with zeromq2

#
# These setup the storage backends to off by default
#
%bcond_with email
%bcond_with files
%bcond_with snmp
%bcond_with spread
%bcond_with stomp
%bcond_with tokyo
%bcond_with zeromq2


# Define version and release number
%define version 0.1.1
%define release 1

Name:      php-aware
Version:   %{version}
Release:   %{release}%{?dist}
Packager:  Mikko Koppanen <mkoppanen@php.net>
Summary:   PHP aware extension
License:   PHP License
Group:     Web/Applications
URL:       http://github.com/mkoppanen/php-aware
Source:    aware-%{version}.tgz
Prefix:    %{_prefix}
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: php-devel, make, gcc, /usr/bin/phpize

%description
Monitoring extension for PHP

%package devel
Summary: Development headers for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description devel
Development headers for %{name}

### Conditional build for email
%if %{with email}
%package email
Summary: Email storage engine for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description email
%{name} backend implementation which sends email.
%endif

%prep
%setup -q -n aware-%{version}

%build
/usr/bin/phpize && %configure && %{__make} %{?_smp_mflags}

# Clean the buildroot so that it does not contain any stuff from previous builds
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}

# Install the extension
%{__make} install INSTALL_ROOT=%{buildroot}

# Create the ini location
%{__mkdir} -p %{buildroot}/etc/php.d

# Preliminary extension ini
echo "extension=aware.so" > %{buildroot}/%{_sysconfdir}/php.d/aware.ini

%if %{with email}
	pushd storage/email
	/usr/bin/phpize && %configure && %{__make} %{?_smp_mflags}
	%{__make} install INSTALL_ROOT=%{buildroot}
	popd
%endif


%clean
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}

%files
%{_libdir}/php/modules/aware.so
%{_sysconfdir}/php.d/aware.ini

%files devel
%{_includedir}/php/ext/aware/php_aware.h
%{_includedir}/php/ext/aware/php_aware_storage.h

%if %{with email}
%files email
%{_libdir}/php/modules/aware-email.so
%endif



%changelog
* Sat Dec 12 2009 Mikko Koppanen <mkoppanen@php.net>
 - Initial spec file
