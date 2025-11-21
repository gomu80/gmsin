"""
Phase Decoder

위상 시프트 패턴 디코딩 및 위상 언래핑
"""

import numpy as np
import cv2
from typing import List, Tuple


class PhaseDecoder:
    """Phase Shifting 디코더"""

    def __init__(self, pattern_generator):
        """
        Args:
            pattern_generator: PhaseShiftGenerator 인스턴스
        """
        self.generator = pattern_generator
        self.width = pattern_generator.width
        self.height = pattern_generator.height
        self.n_steps = pattern_generator.n_steps

    def decode(self, captured_images: List[np.ndarray]) -> dict:
        """
        위상 시프트 이미지 디코딩

        Args:
            captured_images: 캡처된 위상 시프트 이미지

        Returns:
            디코딩 결과 (wrapped_phase, unwrapped_phase, intensity, modulation)
        """
        if len(captured_images) < self.n_steps:
            raise ValueError(f"Need at least {self.n_steps} images")

        # 랩핑된 위상 계산
        wrapped_phase = self._compute_wrapped_phase(
            captured_images[:self.n_steps]
        )

        # 강도 및 변조도 계산
        intensity = self._compute_intensity(captured_images[:self.n_steps])
        modulation = self._compute_modulation(captured_images[:self.n_steps])

        # 위상 언래핑
        unwrapped_phase = self._unwrap_phase(wrapped_phase, modulation)

        return {
            'wrapped_phase': wrapped_phase,
            'unwrapped_phase': unwrapped_phase,
            'intensity': intensity,
            'modulation': modulation
        }

    def _compute_wrapped_phase(self, images: List[np.ndarray]) -> np.ndarray:
        """랩핑된 위상 계산"""
        height, width = images[0].shape[:2]
        n_steps = len(images)

        # Float로 변환
        images_float = []
        for img in images:
            if len(img.shape) == 3:
                img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            images_float.append(img.astype(np.float32))

        # N-step algorithm
        numerator = np.zeros((height, width), dtype=np.float32)
        denominator = np.zeros((height, width), dtype=np.float32)

        for k in range(n_steps):
            phase = 2.0 * np.pi * k / n_steps
            numerator += images_float[k] * np.sin(phase)
            denominator += images_float[k] * np.cos(phase)

        # 위상 계산 (-π ~ π)
        wrapped_phase = np.arctan2(-numerator, denominator)

        return wrapped_phase

    def _compute_intensity(self, images: List[np.ndarray]) -> np.ndarray:
        """평균 강도 계산"""
        images_gray = []
        for img in images:
            if len(img.shape) == 3:
                img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            images_gray.append(img.astype(np.float32))

        return np.mean(images_gray, axis=0)

    def _compute_modulation(self, images: List[np.ndarray]) -> np.ndarray:
        """변조도 계산"""
        images_gray = []
        for img in images:
            if len(img.shape) == 3:
                img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            images_gray.append(img.astype(np.float32))

        I_max = np.max(images_gray, axis=0)
        I_min = np.min(images_gray, axis=0)

        modulation = (I_max - I_min) / (I_max + I_min + 1e-6)
        return modulation

    def _unwrap_phase(self, wrapped_phase: np.ndarray,
                     quality_map: np.ndarray,
                     threshold: float = 0.3) -> np.ndarray:
        """
        위상 언래핑 (Quality-guided path following)

        Args:
            wrapped_phase: 랩핑된 위상
            quality_map: 품질 맵 (변조도)
            threshold: 품질 임계값

        Returns:
            언래핑된 위상
        """
        height, width = wrapped_phase.shape
        unwrapped = wrapped_phase.copy()
        visited = np.zeros((height, width), dtype=bool)

        # 품질이 높은 픽셀부터 처리
        mask = quality_map > threshold
        valid_pixels = np.argwhere(mask)

        if len(valid_pixels) == 0:
            return unwrapped

        # 중심 픽셀부터 시작
        start_pixel = valid_pixels[len(valid_pixels) // 2]
        queue = [tuple(start_pixel)]
        visited[tuple(start_pixel)] = True

        # BFS로 전파
        directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]

        while queue:
            y, x = queue.pop(0)

            for dy, dx in directions:
                ny, nx = y + dy, x + dx

                if (0 <= ny < height and 0 <= nx < width and
                    not visited[ny, nx] and mask[ny, nx]):

                    # 위상 차이 계산
                    phase_diff = wrapped_phase[ny, nx] - unwrapped[y, x]

                    # 2π 단위로 조정
                    k = np.round(phase_diff / (2 * np.pi))
                    unwrapped[ny, nx] = wrapped_phase[ny, nx] - k * 2 * np.pi

                    visited[ny, nx] = True
                    queue.append((ny, nx))

        return unwrapped

    def multi_frequency_unwrap(self,
                               captured_images_list: List[List[np.ndarray]],
                               frequencies: List[int]) -> np.ndarray:
        """
        다중 주파수 위상 언래핑

        Args:
            captured_images_list: 각 주파수별 캡처 이미지 리스트
            frequencies: 주파수 리스트

        Returns:
            언래핑된 절대 위상
        """
        # 각 주파수에 대해 랩핑된 위상 계산
        wrapped_phases = []
        for images in captured_images_list:
            phase = self._compute_wrapped_phase(images)
            wrapped_phases.append(phase)

        # 저주파부터 고주파로 언래핑
        absolute_phase = wrapped_phases[0]

        for i in range(1, len(frequencies)):
            ratio = frequencies[i] / frequencies[i-1]
            k = np.round((absolute_phase * ratio - wrapped_phases[i]) / (2 * np.pi))
            absolute_phase = (wrapped_phases[i] + k * 2 * np.pi) / ratio

        return absolute_phase
