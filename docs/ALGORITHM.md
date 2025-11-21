# 구조광 알고리듬 상세 설명

## 목차

1. [개요](#개요)
2. [구조광 원리](#구조광-원리)
3. [패턴 생성](#패턴-생성)
4. [캘리브레이션](#캘리브레이션)
5. [패턴 디코딩](#패턴-디코딩)
6. [3D 재구성](#3d-재구성)

---

## 개요

구조광(Structured Light) 3D 스캐닝은 알려진 패턴을 대상물에 투영하고, 이를 카메라로 관찰하여 3D 형상을 복원하는 능동적 비전 기법입니다.

### 기본 원리

1. **패턴 투영**: 프로젝터로 코딩된 패턴을 투영
2. **이미지 캡처**: 카메라로 변형된 패턴 캡처
3. **패턴 디코딩**: 각 픽셀의 대응점 찾기
4. **삼각측량**: 카메라-프로젝터 기하학으로 3D 좌표 계산

---

## 구조광 원리

### 스테레오 비전과의 관계

구조광은 능동적 스테레오 비전으로 볼 수 있습니다:
- **카메라**: 관찰자
- **프로젝터**: 역방향 카메라 (빛 발산)
- **패턴**: 텍스처 제공 (대응점 문제 해결)

### 기하학

```
카메라 좌표 (u_c, v_c) <---> 프로젝터 좌표 (u_p, v_p)
                  ↓
            3D 포인트 (X, Y, Z)
```

삼각측량 공식:
```
Z = (B * f) / (d_c - d_p)
X = (u_c * Z) / f_c
Y = (v_c * Z) / f_c
```

여기서:
- B: 베이스라인 (카메라-프로젝터 거리)
- f: 초점거리
- d: 시차 (disparity)

---

## 패턴 생성

### 1. Gray Code 패턴

#### 원리
Gray Code는 인접한 값이 1비트만 차이나는 이진 코드입니다.

#### 장점
- 노이즈에 강건 (1비트 오류 = 작은 오차)
- 구현 간단
- 높은 신뢰도

#### 단점
- 다중 프레임 필요 (n 비트 = 2n 이미지)
- 상대적으로 느림

#### 알고리듬
```python
# 이진수를 Gray Code로 변환
def binary_to_gray(n):
    return n ^ (n >> 1)

# Gray Code를 이진수로 변환
def gray_to_binary(n):
    mask = n
    while mask:
        mask >>= 1
        n ^= mask
    return n
```

#### 패턴 생성 과정
1. 필요한 비트 수 계산: `n_bits = ceil(log2(width))`
2. 각 비트에 대해 스트라이프 패턴 생성
3. 역상 패턴 생성 (노이즈 제거용)

### 2. Phase Shifting 패턴

#### 원리
정현파 패턴의 위상을 변화시켜 투영하고, 위상 정보로부터 깊이를 계산합니다.

#### 장점
- 서브픽셀 정밀도
- 높은 정확도

#### 단점
- 앰비언트 라이트에 민감
- 위상 언래핑 필요

#### 위상 계산 (4-step)
```
I₁ = I₀ + I_m * cos(φ)
I₂ = I₀ + I_m * cos(φ + π/2)
I₃ = I₀ + I_m * cos(φ + π)
I₄ = I₀ + I_m * cos(φ + 3π/2)

φ = arctan((I₄ - I₂) / (I₁ - I₃))
```

#### 다중 주파수 언래핑
위상 모호성을 해결하기 위해 다중 주파수 사용:

```
φ_absolute = unwrap(φ_low) → unwrap(φ_medium) → unwrap(φ_high)
```

### 3. De Bruijn 패턴

#### 원리
De Bruijn 시퀀스는 모든 가능한 길이 n의 부분 시퀀스가 정확히 한 번씩 나타나는 순환 시퀀스입니다.

#### 장점
- 단일 샷 스캐닝 가능
- 실시간 처리 가능

#### 단점
- 해상도 제한
- 복잡한 디코딩

---

## 캘리브레이션

### 1. 카메라 캘리브레이션

#### 목적
카메라 내부 파라미터 추정:
- 초점거리 (f_x, f_y)
- 주점 (c_x, c_y)
- 왜곡 계수 (k₁, k₂, p₁, p₂, k₃)

#### 방법: Zhang's Method
1. 다양한 각도에서 체스보드 촬영
2. 코너 포인트 검출
3. 호모그래피 계산
4. 최적화로 파라미터 추정

#### 카메라 매트릭스
```
K = [f_x  0   c_x]
    [0    f_y c_y]
    [0    0   1  ]
```

### 2. 프로젝터 캘리브레이션

#### 접근법
프로젝터를 "역방향 카메라"로 취급:
- 픽셀이 빛을 받는 대신 발산
- 동일한 핀홀 카메라 모델 적용

#### 방법
1. Gray Code 패턴으로 대응점 찾기
2. 체스보드와 패턴 동시 사용
3. 카메라와 동일한 캘리브레이션 적용

### 3. 스테레오 캘리브레이션

#### 목적
카메라-프로젝터 상대 위치 추정:
- 회전 행렬 (R)
- 변환 벡터 (T)

#### Essential Matrix (E)
```
E = [T]_× * R

여기서 [T]_×는 T의 skew-symmetric matrix
```

#### Fundamental Matrix (F)
```
F = K_p^(-T) * E * K_c^(-1)
```

---

## 패턴 디코딩

### 1. Gray Code 디코딩

#### 알고리듬
```python
for each bit:
    I_normal = captured_images[2*bit]
    I_inverse = captured_images[2*bit + 1]

    # 노이즈 제거를 위한 차이 계산
    diff = I_normal - I_inverse

    # 이진화
    bit_value = (diff > 0)

    # Gray code 구성
    gray_code |= (bit_value << bit)

# Gray code를 이진수로 변환
binary_code = gray_to_binary(gray_code)
```

#### 신뢰도 계산
```python
confidence = |I_normal - I_inverse| / 255
```

### 2. Phase Shifting 디코딩

#### 랩핑된 위상 계산
```python
numerator = Σ(I_k * sin(2πk/N))
denominator = Σ(I_k * cos(2πk/N))
φ_wrapped = arctan2(-numerator, denominator)  # [-π, π]
```

#### 위상 언래핑
Quality-guided path following:
1. 변조도가 높은 픽셀부터 시작
2. 인접 픽셀로 전파
3. 위상 차이가 π 이하가 되도록 2π 배수 조정

```python
φ_diff = φ_neighbor - φ_current
k = round(φ_diff / 2π)
φ_unwrapped = φ_neighbor - k * 2π
```

---

## 3D 재구성

### 1. 삼각측량

#### 기본 원리
두 시점(카메라, 프로젝터)에서 같은 점을 관찰하면 3D 위치를 계산할 수 있습니다.

#### Direct Linear Transformation (DLT)
```
u_c × (P_c * X) = 0
u_p × (P_p * X) = 0
```

여기서:
- P_c, P_p: 카메라/프로젝터 projection matrix
- X: 3D 포인트 (homogeneous)
- u_c, u_p: 2D 이미지 포인트

#### 구현
```python
# Projection matrices
P1 = K_c @ [I | 0]
P2 = K_p @ [R | T]

# Triangulation
A = [u_c[0] * P1[2] - P1[0],
     u_c[1] * P1[2] - P1[1],
     u_p[0] * P2[2] - P2[0],
     u_p[1] * P2[2] - P2[1]]

# SVD로 해 구하기
_, _, V = svd(A)
X = V[-1] / V[-1, 3]  # Normalize
```

### 2. 포인트 클라우드 처리

#### 아웃라이어 제거

**Statistical Outlier Removal:**
```python
for each point:
    distances = distances_to_k_nearest_neighbors
    if mean(distances) > threshold:
        remove point
```

**Radius Outlier Removal:**
```python
for each point:
    neighbors = points_within_radius
    if len(neighbors) < min_neighbors:
        remove point
```

#### 다운샘플링

**Voxel Grid:**
각 복셀 내 포인트들을 중심점으로 대체

```python
voxel_map = {}
for point in points:
    voxel_idx = floor(point / voxel_size)
    voxel_map[voxel_idx].append(point)

downsampled = [mean(voxel_map[idx]) for idx in voxel_map]
```

---

## 성능 최적화

### 1. 패턴 수 최소화
- Hybrid approach: Gray Code + Phase Shifting
- 저주파 Gray Code로 언래핑
- 고주파 Phase Shifting으로 정밀도

### 2. 병렬 처리
- GPU 가속 (CUDA/OpenCL)
- 픽셀별 독립 연산
- 벡터화 연산

### 3. 적응형 스캐닝
- ROI (Region of Interest) 기반
- 동적 노출 조정
- 품질 기반 재스캔

---

## 오차 분석

### 1. 오차 원인
- 캘리브레이션 오차
- 프로젝터 defocus
- 앰비언트 라이트
- 표면 특성 (반사, 투명)

### 2. 오차 감소 방법
- 고품질 캘리브레이션
- HDR 캡처
- 편광 필터
- 다중 뷰 융합

---

## 참고문헌

1. Zhang, S. (2018). "High-speed 3D shape measurement with structured light methods: A review"
2. Salvi, J., et al. (2010). "Pattern codification strategies in structured light systems"
3. Scharstein, D., & Szeliski, R. (2002). "A taxonomy and evaluation of dense two-frame stereo correspondence algorithms"
4. Ghiglia, D. C., & Pritt, M. D. (1998). "Two-Dimensional Phase Unwrapping: Theory, Algorithms, and Software"
