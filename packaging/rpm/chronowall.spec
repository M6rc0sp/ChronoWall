Name:           chronowall
Version:        1.0.0
Release:        1%{?dist}
Summary:        Gerenciador de papel de parede baseado em tempo

License:        MIT
URL:            https://github.com/m6rc0sp/chronowall
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-linguist
BuildRequires:  systemd-devel

Requires:       qt5-qtbase
Requires:       systemd

%description
ChronoWall is a wallpaper scheduler that changes your desktop background based on time.

%prep
%autosetup

%build
mkdir build && cd build
%cmake ..
%make_build

%install
cd build
%make_install

%files
%{_bindir}/chronowall
%{_datadir}/applications/chronowall.desktop
%{_datadir}/icons/hicolor/scalable/apps/chronowall.svg
%{_datadir}/%{name}/translations/*.qm
%{_userunitdir}/chronowall.service

%changelog
* Wed Dec 15 2025 Marcos Barbosa <paulo.marcos0108@live.com> - 1.0.0-1
- Versão inicial do ChronoWall
