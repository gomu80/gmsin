#ifndef STRUCTURED_LIGHT_PHASE_SHIFT_GENERATOR_H
#define STRUCTURED_LIGHT_PHASE_SHIFT_GENERATOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace structured_light {

/**
 * @brief Phase Shifting 패턴 생성기
 *
 * 위상 변조 방식의 구조광 패턴 생성
 * - 정현파 패턴을 위상 변화시켜 투영
 * - 서브픽셀 정밀도
 * - 다중 주파수 언래핑 지원
 */
class PhaseShiftGenerator {
public:
    /**
     * @brief 패턴 방향
     */
    enum class Direction {
        HORIZONTAL,
        VERTICAL,
        BOTH
    };

    /**
     * @brief 생성자
     * @param width 패턴 폭
     * @param height 패턴 높이
     * @param n_steps 위상 스텝 수 (3, 4, 6 등)
     * @param n_periods 주기 수 (프린지 밀도)
     * @param direction 패턴 방향
     */
    PhaseShiftGenerator(int width, int height,
                       int n_steps = 4,
                       int n_periods = 64,
                       Direction direction = Direction::BOTH);

    /**
     * @brief 패턴 생성
     * @return 생성된 패턴 벡터
     */
    std::vector<cv::Mat> generate();

    /**
     * @brief 다중 주파수 패턴 생성
     * @param frequencies 주기 수 리스트 (예: {1, 8, 64})
     * @return 다중 주파수 패턴
     */
    std::vector<cv::Mat> generateMultiFrequency(const std::vector<int>& frequencies);

    /**
     * @brief 패턴 저장
     * @param patterns 패턴 벡터
     * @param output_dir 출력 디렉토리
     * @param prefix 파일명 접두사
     * @return 성공 여부
     */
    bool savePatterns(const std::vector<cv::Mat>& patterns,
                     const std::string& output_dir,
                     const std::string& prefix = "phase_shift");

    /**
     * @brief 캡처된 이미지로부터 랩핑된 위상 계산
     * @param images 위상 시프트 이미지 (n_steps개)
     * @return 랩핑된 위상 맵 (-π ~ π)
     */
    static cv::Mat computeWrappedPhase(const std::vector<cv::Mat>& images);

    /**
     * @brief 평균 강도 계산
     * @param images 위상 시프트 이미지
     * @return 평균 강도 맵
     */
    static cv::Mat computeIntensity(const std::vector<cv::Mat>& images);

    /**
     * @brief 변조도 계산 (품질 지표)
     * @param images 위상 시프트 이미지
     * @return 변조도 맵 (0~1)
     */
    static cv::Mat computeModulation(const std::vector<cv::Mat>& images);

    /**
     * @brief 위상 언래핑 (Quality-guided)
     * @param wrapped_phase 랩핑된 위상
     * @param quality_map 품질 맵 (변조도)
     * @param threshold 품질 임계값
     * @return 언래핑된 위상
     */
    static cv::Mat unwrapPhase(const cv::Mat& wrapped_phase,
                              const cv::Mat& quality_map,
                              float threshold = 0.3f);

    /**
     * @brief 다중 주파수 위상 언래핑
     * @param wrapped_phases 각 주파수별 랩핑된 위상
     * @param frequencies 주파수 리스트
     * @return 언래핑된 절대 위상
     */
    static cv::Mat unwrapPhaseMultiFreq(const std::vector<cv::Mat>& wrapped_phases,
                                       const std::vector<int>& frequencies);

    /**
     * @brief 총 패턴 수
     * @return 패턴 수
     */
    int getTotalPatternCount() const;

    /**
     * @brief 패턴 정보 출력
     */
    void printInfo() const;

private:
    int width_;
    int height_;
    int n_steps_;
    int n_periods_;
    Direction direction_;

    /**
     * @brief 수평 방향 패턴 생성
     */
    std::vector<cv::Mat> generateHorizontal();

    /**
     * @brief 수직 방향 패턴 생성
     */
    std::vector<cv::Mat> generateVertical();

    /**
     * @brief 정현파 패턴 생성
     * @param n_periods 주기 수
     * @param phase 위상 오프셋 (라디안)
     * @param is_horizontal 방향
     * @return 정현파 패턴
     */
    cv::Mat createSinusoidalPattern(int n_periods, double phase, bool is_horizontal);
};

} // namespace structured_light

#endif // STRUCTURED_LIGHT_PHASE_SHIFT_GENERATOR_H
