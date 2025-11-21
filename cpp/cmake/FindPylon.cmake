# FindPylon.cmake
# Basler Pylon SDK를 찾기 위한 CMake 모듈

# Pylon SDK 루트 찾기
if(NOT PYLON_ROOT)
    if(UNIX)
        set(PYLON_ROOT "/opt/pylon" CACHE PATH "Pylon SDK root directory")
    elseif(WIN32)
        set(PYLON_ROOT "C:/Program Files/Basler/pylon 7" CACHE PATH "Pylon SDK root directory")
    endif()
endif()

# 헤더 파일 찾기
find_path(Pylon_INCLUDE_DIR
    NAMES pylon/PylonIncludes.h
    PATHS ${PYLON_ROOT}/include
    DOC "Pylon include directory"
)

# 라이브러리 디렉토리
if(UNIX)
    set(Pylon_LIBRARY_DIR ${PYLON_ROOT}/lib64)
elseif(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(Pylon_LIBRARY_DIR ${PYLON_ROOT}/lib/x64)
    else()
        set(Pylon_LIBRARY_DIR ${PYLON_ROOT}/lib/Win32)
    endif()
endif()

# 라이브러리 찾기
find_library(Pylon_BASE_LIBRARY
    NAMES pylonbase
    PATHS ${Pylon_LIBRARY_DIR}
    DOC "Pylon base library"
)

find_library(Pylon_UTILITY_LIBRARY
    NAMES pylonutility
    PATHS ${Pylon_LIBRARY_DIR}
    DOC "Pylon utility library"
)

find_library(Pylon_GENAPI_LIBRARY
    NAMES GenApi_gcc_v3_1_Basler_pylon
    PATHS ${Pylon_LIBRARY_DIR}
    DOC "GenApi library"
)

find_library(Pylon_GCBASE_LIBRARY
    NAMES GCBase_gcc_v3_1_Basler_pylon
    PATHS ${Pylon_LIBRARY_DIR}
    DOC "GCBase library"
)

# 결과 처리
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Pylon
    REQUIRED_VARS
        Pylon_INCLUDE_DIR
        Pylon_BASE_LIBRARY
        Pylon_UTILITY_LIBRARY
        Pylon_GENAPI_LIBRARY
        Pylon_GCBASE_LIBRARY
    VERSION_VAR Pylon_VERSION
)

if(Pylon_FOUND)
    set(Pylon_INCLUDE_DIRS ${Pylon_INCLUDE_DIR})
    set(Pylon_LIBRARIES
        ${Pylon_BASE_LIBRARY}
        ${Pylon_UTILITY_LIBRARY}
        ${Pylon_GENAPI_LIBRARY}
        ${Pylon_GCBASE_LIBRARY}
    )
    set(Pylon_LIBRARY_DIRS ${Pylon_LIBRARY_DIR})

    mark_as_advanced(
        Pylon_INCLUDE_DIR
        Pylon_BASE_LIBRARY
        Pylon_UTILITY_LIBRARY
        Pylon_GENAPI_LIBRARY
        Pylon_GCBASE_LIBRARY
    )
endif()
