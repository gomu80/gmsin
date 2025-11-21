"""
Stereo Calibration Module

카메라-프로젝터 스테레오 시스템 캘리브레이션
상대적 위치 및 자세 추정
"""

import numpy as np
import cv2
from typing import List, Tuple, Optional
import json
import os


class StereoCalibration:
    """스테레오 캘리브레이션 클래스"""

    def __init__(self):
        """초기화"""
        self.R = None  # Rotation matrix
        self.T = None  # Translation vector
        self.E = None  # Essential matrix
        self.F = None  # Fundamental matrix
        self.rms_error = None

        # 개별 캘리브레이션 데이터
        self.camera_matrix = None
        self.camera_dist = None
        self.projector_matrix = None
        self.projector_dist = None

    def calibrate(self,
                 object_points: List[np.ndarray],
                 camera_points: List[np.ndarray],
                 projector_points: List[np.ndarray],
                 camera_matrix: np.ndarray,
                 camera_dist: np.ndarray,
                 projector_matrix: np.ndarray,
                 projector_dist: np.ndarray,
                 image_size: Tuple[int, int]) -> bool:
        """
        스테레오 캘리브레이션 수행

        Args:
            object_points: 3D 월드 포인트
            camera_points: 카메라 이미지 포인트
            projector_points: 프로젝터 이미지 포인트
            camera_matrix: 카메라 매트릭스
            camera_dist: 카메라 왜곡 계수
            projector_matrix: 프로젝터 매트릭스
            projector_dist: 프로젝터 왜곡 계수
            image_size: 이미지 크기

        Returns:
            성공 여부
        """
        self.camera_matrix = camera_matrix
        self.camera_dist = camera_dist
        self.projector_matrix = projector_matrix
        self.projector_dist = projector_dist

        # 스테레오 캘리브레이션
        flags = cv2.CALIB_FIX_INTRINSIC

        ret, _, _, _, _, self.R, self.T, self.E, self.F = \
            cv2.stereoCalibrate(
                object_points,
                camera_points,
                projector_points,
                camera_matrix, camera_dist,
                projector_matrix, projector_dist,
                image_size,
                flags=flags,
                criteria=(cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER,
                         30, 1e-6)
            )

        self.rms_error = ret

        print(f"Stereo calibration complete! RMS error: {self.rms_error:.4f}")
        return True

    def get_parameters(self) -> dict:
        """캘리브레이션 파라미터 반환"""
        if self.R is None:
            raise ValueError("Stereo system not calibrated.")

        return {
            'camera_matrix': self.camera_matrix,
            'camera_dist': self.camera_dist,
            'projector_matrix': self.projector_matrix,
            'projector_dist': self.projector_dist,
            'R': self.R,
            'T': self.T,
            'E': self.E,
            'F': self.F
        }

    def triangulate_points(self,
                          camera_points: np.ndarray,
                          projector_points: np.ndarray) -> np.ndarray:
        """
        스테레오 삼각측량으로 3D 포인트 계산

        Args:
            camera_points: 카메라 이미지 포인트 (Nx2)
            projector_points: 프로젝터 이미지 포인트 (Nx2)

        Returns:
            3D 포인트 (Nx3)
        """
        if self.R is None:
            raise ValueError("Stereo system not calibrated.")

        # Projection matrices
        P1 = np.hstack([self.camera_matrix, np.zeros((3, 1))])
        P2 = self.projector_matrix @ np.hstack([self.R, self.T])

        # 삼각측량
        points_4d = cv2.triangulatePoints(
            P1, P2,
            camera_points.T, projector_points.T
        )

        # 정규화
        points_3d = points_4d[:3, :] / points_4d[3, :]
        return points_3d.T

    def compute_rectification(self, image_size: Tuple[int, int]) -> dict:
        """
        스테레오 정류 (rectification) 계산

        Args:
            image_size: 이미지 크기

        Returns:
            정류 파라미터
        """
        if self.R is None:
            raise ValueError("Stereo system not calibrated.")

        R1, R2, P1, P2, Q, roi1, roi2 = cv2.stereoRectify(
            self.camera_matrix, self.camera_dist,
            self.projector_matrix, self.projector_dist,
            image_size, self.R, self.T,
            alpha=0
        )

        return {
            'R1': R1, 'R2': R2,
            'P1': P1, 'P2': P2,
            'Q': Q,
            'roi1': roi1, 'roi2': roi2
        }

    def save_calibration(self, filepath: str) -> None:
        """캘리브레이션 결과 저장"""
        if self.R is None:
            raise ValueError("No calibration data to save.")

        data = {
            'camera_matrix': self.camera_matrix.tolist(),
            'camera_dist': self.camera_dist.tolist(),
            'projector_matrix': self.projector_matrix.tolist(),
            'projector_dist': self.projector_dist.tolist(),
            'R': self.R.tolist(),
            'T': self.T.tolist(),
            'E': self.E.tolist(),
            'F': self.F.tolist(),
            'rms_error': float(self.rms_error)
        }

        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)

        print(f"Stereo calibration saved to {filepath}")

    def load_calibration(self, filepath: str) -> bool:
        """캘리브레이션 결과 로드"""
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)

            self.camera_matrix = np.array(data['camera_matrix'])
            self.camera_dist = np.array(data['camera_dist'])
            self.projector_matrix = np.array(data['projector_matrix'])
            self.projector_dist = np.array(data['projector_dist'])
            self.R = np.array(data['R'])
            self.T = np.array(data['T'])
            self.E = np.array(data['E'])
            self.F = np.array(data['F'])
            self.rms_error = data['rms_error']

            print(f"Stereo calibration loaded from {filepath}")
            return True

        except Exception as e:
            print(f"Failed to load calibration: {e}")
            return False

    def print_calibration(self) -> None:
        """캘리브레이션 결과 출력"""
        if self.R is None:
            print("Stereo system not calibrated.")
            return

        print("\n" + "=" * 60)
        print("Stereo Calibration Results")
        print("=" * 60)
        print(f"RMS error: {self.rms_error:.4f}")
        print(f"\nRotation Matrix (R):")
        print(self.R)
        print(f"\nTranslation Vector (T):")
        print(self.T.ravel())
        print(f"\nBaseline: {np.linalg.norm(self.T):.2f} mm")
        print("=" * 60 + "\n")


def demo():
    """스테레오 캘리브레이션 데모"""
    calibrator = StereoCalibration()

    print("Stereo Calibration Demo")
    print("=" * 60)
    print("Stereo calibration requires:")
    print("1. Calibrated camera")
    print("2. Calibrated projector")
    print("3. Corresponding point pairs")
    print("=" * 60)

    return calibrator


if __name__ == '__main__':
    calibrator = demo()
