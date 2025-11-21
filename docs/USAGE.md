# 사용 가이드

## 설치

### 1. 저장소 클론
```bash
git clone https://github.com/gomu80/gmsin.git
cd gmsin
```

### 2. 가상 환경 생성 (권장)
```bash
python -m venv venv
source venv/bin/activate  # Linux/Mac
# 또는
venv\Scripts\activate  # Windows
```

### 3. 의존성 설치
```bash
pip install -r requirements.txt
```

### 4. 패키지 설치
```bash
pip install -e .
```

---

## 빠른 시작

### 1. 패턴 생성
```python
from structured_light.pattern import GrayCodeGenerator

# Gray Code 패턴 생성
generator = GrayCodeGenerator(width=1920, height=1080)
patterns = generator.generate()

# 패턴 저장
generator.save_patterns(patterns, 'data/patterns/gray_code')
```

### 2. 캘리브레이션
```python
from structured_light.calibration import CameraCalibration

# 카메라 캘리브레이션
calibrator = CameraCalibration(
    checkerboard_size=(9, 6),
    square_size=25.0
)

# 이미지로부터 캘리브레이션
calibrator.calibrate_from_directory('data/calibration/camera')
calibrator.save_calibration('data/calibration/camera_calibration.json')
```

### 3. 스캔 수행
```python
from structured_light import StructuredLightScanner

# 스캐너 초기화
scanner = StructuredLightScanner()
scanner.initialize()

# 캘리브레이션 로드
scanner.load_calibration('data/calibration')

# 스캔
point_cloud = scanner.scan(pattern_type='gray_code')

# 결과 저장
scanner.save_point_cloud('output.ply')
```

---

## 상세 사용법

### 패턴 타입별 사용

#### Gray Code
```python
from structured_light.pattern import GrayCodeGenerator

generator = GrayCodeGenerator(
    width=1920,
    height=1080,
    direction='both',      # 'horizontal', 'vertical', 'both'
    inverse=True          # 역상 패턴 포함
)

patterns = generator.generate()
print(f"Generated {len(patterns)} patterns")

# 패턴 정보
info = generator.get_pattern_info()
print(f"Total patterns: {info['total_patterns']}")
```

#### Phase Shifting
```python
from structured_light.pattern import PhaseShiftGenerator

generator = PhaseShiftGenerator(
    width=1920,
    height=1080,
    n_steps=4,           # 위상 스텝 수
    n_periods=64,        # 주기 수
    direction='both'
)

patterns = generator.generate()

# 다중 주파수 패턴
multi_freq = generator.generate_multi_frequency([1, 8, 64])
```

#### De Bruijn
```python
from structured_light.pattern import DeBruijnGenerator

generator = DeBruijnGenerator(
    width=1920,
    height=1080,
    window_size=5,       # 윈도우 크기
    n_colors=2          # 색상 수
)

patterns = generator.generate()
```

### 캘리브레이션 워크플로우

#### 1. 준비물
- 체스보드 패턴 (권장: 9x6 내부 코너)
- 정확한 정사각형 크기 (예: 25mm)

#### 2. 이미지 캡처
```python
from structured_light.capture import CameraController

with CameraController(camera_id=0) as camera:
    # 다양한 각도에서 10-20장 촬영
    images = camera.capture_sequence(n_images=15, delay_between=1.0)

    # 이미지 저장
    for i, img in enumerate(images):
        cv2.imwrite(f'data/calibration/camera/img_{i:03d}.png', img)
```

#### 3. 캘리브레이션 실행
```python
from structured_light.calibration import CameraCalibration

calibrator = CameraCalibration(
    checkerboard_size=(9, 6),
    square_size=25.0
)

success = calibrator.calibrate_from_directory('data/calibration/camera')

if success:
    calibrator.print_calibration()
    calibrator.save_calibration('data/calibration/camera_calibration.json')
```

### 고급 스캐닝

#### Hybrid 방식 (Gray Code + Phase Shifting)
```python
from structured_light.pattern import GrayCodeGenerator, PhaseShiftGenerator
from structured_light.decode import GrayCodeDecoder, PhaseDecoder

# 1. Gray Code로 대략적인 대응점 찾기
gray_gen = GrayCodeGenerator(width=1920, height=1080)
gray_patterns = gray_gen.generate()
# ... 투영 및 캡처 ...

gray_decoder = GrayCodeDecoder(gray_gen)
coarse_coords = gray_decoder.decode(gray_captured)

# 2. Phase Shifting으로 정밀 측정
phase_gen = PhaseShiftGenerator(width=1920, height=1080, n_periods=64)
phase_patterns = phase_gen.generate()
# ... 투영 및 캡처 ...

phase_decoder = PhaseDecoder(phase_gen)
result = phase_decoder.decode(phase_captured)

# 3. Gray Code로 위상 언래핑
unwrapped_phase = phase_decoder._unwrap_phase(
    result['wrapped_phase'],
    result['modulation']
)
```

### 포인트 클라우드 처리

#### 아웃라이어 제거
```python
from structured_light.reconstruction import PointCloudProcessor

processor = PointCloudProcessor()
processor.set_point_cloud(point_cloud)

# 통계적 아웃라이어 제거
filtered = processor.filter_outliers(
    method='statistical',
    nb_neighbors=20,
    std_ratio=2.0
)

# 또는 반경 기반 제거
filtered = processor.filter_outliers(
    method='radius',
    nb_points=16,
    radius=0.05
)
```

#### 다운샘플링
```python
downsampled = processor.downsample(voxel_size=0.01)
```

#### 저장
```python
processor.save_ply('output.ply')
processor.save_xyz('output.xyz')
```

#### 시각화
```python
processor.visualize()  # Open3D 필요
```

---

## 설정 파일 사용

### YAML 설정 파일
```yaml
# config/scanner_config.yaml
camera:
  id: 0
  resolution:
    width: 1920
    height: 1080

projector:
  resolution:
    width: 1920
    height: 1080

pattern:
  type: gray_code
  gray_code:
    direction: both
    inverse: true
```

### 설정 로드
```python
import yaml

with open('config/scanner_config.yaml') as f:
    config = yaml.safe_load(f)

scanner = StructuredLightScanner(config)
```

---

## 예제 실행

### 패턴 생성 예제
```bash
python examples/pattern_generation.py
```

### 캘리브레이션 예제
```bash
python examples/calibration.py
```

### 기본 스캔 예제
```bash
python examples/basic_scan.py
```

---

## 문제 해결

### Q: 체스보드 코너가 검출되지 않습니다
A:
- 조명이 균일한지 확인
- 체스보드가 평평한지 확인
- 이미지 해상도가 충분한지 확인
- 체스보드 크기 설정이 정확한지 확인

### Q: 캘리브레이션 오차가 큽니다
A:
- 더 많은 이미지 사용 (15-20장)
- 다양한 각도와 거리에서 촬영
- 이미지 초점이 맞는지 확인
- 체스보드 정사각형 크기가 정확한지 확인

### Q: 포인트 클라우드에 노이즈가 많습니다
A:
- 앰비언트 라이트 최소화
- 노출 시간 조정
- 역상 패턴 사용
- 아웃라이어 제거 적용
- 신뢰도 임계값 높이기

### Q: 재구성 속도가 느립니다
A:
- 패턴 수 최소화 (Hybrid 방식)
- 다운샘플링 적용
- GPU 가속 활성화
- ROI 기반 스캔

---

## 성능 최적화 팁

### 1. 패턴 수 최소화
- Gray Code: 방향 하나만 사용
- Phase Shifting: 3-step 사용
- Hybrid: 저주파 Gray Code + 고주파 Phase

### 2. 병렬 처리
```python
# NumPy 벡터화 연산 사용
# OpenCV GPU 함수 사용
```

### 3. 메모리 최적화
```python
# 스트리밍 처리
# 불필요한 복사 방지
# 적절한 데이터 타입 선택 (uint8 vs float32)
```

---

## API 참조

자세한 API 문서는 각 모듈의 docstring을 참조하세요:

```python
help(GrayCodeGenerator)
help(PhaseShiftGenerator)
help(CameraCalibration)
help(Triangulation)
```

---

## 추가 리소스

- [알고리듬 상세 설명](ALGORITHM.md)
- [GitHub 저장소](https://github.com/gomu80/gmsin)
- [이슈 트래커](https://github.com/gomu80/gmsin/issues)
