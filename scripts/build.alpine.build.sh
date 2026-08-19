#!/bin/bash
set -xe

export PKG_CONFIG_PATH=/usr/lib64/pkgconfig
cmake -DCMAKE_BUILD_TYPE=Release .
make -j$(nproc)
rm subconverter
# shellcheck disable=SC2046
g++ -o base/subconverter $(find CMakeFiles/subconverter.dir/src/ -name "*.o")  -static -lpcre2-8 -lyaml-cpp -L/usr/lib64 -lcurl -lmbedtls -lmbedcrypto -lmbedx509 -lz -l:quickjs/libquickjs.a -llibcron -O3 -s

python3 -m ensurepip
python3 -m pip install gitpython
python3 scripts/update_rules.py -c scripts/rules_config.conf

cd base
chmod +rx subconverter
chmod +r ./*
cd ..
mv base subconverter
