%global __requires_exclude ^libjwt\\.so.*$

Name:       libjwt-lhdist
Version:    1
Release:    1
Summary:    custom libjwt linked against openssl 1.0.2 wrapped in rpm
License:    See https://github.com/benmcollins/libjwt

%description
custom libjwt linked against openssl 1.0.2 wrapped in rpm

%prep
git clone https://github.com/benmcollins/libjwt.git

%build
cd libjwt
autoreconf -i 
# TODO - remove PKG_CONFIG_PATH once libcheck is installed through package manager
./configure PKG_CONFIG_PATH=/usr/lib/pkgconfig --prefix=/usr 
make all
make check

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr
cd libjwt
make install prefix=%{buildroot}/usr
cd %{buildroot}
find ./ -type f -or -type l | grep usr | cut -d '.' -f 2- > %{buildroot}/../libjwt_install_files.txt
find ./ -type f -or -type l | grep usr | xargs -I{} chmod 777 {}

%files -f %{buildroot}/../libjwt_install_files.txt

%changelog