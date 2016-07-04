# build file to generate the distribution binary tarball

make clean
./configure --prefix=/tmp/kcts-build
make install-strip
tar czf kcts-$1.bin.tgz -C /tmp/kcts-build/bin kcts

make clean

./configure \
  $PKGCFG \
  $CROSSCFG \
  --with-ptw32=$PREFIX/i686-w64-mingw32.static \
  --enable-static \
  PTW32_LIBS="-lpthread -lpcreposix -lpcre -lregex" \
  XMLRPC_C_CONFIG=$PREFIX/bin/xmlrpc-c-config \
  FLTK_CONFIG=$PREFIX/bin/i686-w64-mingw32.static-fltk-config

make

$PREFIX/bin/i686-pc-mingw32-strip src/kcts.exe
make nsisinst
mv src/*setup*exe .

make clean

# build the distribution tarball
./configure
make distcheck
make clean
