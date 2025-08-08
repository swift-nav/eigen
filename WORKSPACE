workspace(name = "eigen")

http_archive(
    name = "mkl_headers",
    build_file = "@rules_swiftnav//third_party:mkl_headers.BUILD",
    sha256 = "b24d12a8e18ba23de5c659a33fb184a7ac6019d4b159e78f628d7c8de225f77a",
    urls = [
        "https://swiftnav-public-mkl-sharedlibs.s3.us-west-2.amazonaws.com/mkl-include-2023.1.0-intel_46342.tar.bz2",
    ],
)

http_archive(
    name = "mkl",
    build_file = "@rules_swiftnav//third_party:mkl.BUILD",
    sha256 = "c63adbfdbdc7c4992384a2d89cd62211e4a9f8061e3e841af1a269699531cb02",
    strip_prefix = "lib",
    urls = [
        "https://swiftnav-public-mkl-sharedlibs.s3.us-west-2.amazonaws.com/mkl-static-2023.1.0-intel_46342.tar.bz2",
    ],
)
