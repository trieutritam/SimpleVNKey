mkdir build
pushd build
/opt/homebrew/bin/cmake --no-warn-unused-cli \
                -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
                -DCMAKE_BUILD_TYPE:STRING=Debug \
                -G "Unix Makefiles" -S ../
popd