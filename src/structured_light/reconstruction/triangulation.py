"""
Triangulation Module

스테레오 삼각측량으로 3D 좌표 계산
"""

import numpy as np
import cv2
from typing import Tuple, Optional


class Triangulation:
    """삼각측량 클래스"""

    def __init__(self, stereo_params: dict):
        """
        Args:
            stereo_params: 스테레오 캘리브레이션 파라미터
        """
        self.camera_matrix = stereo_params['camera_matrix']
        self.camera_dist = stereo_params['camera_dist']
        self.projector_matrix = stereo_params['projector_matrix']
        self.projector_dist = stereo_params['projector_dist']
        self.R = stereo_params['R']
        self.T = stereo_params['T']

        # Projection matrices
        self.P1 = np.hstack([self.camera_matrix, np.zeros((3, 1))])
        self.P2 = self.projector_matrix @ np.hstack([self.R, self.T])

    def reconstruct_from_correspondence(self,
                                       camera_coords: np.ndarray,
                                       projector_coords: np.ndarray,
                                       confidence: Optional[np.ndarray] = None,
                                       threshold: float = 0.3) -> np.ndarray:
        """
        대응점으로부터 3D 재구성

        Args:
            camera_coords: 카메라 좌표 (H, W, 2)
            projector_coords: 프로젝터 좌표 (H, W, 2)
            confidence: 신뢰도 맵 (H, W)
            threshold: 신뢰도 임계값

        Returns:
            3D 포인트 클라우드 (N, 6) - [x, y, z, r, g, b]
        """
        height, width = camera_coords.shape[:2]

        # 유효한 픽셀 마스크
        valid_mask = (projector_coords[:, :, 0] >= 0)
        if confidence is not None:
            valid_mask &= (confidence > threshold)

        # 유효한 포인트 추출
        valid_pixels = np.argwhere(valid_mask)
        n_points = len(valid_pixels)

        if n_points == 0:
            return np.array([])

        # 카메라 및 프로젝터 좌표
        cam_points = valid_pixels[:, ::-1].astype(np.float32)  # [x, y]
        proj_points = projector_coords[valid_mask].astype(np.float32)

        # 삼각측량
        points_3d = self._triangulate_points(cam_points, proj_points)

        # 색상 정보 추가 (카메라 이미지에서)
        # 여기서는 기본값 사용
        colors = np.ones((n_points, 3), dtype=np.uint8) * 128

        # 포인트 클라우드 생성
        point_cloud = np.hstack([points_3d, colors])

        return point_cloud

    def _triangulate_points(self,
                           camera_points: np.ndarray,
                           projector_points: np.ndarray) -> np.ndarray:
        """
        삼각측량으로 3D 포인트 계산

        Args:
            camera_points: 카메라 이미지 포인트 (N, 2)
            projector_points: 프로젝터 이미지 포인트 (N, 2)

        Returns:
            3D 포인트 (N, 3)
        """
        # OpenCV triangulatePoints 사용
        points_4d = cv2.triangulatePoints(
            self.P1, self.P2,
            camera_points.T, projector_points.T
        )

        # 정규화
        points_3d = points_4d[:3, :] / points_4d[3, :]
        return points_3d.T

    def reconstruct_from_phase(self,
                              phase_map: np.ndarray,
                              intensity: Optional[np.ndarray] = None,
                              modulation: Optional[np.ndarray] = None,
                              threshold: float = 0.3) -> np.ndarray:
        """
        위상 맵으로부터 3D 재구성

        Args:
            phase_map: 언래핑된 위상 맵
            intensity: 강도 맵
            modulation: 변조도 맵
            threshold: 변조도 임계값

        Returns:
            3D 포인트 클라우드
        """
        height, width = phase_map.shape

        # 위상을 프로젝터 좌표로 변환
        # phase = 2π * n_periods * x / width
        n_periods = 64  # pattern_generator에서 가져와야 함
        projector_x = (phase_map / (2 * np.pi)) * (width / n_periods)

        # 프로젝터 좌표 생성
        projector_coords = np.zeros((height, width, 2), dtype=np.float32)
        projector_coords[:, :, 0] = projector_x
        projector_coords[:, :, 1] = np.arange(height)[:, np.newaxis]

        # 카메라 좌표
        camera_coords = np.zeros((height, width, 2), dtype=np.float32)
        y_grid, x_grid = np.mgrid[0:height, 0:width]
        camera_coords[:, :, 0] = x_grid
        camera_coords[:, :, 1] = y_grid

        # 재구성
        return self.reconstruct_from_correspondence(
            camera_coords, projector_coords, modulation, threshold
        )

    def compute_depth_map(self,
                         camera_coords: np.ndarray,
                         projector_coords: np.ndarray) -> np.ndarray:
        """
        깊이 맵 계산

        Args:
            camera_coords: 카메라 좌표
            projector_coords: 프로젝터 좌표

        Returns:
            깊이 맵 (H, W)
        """
        height, width = camera_coords.shape[:2]
        depth_map = np.zeros((height, width), dtype=np.float32)

        valid_mask = (projector_coords[:, :, 0] >= 0)
        valid_pixels = np.argwhere(valid_mask)

        for y, x in valid_pixels:
            cam_pt = np.array([x, y], dtype=np.float32)
            proj_pt = projector_coords[y, x].astype(np.float32)

            # 3D 포인트 계산
            point_3d = self._triangulate_points(
                cam_pt.reshape(1, 2),
                proj_pt.reshape(1, 2)
            )

            # Z 좌표를 깊이로 사용
            depth_map[y, x] = point_3d[0, 2]

        return depth_map
