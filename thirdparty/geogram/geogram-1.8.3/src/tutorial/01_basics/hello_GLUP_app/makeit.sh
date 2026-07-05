# Instructions to compile this program
# are explained in tutorials/README.txt

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/:$GEOGRAM_INSTALL_ROOT/lib/pkgconfig
g++ main.cpp `pkg-config --cflags --libs geogram_gfx` -o hello_GLUP_app
