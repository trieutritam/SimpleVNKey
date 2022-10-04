cmake --build build --config Debug --target all -j 10 --
cmake --build build || exit 1

pushd build
ctest
popd