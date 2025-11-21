"""
Gray Code Pattern Generator

이진 코딩 방식의 구조광 패턴 생성
Gray Code는 인접한 코드가 1비트만 차이나는 특성으로
노이즈에 강건한 패턴을 제공합니다.
"""

import numpy as np
import cv2
from typing import List, Tuple, Optional


class GrayCodeGenerator:
    """Gray Code 패턴 생성기"""

    def __init__(self, width: int = 1920, height: int = 1080,
                 direction: str = 'both', inverse: bool = True):
        """
        Args:
            width: 패턴 폭 (프로젝터 해상도)
            height: 패턴 높이 (프로젝터 해상도)
            direction: 패턴 방향 ('horizontal', 'vertical', 'both')
            inverse: 역상 패턴 포함 여부 (노이즈 제거용)
        """
        self.width = width
        self.height = height
        self.direction = direction
        self.inverse = inverse

        # 필요한 비트 수 계산
        self.n_bits_h = int(np.ceil(np.log2(width)))
        self.n_bits_v = int(np.ceil(np.log2(height)))

    def generate(self) -> List[np.ndarray]:
        """
        Gray Code 패턴 생성

        Returns:
            패턴 이미지 리스트 (각 비트별 + 역상)
        """
        patterns = []

        if self.direction in ['horizontal', 'both']:
            patterns.extend(self._generate_horizontal())

        if self.direction in ['vertical', 'both']:
            patterns.extend(self._generate_vertical())

        # 전체 흰색/검은색 패턴 추가 (앰비언트 라이트 측정용)
        patterns.append(np.ones((self.height, self.width), dtype=np.uint8) * 255)
        patterns.append(np.zeros((self.height, self.width), dtype=np.uint8))

        return patterns

    def _generate_horizontal(self) -> List[np.ndarray]:
        """수평 방향 Gray Code 패턴 생성"""
        patterns = []

        for bit in range(self.n_bits_h):
            pattern = self._create_stripe_pattern(
                self.width, self.height, bit, 'horizontal'
            )
            patterns.append(pattern)

            if self.inverse:
                patterns.append(255 - pattern)

        return patterns

    def _generate_vertical(self) -> List[np.ndarray]:
        """수직 방향 Gray Code 패턴 생성"""
        patterns = []

        for bit in range(self.n_bits_v):
            pattern = self._create_stripe_pattern(
                self.width, self.height, bit, 'vertical'
            )
            patterns.append(pattern)

            if self.inverse:
                patterns.append(255 - pattern)

        return patterns

    def _create_stripe_pattern(self, width: int, height: int,
                              bit: int, direction: str) -> np.ndarray:
        """
        특정 비트에 대한 스트라이프 패턴 생성

        Args:
            width: 패턴 폭
            height: 패턴 높이
            bit: 비트 인덱스 (0부터 시작)
            direction: 'horizontal' 또는 'vertical'

        Returns:
            Gray Code 패턴 이미지
        """
        pattern = np.zeros((height, width), dtype=np.uint8)

        # 스트라이프 주기 계산
        period = 2 ** (bit + 1)

        if direction == 'horizontal':
            # 수평 스트라이프
            for x in range(width):
                gray_code = self._binary_to_gray(x)
                if (gray_code >> bit) & 1:
                    pattern[:, x] = 255
        else:
            # 수직 스트라이프
            for y in range(height):
                gray_code = self._binary_to_gray(y)
                if (gray_code >> bit) & 1:
                    pattern[y, :] = 255

        return pattern

    @staticmethod
    def _binary_to_gray(n: int) -> int:
        """이진수를 Gray Code로 변환"""
        return n ^ (n >> 1)

    @staticmethod
    def _gray_to_binary(n: int) -> int:
        """Gray Code를 이진수로 변환"""
        mask = n
        while mask:
            mask >>= 1
            n ^= mask
        return n

    def get_pattern_info(self) -> dict:
        """패턴 정보 반환"""
        n_patterns = 0

        if self.direction in ['horizontal', 'both']:
            n_patterns += self.n_bits_h * (2 if self.inverse else 1)

        if self.direction in ['vertical', 'both']:
            n_patterns += self.n_bits_v * (2 if self.inverse else 1)

        n_patterns += 2  # 흰색/검은색 패턴

        return {
            'width': self.width,
            'height': self.height,
            'direction': self.direction,
            'n_bits_horizontal': self.n_bits_h if self.direction in ['horizontal', 'both'] else 0,
            'n_bits_vertical': self.n_bits_v if self.direction in ['vertical', 'both'] else 0,
            'inverse': self.inverse,
            'total_patterns': n_patterns
        }

    def save_patterns(self, patterns: List[np.ndarray],
                     output_dir: str, prefix: str = 'gray_code') -> None:
        """
        패턴을 파일로 저장

        Args:
            patterns: 패턴 리스트
            output_dir: 출력 디렉토리
            prefix: 파일명 접두사
        """
        import os
        os.makedirs(output_dir, exist_ok=True)

        for i, pattern in enumerate(patterns):
            filename = os.path.join(output_dir, f'{prefix}_{i:03d}.png')
            cv2.imwrite(filename, pattern)

    def visualize_patterns(self, patterns: List[np.ndarray],
                          max_display: int = 8) -> None:
        """
        패턴 시각화

        Args:
            patterns: 패턴 리스트
            max_display: 최대 표시 개수
        """
        import matplotlib.pyplot as plt

        n_display = min(len(patterns), max_display)
        cols = 4
        rows = (n_display + cols - 1) // cols

        fig, axes = plt.subplots(rows, cols, figsize=(15, 4*rows))
        axes = axes.flatten() if n_display > 1 else [axes]

        for i in range(n_display):
            axes[i].imshow(patterns[i], cmap='gray')
            axes[i].set_title(f'Pattern {i}')
            axes[i].axis('off')

        for i in range(n_display, len(axes)):
            axes[i].axis('off')

        plt.tight_layout()
        plt.show()


def demo():
    """Gray Code 패턴 생성 데모"""
    # 패턴 생성기 초기화
    generator = GrayCodeGenerator(width=1920, height=1080, direction='both')

    # 패턴 정보 출력
    info = generator.get_pattern_info()
    print("Gray Code Pattern Generator")
    print("=" * 50)
    print(f"Resolution: {info['width']} x {info['height']}")
    print(f"Direction: {info['direction']}")
    print(f"Horizontal bits: {info['n_bits_horizontal']}")
    print(f"Vertical bits: {info['n_bits_vertical']}")
    print(f"Inverse patterns: {info['inverse']}")
    print(f"Total patterns: {info['total_patterns']}")
    print("=" * 50)

    # 패턴 생성
    print("\nGenerating patterns...")
    patterns = generator.generate()
    print(f"Generated {len(patterns)} patterns")

    # 패턴 저장 (옵션)
    # generator.save_patterns(patterns, 'data/patterns/gray_code')

    return patterns


if __name__ == '__main__':
    patterns = demo()
