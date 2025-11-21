"""
Phase Shifting Pattern Generator

위상 변조 방식의 구조광 패턴 생성
정현파 패턴을 위상을 변화시켜 투영하여
서브픽셀 정밀도의 3D 측정 가능
"""

import numpy as np
import cv2
from typing import List, Tuple, Optional


class PhaseShiftGenerator:
    """Phase Shifting 패턴 생성기"""

    def __init__(self, width: int = 1920, height: int = 1080,
                 n_steps: int = 4, n_periods: int = 64,
                 direction: str = 'both'):
        """
        Args:
            width: 패턴 폭
            height: 패턴 높이
            n_steps: 위상 스텝 수 (3, 4, 6 등)
            n_periods: 주기 수 (프린지 밀도)
            direction: 패턴 방향 ('horizontal', 'vertical', 'both')
        """
        self.width = width
        self.height = height
        self.n_steps = n_steps
        self.n_periods = n_periods
        self.direction = direction

    def generate(self) -> List[np.ndarray]:
        """
        Phase Shifting 패턴 생성

        Returns:
            패턴 이미지 리스트
        """
        patterns = []

        if self.direction in ['horizontal', 'both']:
            patterns.extend(self._generate_horizontal())

        if self.direction in ['vertical', 'both']:
            patterns.extend(self._generate_vertical())

        return patterns

    def _generate_horizontal(self) -> List[np.ndarray]:
        """수평 방향 위상 시프트 패턴 생성"""
        patterns = []

        for step in range(self.n_steps):
            phase = 2.0 * np.pi * step / self.n_steps
            pattern = self._create_sinusoidal_pattern(
                self.width, self.height, self.n_periods,
                phase, 'horizontal'
            )
            patterns.append(pattern)

        return patterns

    def _generate_vertical(self) -> List[np.ndarray]:
        """수직 방향 위상 시프트 패턴 생성"""
        patterns = []

        for step in range(self.n_steps):
            phase = 2.0 * np.pi * step / self.n_steps
            pattern = self._create_sinusoidal_pattern(
                self.width, self.height, self.n_periods,
                phase, 'vertical'
            )
            patterns.append(pattern)

        return patterns

    def _create_sinusoidal_pattern(self, width: int, height: int,
                                   n_periods: int, phase: float,
                                   direction: str) -> np.ndarray:
        """
        정현파 패턴 생성

        Args:
            width: 패턴 폭
            height: 패턴 높이
            n_periods: 주기 수
            phase: 위상 오프셋 (라디안)
            direction: 'horizontal' 또는 'vertical'

        Returns:
            정현파 패턴 이미지
        """
        pattern = np.zeros((height, width), dtype=np.float32)

        if direction == 'horizontal':
            # 수평 방향 정현파
            x = np.arange(width)
            sinusoid = 127.5 * (1.0 + np.sin(2.0 * np.pi * n_periods * x / width + phase))
            pattern = np.tile(sinusoid, (height, 1))
        else:
            # 수직 방향 정현파
            y = np.arange(height)
            sinusoid = 127.5 * (1.0 + np.sin(2.0 * np.pi * n_periods * y / height + phase))
            pattern = np.tile(sinusoid.reshape(-1, 1), (1, width))

        return pattern.astype(np.uint8)

    def generate_multi_frequency(self, frequencies: List[int]) -> List[np.ndarray]:
        """
        다중 주파수 패턴 생성 (위상 언래핑용)

        Args:
            frequencies: 주기 수 리스트 (예: [1, 8, 64])

        Returns:
            다중 주파수 패턴 리스트
        """
        patterns = []

        for freq in frequencies:
            temp_generator = PhaseShiftGenerator(
                self.width, self.height, self.n_steps, freq, self.direction
            )
            patterns.extend(temp_generator.generate())

        return patterns

    def get_pattern_info(self) -> dict:
        """패턴 정보 반환"""
        n_patterns = self.n_steps

        if self.direction == 'both':
            n_patterns *= 2

        return {
            'width': self.width,
            'height': self.height,
            'direction': self.direction,
            'n_steps': self.n_steps,
            'n_periods': self.n_periods,
            'total_patterns': n_patterns
        }

    def save_patterns(self, patterns: List[np.ndarray],
                     output_dir: str, prefix: str = 'phase_shift') -> None:
        """패턴을 파일로 저장"""
        import os
        os.makedirs(output_dir, exist_ok=True)

        for i, pattern in enumerate(patterns):
            filename = os.path.join(output_dir, f'{prefix}_{i:03d}.png')
            cv2.imwrite(filename, pattern)

    def visualize_patterns(self, patterns: List[np.ndarray]) -> None:
        """패턴 시각화"""
        import matplotlib.pyplot as plt

        n_display = len(patterns)
        cols = min(4, n_display)
        rows = (n_display + cols - 1) // cols

        fig, axes = plt.subplots(rows, cols, figsize=(15, 4*rows))
        if n_display == 1:
            axes = [axes]
        else:
            axes = axes.flatten()

        for i in range(n_display):
            axes[i].imshow(patterns[i], cmap='gray')
            axes[i].set_title(f'Phase Step {i}')
            axes[i].axis('off')

        for i in range(n_display, len(axes)):
            axes[i].axis('off')

        plt.tight_layout()
        plt.show()

    @staticmethod
    def compute_wrapped_phase(images: List[np.ndarray]) -> np.ndarray:
        """
        캡처된 이미지로부터 랩핑된 위상 계산

        Args:
            images: 위상 시프트 이미지 리스트 (n_steps개)

        Returns:
            랩핑된 위상 맵 (-π ~ π)
        """
        n_steps = len(images)
        height, width = images[0].shape[:2]

        # 이미지를 float로 변환
        images_float = [img.astype(np.float32) for img in images]

        if n_steps == 3:
            # 3-step algorithm
            I1, I2, I3 = images_float
            numerator = np.sqrt(3) * (I1 - I3)
            denominator = 2 * I2 - I1 - I3
        elif n_steps == 4:
            # 4-step algorithm
            I1, I2, I3, I4 = images_float
            numerator = I4 - I2
            denominator = I1 - I3
        elif n_steps == 6:
            # 6-step algorithm
            I = np.array(images_float)
            numerator = np.sqrt(3) * (I[5] + I[4] - I[1] - I[0])
            denominator = I[3] + 2*I[2] + I[1] - I[0] - 2*I[5] - I[4]
        else:
            # General N-step algorithm
            I = np.array(images_float)
            numerator = np.zeros((height, width), dtype=np.float32)
            denominator = np.zeros((height, width), dtype=np.float32)

            for k in range(n_steps):
                phase = 2.0 * np.pi * k / n_steps
                numerator += I[k] * np.sin(phase)
                denominator += I[k] * np.cos(phase)

        # 위상 계산
        phase = np.arctan2(numerator, denominator)

        return phase

    @staticmethod
    def compute_intensity(images: List[np.ndarray]) -> np.ndarray:
        """
        평균 강도 계산 (텍스처 정보)

        Args:
            images: 위상 시프트 이미지 리스트

        Returns:
            평균 강도 맵
        """
        return np.mean(images, axis=0).astype(np.uint8)

    @staticmethod
    def compute_modulation(images: List[np.ndarray]) -> np.ndarray:
        """
        변조도 계산 (품질 지표)

        Args:
            images: 위상 시프트 이미지 리스트

        Returns:
            변조도 맵 (0~1)
        """
        images_float = [img.astype(np.float32) for img in images]
        I_max = np.max(images_float, axis=0)
        I_min = np.min(images_float, axis=0)
        I_mean = np.mean(images_float, axis=0)

        # 변조도 계산
        modulation = (I_max - I_min) / (I_max + I_min + 1e-6)

        return modulation


def demo():
    """Phase Shifting 패턴 생성 데모"""
    # 패턴 생성기 초기화
    generator = PhaseShiftGenerator(
        width=1920, height=1080,
        n_steps=4, n_periods=64,
        direction='both'
    )

    # 패턴 정보 출력
    info = generator.get_pattern_info()
    print("Phase Shifting Pattern Generator")
    print("=" * 50)
    print(f"Resolution: {info['width']} x {info['height']}")
    print(f"Direction: {info['direction']}")
    print(f"Phase steps: {info['n_steps']}")
    print(f"Periods: {info['n_periods']}")
    print(f"Total patterns: {info['total_patterns']}")
    print("=" * 50)

    # 패턴 생성
    print("\nGenerating patterns...")
    patterns = generator.generate()
    print(f"Generated {len(patterns)} patterns")

    # 다중 주파수 패턴 생성 (옵션)
    print("\nGenerating multi-frequency patterns...")
    multi_freq_patterns = generator.generate_multi_frequency([1, 8, 64])
    print(f"Generated {len(multi_freq_patterns)} multi-frequency patterns")

    return patterns


if __name__ == '__main__':
    patterns = demo()
