from conan.packager import ConanMultiPackager
import os

if __name__ == "__main__":
    version = os.getenv("TRAVIS_TAG") or os.getenv("APPVEYOR_REPO_TAG_NAME") or "dev"
    reference = "c-blosc/%s" % version
    upload = os.getenv("CONAN_UPLOAD") if (version != "dev") else False
    builder = ConanMultiPackager(reference=reference, upload=upload)
    builder.add_common_builds(shared_option_name="c-blosc:shared")
    builder.run()
