"""
Camera Calibration Module

카메라 내부 파라미터 캘리브레이션
- 초점 거리 (focal length)
- 주점 (principal point)
- 왜곡 계수 (distortion coefficients)
"""

import numpy as np
import cv2
import glob
import os
from typing import List, Tuple, Optional, Dict
import json


class CameraCalibration:
    """카메라 캘리브레이션 클래스"""

    def __init__(self, checkerboard_size: Tuple[int, int] = (9, 6),
                 square_size: float = 25.0):
        """
        Args:
            checkerboard_size: 체스보드 내부 코너 수 (가로, 세로)
            square_size: 체스보드 정사각형 크기 (mm)
        """
        self.checkerboard_size = checkerboard_size
        self.square_size = square_size

        # 캘리브레이션 결과
        self.camera_matrix = None
        self.dist_coeffs = None
        self.rvecs = None
        self.tvecs = None
        self.rms_error = None
        self.image_size = None

        # 3D 체스보드 포인트 준비
        self.objp = self._prepare_object_points()

    def _prepare_object_points(self) -> np.ndarray:
        """3D 체스보드 포인트 생성"""
        objp = np.zeros((self.checkerboard_size[0] * self.checkerboard_size[1], 3),
                       dtype=np.float32)
        objp[:, :2] = np.mgrid[0:self.checkerboard_size[0],
                               0:self.checkerboard_size[1]].T.reshape(-1, 2)
        objp *= self.square_size
        return objp

    def calibrate_from_images(self, image_paths: List[str],
                             flags: Optional[int] = None) -> bool:
        """
        이미지 파일로부터 캘리브레이션 수행

        Args:
            image_paths: 체스보드 이미지 경로 리스트
            flags: OpenCV 캘리브레이션 플래그

        Returns:
            성공 여부
        """
        if flags is None:
            flags = (cv2.CALIB_CB_ADAPTIVE_THRESH +
                    cv2.CALIB_CB_NORMALIZE_IMAGE +
                    cv2.CALIB_CB_FAST_CHECK)

        object_points = []  # 3D 포인트
        image_points = []   # 2D 포인트

        print(f"Processing {len(image_paths)} images...")

        for i, image_path in enumerate(image_paths):
            img = cv2.imread(image_path)
            if img is None:
                print(f"Failed to load image: {image_path}")
                continue

            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

            if self.image_size is None:
                self.image_size = gray.shape[::-1]

            # 체스보드 코너 찾기
            ret, corners = cv2.findChessboardCorners(
                gray, self.checkerboard_size, flags
            )

            if ret:
                object_points.append(self.objp)

                # 서브픽셀 정밀도로 코너 위치 개선
                criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER,
                           30, 0.001)
                corners_refined = cv2.cornerSubPix(
                    gray, corners, (11, 11), (-1, -1), criteria
                )
                image_points.append(corners_refined)

                print(f"  [{i+1}/{len(image_paths)}] Found corners: {image_path}")
            else:
                print(f"  [{i+1}/{len(image_paths)}] No corners found: {image_path}")

        if len(object_points) < 3:
            print(f"Error: Need at least 3 valid images, got {len(object_points)}")
            return False

        print(f"\nCalibrating with {len(object_points)} images...")

        # 카메라 캘리브레이션 수행
        ret, self.camera_matrix, self.dist_coeffs, self.rvecs, self.tvecs = \
            cv2.calibrateCamera(
                object_points, image_points, self.image_size,
                None, None
            )

        self.rms_error = ret

        print(f"Calibration complete! RMS error: {self.rms_error:.4f}")
        return True

    def calibrate_from_directory(self, directory: str,
                                pattern: str = "*.png") -> bool:
        """
        디렉토리에서 이미지를 읽어 캘리브레이션

        Args:
            directory: 이미지 디렉토리
            pattern: 파일 패턴

        Returns:
            성공 여부
        """
        search_path = os.path.join(directory, pattern)
        image_paths = glob.glob(search_path)

        if not image_paths:
            print(f"No images found in {search_path}")
            return False

        return self.calibrate_from_images(image_paths)

    def undistort_image(self, image: np.ndarray) -> np.ndarray:
        """
        왜곡 보정

        Args:
            image: 입력 이미지

        Returns:
            왜곡 보정된 이미지
        """
        if self.camera_matrix is None or self.dist_coeffs is None:
            raise ValueError("Camera not calibrated. Run calibration first.")

        return cv2.undistort(image, self.camera_matrix, self.dist_coeffs)

    def get_optimal_new_camera_matrix(self, alpha: float = 1.0) -> np.ndarray:
        """
        최적화된 새 카메라 매트릭스 계산

        Args:
            alpha: 0 = 유효 픽셀만, 1 = 모든 픽셀 포함

        Returns:
            최적화된 카메라 매트릭스
        """
        if self.camera_matrix is None or self.image_size is None:
            raise ValueError("Camera not calibrated.")

        new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(
            self.camera_matrix, self.dist_coeffs, self.image_size, alpha
        )

        return new_camera_matrix

    def save_calibration(self, filepath: str) -> None:
        """
        캘리브레이션 결과 저장

        Args:
            filepath: 저장 경로 (JSON)
        """
        if self.camera_matrix is None:
            raise ValueError("No calibration data to save.")

        data = {
            'camera_matrix': self.camera_matrix.tolist(),
            'dist_coeffs': self.dist_coeffs.tolist(),
            'rms_error': float(self.rms_error),
            'image_size': self.image_size,
            'checkerboard_size': self.checkerboard_size,
            'square_size': float(self.square_size)
        }

        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)

        print(f"Calibration saved to {filepath}")

    def load_calibration(self, filepath: str) -> bool:
        """
        캘리브레이션 결과 로드

        Args:
            filepath: 로드 경로 (JSON)

        Returns:
            성공 여부
        """
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)

            self.camera_matrix = np.array(data['camera_matrix'])
            self.dist_coeffs = np.array(data['dist_coeffs'])
            self.rms_error = data['rms_error']
            self.image_size = tuple(data['image_size'])
            self.checkerboard_size = tuple(data['checkerboard_size'])
            self.square_size = data['square_size']

            print(f"Calibration loaded from {filepath}")
            return True

        except Exception as e:
            print(f"Failed to load calibration: {e}")
            return False

    def print_calibration(self) -> None:
        """캘리브레이션 결과 출력"""
        if self.camera_matrix is None:
            print("Camera not calibrated.")
            return

        print("\n" + "=" * 60)
        print("Camera Calibration Results")
        print("=" * 60)
        print(f"Image size: {self.image_size}")
        print(f"RMS error: {self.rms_error:.4f}")
        print(f"\nCamera Matrix (K):")
        print(self.camera_matrix)
        print(f"\nDistortion Coefficients:")
        print(self.dist_coeffs.ravel())

        # 주요 파라미터 추출
        fx = self.camera_matrix[0, 0]
        fy = self.camera_matrix[1, 1]
        cx = self.camera_matrix[0, 2]
        cy = self.camera_matrix[1, 2]

        print(f"\nFocal Length:")
        print(f"  fx = {fx:.2f} pixels")
        print(f"  fy = {fy:.2f} pixels")
        print(f"\nPrincipal Point:")
        print(f"  cx = {cx:.2f} pixels")
        print(f"  cy = {cy:.2f} pixels")
        print("=" * 60 + "\n")

    def compute_reprojection_error(self, object_points: List[np.ndarray],
                                   image_points: List[np.ndarray]) -> float:
        """
        재투영 오차 계산

        Args:
            object_points: 3D 포인트 리스트
            image_points: 2D 포인트 리스트

        Returns:
            평균 재투영 오차
        """
        if self.camera_matrix is None:
            raise ValueError("Camera not calibrated.")

        total_error = 0
        total_points = 0

        for i in range(len(object_points)):
            # 3D 포인트를 이미지로 재투영
            imgpoints2, _ = cv2.projectPoints(
                object_points[i], self.rvecs[i], self.tvecs[i],
                self.camera_matrix, self.dist_coeffs
            )

            # 오차 계산
            error = cv2.norm(image_points[i], imgpoints2, cv2.NORM_L2)
            total_error += error ** 2
            total_points += len(object_points[i])

        mean_error = np.sqrt(total_error / total_points)
        return mean_error


def demo():
    """카메라 캘리브레이션 데모"""
    calibrator = CameraCalibration(checkerboard_size=(9, 6), square_size=25.0)

    print("Camera Calibration Demo")
    print("=" * 60)
    print("To perform calibration:")
    print("1. Print a checkerboard pattern")
    print("2. Capture 10-20 images from different angles")
    print("3. Place images in 'data/calibration/camera/' directory")
    print("4. Run calibration")
    print("=" * 60)

    # 캘리브레이션 수행 (이미지가 있는 경우)
    # success = calibrator.calibrate_from_directory('data/calibration/camera/')
    #
    # if success:
    #     calibrator.print_calibration()
    #     calibrator.save_calibration('data/calibration/camera_calibration.json')

    return calibrator


if __name__ == '__main__':
    calibrator = demo()
