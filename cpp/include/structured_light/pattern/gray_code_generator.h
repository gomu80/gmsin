#ifndef STRUCTURED_LIGHT_GRAY_CODE_GENERATOR_H
#define STRUCTURED_LIGHT_GRAY_CODE_GENERATOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace structured_light {

/**
 * @brief Gray Code 패턴 생성기
 *
 * 이진 코딩 방식의 구조광 패턴 생성
 * Gray Code는 인접한 코드가 1비트만 차이나서 노이즈에 강건
 */
class GrayCodeGenerator {
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
     * @param direction 패턴 방향
     * @param use_inverse 역상 패턴 사용 여부
     */
    GrayCodeGenerator(int width, int height,
                     Direction direction = Direction::BOTH,
                     bool use_inverse = true);

    /**
     * @brief 패턴 생성
     * @return 생성된 패턴 벡터
     */
    std::vector<cv::Mat> generate();

    /**
     * @brief 패턴 저장
     * @param patterns 패턴 벡터
     * @param output_dir 출력 디렉토리
     * @param prefix 파일명 접두사
     * @return 성공 여부
     */
    bool savePatterns(const std::vector<cv::Mat>& patterns,
                     const std::string& output_dir,
                     const std::string& prefix = "gray_code");

    /**
     * @brief 필요한 비트 수 계산
     * @return 비트 수
     */
    int getRequiredBits() const;

    /**
     * @brief 총 패턴 수 계산
     * @return 패턴 수
     */
    int getTotalPatternCount() const;

    /**
     * @brief 패턴 정보 출력
     */
    void printInfo() const;

    // 정적 유틸리티 함수
    static int binaryToGray(int n);
    static int grayToBinary(int n);

private:
    int width_;
    int height_;
    Direction direction_;
    bool use_inverse_;

    int n_bits_horizontal_;
    int n_bits_vertical_;

    /**
     * @brief 수평 방향 패턴 생성
     */
    std::vector<cv::Mat> generateHorizontal();

    /**
     * @brief 수직 방향 패턴 생성
     */
    std::vector<cv::Mat> generateVertical();

    /**
     * @brief 스트라이프 패턴 생성
     * @param bit 비트 인덱스
     * @param is_horizontal 방향
     * @return 패턴 이미지
     */
    cv::Mat createStripePattern(int bit, bool is_horizontal);
};

} // namespace structured_light

#endif // STRUCTURED_LIGHT_GRAY_CODE_GENERATOR_H
