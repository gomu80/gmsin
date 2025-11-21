"""
Gray Code Decoder

Gray Code 패턴 디코딩
"""

import numpy as np
import cv2
from typing import List, Tuple


class GrayCodeDecoder:
    """Gray Code 디코더"""

    def __init__(self, pattern_generator):
        """
        Args:
            pattern_generator: GrayCodeGenerator 인스턴스
        """
        self.generator = pattern_generator
        self.width = pattern_generator.width
        self.height = pattern_generator.height

    def decode(self, captured_images: List[np.ndarray]) -> np.ndarray:
        """
        캡처된 이미지로부터 Gray Code 디코딩

        Args:
            captured_images: 캡처된 패턴 이미지 리스트

        Returns:
            디코딩된 좌표 맵 (H, W, 2) - (x, y) 좌표
        """
        if len(captured_images) < 2:
            raise ValueError("Need at least 2 images for decoding")

        img_height, img_width = captured_images[0].shape[:2]
        coord_map = np.full((img_height, img_width, 2), -1, dtype=np.int32)

        # 역상 이미지 쌍으로 노이즈 제거
        if self.generator.inverse:
            # 수평 방향 디코딩
            if self.generator.direction in ['horizontal', 'both']:
                x_coords = self._decode_direction(
                    captured_images[:2*self.generator.n_bits_h],
                    self.generator.n_bits_h
                )
                coord_map[:, :, 0] = x_coords

            # 수직 방향 디코딩
            if self.generator.direction in ['vertical', 'both']:
                offset = 2 * self.generator.n_bits_h if self.generator.direction == 'both' else 0
                y_coords = self._decode_direction(
                    captured_images[offset:offset + 2*self.generator.n_bits_v],
                    self.generator.n_bits_v
                )
                coord_map[:, :, 1] = y_coords

        return coord_map

    def _decode_direction(self, images: List[np.ndarray],
                         n_bits: int) -> np.ndarray:
        """
        한 방향에 대한 디코딩

        Args:
            images: 패턴 이미지 리스트 (정상 + 역상)
            n_bits: 비트 수

        Returns:
            디코딩된 좌표
        """
        img_height, img_width = images[0].shape[:2]
        gray_code = np.zeros((img_height, img_width), dtype=np.int32)

        # 각 비트에 대해
        for bit in range(n_bits):
            # 역상 쌍을 이용한 노이즈 제거
            img_normal = images[2*bit].astype(np.float32)
            img_inverse = images[2*bit + 1].astype(np.float32)

            if len(img_normal.shape) == 3:
                img_normal = cv2.cvtColor(img_normal, cv2.COLOR_BGR2GRAY)
                img_inverse = cv2.cvtColor(img_inverse, cv2.COLOR_BGR2GRAY)

            # 차이를 이용한 이진화
            diff = img_normal - img_inverse
            bit_value = (diff > 0).astype(np.int32)

            # Gray code 구성
            gray_code |= (bit_value << bit)

        # Gray code를 이진수로 변환
        binary_code = self._gray_to_binary_array(gray_code, n_bits)

        return binary_code

    @staticmethod
    def _gray_to_binary_array(gray_array: np.ndarray,
                             n_bits: int) -> np.ndarray:
        """
        Gray code 배열을 이진수 배열로 변환

        Args:
            gray_array: Gray code 배열
            n_bits: 비트 수

        Returns:
            이진수 배열
        """
        binary_array = gray_array.copy()

        for i in range(n_bits - 1):
            binary_array ^= (binary_array >> 1)

        return binary_array

    def compute_confidence(self, captured_images: List[np.ndarray]) -> np.ndarray:
        """
        디코딩 신뢰도 계산

        Args:
            captured_images: 캡처된 이미지 리스트

        Returns:
            신뢰도 맵 (0~1)
        """
        if not self.generator.inverse:
            raise ValueError("Confidence requires inverse patterns")

        img_height, img_width = captured_images[0].shape[:2]
        confidence = np.ones((img_height, img_width), dtype=np.float32)

        n_pairs = len(captured_images) // 2

        for i in range(n_pairs):
            img_normal = captured_images[2*i].astype(np.float32)
            img_inverse = captured_images[2*i + 1].astype(np.float32)

            if len(img_normal.shape) == 3:
                img_normal = cv2.cvtColor(img_normal, cv2.COLOR_BGR2GRAY)
                img_inverse = cv2.cvtColor(img_inverse, cv2.COLOR_BGR2GRAY)

            # 차이의 절대값 (큰 값일수록 신뢰도 높음)
            diff = np.abs(img_normal - img_inverse)
            confidence = np.minimum(confidence, diff / 255.0)

        return confidence
