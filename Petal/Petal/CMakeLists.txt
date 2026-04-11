
add_library(Petal SHARED
        Source/Engine.cpp
        Source/Engine.h
        Source/pch.h
        Source/Utils/Logging/Logger.cpp
        Source/Utils/Logging/Logger.h
        Source/Utils/Logging/LogLevel.h
        Source/Utils/Logging/LoggerSystem.cpp
        Source/Utils/Logging/LoggerSystem.h
        Source/Utils/Memory/Ref.h
        Source/Utils/Memory/MemorySystem.cpp
        Source/Utils/Memory/MemorySystem.h
)

target_compile_definitions(Petal PRIVATE PETAL_EXPORTS)

target_include_directories(Petal PUBLIC Source/)
target_include_directories(Petal PUBLIC Source/Utils)

target_precompile_headers(Petal PRIVATE Source/pch.h)