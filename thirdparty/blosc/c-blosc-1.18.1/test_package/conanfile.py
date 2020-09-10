from conans import ConanFile, CMake, tools


class CbloscTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin" )
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy("*.so*", dst="bin", src="lib")

    def test(self):
        with tools.chdir("bin"):
            if tools.os_info.is_windows:
                self.run("example")
            else:
                prefix = "DYLD_LIBRARY_PATH=." if tools.os_info.is_macos else ""
                self.run("%s ./example" % prefix)
