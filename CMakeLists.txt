cmake_minimum_required(VERSION 3.20)

project(Sigma LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(DEFINED ENV{DXSDK_DIR})
    file(TO_CMAKE_PATH "$ENV{DXSDK_DIR}" DIRECTX_SDK_ROOT)
else()
    set(DIRECTX_SDK_ROOT "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
endif()

set(DIRECTX_SDK_INCLUDE_DIR "${DIRECTX_SDK_ROOT}/Include")
set(DIRECTX_SDK_LIB_DIR "${DIRECTX_SDK_ROOT}/Lib/x86")

if(NOT EXISTS "${DIRECTX_SDK_INCLUDE_DIR}/d3d9.h")
    message(FATAL_ERROR "DirectX SDK include path was not found: ${DIRECTX_SDK_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${DIRECTX_SDK_LIB_DIR}/d3d9.lib")
    message(FATAL_ERROR "DirectX SDK x86 library path was not found: ${DIRECTX_SDK_LIB_DIR}")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Sigma links the x86 DirectX SDK libraries. Configure CMake for Win32/x86.")
endif()

find_package(assimp CONFIG QUIET)
if(NOT assimp_FOUND)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    if(POLICY CMP0175)
        set(CMAKE_POLICY_DEFAULT_CMP0175 OLD)
    endif()

    FetchContent_Declare(
        assimp
        URL https://github.com/assimp/assimp/archive/refs/tags/v5.4.3.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(assimp)
endif()

if(TARGET assimp::assimp)
    set(SIGMA_ASSIMP_TARGET assimp::assimp)
elseif(TARGET assimp)
    set(SIGMA_ASSIMP_TARGET assimp)
else()
    message(FATAL_ERROR "Assimp target was not found after package discovery.")
endif()

FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
if(POLICY CMP0169)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0169 OLD)
endif()
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()
if(POLICY CMP0169)
    cmake_policy(POP)
endif()

FetchContent_Declare(
    tinyxml2
    URL https://github.com/leethomason/tinyxml2/archive/refs/tags/10.0.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(tinyxml2)

if(TARGET tinyxml2::tinyxml2)
    set(SIGMA_TINYXML2_TARGET tinyxml2::tinyxml2)
elseif(TARGET tinyxml2)
    set(SIGMA_TINYXML2_TARGET tinyxml2)
else()
    message(FATAL_ERROR "tinyxml2 target was not found after FetchContent.")
endif()

set(SIGMA_IMGUI_SOURCES
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp"
)

file(GLOB_RECURSE SIGMA_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(Sigma WIN32 ${SIGMA_SOURCES} ${SIGMA_IMGUI_SOURCES})

target_include_directories(Sigma PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${DIRECTX_SDK_INCLUDE_DIR}"
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
)

target_compile_definitions(Sigma PRIVATE
    WIN32
    _WINDOWS
    NOMINMAX
    IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
)

if(MSVC)
    target_compile_options(Sigma PRIVATE /W4)
endif()

target_link_directories(Sigma PRIVATE "${DIRECTX_SDK_LIB_DIR}")

target_link_libraries(Sigma PRIVATE
    ${SIGMA_ASSIMP_TARGET}
    ${SIGMA_TINYXML2_TARGET}
    d3d9
    d3dx9
    dxguid
    user32
    gdi32
    winmm
)

set(SIGMA_OUTPUT_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Directory for Sigma.exe and runtime assets.")

set_target_properties(Sigma PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SIGMA_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SIGMA_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SIGMA_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SIGMA_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${SIGMA_OUTPUT_DIR}"
)

add_custom_command(TARGET Sigma POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:Sigma>/assets"
    COMMENT "Copying runtime assets"
)
