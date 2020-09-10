import os
from conans import ConanFile, CMake, tools


class CbloscConan(ConanFile):
    name = "c-blosc"
    description = "An extremely fast, multi-threaded, meta-compressor library"
    license = "BSD"
    url = "https://github.com/Blosc/c-blosc"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    exports_sources = "*", "!test_package/*", "!appveyor*", "!.*.yml", "!bench/plot-speeds.py", "!build.py", "!.*"

    @property
    def run_tests(self):
        return "CONAN_RUN_TESTS" in os.environ

    def build(self):
        os.mkdir("build")
        tools.replace_in_file("CMakeLists.txt", "project(blosc)", '''project(blosc)
            include(${CMAKE_BINARY_DIR}/../conanbuildinfo.cmake)
            conan_basic_setup(NO_OUTPUT_DIRS)''')
        cmake = CMake(self)
        cmake.definitions["BUILD_TESTS"] = "ON" if self.run_tests else "OFF"
        cmake.definitions["BUILD_BENCHMARKS"] = "ON" if self.run_tests else "OFF"
        cmake.definitions["BUILD_SHARED"] = "ON" if (self.options.shared or self.run_tests) else "OFF"
        cmake.definitions["BUILD_STATIC"] = "OFF" if self.options.shared else "ON"
        cmake.configure(build_folder="build")
        cmake.build()

        if self.run_tests:
            self.output.warn("Running tests!!")
            self.launch_tests()

    def launch_tests(self):
        """Conan will remove rpaths from shared libs to be able to reuse the shared libs, we need
        to tell the tests where to find the shared libs"""
        test_args = "-VV" if tools.os_info.is_windows else ""
        with tools.chdir("build"):
            outdir = os.path.join(self.build_folder, "build", "blosc")
            if tools.os_info.is_macos:
                prefix = "DYLD_LIBRARY_PATH=%s" % outdir
            elif tools.os_info.is_windows:
                prefix = "PATH=%s;%%PATH%%" % outdir
            elif tools.os_info.is_linux:
                prefix = "LD_LIBRARY_PATH=%s" % outdir
            else:
                return
            with tools.environment_append({'CTEST_OUTPUT_ON_FAILURE': '1'}):
                self.run("%s ctest %s" % (prefix, test_args))

    def package(self):
        self.copy("blosc.h", dst="include", src="blosc")
        self.copy("blosc-export.h", dst="include", src="blosc")
        self.copy("*libblosc.a", dst="lib", keep_path=False)

        if self.options.shared:
            self.copy("*/blosc.lib", dst="lib", keep_path=False)
            self.copy("*blosc.dll", dst="bin", keep_path=False)
            self.copy("*blosc.*dylib*", dst="lib", keep_path=False, symlinks=True)
            self.copy("*blosc.so*", dst="lib", keep_path=False, symlinks=True)
            self.copy("*libblosc.dll.a", dst="lib", keep_path=False) # Mingw
        else:
            self.copy("*libblosc.lib", dst="lib", src="", keep_path=False)

    def package_info(self):
        if self.settings.compiler == "Visual Studio" and not self.options.shared:
            self.cpp_info.libs = ["libblosc"]
        else:
            self.cpp_info.libs = ["blosc"]
        if self.settings.os == "Linux":
            self.cpp_info.libs.append("pthread")
