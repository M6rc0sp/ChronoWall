Name:           chronowall
Version:        1.0.0
Release:        1%{?dist}
Summary:        Gerenciador de papel de parede automático
License:        MIT
URL:            https://github.com/M6rc0sp/chronowall
BuildRequires:  cmake qt6-qtbase-devel
Requires:       qt6-qtbase

%description
Aplicação para mudar automaticamente o papel de parede baseado em períodos do dia.

%build
%cmake
%cmake_build

%install
%cmake_install

%files
%{_bindir}/chronowall
%{_userunitdir}/chronowall.service
