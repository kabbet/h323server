#!/bin/bash
set -e
filename=sources.gni

if [ ! -f "$filename" ]; then
  python ../build/generate_gni.py .
fi

if [ -d "out" ]; then
  rm -rf out
fi

echo "Building mtlog... [Debug Version]"
gn gen out/Debug --args='is_debug=true' -v 
ninja -C out/Debug demo:install -v > ../../10-common/version/compileinfo/demo_linux64_gn_d.txt

# echo "Building mtlog... [Release Version]"
# gn gen out/Release --args='is_debug=false'
# ninja -C out/Release demo:install -v

#clean out
#rm -rf out

echo -e "compile done\n"
