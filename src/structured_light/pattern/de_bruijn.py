"""
De Bruijn Pattern Generator

De Bruijn 시퀀스 기반 단일 샷 구조광 패턴 생성
단일 이미지로 3D 재구성이 가능한 유일 코딩 패턴
"""

import numpy as np
import cv2
from typing import List, Tuple, Optional, Set


class DeBruijnGenerator:
    """De Bruijn 패턴 생성기"""

    def __init__(self, width: int = 1920, height: int = 1080,
                 window_size: int = 5, n_colors: int = 2):
        """
        Args:
            width: 패턴 폭
            height: 패턴 높이
            window_size: 윈도우 크기 (고유 시퀀스 길이)
            n_colors: 색상 수 (2 = 이진, 3+ = 다중 레벨)
        """
        self.width = width
        self.height = height
        self.window_size = window_size
        self.n_colors = n_colors

        # De Bruijn 시퀀스 생성
        self.sequence = self._generate_de_bruijn_sequence(n_colors, window_size)

    def generate(self) -> List[np.ndarray]:
        """
        De Bruijn 패턴 생성

        Returns:
            패턴 이미지 (단일 이미지)
        """
        # 수평 패턴 생성
        pattern_h = self._create_stripe_pattern(self.sequence, 'horizontal')

        # 수직 패턴 생성 (선택적)
        pattern_v = self._create_stripe_pattern(self.sequence, 'vertical')

        # 컬러 인코딩 패턴 (선택적)
        pattern_color = self._create_color_pattern()

        return [pattern_h, pattern_v, pattern_color]

    def _generate_de_bruijn_sequence(self, k: int, n: int) -> List[int]:
        """
        De Bruijn 시퀀스 생성 (Martin's algorithm)

        Args:
            k: 알파벳 크기 (색상 수)
            n: 윈도우 크기

        Returns:
            De Bruijn 시퀀스
        """
        alphabet = list(range(k))
        a = [0] * k * n
        sequence = []

        def db(t: int, p: int):
            if t > n:
                if n % p == 0:
                    sequence.extend(a[1:p + 1])
            else:
                a[t] = a[t - p]
                db(t + 1, p)
                for j in range(a[t - p] + 1, k):
                    a[t] = j
                    db(t + 1, t)

        db(1, 1)
        return sequence

    def _create_stripe_pattern(self, sequence: List[int],
                               direction: str) -> np.ndarray:
        """
        스트라이프 패턴 생성

        Args:
            sequence: De Bruijn 시퀀스
            direction: 'horizontal' 또는 'vertical'

        Returns:
            패턴 이미지
        """
        pattern = np.zeros((self.height, self.width), dtype=np.uint8)

        # 시퀀스를 이미지 크기에 맞게 반복
        if direction == 'horizontal':
            stripe_width = max(1, self.width // len(sequence))
            for i, val in enumerate(sequence):
                x_start = i * stripe_width
                x_end = min((i + 1) * stripe_width, self.width)
                intensity = int(255 * val / (self.n_colors - 1))
                pattern[:, x_start:x_end] = intensity
        else:
            stripe_height = max(1, self.height // len(sequence))
            for i, val in enumerate(sequence):
                y_start = i * stripe_height
                y_end = min((i + 1) * stripe_height, self.height)
                intensity = int(255 * val / (self.n_colors - 1))
                pattern[y_start:y_end, :] = intensity

        return pattern

    def _create_color_pattern(self) -> np.ndarray:
        """
        컬러 De Bruijn 패턴 생성 (다중 채널 인코딩)

        Returns:
            컬러 패턴 이미지 (BGR)
        """
        pattern = np.zeros((self.height, self.width, 3), dtype=np.uint8)

        # RGB 채널에 각각 다른 De Bruijn 시퀀스 적용
        for channel in range(3):
            seq = self._generate_de_bruijn_sequence(self.n_colors, self.window_size)
            stripe_width = max(1, self.width // len(seq))

            for i, val in enumerate(seq):
                x_start = i * stripe_width
                x_end = min((i + 1) * stripe_width, self.width)
                intensity = int(255 * val / (self.n_colors - 1))
                pattern[:, x_start:x_end, channel] = intensity

        return pattern

    def decode_pattern(self, captured_image: np.ndarray) -> np.ndarray:
        """
        캡처된 이미지에서 De Bruijn 코드 디코딩

        Args:
            captured_image: 캡처된 패턴 이미지

        Returns:
            디코딩된 좌표 맵
        """
        if len(captured_image.shape) == 3:
            gray = cv2.cvtColor(captured_image, cv2.COLOR_BGR2GRAY)
        else:
            gray = captured_image

        height, width = gray.shape
        coord_map = np.zeros((height, width), dtype=np.int32)

        # 각 픽셀에서 윈도우 추출 및 디코딩
        half_window = self.window_size // 2

        for y in range(height):
            for x in range(half_window, width - half_window):
                # 윈도우 추출
                window = gray[y, x-half_window:x+half_window+1]

                # 이진화
                threshold = np.mean(window)
                binary_window = (window > threshold).astype(int)

                # 시퀀스에서 위치 찾기
                coord = self._find_sequence_position(binary_window.tolist())
                coord_map[y, x] = coord

        return coord_map

    def _find_sequence_position(self, window: List[int]) -> int:
        """
        De Bruijn 시퀀스에서 윈도우 위치 찾기

        Args:
            window: 이진 윈도우 시퀀스

        Returns:
            시퀀스 내 위치 (-1: 찾지 못함)
        """
        window_tuple = tuple(window)
        seq_len = len(self.sequence)

        for i in range(seq_len):
            match = True
            for j in range(len(window)):
                if self.sequence[(i + j) % seq_len] != window[j]:
                    match = False
                    break
            if match:
                return i

        return -1

    def get_pattern_info(self) -> dict:
        """패턴 정보 반환"""
        return {
            'width': self.width,
            'height': self.height,
            'window_size': self.window_size,
            'n_colors': self.n_colors,
            'sequence_length': len(self.sequence),
            'unique_windows': self.n_colors ** self.window_size,
            'total_patterns': 3  # horizontal, vertical, color
        }

    def save_patterns(self, patterns: List[np.ndarray],
                     output_dir: str, prefix: str = 'de_bruijn') -> None:
        """패턴을 파일로 저장"""
        import os
        os.makedirs(output_dir, exist_ok=True)

        names = ['horizontal', 'vertical', 'color']
        for i, (pattern, name) in enumerate(zip(patterns, names)):
            filename = os.path.join(output_dir, f'{prefix}_{name}.png')
            cv2.imwrite(filename, pattern)

    def visualize_patterns(self, patterns: List[np.ndarray]) -> None:
        """패턴 시각화"""
        import matplotlib.pyplot as plt

        fig, axes = plt.subplots(1, 3, figsize=(15, 5))

        titles = ['Horizontal', 'Vertical', 'Color']
        for i, (pattern, title) in enumerate(zip(patterns, titles)):
            if len(pattern.shape) == 3:
                axes[i].imshow(cv2.cvtColor(pattern, cv2.COLOR_BGR2RGB))
            else:
                axes[i].imshow(pattern, cmap='gray')
            axes[i].set_title(title)
            axes[i].axis('off')

        plt.tight_layout()
        plt.show()

    def visualize_sequence(self) -> None:
        """De Bruijn 시퀀스 시각화"""
        import matplotlib.pyplot as plt

        seq_len = min(100, len(self.sequence))
        sequence_subset = self.sequence[:seq_len]

        plt.figure(figsize=(15, 3))
        plt.bar(range(seq_len), sequence_subset, width=1.0, edgecolor='black')
        plt.xlabel('Position')
        plt.ylabel('Value')
        plt.title(f'De Bruijn Sequence (first {seq_len} elements)')
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        plt.show()


def demo():
    """De Bruijn 패턴 생성 데모"""
    # 패턴 생성기 초기화
    generator = DeBruijnGenerator(
        width=1920, height=1080,
        window_size=5, n_colors=2
    )

    # 패턴 정보 출력
    info = generator.get_pattern_info()
    print("De Bruijn Pattern Generator")
    print("=" * 50)
    print(f"Resolution: {info['width']} x {info['height']}")
    print(f"Window size: {info['window_size']}")
    print(f"Number of colors: {info['n_colors']}")
    print(f"Sequence length: {info['sequence_length']}")
    print(f"Unique windows: {info['unique_windows']}")
    print(f"Total patterns: {info['total_patterns']}")
    print("=" * 50)

    # 패턴 생성
    print("\nGenerating patterns...")
    patterns = generator.generate()
    print(f"Generated {len(patterns)} patterns")

    # 시퀀스 시각화 (옵션)
    # generator.visualize_sequence()

    return patterns


if __name__ == '__main__':
    patterns = demo()
