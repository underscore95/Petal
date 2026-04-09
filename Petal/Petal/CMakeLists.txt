
add_library(Petal SHARED
        Source/Engine.cpp
        Source/Engine.h
        Source/API.h
        Source/pch.h
)

target_compile_definitions(Petal PRIVATE PETAL_EXPORTS)

target_include_directories(Petal PUBLIC Source/)

target_precompile_headers(Petal PRIVATE Source/pch.h)