git clone --depth 1 --branch $1 https://github.com/doxygen/doxygen.git
cmake -B doxygen/.build -G Ninja                                 \
  -DCMAKE_CXX_COMPILER=${EIGEN_CI_CXX_COMPILER}                  \
  -DCMAKE_C_COMPILER=${EIGEN_CI_C_COMPILER}                      \
  doxygen
cmake --build doxygen/.build -t install
