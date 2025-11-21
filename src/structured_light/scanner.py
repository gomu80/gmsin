"""
Structured Light Scanner

구조광 3D 스캐닝의 메인 파이프라인
"""

import numpy as np
import cv2
from typing import Optional, List, Tuple
import os

from .pattern import GrayCodeGenerator, PhaseShiftGenerator
from .calibration import CameraCalibration, ProjectorCalibration, StereoCalibration
from .capture import CameraController
from .decode import GrayCodeDecoder, PhaseDecoder
from .reconstruction import Triangulation, PointCloudProcessor


class StructuredLightScanner:
    """구조광 스캐너 메인 클래스"""

    def __init__(self, config: Optional[dict] = None):
        """
        Args:
            config: 설정 딕셔너리
        """
        # 기본 설정
        self.config = {
            'projector_width': 1920,
            'projector_height': 1080,
            'camera_id': 0,
            'pattern_type': 'gray_code',  # 'gray_code', 'phase_shift', 'hybrid'
            'checkerboard_size': (9, 6),
            'square_size': 25.0,
        }

        if config:
            self.config.update(config)

        # 컴포넌트
        self.pattern_generator = None
        self.camera = None
        self.camera_calibration = None
        self.projector_calibration = None
        self.stereo_calibration = None

        # 데이터
        self.captured_images = []
        self.point_cloud = None

    def initialize(self) -> bool:
        """시스템 초기화"""
        print("Initializing Structured Light Scanner...")

        # 카메라 초기화
        self.camera = CameraController(self.config['camera_id'])
        if not self.camera.open():
            return False

        # 패턴 생성기 초기화
        if self.config['pattern_type'] == 'gray_code':
            self.pattern_generator = GrayCodeGenerator(
                width=self.config['projector_width'],
                height=self.config['projector_height'],
                direction='both',
                inverse=True
            )
        elif self.config['pattern_type'] == 'phase_shift':
            self.pattern_generator = PhaseShiftGenerator(
                width=self.config['projector_width'],
                height=self.config['projector_height'],
                n_steps=4,
                n_periods=64,
                direction='both'
            )

        print("Initialization complete!")
        return True

    def calibrate(self,
                 calibration_images_dir: Optional[str] = None,
                 save_dir: str = 'data/calibration') -> bool:
        """
        시스템 캘리브레이션

        Args:
            calibration_images_dir: 캘리브레이션 이미지 디렉토리
            save_dir: 결과 저장 디렉토리

        Returns:
            성공 여부
        """
        print("\n" + "="*60)
        print("Starting Calibration")
        print("="*60)

        # 카메라 캘리브레이션
        print("\n1. Camera Calibration")
        self.camera_calibration = CameraCalibration(
            checkerboard_size=self.config['checkerboard_size'],
            square_size=self.config['square_size']
        )

        if calibration_images_dir:
            camera_dir = os.path.join(calibration_images_dir, 'camera')
            if os.path.exists(camera_dir):
                success = self.camera_calibration.calibrate_from_directory(camera_dir)
                if success:
                    self.camera_calibration.print_calibration()
                    self.camera_calibration.save_calibration(
                        os.path.join(save_dir, 'camera_calibration.json')
                    )
            else:
                print(f"Directory not found: {camera_dir}")
                print("Skipping camera calibration")

        # 프로젝터 캘리브레이션
        print("\n2. Projector Calibration")
        self.projector_calibration = ProjectorCalibration(
            projector_size=(self.config['projector_width'],
                          self.config['projector_height']),
            checkerboard_size=self.config['checkerboard_size'],
            square_size=self.config['square_size']
        )

        # 스테레오 캘리브레이션
        print("\n3. Stereo Calibration")
        self.stereo_calibration = StereoCalibration()

        print("\nCalibration complete!")
        print("="*60)
        return True

    def load_calibration(self, calibration_dir: str = 'data/calibration') -> bool:
        """
        저장된 캘리브레이션 로드

        Args:
            calibration_dir: 캘리브레이션 파일 디렉토리

        Returns:
            성공 여부
        """
        print("Loading calibration...")

        # 카메라 캘리브레이션 로드
        self.camera_calibration = CameraCalibration()
        camera_file = os.path.join(calibration_dir, 'camera_calibration.json')
        if os.path.exists(camera_file):
            self.camera_calibration.load_calibration(camera_file)
        else:
            print(f"Camera calibration file not found: {camera_file}")

        # 프로젝터 캘리브레이션 로드
        self.projector_calibration = ProjectorCalibration()
        projector_file = os.path.join(calibration_dir, 'projector_calibration.json')
        if os.path.exists(projector_file):
            self.projector_calibration.load_calibration(projector_file)

        # 스테레오 캘리브레이션 로드
        self.stereo_calibration = StereoCalibration()
        stereo_file = os.path.join(calibration_dir, 'stereo_calibration.json')
        if os.path.exists(stereo_file):
            self.stereo_calibration.load_calibration(stereo_file)

        return True

    def scan(self, pattern_type: Optional[str] = None) -> Optional[np.ndarray]:
        """
        3D 스캔 수행

        Args:
            pattern_type: 패턴 타입 ('gray_code', 'phase_shift')

        Returns:
            포인트 클라우드 (N, 6) [x, y, z, r, g, b]
        """
        if pattern_type:
            self.config['pattern_type'] = pattern_type

        print("\n" + "="*60)
        print("Starting 3D Scan")
        print(f"Pattern type: {self.config['pattern_type']}")
        print("="*60)

        # 패턴 생성
        print("\n1. Generating patterns...")
        patterns = self.pattern_generator.generate()
        print(f"Generated {len(patterns)} patterns")

        # 패턴 투영 및 캡처
        print("\n2. Capturing images...")
        print("Please project patterns and capture images manually")
        print("This is a simulation - in real use, patterns would be projected")

        # 실제 시스템에서는 여기서 패턴을 프로젝터로 투영하고
        # 각 패턴마다 카메라로 이미지를 캡처합니다

        # 패턴 디코딩
        print("\n3. Decoding patterns...")
        if self.config['pattern_type'] == 'gray_code':
            decoder = GrayCodeDecoder(self.pattern_generator)
            # decoded_coords = decoder.decode(self.captured_images)
            print("Gray code decoding (simulation)")
        elif self.config['pattern_type'] == 'phase_shift':
            decoder = PhaseDecoder(self.pattern_generator)
            # result = decoder.decode(self.captured_images)
            print("Phase shift decoding (simulation)")

        # 3D 재구성
        print("\n4. 3D Reconstruction...")
        if self.stereo_calibration and self.stereo_calibration.R is not None:
            triangulator = Triangulation(self.stereo_calibration.get_parameters())
            # self.point_cloud = triangulator.reconstruct_from_correspondence(...)
            print("3D reconstruction (simulation)")
        else:
            print("Warning: Stereo calibration not available")

        print("\nScan complete!")
        print("="*60)

        return self.point_cloud

    def save_point_cloud(self, filepath: str, format: str = 'ply') -> bool:
        """
        포인트 클라우드 저장

        Args:
            filepath: 저장 경로
            format: 파일 형식 ('ply', 'xyz')

        Returns:
            성공 여부
        """
        if self.point_cloud is None:
            print("No point cloud to save")
            return False

        processor = PointCloudProcessor()
        processor.set_point_cloud(self.point_cloud)

        if format == 'ply':
            return processor.save_ply(filepath)
        elif format == 'xyz':
            return processor.save_xyz(filepath)
        else:
            print(f"Unknown format: {format}")
            return False

    def visualize_point_cloud(self):
        """포인트 클라우드 시각화"""
        if self.point_cloud is None:
            print("No point cloud to visualize")
            return

        processor = PointCloudProcessor()
        processor.set_point_cloud(self.point_cloud)
        processor.visualize()

    def cleanup(self):
        """리소스 정리"""
        if self.camera:
            self.camera.close()
        print("Cleanup complete")

    def __enter__(self):
        """Context manager 진입"""
        self.initialize()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager 종료"""
        self.cleanup()


def main():
    """메인 데모"""
    print("Structured Light Scanner Demo")
    print("="*60)

    # 스캐너 생성
    scanner = StructuredLightScanner()

    # 초기화
    scanner.initialize()

    # 캘리브레이션 (옵션)
    # scanner.calibrate(calibration_images_dir='data/calibration')

    # 또는 저장된 캘리브레이션 로드
    # scanner.load_calibration('data/calibration')

    # 스캔 수행
    # point_cloud = scanner.scan(pattern_type='gray_code')

    # 결과 저장
    # scanner.save_point_cloud('data/results/scan_result.ply')

    # 시각화
    # scanner.visualize_point_cloud()

    # 정리
    scanner.cleanup()


if __name__ == '__main__':
    main()
