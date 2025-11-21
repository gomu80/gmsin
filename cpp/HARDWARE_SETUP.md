# 하드웨어 설정 가이드

## 시스템 구성

### 필요한 장비
1. **컴퓨터**: Ubuntu 20.04+, USB 3.0 포트
2. **Basler 카메라**: acA1920-40um (또는 호환 모델)
3. **TI DLP LightCrafter 4500**
4. **삼각대**: 카메라와 프로젝터 고정용
5. **캘리브레이션 타겟**: 체스보드 (9x6, 25mm)

## Basler 카메라 설정

### 1. 하드웨어 연결
- USB 3.0 케이블로 컴퓨터에 연결
- 카메라에 전원 공급 (USB 3.0 제공 또는 별도 어댑터)

### 2. 소프트웨어 설정

#### Pylon SDK 설치 확인
```bash
# Pylon 버전 확인
/opt/pylon/bin/pylon-config --version

# 사용 가능한 카메라 확인
/opt/pylon/bin/PylonViewerApp
```

#### USB 권한 설정
```bash
# udev 규칙 추가
sudo nano /etc/udev/rules.d/69-basler.rules

# 다음 내용 추가:
SUBSYSTEM=="usb", ATTRS{idVendor}=="2676", GROUP="pylon", MODE="0666"

# udev 재시작
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### 3. 카메라 파라미터 최적화

```cpp
// 구조광 스캐닝에 최적화된 설정

// 1. 노출 시간
camera.setExposureTime(5000);  // 5ms (밝은 프로젝터 패턴용)

// 2. 게인 최소화 (노이즈 감소)
camera.setGain(0);

// 3. 프레임 레이트
camera.setFrameRate(10);  // 안정적인 캡처

// 4. 트리거 모드 (프로젝터 동기화)
camera.setTriggerMode(true);

// 5. 픽셀 포맷
// Mono8 또는 BayerRG8 설정
```

### 4. 렌즈 설정
- 초점: 스캔 대상 거리에 맞춰 수동 조정
- 조리개: F/5.6-F/8 (DOF와 밝기 균형)
- 왜곡 최소화를 위해 광각 렌즈 피하기

## TI DLP LightCrafter 4500 설정

### 1. 하드웨어 연결

#### USB 연결
- Mini-USB 케이블로 컴퓨터에 연결
- 전원 어댑터 연결 (12V, 4A)

#### HDMI 연결 (비디오 모드용)
- HDMI 케이블로 컴퓨터 GPU에 연결
- 듀얼 모니터 설정 필요

### 2. 소프트웨어 설정

#### GUI 도구 (Windows 전용)
```
TI에서 제공하는 DLPC350 GUI 도구 사용
- 펌웨어 업데이트
- LED 전류 설정
- 패턴 시퀀스 테스트
```

#### Linux USB 설정
```bash
# USB 장치 확인
lsusb | grep "0451:6401"

# 권한 설정
sudo nano /etc/udev/rules.d/70-lightcrafter.rules

# 다음 내용 추가:
SUBSYSTEM=="usb", ATTRS{idVendor}=="0451", ATTRS{idProduct}=="6401", MODE="0666"

# udev 재시작
sudo udevadm control --reload-rules
```

### 3. LED 설정

#### 밝기 최적화
```cpp
// 최대 밝기 (255)는 LED 수명 단축
// 권장: 180-200

projector.setLEDCurrent(200, 200, 200);  // R, G, B

// 모노 패턴용: 모든 LED 활성화
projector.setLEDEnable(true, true, true);
```

#### LED 색상 선택
- **흰색** (R+G+B): 최대 밝기, 모노 패턴
- **레드**: 긴 파장, 깊이 침투
- **블루**: 짧은 파장, 고해상도

### 4. 패턴 시퀀스 모드

#### 패턴 업로드
```cpp
PatternSequence seq;
seq.bit_depth = 1;           // 1-bit 이진 패턴
seq.pattern_count = 24;      // Gray Code
seq.exposure_us = 200;       // 200us 노출
seq.frame_period_us = 250;   // 4kHz
seq.trigger_out = true;      // 카메라 트리거

projector.uploadPatternSequence(seq);
```

#### 타이밍 설정
```
최소 프레임 주기 = 노출 시간 + 150us (오버헤드)
최대 속도: 4kHz (250us 주기)

권장 설정:
- 노출: 200us
- 주기: 500us (2kHz)
- 안정적인 동작
```

## 카메라-프로젝터 동기화

### 1. 하드웨어 트리거

#### 연결
```
LightCrafter TRIG OUT -> Basler TRIG IN

핀 배치:
- LightCrafter: GPIO 핀 (확인 필요)
- Basler: I/O 커넥터
```

#### 카메라 트리거 설정
```cpp
camera.setTriggerMode(true);

// 트리거 소스: 외부 (Line1)
// 트리거 활성화: Rising edge
// 트리거 지연: 0
```

#### 프로젝터 트리거 출력
```cpp
projector.setTriggerOutput(true, true);  // Enable, Positive

// 각 패턴 시작 시 트리거 펄스 출력
```

### 2. 소프트웨어 동기화

```cpp
// 간단한 지연 기반 동기화
void synchronizedCapture(LightCrafterProjector& proj,
                        BaslerCamera& cam,
                        const cv::Mat& pattern) {
    // 패턴 투영
    proj.projectImage(pattern);

    // 안정화 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 캡처
    cv::Mat image;
    cam.grabImage(image);
}
```

## 시스템 배치

### 1. 기하학적 배치

```
        카메라
          |
          |  (베이스라인: 200-500mm)
          |
        대상물
          |
          |
       프로젝터
```

#### 권장 사항
- 베이스라인: 200-500mm (측정 범위 고려)
- 각도: 15-30도 (오클루전 최소화)
- 거리: 500-1500mm (렌즈와 프로젝터 초점 거리)

### 2. 조명 환경
- **앰비언트 라이트 최소화**
  - 암실 또는 차광 커튼
  - 형광등 끄기
  - 자연광 차단

- **반사 제거**
  - 광택 표면 피하기
  - 편광 필터 사용 (선택)

### 3. 진동 제어
- 안정적인 삼각대 사용
- 바닥 진동 최소화
- 에어컨/팬 바람 차단

## 캘리브레이션 절차

### 1. 체스보드 준비
- 크기: 9x6 내부 코너
- 정사각형: 25mm
- 평평한 판에 인쇄 부착

### 2. 카메라 캘리브레이션

```cpp
CameraCalibration calib(cv::Size(9, 6), 25.0);

// 15-20장의 이미지 캡처
// 다양한 각도, 거리, 방향
vector<cv::Mat> images;
for (int i = 0; i < 20; ++i) {
    // 체스보드 위치 변경
    cv::Mat img;
    camera.grabImage(img);
    images.push_back(img);
}

calib.calibrate(images);
calib.save("camera_calibration.json");
```

### 3. 프로젝터-카메라 캘리브레이션

```cpp
// Gray Code 패턴 투영 및 캡처
// 체스보드와 함께 캡처
// 대응점 추출

StereoCalibration stereo;
stereo.calibrate(object_points, camera_points, projector_points);
stereo.save("stereo_calibration.json");
```

## 문제 해결

### 카메라 관련

#### 이미지가 너무 어두움
- LED 전류 증가
- 노출 시간 증가
- 카메라 게인 조정 (노이즈 주의)

#### 이미지가 흐림
- 초점 재조정
- 진동 확인
- 노출 시간 감소 (모션 블러)

### 프로젝터 관련

#### 패턴이 표시되지 않음
- HDMI 연결 확인
- 디스플레이 모드 확인
- LED 활성화 확인

#### 패턴이 왜곡됨
- 키스톤 보정 (프로젝터 수직 배치)
- 초점 조정
- 투영 거리 확인

### 동기화 문제

#### 패턴과 캡처 불일치
- 지연 시간 조정
- 트리거 연결 확인
- 프레임 레이트 감소

## 성능 최적화

### 1. 속도 향상
- 패턴 수 최소화 (Hybrid 방식)
- 비트 깊이 감소
- 트리거 모드 사용

### 2. 정확도 향상
- 역상 패턴 사용
- Phase Shifting 추가
- 다중 주파수 언래핑

### 3. 안정성 향상
- 신뢰도 임계값 조정
- 아웃라이어 제거
- 다중 뷰 융합

## 유지보수

### 정기 점검
- 렌즈 청소 (먼지, 지문)
- 체스보드 상태 확인
- 케이블 연결 확인

### 재캘리브레이션
- 월 1회 또는 시스템 이동 후
- 온도 변화 큰 경우
- 정확도 저하 감지 시
