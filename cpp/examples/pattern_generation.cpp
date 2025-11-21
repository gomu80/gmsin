/**
 * @file pattern_generation.cpp
 * @brief 패턴 생성 예제
 *
 * Gray Code, Phase Shifting, De Bruijn 패턴 생성 및 저장
 */

#include "structured_light/pattern/gray_code_generator.h"
#include "structured_light/pattern/phase_shift_generator.h"
#include "structured_light/pattern/de_bruijn_generator.h"
#include <iostream>

using namespace structured_light;

void generateGrayCode() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Gray Code Pattern Generation" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // LightCrafter 4500 해상도
    GrayCodeGenerator generator(912, 1140,
                                GrayCodeGenerator::Direction::BOTH,
                                true);

    // 패턴 정보 출력
    generator.printInfo();

    // 패턴 생성
    std::cout << "\nGenerating patterns..." << std::endl;
    auto patterns = generator.generate();

    // 패턴 저장
    generator.savePatterns(patterns, "data/patterns/gray_code");

    // 첫 몇 개 패턴 미리보기
    std::cout << "\nDisplaying first 4 patterns..." << std::endl;
    for (int i = 0; i < std::min(4, static_cast<int>(patterns.size())); ++i) {
        cv::imshow("Gray Code Pattern " + std::to_string(i), patterns[i]);
    }

    std::cout << "Press any key to continue..." << std::endl;
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void generatePhaseShift() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Phase Shifting Pattern Generation" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    PhaseShiftGenerator generator(912, 1140,
                                  4,    // 4-step
                                  64,   // 64 periods
                                  PhaseShiftGenerator::Direction::BOTH);

    // 패턴 정보 출력
    generator.printInfo();

    // 패턴 생성
    std::cout << "\nGenerating patterns..." << std::endl;
    auto patterns = generator.generate();

    // 패턴 저장
    generator.savePatterns(patterns, "data/patterns/phase_shift");

    // 다중 주파수 패턴 생성
    std::cout << "\nGenerating multi-frequency patterns..." << std::endl;
    std::vector<int> frequencies = {1, 8, 64};
    auto multi_freq = generator.generateMultiFrequency(frequencies);

    generator.savePatterns(multi_freq, "data/patterns/phase_shift_multifreq");

    // 패턴 미리보기
    std::cout << "\nDisplaying patterns..." << std::endl;
    for (int i = 0; i < std::min(4, static_cast<int>(patterns.size())); ++i) {
        cv::imshow("Phase Shift Pattern " + std::to_string(i), patterns[i]);
    }

    std::cout << "Press any key to continue..." << std::endl;
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void generateDeBruijn() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "De Bruijn Pattern Generation" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    DeBruijnGenerator generator(912, 1140,
                                5,    // window size
                                2);   // binary

    // 패턴 정보 출력
    generator.printInfo();

    // 시퀀스 시각화
    generator.visualizeSequence(100);

    // 패턴 생성
    std::cout << "\nGenerating patterns..." << std::endl;
    auto patterns = generator.generate();

    // 패턴 저장
    generator.savePatterns(patterns, "data/patterns/de_bruijn");

    // 패턴 미리보기
    std::cout << "\nDisplaying patterns..." << std::endl;
    std::vector<std::string> names = {"Horizontal", "Vertical", "Color"};

    for (size_t i = 0; i < patterns.size(); ++i) {
        cv::imshow("De Bruijn " + names[i], patterns[i]);
    }

    // De Bruijn 그래프 시각화
    cv::Mat graph = visualizeDeBruijnGraph(2, 5);
    cv::imshow("De Bruijn Graph", graph);

    std::cout << "Press any key to continue..." << std::endl;
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void comparePatterns() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Pattern Comparison" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    const int width = 912;
    const int height = 1140;

    // 각 패턴 타입별 생성
    GrayCodeGenerator gray_gen(width, height,
                               GrayCodeGenerator::Direction::HORIZONTAL,
                               true);

    PhaseShiftGenerator phase_gen(width, height, 4, 64,
                                  PhaseShiftGenerator::Direction::HORIZONTAL);

    DeBruijnGenerator debruijn_gen(width, height, 5, 2);

    // 패턴 수 비교
    std::cout << "\nPattern Count Comparison:" << std::endl;
    std::cout << "  Gray Code:     " << gray_gen.getTotalPatternCount() << " patterns" << std::endl;
    std::cout << "  Phase Shift:   " << phase_gen.getTotalPatternCount() << " patterns" << std::endl;
    std::cout << "  De Bruijn:     3 patterns (single-shot)" << std::endl;

    // 스캔 시간 추정 (4kHz 투영 기준)
    std::cout << "\nEstimated Scan Time @ 4kHz:" << std::endl;
    std::cout << "  Gray Code:     " << (gray_gen.getTotalPatternCount() / 4000.0) << " seconds" << std::endl;
    std::cout << "  Phase Shift:   " << (phase_gen.getTotalPatternCount() / 4000.0) << " seconds" << std::endl;
    std::cout << "  De Bruijn:     " << (3 / 4000.0) << " seconds (0.75ms)" << std::endl;

    // 특징 비교
    std::cout << "\nCharacteristics:" << std::endl;
    std::cout << "  Gray Code:" << std::endl;
    std::cout << "    + Robust to noise" << std::endl;
    std::cout << "    + High reliability" << std::endl;
    std::cout << "    - Multiple frames required" << std::endl;
    std::cout << "    - Lower resolution" << std::endl;

    std::cout << "\n  Phase Shifting:" << std::endl;
    std::cout << "    + Sub-pixel precision" << std::endl;
    std::cout << "    + High accuracy" << std::endl;
    std::cout << "    - Sensitive to ambient light" << std::endl;
    std::cout << "    - Requires unwrapping" << std::endl;

    std::cout << "\n  De Bruijn:" << std::endl;
    std::cout << "    + Single-shot capable" << std::endl;
    std::cout << "    + Real-time scanning" << std::endl;
    std::cout << "    - Complex decoding" << std::endl;
    std::cout << "    - Limited resolution" << std::endl;

    std::cout << "\nRecommendation: Use Hybrid approach" << std::endl;
    std::cout << "  - Gray Code (coarse) + Phase Shift (fine)" << std::endl;
    std::cout << "  - Best accuracy and reliability" << std::endl;
}

void hybridApproach() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Hybrid Approach Example" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    const int width = 912;
    const int height = 1140;

    std::cout << "\nGenerating hybrid patterns..." << std::endl;
    std::cout << "Strategy: Coarse Gray Code + Fine Phase Shifting" << std::endl;

    // 1. 저주파 Gray Code (빠른 언래핑용)
    GrayCodeGenerator gray_gen(width, height,
                               GrayCodeGenerator::Direction::HORIZONTAL,
                               true);
    auto gray_patterns = gray_gen.generate();

    std::cout << "\n1. Gray Code patterns: " << gray_patterns.size() << std::endl;
    std::cout << "   Purpose: Coarse correspondence + unwrapping" << std::endl;

    // 2. 고주파 Phase Shifting (정밀 측정용)
    PhaseShiftGenerator phase_gen(width, height, 4, 64,
                                  PhaseShiftGenerator::Direction::HORIZONTAL);
    auto phase_patterns = phase_gen.generate();

    std::cout << "\n2. Phase Shift patterns: " << phase_patterns.size() << std::endl;
    std::cout << "   Purpose: Sub-pixel precision" << std::endl;

    // 총 패턴 수
    int total = gray_patterns.size() + phase_patterns.size();
    std::cout << "\nTotal patterns: " << total << std::endl;
    std::cout << "Scan time @ 4kHz: " << (total / 4000.0) << " seconds" << std::endl;

    // 저장
    std::cout << "\nSaving hybrid patterns..." << std::endl;
    gray_gen.savePatterns(gray_patterns, "data/patterns/hybrid/gray_code");
    phase_gen.savePatterns(phase_patterns, "data/patterns/hybrid/phase_shift");

    std::cout << "Hybrid patterns saved to data/patterns/hybrid/" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Pattern Generation Examples\n";
    std::cout << "========================================\n";

    try {
        // 1. Gray Code 패턴
        generateGrayCode();

        // 2. Phase Shifting 패턴
        generatePhaseShift();

        // 3. De Bruijn 패턴
        generateDeBruijn();

        // 4. 패턴 비교
        comparePatterns();

        // 5. Hybrid 접근법
        hybridApproach();

        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "All patterns generated successfully!" << std::endl;
        std::cout << "Check 'data/patterns/' directory" << std::endl;
        std::cout << std::string(70, '=') << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
