#ifndef STRUCTURED_LIGHT_DE_BRUIJN_GENERATOR_H
#define STRUCTURED_LIGHT_DE_BRUIJN_GENERATOR_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <map>

namespace structured_light {

/**
 * @brief De Bruijn 패턴 생성기
 *
 * De Bruijn 시퀀스 기반 단일 샷 구조광 패턴
 * - 모든 길이 n의 부분 시퀀스가 정확히 한 번 나타남
 * - 단일 이미지로 3D 재구성 가능
 * - 고속 스캐닝에 적합
 */
class DeBruijnGenerator {
public:
    /**
     * @brief 생성자
     * @param width 패턴 폭
     * @param height 패턴 높이
     * @param window_size 윈도우 크기 (고유 시퀀스 길이)
     * @param n_colors 색상 수 (2 = 이진, 3+ = 다중 레벨)
     */
    DeBruijnGenerator(int width, int height,
                     int window_size = 5,
                     int n_colors = 2);

    /**
     * @brief 패턴 생성
     * @return 생성된 패턴 벡터 (horizontal, vertical, color)
     */
    std::vector<cv::Mat> generate();

    /**
     * @brief 수평 방향 패턴만 생성
     * @return 수평 패턴
     */
    cv::Mat generateHorizontal();

    /**
     * @brief 수직 방향 패턴만 생성
     * @return 수직 패턴
     */
    cv::Mat generateVertical();

    /**
     * @brief 컬러 패턴 생성 (RGB 채널 인코딩)
     * @return 컬러 패턴 (3채널)
     */
    cv::Mat generateColor();

    /**
     * @brief 패턴 저장
     * @param patterns 패턴 벡터
     * @param output_dir 출력 디렉토리
     * @param prefix 파일명 접두사
     * @return 성공 여부
     */
    bool savePatterns(const std::vector<cv::Mat>& patterns,
                     const std::string& output_dir,
                     const std::string& prefix = "de_bruijn");

    /**
     * @brief 캡처된 이미지에서 패턴 디코딩
     * @param captured 캡처된 이미지
     * @return 디코딩된 좌표 맵 (x, y)
     */
    cv::Mat decode(const cv::Mat& captured);

    /**
     * @brief De Bruijn 시퀀스 가져오기
     * @return 시퀀스 벡터
     */
    const std::vector<int>& getSequence() const { return sequence_; }

    /**
     * @brief 시퀀스 길이
     * @return 길이
     */
    size_t getSequenceLength() const { return sequence_.size(); }

    /**
     * @brief 가능한 고유 윈도우 수
     * @return 고유 윈도우 수
     */
    int getUniqueWindowCount() const;

    /**
     * @brief 패턴 정보 출력
     */
    void printInfo() const;

    /**
     * @brief 시퀀스 시각화
     * @param max_length 표시할 최대 길이
     */
    void visualizeSequence(int max_length = 100) const;

private:
    int width_;
    int height_;
    int window_size_;
    int n_colors_;

    std::vector<int> sequence_;
    std::map<std::vector<int>, int> window_map_; // 윈도우 -> 위치 매핑

    /**
     * @brief De Bruijn 시퀀스 생성 (Martin's algorithm)
     * @param k 알파벳 크기 (색상 수)
     * @param n 윈도우 크기
     * @return De Bruijn 시퀀스
     */
    std::vector<int> generateDeBruijnSequence(int k, int n);

    /**
     * @brief Martin's algorithm 재귀 함수
     * @param sequence 현재 시퀀스
     * @param t 현재 위치
     * @param p 주기
     * @param a 작업 배열
     * @param k 알파벳 크기
     * @param n 윈도우 크기
     */
    void martinAlgorithm(std::vector<int>& sequence, int t, int p,
                        std::vector<int>& a, int k, int n);

    /**
     * @brief 스트라이프 패턴 생성
     * @param is_horizontal 방향
     * @return 패턴
     */
    cv::Mat createStripePattern(bool is_horizontal);

    /**
     * @brief 시퀀스에서 윈도우 위치 찾기
     * @param window 윈도우 값들
     * @return 위치 (-1: 찾지 못함)
     */
    int findWindowPosition(const std::vector<int>& window);

    /**
     * @brief 윈도우 맵 생성 (디코딩 최적화용)
     */
    void buildWindowMap();
};

/**
 * @brief De Bruijn 시퀀스 생성 (유틸리티 함수)
 * @param k 알파벳 크기
 * @param n 윈도우 크기
 * @return De Bruijn 시퀀스
 */
std::vector<int> generateDeBruijnSequence(int k, int n);

/**
 * @brief De Bruijn 그래프 생성 (시각화용)
 * @param k 알파벳 크기
 * @param n 윈도우 크기
 * @return 그래프 이미지
 */
cv::Mat visualizeDeBruijnGraph(int k, int n);

} // namespace structured_light

#endif // STRUCTURED_LIGHT_DE_BRUIJN_GENERATOR_H
