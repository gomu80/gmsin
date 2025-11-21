"""
Point Cloud Processing Module

포인트 클라우드 처리 및 저장
"""

import numpy as np
import cv2
from typing import Optional, Tuple
import os


class PointCloudProcessor:
    """포인트 클라우드 처리 클래스"""

    def __init__(self):
        """초기화"""
        self.points = None
        self.colors = None

    def set_point_cloud(self, point_cloud: np.ndarray):
        """
        포인트 클라우드 설정

        Args:
            point_cloud: (N, 6) 배열 [x, y, z, r, g, b]
        """
        if point_cloud.shape[1] >= 3:
            self.points = point_cloud[:, :3]
            if point_cloud.shape[1] >= 6:
                self.colors = point_cloud[:, 3:6].astype(np.uint8)
            else:
                self.colors = np.ones((len(point_cloud), 3), dtype=np.uint8) * 128

    def filter_outliers(self, method: str = 'statistical',
                       **kwargs) -> np.ndarray:
        """
        아웃라이어 제거

        Args:
            method: 'statistical' 또는 'radius'
            **kwargs: 추가 파라미터

        Returns:
            필터링된 포인트 클라우드
        """
        if self.points is None:
            raise ValueError("No point cloud set")

        if method == 'statistical':
            return self._statistical_outlier_removal(**kwargs)
        elif method == 'radius':
            return self._radius_outlier_removal(**kwargs)
        else:
            raise ValueError(f"Unknown method: {method}")

    def _statistical_outlier_removal(self, nb_neighbors: int = 20,
                                    std_ratio: float = 2.0) -> np.ndarray:
        """통계적 아웃라이어 제거"""
        try:
            import open3d as o3d

            # Open3D 포인트 클라우드 생성
            pcd = o3d.geometry.PointCloud()
            pcd.points = o3d.utility.Vector3dVector(self.points)

            # 아웃라이어 제거
            pcd_filtered, ind = pcd.remove_statistical_outlier(
                nb_neighbors=nb_neighbors,
                std_ratio=std_ratio
            )

            # 결과 반환
            points_filtered = np.asarray(pcd_filtered.points)
            colors_filtered = self.colors[ind] if self.colors is not None else None

            if colors_filtered is not None:
                return np.hstack([points_filtered, colors_filtered])
            else:
                return points_filtered

        except ImportError:
            print("Open3D not available, skipping outlier removal")
            return np.hstack([self.points, self.colors])

    def _radius_outlier_removal(self, nb_points: int = 16,
                               radius: float = 0.05) -> np.ndarray:
        """반경 기반 아웃라이어 제거"""
        try:
            import open3d as o3d

            pcd = o3d.geometry.PointCloud()
            pcd.points = o3d.utility.Vector3dVector(self.points)

            pcd_filtered, ind = pcd.remove_radius_outlier(
                nb_points=nb_points,
                radius=radius
            )

            points_filtered = np.asarray(pcd_filtered.points)
            colors_filtered = self.colors[ind] if self.colors is not None else None

            if colors_filtered is not None:
                return np.hstack([points_filtered, colors_filtered])
            else:
                return points_filtered

        except ImportError:
            return np.hstack([self.points, self.colors])

    def downsample(self, voxel_size: float = 0.01) -> np.ndarray:
        """
        복셀 다운샘플링

        Args:
            voxel_size: 복셀 크기

        Returns:
            다운샘플링된 포인트 클라우드
        """
        try:
            import open3d as o3d

            pcd = o3d.geometry.PointCloud()
            pcd.points = o3d.utility.Vector3dVector(self.points)
            if self.colors is not None:
                pcd.colors = o3d.utility.Vector3dVector(self.colors / 255.0)

            pcd_down = pcd.voxel_down_sample(voxel_size=voxel_size)

            points_down = np.asarray(pcd_down.points)
            colors_down = (np.asarray(pcd_down.colors) * 255).astype(np.uint8)

            return np.hstack([points_down, colors_down])

        except ImportError:
            return np.hstack([self.points, self.colors])

    def save_ply(self, filepath: str) -> bool:
        """
        PLY 형식으로 저장

        Args:
            filepath: 저장 경로

        Returns:
            성공 여부
        """
        if self.points is None:
            raise ValueError("No point cloud set")

        try:
            os.makedirs(os.path.dirname(filepath), exist_ok=True)

            with open(filepath, 'w') as f:
                # 헤더
                f.write("ply\n")
                f.write("format ascii 1.0\n")
                f.write(f"element vertex {len(self.points)}\n")
                f.write("property float x\n")
                f.write("property float y\n")
                f.write("property float z\n")

                if self.colors is not None:
                    f.write("property uchar red\n")
                    f.write("property uchar green\n")
                    f.write("property uchar blue\n")

                f.write("end_header\n")

                # 데이터
                for i in range(len(self.points)):
                    x, y, z = self.points[i]
                    f.write(f"{x} {y} {z}")

                    if self.colors is not None:
                        r, g, b = self.colors[i]
                        f.write(f" {r} {g} {b}")

                    f.write("\n")

            print(f"Point cloud saved to {filepath}")
            return True

        except Exception as e:
            print(f"Failed to save point cloud: {e}")
            return False

    def save_xyz(self, filepath: str) -> bool:
        """XYZ 형식으로 저장"""
        if self.points is None:
            raise ValueError("No point cloud set")

        try:
            os.makedirs(os.path.dirname(filepath), exist_ok=True)
            np.savetxt(filepath, self.points, fmt='%.6f')
            print(f"Point cloud saved to {filepath}")
            return True

        except Exception as e:
            print(f"Failed to save point cloud: {e}")
            return False

    def visualize(self):
        """포인트 클라우드 시각화"""
        try:
            import open3d as o3d

            pcd = o3d.geometry.PointCloud()
            pcd.points = o3d.utility.Vector3dVector(self.points)

            if self.colors is not None:
                pcd.colors = o3d.utility.Vector3dVector(self.colors / 255.0)

            o3d.visualization.draw_geometries([pcd])

        except ImportError:
            print("Open3D not available for visualization")

    def get_statistics(self) -> dict:
        """포인트 클라우드 통계"""
        if self.points is None:
            return {}

        return {
            'n_points': len(self.points),
            'min': self.points.min(axis=0),
            'max': self.points.max(axis=0),
            'mean': self.points.mean(axis=0),
            'std': self.points.std(axis=0)
        }
