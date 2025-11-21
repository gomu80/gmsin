#!/usr/bin/env python3
"""
Basic Scanning Example

구조광 스캐닝 기본 예제
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'src'))

from structured_light import StructuredLightScanner


def main():
    """기본 스캔 예제"""
    print("Structured Light - Basic Scan Example")
    print("="*60)

    # 설정
    config = {
        'projector_width': 1920,
        'projector_height': 1080,
        'camera_id': 0,
        'pattern_type': 'gray_code',
        'checkerboard_size': (9, 6),
        'square_size': 25.0,
    }

    # 스캐너 초기화
    scanner = StructuredLightScanner(config)
    scanner.initialize()

    # 캘리브레이션 로드 (선택)
    # scanner.load_calibration('data/calibration')

    # 스캔 수행
    print("\nPerforming scan...")
    point_cloud = scanner.scan(pattern_type='gray_code')

    # 결과 저장
    if point_cloud is not None:
        output_path = 'data/results/basic_scan.ply'
        scanner.save_point_cloud(output_path, format='ply')
        print(f"\nPoint cloud saved to: {output_path}")

        # 시각화 (Open3D 필요)
        # scanner.visualize_point_cloud()

    # 정리
    scanner.cleanup()

    print("\nScan complete!")


if __name__ == '__main__':
    main()
