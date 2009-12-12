# Define version and release number
%define version @PACKAGE_VERSION@
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

%clean
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}

%files
%{_libdir}/php/modules/aware.so
%{_sysconfdir}/php.d/aware.ini
%{_includedir}/php/ext/aware/php_aware.h
%{_includedir}/php/ext/aware/php_aware_storage.h

%changelog
* Sat Dec 12 2009 Mikko Koppanen <mkoppanen@php.net>
 - Initial spec file
