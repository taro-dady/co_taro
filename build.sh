
#!/bin/bash
rm -rf build
mkdir build && cd build && cmake -D YAML_BUILD_SHARED_LIBS=ON YAML_CPP_BUILD_TESTS=OFF YAML_CPP_BUILD_TOOLS=OFF .. && make -j 4
