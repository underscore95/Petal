# STB is setup in a really strange way which makes it difficult to use with vcpkg
# I decided it would be easiest to make a separate project which I copy the STB headers into (which is how the library is intended to be distributed)
# and to handle all the setup process.
# This has the downside that updating STB or moving to a fork is more annoying, which is a tradeoff I'm willing to make.
# For more information, see https://github.com/microsoft/vcpkg/issues/36037

add_library(stb STATIC)

target_sources(stb PRIVATE
        Source/stb_image.cpp
        Source/stb_image.h
)

target_include_directories(stb PUBLIC /Source/)
