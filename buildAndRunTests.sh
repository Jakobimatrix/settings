if [ -d testbuild ]; then
  rm -rf testbuild/
fi

mkdir testbuild && cd testbuild || exit 1

cmake -DTEST_SETTINGS=True -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG=-g ..
#cmake -DTEST_SETTINGS=True ..
cmake --build .
ctest --output-on-failure


