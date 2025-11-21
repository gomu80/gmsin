#!/usr/bin/env python3
"""
Pattern Generation Example

다양한 구조광 패턴 생성 예제
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'src'))

from structured_light.pattern import (
    GrayCodeGenerator,
    PhaseShiftGenerator,
    DeBruijnGenerator
)


def demo_gray_code():
    """Gray Code 패턴 생성"""
    print("\n" + "="*60)
    print("Gray Code Pattern Generation")
    print("="*60)

    generator = GrayCodeGenerator(
        width=1920,
        height=1080,
        direction='both',
        inverse=True
    )

    # 패턴 정보
    info = generator.get_pattern_info()
    print(f"Resolution: {info['width']} x {info['height']}")
    print(f"Direction: {info['direction']}")
    print(f"Total patterns: {info['total_patterns']}")

    # 패턴 생성
    patterns = generator.generate()
    print(f"Generated {len(patterns)} patterns")

    # 패턴 저장
    generator.save_patterns(patterns, 'data/patterns/gray_code')
    print("Patterns saved to: data/patterns/gray_code")

    return patterns


def demo_phase_shift():
    """Phase Shifting 패턴 생성"""
    print("\n" + "="*60)
    print("Phase Shifting Pattern Generation")
    print("="*60)

    generator = PhaseShiftGenerator(
        width=1920,
        height=1080,
        n_steps=4,
        n_periods=64,
        direction='both'
    )

    # 패턴 정보
    info = generator.get_pattern_info()
    print(f"Resolution: {info['width']} x {info['height']}")
    print(f"Phase steps: {info['n_steps']}")
    print(f"Periods: {info['n_periods']}")
    print(f"Total patterns: {info['total_patterns']}")

    # 패턴 생성
    patterns = generator.generate()
    print(f"Generated {len(patterns)} patterns")

    # 패턴 저장
    generator.save_patterns(patterns, 'data/patterns/phase_shift')
    print("Patterns saved to: data/patterns/phase_shift")

    # 다중 주파수 패턴
    print("\nGenerating multi-frequency patterns...")
    multi_freq = generator.generate_multi_frequency([1, 8, 64])
    print(f"Generated {len(multi_freq)} multi-frequency patterns")

    return patterns


def demo_de_bruijn():
    """De Bruijn 패턴 생성"""
    print("\n" + "="*60)
    print("De Bruijn Pattern Generation")
    print("="*60)

    generator = DeBruijnGenerator(
        width=1920,
        height=1080,
        window_size=5,
        n_colors=2
    )

    # 패턴 정보
    info = generator.get_pattern_info()
    print(f"Resolution: {info['width']} x {info['height']}")
    print(f"Window size: {info['window_size']}")
    print(f"Sequence length: {info['sequence_length']}")
    print(f"Total patterns: {info['total_patterns']}")

    # 패턴 생성
    patterns = generator.generate()
    print(f"Generated {len(patterns)} patterns")

    # 패턴 저장
    generator.save_patterns(patterns, 'data/patterns/de_bruijn')
    print("Patterns saved to: data/patterns/de_bruijn")

    return patterns


def main():
    """메인 함수"""
    print("Pattern Generation Examples")
    print("="*60)

    # Gray Code
    gray_patterns = demo_gray_code()

    # Phase Shifting
    phase_patterns = demo_phase_shift()

    # De Bruijn
    debruijn_patterns = demo_de_bruijn()

    print("\n" + "="*60)
    print("All patterns generated successfully!")
    print("="*60)


if __name__ == '__main__':
    main()
