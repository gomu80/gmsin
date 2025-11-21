# Structured Light Camera Algorithm

구조광 3D 스캐닝 시스템을 위한 포괄적인 알고리듬 구현

## 개요

이 프로젝트는 구조광(Structured Light) 기반 3D 스캐닝 시스템을 구현합니다. 프로젝터로 패턴을 투영하고 카메라로 캡처하여 3D 형상을 재구성하는 완전한 파이프라인을 제공합니다.

## 주요 기능

### 1. 패턴 생성 (Pattern Generation)
- **Gray Code 패턴**: 이진 코딩 방식
- **Phase Shifting 패턴**: 고정밀 측정
- **De Bruijn 패턴**: 단일 샷 스캐닝
- 사용자 정의 패턴 생성 지원

### 2. 캘리브레이션 (Calibration)
- 카메라 내부 파라미터 캘리브레이션
- 프로젝터 내부 파라미터 캘리브레이션
- 카메라-프로젝터 스테레오 캘리브레이션
- 체스보드 기반 자동 캘리브레이션

### 3. 이미지 캡처 (Image Capture)
- 실시간 카메라 제어
- 자동/수동 노출 제어
- 배치 이미지 캡처
- 프로젝터 동기화 지원

### 4. 패턴 디코딩 (Pattern Decoding)
- Gray Code 디코딩
- Phase Unwrapping
- 서브픽셀 정밀도
- 노이즈 필터링

### 5. 3D 재구성 (3D Reconstruction)
- 삼각측량 기반 깊이 계산
- 포인트 클라우드 생성
- 메쉬 생성 지원
- 다양한 출력 형식 (PLY, OBJ, XYZ)

## 시스템 아키텍처

```
structured_light/
├── pattern/          # 패턴 생성
│   ├── gray_code.py
│   ├── phase_shift.py
│   └── de_bruijn.py
├── calibration/      # 캘리브레이션
│   ├── camera.py
│   ├── projector.py
│   └── stereo.py
├── capture/          # 이미지 캡처
│   ├── camera_control.py
│   └── synchronization.py
├── decode/           # 패턴 디코딩
│   ├── gray_decoder.py
│   ├── phase_decoder.py
│   └── unwrapping.py
└── reconstruction/   # 3D 재구성
    ├── triangulation.py
    ├── point_cloud.py
    └── mesh.py
```

## 설치

```bash
pip install -r requirements.txt
```

## 사용 방법

### 1. 기본 워크플로우

```python
from structured_light import StructuredLightScanner

# 스캐너 초기화
scanner = StructuredLightScanner()

# 캘리브레이션 수행
scanner.calibrate(camera_id=0, projector_resolution=(1920, 1080))

# 스캔 실행
point_cloud = scanner.scan(pattern_type='gray_code')

# 결과 저장
scanner.save_point_cloud('output.ply')
```

### 2. 고급 사용

```python
from structured_light.pattern import GrayCodeGenerator
from structured_light.calibration import StereoCalibration
from structured_light.reconstruction import Triangulation

# 패턴 생성
pattern_gen = GrayCodeGenerator(width=1920, height=1080)
patterns = pattern_gen.generate()

# 캘리브레이션
calibrator = StereoCalibration()
calibrator.calibrate_from_images('calibration_images/')

# 3D 재구성
triangulator = Triangulation(calibrator.get_parameters())
points_3d = triangulator.reconstruct(decoded_images)
```

## 예제

- `examples/basic_scan.py`: 기본 스캐닝 예제
- `examples/calibration.py`: 캘리브레이션 예제
- `examples/pattern_generation.py`: 패턴 생성 예제
- `examples/advanced_reconstruction.py`: 고급 재구성 예제

## 알고리듬 설명

### Gray Code 방식
- 이진 패턴을 순차적으로 투영
- 강건하고 신뢰성 높음
- 다중 프레임 필요

### Phase Shifting 방식
- 정현파 패턴을 위상 변화시켜 투영
- 높은 정밀도
- 앰비언트 라이트에 민감

### Hybrid 방식
- Gray Code + Phase Shifting 조합
- 강건성과 정밀도 동시 확보
- 권장 방식

## 성능 최적화

- NumPy 벡터화 연산 사용
- OpenCV GPU 가속 지원
- 멀티스레딩 이미지 처리
- 메모리 효율적인 스트리밍 처리

## 요구사항

- Python 3.8+
- OpenCV 4.5+
- NumPy
- SciPy
- Open3D (포인트 클라우드 처리)

## 라이선스

MIT License

## 기여

이슈 및 풀 리퀘스트를 환영합니다.

## 참고문헌

1. Zhang, S. (2018). "High-speed 3D shape measurement with structured light methods"
2. Salvi, J., et al. (2010). "Pattern codification strategies in structured light systems"
3. Sansoni, G., et al. (2009). "State-of-the-art and applications of 3D imaging sensors"
