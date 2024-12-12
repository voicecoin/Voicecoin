Name:           voicecoin
Version:        0.6.3
Release:        1%{?dist}
Summary:        Voicecoin Wallet
Group:          Applications/Internet
Vendor:         Voicecoin
License:        GPLv3
URL:            https://www.voicecoin.com
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires:  autoconf automake libtool gcc-c++ openssl-devel >= 1:1.0.2d libdb4-devel libdb4-cxx-devel miniupnpc-devel boost-devel boost-static
Requires:       openssl >= 1:1.0.2d libdb4 libdb4-cxx miniupnpc logrotate

%description
Voicecoin Wallet

%prep
%setup -q

%build
./autogen.sh
./configure
make

%install
%{__rm} -rf $RPM_BUILD_ROOT
%{__mkdir} -p $RPM_BUILD_ROOT%{_bindir} $RPM_BUILD_ROOT/etc/voicecoin $RPM_BUILD_ROOT/etc/ssl/emc $RPM_BUILD_ROOT/var/lib/emc/.voicecoin $RPM_BUILD_ROOT/usr/lib/systemd/system $RPM_BUILD_ROOT/etc/logrotate.d
%{__install} -m 755 src/voicecoind $RPM_BUILD_ROOT%{_bindir}
%{__install} -m 755 src/voicecoin-cli $RPM_BUILD_ROOT%{_bindir}
%{__install} -m 600 contrib/redhat/voicecoin.conf $RPM_BUILD_ROOT/var/lib/emc/.voicecoin
%{__install} -m 644 contrib/redhat/voicecoind.service $RPM_BUILD_ROOT/usr/lib/systemd/system
%{__install} -m 644 contrib/redhat/voicecoind.logrotate $RPM_BUILD_ROOT/etc/logrotate.d/voicecoind
%{__mv} -f contrib/redhat/emc $RPM_BUILD_ROOT%{_bindir}

%clean
%{__rm} -rf $RPM_BUILD_ROOT

%pretrans
getent passwd emc >/dev/null && { [ -f /usr/bin/voicecoind ] || { echo "Looks like user 'emc' already exists and have to be deleted before continue."; exit 1; }; } || useradd -r -M -d /var/lib/emc -s /bin/false emc

%post
[ $1 == 1 ] && {
  sed -i -e "s/\(^rpcpassword=MySuperPassword\)\(.*\)/rpcpassword=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)/" /var/lib/emc/.voicecoin/voicecoin.conf
  openssl req -nodes -x509 -newkey rsa:4096 -keyout /etc/ssl/emc/voicecoin.key -out /etc/ssl/emc/voicecoin.crt -days 3560 -subj /C=US/ST=Oregon/L=Portland/O=IT/CN=voicecoin.emc
  ln -sf /var/lib/emc/.voicecoin/voicecoin.conf /etc/voicecoin/voicecoin.conf
  ln -sf /etc/ssl/emc /etc/voicecoin/certs
  chown emc.emc /etc/ssl/emc/voicecoin.key /etc/ssl/emc/voicecoin.crt
  chmod 600 /etc/ssl/emc/voicecoin.key
} || exit 0

%posttrans
[ -f /var/lib/emc/.voicecoin/addr.dat ] && { cd /var/lib/emc/.voicecoin && rm -rf database addr.dat nameindex* blk* *.log .lock; }
sed -i -e 's|rpcallowip=\*|rpcallowip=0.0.0.0/0|' /var/lib/emc/.voicecoin/voicecoin.conf
systemctl daemon-reload
systemctl status voicecoind >/dev/null && systemctl restart voicecoind || exit 0

%preun
[ $1 == 0 ] && {
  systemctl is-enabled voicecoind >/dev/null && systemctl disable voicecoind >/dev/null || true
  systemctl status voicecoind >/dev/null && systemctl stop voicecoind >/dev/null || true
  pkill -9 -u emc > /dev/null 2>&1
  getent passwd emc >/dev/null && userdel emc >/dev/null 2>&1 || true
  rm -f /etc/ssl/emc/voicecoin.key /etc/ssl/emc/voicecoin.crt /etc/voicecoin/voicecoin.conf /etc/voicecoin/certs
} || exit 0

%files
%doc COPYING
%attr(750,emc,emc) %dir /etc/voicecoin
%attr(750,emc,emc) %dir /etc/ssl/emc
%attr(700,emc,emc) %dir /var/lib/emc
%attr(700,emc,emc) %dir /var/lib/emc/.voicecoin
%attr(600,emc,emc) %config(noreplace) /var/lib/emc/.voicecoin/voicecoin.conf
%attr(4750,emc,emc) %{_bindir}/voicecoin-cli
%defattr(-,root,root)
%config(noreplace) /etc/logrotate.d/voicecoind
%{_bindir}/voicecoind
%{_bindir}/emc
/usr/lib/systemd/system/voicecoind.service

%changelog
* Thu Aug 31 2017 Aspanta Limited <info@aspanta.com> 0.6.3
- There is no changelog available. Please refer to the CHANGELOG file or visit the website.
