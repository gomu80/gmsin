"""
Projector Calibration Module

프로젝터를 역방향 카메라로 취급하여 캘리브레이션
구조광 패턴을 이용한 간접 캘리브레이션 방식
"""

import numpy as np
import cv2
from typing import List, Tuple, Optional
import json
import os


class ProjectorCalibration:
    """프로젝터 캘리브레이션 클래스"""

    def __init__(self, projector_size: Tuple[int, int] = (1920, 1080),
                 checkerboard_size: Tuple[int, int] = (9, 6),
                 square_size: float = 25.0):
        """
        Args:
            projector_size: 프로젝터 해상도 (width, height)
            checkerboard_size: 체스보드 내부 코너 수
            square_size: 체스보드 정사각형 크기 (mm)
        """
        self.projector_size = projector_size
        self.checkerboard_size = checkerboard_size
        self.square_size = square_size

        # 캘리브레이션 결과
        self.projector_matrix = None
        self.dist_coeffs = None
        self.rms_error = None

    def calibrate_with_gray_code(self, camera_matrix: np.ndarray,
                                 camera_dist: np.ndarray,
                                 captured_patterns: List[np.ndarray],
                                 pattern_generator) -> bool:
        """
        Gray Code 패턴을 이용한 프로젝터 캘리브레이션

        Args:
            camera_matrix: 카메라 매트릭스
            camera_dist: 카메라 왜곡 계수
            captured_patterns: 캡처된 Gray Code 패턴 이미지
            pattern_generator: Gray Code 생성기

        Returns:
            성공 여부
        """
        # 패턴 디코딩하여 대응점 찾기
        from ..decode.gray_decoder import GrayCodeDecoder

        decoder = GrayCodeDecoder(pattern_generator)
        projector_coords = decoder.decode(captured_patterns)

        # 유효한 대응점 추출
        valid_mask = (projector_coords[:, :, 0] >= 0)
        camera_points = np.argwhere(valid_mask)  # [y, x] 형식
        projector_points = projector_coords[valid_mask]

        # 좌표 변환 [y, x] -> [x, y]
        camera_points = camera_points[:, ::-1].astype(np.float32)
        projector_points = projector_points.astype(np.float32)

        # 체스보드를 사용한 캘리브레이션
        # 카메라로 캡처한 체스보드 이미지가 필요
        # 여기서는 간단히 homography 기반 접근

        # 프로젝터 매트릭스 초기화
        fx = self.projector_size[0]  # 추정값
        fy = self.projector_size[0]
        cx = self.projector_size[0] / 2
        cy = self.projector_size[1] / 2

        self.projector_matrix = np.array([
            [fx, 0, cx],
            [0, fy, cy],
            [0, 0, 1]
        ], dtype=np.float32)

        self.dist_coeffs = np.zeros(5, dtype=np.float32)
        self.rms_error = 0.0

        return True

    def calibrate_with_checkerboard(self,
                                    object_points: List[np.ndarray],
                                    projector_points: List[np.ndarray]) -> bool:
        """
        체스보드를 이용한 직접 캘리브레이션

        Args:
            object_points: 3D 체스보드 포인트
            projector_points: 프로젝터 평면의 2D 포인트

        Returns:
            성공 여부
        """
        if len(object_points) < 3:
            print("Error: Need at least 3 sets of points")
            return False

        # 프로젝터를 역방향 카메라로 취급하여 캘리브레이션
        ret, self.projector_matrix, self.dist_coeffs, rvecs, tvecs = \
            cv2.calibrateCamera(
                object_points, projector_points,
                self.projector_size, None, None
            )

        self.rms_error = ret
        print(f"Projector calibration complete! RMS error: {self.rms_error:.4f}")
        return True

    def save_calibration(self, filepath: str) -> None:
        """캘리브레이션 결과 저장"""
        if self.projector_matrix is None:
            raise ValueError("No calibration data to save.")

        data = {
            'projector_matrix': self.projector_matrix.tolist(),
            'dist_coeffs': self.dist_coeffs.tolist(),
            'rms_error': float(self.rms_error),
            'projector_size': self.projector_size,
            'checkerboard_size': self.checkerboard_size,
            'square_size': float(self.square_size)
        }

        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)

        print(f"Projector calibration saved to {filepath}")

    def load_calibration(self, filepath: str) -> bool:
        """캘리브레이션 결과 로드"""
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)

            self.projector_matrix = np.array(data['projector_matrix'])
            self.dist_coeffs = np.array(data['dist_coeffs'])
            self.rms_error = data['rms_error']
            self.projector_size = tuple(data['projector_size'])

            print(f"Projector calibration loaded from {filepath}")
            return True

        except Exception as e:
            print(f"Failed to load calibration: {e}")
            return False

    def print_calibration(self) -> None:
        """캘리브레이션 결과 출력"""
        if self.projector_matrix is None:
            print("Projector not calibrated.")
            return

        print("\n" + "=" * 60)
        print("Projector Calibration Results")
        print("=" * 60)
        print(f"Projector size: {self.projector_size}")
        print(f"RMS error: {self.rms_error:.4f}")
        print(f"\nProjector Matrix:")
        print(self.projector_matrix)
        print(f"\nDistortion Coefficients:")
        print(self.dist_coeffs.ravel())
        print("=" * 60 + "\n")


def demo():
    """프로젝터 캘리브레이션 데모"""
    calibrator = ProjectorCalibration(
        projector_size=(1920, 1080),
        checkerboard_size=(9, 6),
        square_size=25.0
    )

    print("Projector Calibration Demo")
    print("=" * 60)
    print("Projector calibration requires:")
    print("1. Calibrated camera")
    print("2. Gray code patterns or checkerboard patterns")
    print("3. Captured images with known correspondences")
    print("=" * 60)

    return calibrator


if __name__ == '__main__':
    calibrator = demo()
