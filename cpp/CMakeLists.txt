cmake_minimum_required(VERSION 3.15)
project(StructuredLightScanner VERSION 1.0.0 LANGUAGES CXX)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 컴파일러 플래그
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 출력 디렉토리
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# CMake 모듈 경로
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# === 의존성 찾기 ===

# OpenCV (필수)
find_package(OpenCV 4.0 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Basler Pylon SDK (필수)
find_package(Pylon QUIET)
if(NOT Pylon_FOUND)
    message(STATUS "Pylon SDK not found. Looking in default locations...")
    set(PYLON_ROOT "/opt/pylon" CACHE PATH "Pylon SDK root directory")

    if(EXISTS ${PYLON_ROOT})
        set(Pylon_INCLUDE_DIRS ${PYLON_ROOT}/include)
        set(Pylon_LIBRARY_DIRS ${PYLON_ROOT}/lib64)
        set(Pylon_LIBRARIES
            ${Pylon_LIBRARY_DIRS}/libpylonbase.so
            ${Pylon_LIBRARY_DIRS}/libpylonutility.so
            ${Pylon_LIBRARY_DIRS}/libGenApi_gcc_v3_1_Basler_pylon.so
            ${Pylon_LIBRARY_DIRS}/libGCBase_gcc_v3_1_Basler_pylon.so
        )
        include_directories(${Pylon_INCLUDE_DIRS})
        link_directories(${Pylon_LIBRARY_DIRS})
        message(STATUS "Pylon SDK found at: ${PYLON_ROOT}")
    else()
        message(WARNING "Pylon SDK not found. Please install Basler Pylon SDK.")
        message(WARNING "Download from: https://www.baslerweb.com/en/products/software/basler-pylon-camera-software-suite/")
    endif()
endif()

# TI DLP LightCrafter SDK (선택)
set(LIGHTCRAFTER_SDK_ROOT "/opt/dlpc350" CACHE PATH "LightCrafter SDK root")
if(EXISTS ${LIGHTCRAFTER_SDK_ROOT})
    set(LIGHTCRAFTER_FOUND TRUE)
    include_directories(${LIGHTCRAFTER_SDK_ROOT}/include)
    link_directories(${LIGHTCRAFTER_SDK_ROOT}/lib)
    message(STATUS "LightCrafter SDK found at: ${LIGHTCRAFTER_SDK_ROOT}")
else()
    message(STATUS "LightCrafter SDK not found. Using simulation mode.")
    set(LIGHTCRAFTER_FOUND FALSE)
endif()

# Eigen3 (선택, 행렬 연산)
find_package(Eigen3 3.3 QUIET)
if(Eigen3_FOUND)
    include_directories(${EIGEN3_INCLUDE_DIR})
    message(STATUS "Eigen3 found")
endif()

# PCL (Point Cloud Library, 선택)
find_package(PCL 1.8 QUIET)
if(PCL_FOUND)
    include_directories(${PCL_INCLUDE_DIRS})
    link_directories(${PCL_LIBRARY_DIRS})
    add_definitions(${PCL_DEFINITIONS})
    message(STATUS "PCL found")
endif()

# === 소스 파일 ===

set(HARDWARE_SOURCES
    src/hardware/basler_camera.cpp
    src/hardware/lightcrafter_projector.cpp
)

set(PATTERN_SOURCES
    src/pattern/gray_code_generator.cpp
    src/pattern/phase_shift_generator.cpp
    src/pattern/de_bruijn_generator.cpp
)

# set(CALIBRATION_SOURCES
#     src/calibration/camera_calibration.cpp
#     src/calibration/projector_calibration.cpp
#     src/calibration/stereo_calibration.cpp
# )

# set(DECODER_SOURCES
#     src/decoder/gray_code_decoder.cpp
#     src/decoder/phase_decoder.cpp
# )

# set(RECONSTRUCTION_SOURCES
#     src/reconstruction/triangulation.cpp
#     src/reconstruction/point_cloud.cpp
# )

# set(CORE_SOURCES
#     src/scanner.cpp
# )

# === 라이브러리 빌드 ===

add_library(structured_light_core SHARED
    ${HARDWARE_SOURCES}
    ${PATTERN_SOURCES}
    # ${CALIBRATION_SOURCES}
    # ${DECODER_SOURCES}
    # ${RECONSTRUCTION_SOURCES}
    # ${CORE_SOURCES}
)

target_include_directories(structured_light_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(structured_light_core PUBLIC
    ${OpenCV_LIBS}
)

# Pylon 링크
if(Pylon_FOUND OR EXISTS ${PYLON_ROOT})
    target_link_libraries(structured_light_core PUBLIC ${Pylon_LIBRARIES})
endif()

# LightCrafter 링크
if(LIGHTCRAFTER_FOUND)
    target_link_libraries(structured_light_core PUBLIC dlpc350)
    target_compile_definitions(structured_light_core PUBLIC HAVE_LIGHTCRAFTER)
endif()

# PCL 링크
if(PCL_FOUND)
    target_link_libraries(structured_light_core PUBLIC ${PCL_LIBRARIES})
    target_compile_definitions(structured_light_core PUBLIC HAVE_PCL)
endif()

# === 예제 실행 파일 ===

# 기본 스캔 예제 (나중에 구현)
# add_executable(basic_scan examples/basic_scan.cpp)
# target_link_libraries(basic_scan structured_light_core)

# 캘리브레이션 예제 (나중에 구현)
# add_executable(calibration_example examples/calibration_example.cpp)
# target_link_libraries(calibration_example structured_light_core)

# 패턴 생성 예제
add_executable(pattern_generation examples/pattern_generation.cpp)
target_link_libraries(pattern_generation structured_light_core)

# 하드웨어 테스트
add_executable(hardware_test examples/hardware_test.cpp)
target_link_libraries(hardware_test structured_light_core)

# === 설치 ===

install(TARGETS structured_light_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(TARGETS pattern_generation hardware_test
    RUNTIME DESTINATION bin
)

# === 테스트 (선택) ===

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# === 정보 출력 ===

message(STATUS "========================================")
message(STATUS "Structured Light Scanner Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "Pylon SDK: ${Pylon_FOUND}")
message(STATUS "LightCrafter SDK: ${LIGHTCRAFTER_FOUND}")
message(STATUS "Eigen3: ${Eigen3_FOUND}")
message(STATUS "PCL: ${PCL_FOUND}")
message(STATUS "========================================")
