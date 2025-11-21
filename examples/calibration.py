#!/usr/bin/env python3
"""
Calibration Example

카메라 및 프로젝터 캘리브레이션 예제
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'src'))

from structured_light.calibration import (
    CameraCalibration,
    ProjectorCalibration,
    StereoCalibration
)


def demo_camera_calibration():
    """카메라 캘리브레이션 예제"""
    print("\n" + "="*60)
    print("Camera Calibration")
    print("="*60)

    calibrator = CameraCalibration(
        checkerboard_size=(9, 6),
        square_size=25.0  # mm
    )

    print("\nInstructions:")
    print("1. Print a checkerboard pattern")
    print("2. Capture 10-20 images from different angles")
    print("3. Save images in: data/calibration/camera/")
    print("4. Run calibration\n")

    # 캘리브레이션 수행 (이미지가 있는 경우)
    calibration_dir = 'data/calibration/camera'
    if os.path.exists(calibration_dir):
        success = calibrator.calibrate_from_directory(calibration_dir)

        if success:
            calibrator.print_calibration()

            # 결과 저장
            calibrator.save_calibration('data/calibration/camera_calibration.json')
            print("Calibration saved!")

        return calibrator
    else:
        print(f"Directory not found: {calibration_dir}")
        print("Skipping actual calibration")
        return None


def demo_projector_calibration():
    """프로젝터 캘리브레이션 예제"""
    print("\n" + "="*60)
    print("Projector Calibration")
    print("="*60)

    calibrator = ProjectorCalibration(
        projector_size=(1920, 1080),
        checkerboard_size=(9, 6),
        square_size=25.0
    )

    print("\nInstructions:")
    print("1. Project Gray code patterns")
    print("2. Capture images with checkerboard")
    print("3. Establish camera-projector correspondences")
    print("4. Run calibration\n")

    # 실제 캘리브레이션은 Gray Code 패턴과
    # 카메라-프로젝터 대응점이 필요합니다

    return calibrator


def demo_stereo_calibration():
    """스테레오 캘리브레이션 예제"""
    print("\n" + "="*60)
    print("Stereo Calibration")
    print("="*60)

    calibrator = StereoCalibration()

    print("\nInstructions:")
    print("1. Complete camera calibration")
    print("2. Complete projector calibration")
    print("3. Capture synchronized images")
    print("4. Run stereo calibration\n")

    # 스테레오 캘리브레이션은 개별 캘리브레이션과
    # 대응점이 필요합니다

    return calibrator


def main():
    """메인 함수"""
    print("Calibration Examples")
    print("="*60)

    # 카메라 캘리브레이션
    camera_calib = demo_camera_calibration()

    # 프로젝터 캘리브레이션
    projector_calib = demo_projector_calibration()

    # 스테레오 캘리브레이션
    stereo_calib = demo_stereo_calibration()

    print("\n" + "="*60)
    print("Calibration examples complete!")
    print("="*60)


if __name__ == '__main__':
    main()
