cmake_minimum_required(VERSION 4.0.0)
project(tasks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_COMPILER_IMPORT_STD ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILE, "clang++")
set(CMAKE_CXX_FLAGS, "-stdlib=libc++ -Wreserved-module-identifier")

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19)
    message(FATAL_ERROR "Clang compiler version should be over 19")
endif ()
string(REGEX REPLACE "^([0-9]+).*" "\\1" CLANG_VERSION ${CMAKE_CXX_COMPILER_VERSION})

set(STD_CPPM_FOLDER "/usr/lib/llvm-${CLANG_VERSION}/share/libc++/v1/")
set(STD_CPPM_SOURCE "${STD_CPPM_FOLDER}/std.cppm")
set(STD_MODULE_PATH "${CMAKE_BINARY_DIR}/std.pcm")

if (NOT EXISTS ${STD_MODULE_PATH})
    if (NOT EXISTS ${STD_CPPM_SOURCE})
        message(FATAL_ERROR "std.cppm not found at: ${STD_CPPM_SOURCE}")
    endif ()

    message("Precompiling standard library module std.pcm...")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -std=c++23 -stdlib=libc++ -Wno-reserved-module-identifier --precompile -o ${STD_MODULE_PATH} ${STD_CPPM_SOURCE} RESULT_VARIABLE result)
    if (result)
        message(FATAL_ERROR "Failed to generate std.pcm: ${result}")
    endif ()
    message("Successfully generated: ${STD_MODULE_PATH}")
endif ()

add_compile_options(-stdlib=libc++ -fmodule-file=std=${STD_MODULE_PATH})
add_library(unused_std_target STATIC)
target_sources(unused_std_target PRIVATE FILE_SET CXX_MODULES BASE_DIRS ${STD_CPPM_FOLDER} FILES ${STD_CPPM_SOURCE})

file(GLOB LIB_MODULE_INTERFACES "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cppm")
file(GLOB LIB_MODULE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp")
add_library(Lib)
target_sources(Lib
        PUBLIC FILE_SET CXX_MODULES TYPE CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/lib" FILES ${LIB_MODULE_INTERFACES}
        PRIVATE ${LIB_MODULE_SOURCES}
)

foreach (PART RANGE 1 10)
    foreach (TASK_NUMBER RANGE 1 100)
        set(SOURCE_FILE "${PART}/${TASK_NUMBER}.cpp")
        if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
            set(TARGET_NAME "${PART}_${TASK_NUMBER}")
            add_executable(${TARGET_NAME} ${SOURCE_FILE})
            target_link_libraries(${TARGET_NAME} PRIVATE Lib)
        endif ()
    endforeach ()
endforeach ()