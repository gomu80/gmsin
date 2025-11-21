"""
Camera Control Module

카메라 제어 및 이미지 캡처
"""

import cv2
import numpy as np
from typing import List, Optional, Tuple
import time


class CameraController:
    """카메라 컨트롤러 클래스"""

    def __init__(self, camera_id: int = 0):
        """
        Args:
            camera_id: 카메라 장치 ID
        """
        self.camera_id = camera_id
        self.cap = None
        self.is_open = False

    def open(self) -> bool:
        """카메라 열기"""
        self.cap = cv2.VideoCapture(self.camera_id)
        if not self.cap.isOpened():
            print(f"Failed to open camera {self.camera_id}")
            return False

        self.is_open = True
        print(f"Camera {self.camera_id} opened successfully")
        return True

    def close(self) -> None:
        """카메라 닫기"""
        if self.cap is not None:
            self.cap.release()
            self.is_open = False
            print("Camera closed")

    def capture(self, delay: float = 0.0) -> Optional[np.ndarray]:
        """
        이미지 캡처

        Args:
            delay: 캡처 전 대기 시간 (초)

        Returns:
            캡처된 이미지 또는 None
        """
        if not self.is_open:
            print("Camera not opened")
            return None

        if delay > 0:
            time.sleep(delay)

        ret, frame = self.cap.read()
        if not ret:
            print("Failed to capture image")
            return None

        return frame

    def capture_sequence(self, n_images: int,
                        delay_between: float = 0.5) -> List[np.ndarray]:
        """
        연속 이미지 캡처

        Args:
            n_images: 캡처할 이미지 수
            delay_between: 이미지 간 대기 시간

        Returns:
            캡처된 이미지 리스트
        """
        images = []

        print(f"Capturing {n_images} images...")
        for i in range(n_images):
            img = self.capture(delay=delay_between)
            if img is not None:
                images.append(img)
                print(f"  Captured {i+1}/{n_images}")
            else:
                print(f"  Failed to capture {i+1}/{n_images}")

        return images

    def set_resolution(self, width: int, height: int) -> bool:
        """카메라 해상도 설정"""
        if not self.is_open:
            return False

        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

        actual_width = self.cap.get(cv2.CAP_PROP_FRAME_WIDTH)
        actual_height = self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT)

        print(f"Resolution set to {int(actual_width)}x{int(actual_height)}")
        return True

    def set_exposure(self, exposure: float) -> bool:
        """노출 설정"""
        if not self.is_open:
            return False

        self.cap.set(cv2.CAP_PROP_EXPOSURE, exposure)
        return True

    def set_auto_exposure(self, enabled: bool) -> bool:
        """자동 노출 설정"""
        if not self.is_open:
            return False

        self.cap.set(cv2.CAP_PROP_AUTO_EXPOSURE, 1 if enabled else 0)
        return True

    def get_properties(self) -> dict:
        """카메라 속성 정보"""
        if not self.is_open:
            return {}

        return {
            'width': int(self.cap.get(cv2.CAP_PROP_FRAME_WIDTH)),
            'height': int(self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT)),
            'fps': self.cap.get(cv2.CAP_PROP_FPS),
            'exposure': self.cap.get(cv2.CAP_PROP_EXPOSURE),
            'brightness': self.cap.get(cv2.CAP_PROP_BRIGHTNESS),
            'contrast': self.cap.get(cv2.CAP_PROP_CONTRAST)
        }

    def __enter__(self):
        """Context manager 진입"""
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager 종료"""
        self.close()
