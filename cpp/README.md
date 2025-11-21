# Structured Light 3D Scanner - C++ Implementation

구조광 기반 3D 스캐닝 시스템 C++ 구현

## 하드웨어

### 지원 하드웨어
- **카메라**: Basler Pylon (USB/GigE)
- **프로젝터**: TI DLP LightCrafter 4500

### 하드웨어 사양

#### Basler 카메라
- USB 3.0 / GigE 인터페이스
- 모노/컬러 센서 지원
- GenICam 표준 준수
- 고속 이미지 캡처

#### TI DLP LightCrafter 4500
- 해상도: 912 x 1140
- 최대 4kHz 패턴 투영
- 패턴 시퀀스 모드
- USB/네트워크 제어

## 시스템 요구사항

### 운영체제
- Linux (Ubuntu 20.04 이상 권장)
- Windows 10/11 (실험적 지원)

### 빌드 도구
- CMake 3.15 이상
- C++17 지원 컴파일러 (GCC 7+ 또는 Clang 5+)

### 의존성 라이브러리

#### 필수
- **OpenCV 4.0+**: 이미지 처리
- **Basler Pylon SDK**: 카메라 제어
  - 다운로드: https://www.baslerweb.com/en/downloads/software-downloads/

#### 선택
- **TI DLP LightCrafter SDK**: 프로젝터 제어
  - 다운로드: http://www.ti.com/tool/DLPLCR4500EVM
- **Eigen3**: 행렬 연산 최적화
- **PCL (Point Cloud Library)**: 포인트 클라우드 처리

## 설치

### 1. 의존성 설치 (Ubuntu)

```bash
# OpenCV 설치
sudo apt update
sudo apt install libopencv-dev

# Eigen3 설치 (선택)
sudo apt install libeigen3-dev

# PCL 설치 (선택)
sudo apt install libpcl-dev
```

### 2. Basler Pylon SDK 설치

```bash
# Basler 웹사이트에서 pylon SDK 다운로드
# 예: pylon_7.4.0.14900_linux-x86_64_debs.tar.gz

tar -xzf pylon_7.4.0.14900_linux-x86_64_debs.tar.gz
cd pylon_7.4.0.14900_linux-x86_64_debs
sudo dpkg -i *.deb

# 사용자를 pylon 그룹에 추가
sudo usermod -aG pylon $USER
```

### 3. TI LightCrafter SDK 설치 (선택)

```bash
# TI 웹사이트에서 DLPC350 SDK 다운로드
# /opt/dlpc350에 설치 권장

sudo mkdir -p /opt/dlpc350
# SDK 파일들을 /opt/dlpc350에 복사
```

### 4. 프로젝트 빌드

```bash
# 저장소 클론
git clone https://github.com/gomu80/gmsin.git
cd gmsin/cpp

# 빌드 디렉토리 생성
mkdir build
cd build

# CMake 구성
cmake ..

# 빌드
make -j$(nproc)

# 설치 (선택)
sudo make install
```

### 빌드 옵션

```bash
# Release 빌드
cmake -DCMAKE_BUILD_TYPE=Release ..

# Pylon SDK 경로 지정
cmake -DPYLON_ROOT=/opt/pylon ..

# LightCrafter SDK 경로 지정
cmake -DLIGHTCRAFTER_SDK_ROOT=/opt/dlpc350 ..

# 테스트 빌드
cmake -DBUILD_TESTS=ON ..
```

## 사용법

### 1. 하드웨어 테스트

```bash
# 카메라와 프로젝터 연결 테스트
./bin/hardware_test
```

### 2. 패턴 생성

```bash
# Gray Code 패턴 생성
./bin/pattern_generation
```

### 3. 캘리브레이션

```bash
# 시스템 캘리브레이션
./bin/calibration_example
```

### 4. 3D 스캔

```bash
# 기본 스캔 실행
./bin/basic_scan
```

## 프로그래밍 예제

### 카메라 제어

```cpp
#include "structured_light/hardware/basler_camera.h"

using namespace structured_light;

int main() {
    // 카메라 초기화
    BaslerCamera camera(0);
    camera.open();

    // 노출 설정
    camera.setExposureTime(10000); // 10ms

    // 이미지 캡처
    cv::Mat image;
    camera.grabImage(image);

    camera.close();
    return 0;
}
```

### 프로젝터 제어

```cpp
#include "structured_light/hardware/lightcrafter_projector.h"

using namespace structured_light;

int main() {
    // 프로젝터 연결
    LightCrafterProjector projector;
    projector.connect();
    projector.initialize();

    // 이미지 투영
    cv::Mat pattern = cv::imread("pattern.png", cv::IMREAD_GRAYSCALE);
    projector.projectImage(pattern);

    projector.disconnect();
    return 0;
}
```

### 패턴 생성 및 투영

```cpp
#include "structured_light/pattern/gray_code_generator.h"
#include "structured_light/hardware/lightcrafter_projector.h"
#include "structured_light/hardware/basler_camera.h"

using namespace structured_light;

int main() {
    // 패턴 생성
    GrayCodeGenerator generator(912, 1140,
                                GrayCodeGenerator::Direction::BOTH,
                                true);
    auto patterns = generator.generate();

    // 하드웨어 초기화
    LightCrafterProjector projector;
    BaslerCamera camera(0);

    projector.connect();
    projector.initialize();
    camera.open();

    // 패턴 투영 및 캡처
    std::vector<cv::Mat> captured_images;

    for (const auto& pattern : patterns) {
        projector.projectImage(pattern);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        cv::Mat captured;
        camera.grabImage(captured);
        captured_images.push_back(captured);
    }

    // 정리
    camera.close();
    projector.disconnect();

    return 0;
}
```

## 프로젝트 구조

```
cpp/
├── CMakeLists.txt              # CMake 빌드 설정
├── include/                    # 헤더 파일
│   └── structured_light/
│       ├── hardware/          # 하드웨어 제어
│       │   ├── basler_camera.h
│       │   └── lightcrafter_projector.h
│       ├── pattern/           # 패턴 생성
│       │   ├── gray_code_generator.h
│       │   └── phase_shift_generator.h
│       ├── calibration/       # 캘리브레이션
│       ├── decoder/           # 패턴 디코딩
│       └── reconstruction/    # 3D 재구성
├── src/                       # 소스 파일
│   ├── hardware/
│   ├── pattern/
│   ├── calibration/
│   ├── decoder/
│   ├── reconstruction/
│   └── scanner.cpp           # 메인 스캐너
├── examples/                  # 예제
│   ├── hardware_test.cpp
│   ├── pattern_generation.cpp
│   ├── calibration_example.cpp
│   └── basic_scan.cpp
├── tests/                     # 테스트
├── cmake/                     # CMake 모듈
└── data/                      # 데이터 디렉토리
    ├── patterns/
    ├── captured/
    ├── calibration/
    └── results/
```

## 성능

### 스캔 속도
- Gray Code: ~2초 (24 패턴 @ 4kHz)
- Phase Shifting: ~1초 (8 패턴)
- Hybrid: ~3초

### 정밀도
- 서브픽셀 정밀도 (<0.1 픽셀)
- 깊이 정확도: ~0.1mm (거리 의존)

## 문제 해결

### Basler 카메라를 찾을 수 없음

```bash
# Pylon Viewer로 카메라 확인
/opt/pylon/bin/PylonViewerApp

# USB 권한 확인
lsusb | grep Basler
ls -l /dev/bus/usb/

# pylon 그룹 확인
groups $USER
```

### LightCrafter 연결 실패

```bash
# USB 장치 확인
lsusb | grep 0451:6401

# USB 권한 설정
sudo chmod 666 /dev/bus/usb/XXX/YYY
```

### 빌드 에러

```bash
# Pylon 경로 명시
cmake -DPYLON_ROOT=/opt/pylon ..

# OpenCV 버전 확인
pkg-config --modversion opencv4
```

## 기술 지원

- GitHub Issues: https://github.com/gomu80/gmsin/issues
- 문서: [ALGORITHM.md](../docs/ALGORITHM.md)
- 하드웨어 설정: [HARDWARE_SETUP.md](HARDWARE_SETUP.md)

## 라이선스

MIT License

## 참고자료

### 하드웨어 문서
- [Basler Pylon Programmer's Guide](https://www.baslerweb.com/en/downloads/software-downloads/)
- [TI DLP LightCrafter 4500 User Guide](http://www.ti.com/lit/ug/dlpu011a/dlpu011a.pdf)

### 알고리듬
- Zhang, S. (2018). "High-speed 3D shape measurement with structured light methods"
- Salvi, J., et al. (2010). "Pattern codification strategies in structured light systems"
