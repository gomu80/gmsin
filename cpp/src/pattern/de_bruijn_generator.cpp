#include "structured_light/pattern/de_bruijn_generator.h"
#include <iostream>
#include <cmath>
#include <filesystem>
#include <algorithm>
#include <sstream>

namespace fs = std::filesystem;

namespace structured_light {

DeBruijnGenerator::DeBruijnGenerator(int width, int height,
                                   int window_size, int n_colors)
    : width_(width)
    , height_(height)
    , window_size_(window_size)
    , n_colors_(n_colors)
{
    // De Bruijn 시퀀스 생성
    sequence_ = generateDeBruijnSequence(n_colors_, window_size_);

    // 윈도우 맵 생성 (디코딩 최적화)
    buildWindowMap();

    std::cout << "De Bruijn sequence generated: length = " << sequence_.size() << std::endl;
}

std::vector<cv::Mat> DeBruijnGenerator::generate() {
    std::vector<cv::Mat> patterns;

    // 수평 패턴
    patterns.push_back(generateHorizontal());

    // 수직 패턴
    patterns.push_back(generateVertical());

    // 컬러 패턴
    patterns.push_back(generateColor());

    std::cout << "Generated " << patterns.size() << " De Bruijn patterns" << std::endl;
    return patterns;
}

cv::Mat DeBruijnGenerator::generateHorizontal() {
    return createStripePattern(true);
}

cv::Mat DeBruijnGenerator::generateVertical() {
    return createStripePattern(false);
}

cv::Mat DeBruijnGenerator::generateColor() {
    cv::Mat pattern = cv::Mat::zeros(height_, width_, CV_8UC3);

    // 각 RGB 채널에 다른 시퀀스 적용
    for (int channel = 0; channel < 3; ++channel) {
        // 각 채널용 시퀀스 생성 (시드 변경)
        std::vector<int> channel_seq = generateDeBruijnSequence(n_colors_, window_size_);

        // 시퀀스를 순환시켜 다르게 만들기
        std::rotate(channel_seq.begin(),
                   channel_seq.begin() + channel * (channel_seq.size() / 3),
                   channel_seq.end());

        // 스트라이프 폭 계산
        int stripe_width = std::max(1, width_ / static_cast<int>(channel_seq.size()));

        for (size_t i = 0; i < channel_seq.size(); ++i) {
            int x_start = i * stripe_width;
            int x_end = std::min((i + 1) * stripe_width, width_);

            if (x_start >= width_) break;

            // 강도 계산
            int intensity = (255 * channel_seq[i]) / (n_colors_ - 1);

            // 해당 채널에만 값 설정
            cv::Mat roi = pattern(cv::Rect(x_start, 0, x_end - x_start, height_));
            for (int y = 0; y < height_; ++y) {
                for (int x = x_start; x < x_end; ++x) {
                    pattern.at<cv::Vec3b>(y, x)[channel] = intensity;
                }
            }
        }
    }

    return pattern;
}

cv::Mat DeBruijnGenerator::createStripePattern(bool is_horizontal) {
    cv::Mat pattern = cv::Mat::zeros(height_, width_, CV_8UC1);

    // 스트라이프 크기 계산
    int dimension = is_horizontal ? width_ : height_;
    int stripe_size = std::max(1, dimension / static_cast<int>(sequence_.size()));

    for (size_t i = 0; i < sequence_.size(); ++i) {
        int start = i * stripe_size;
        int end = std::min((i + 1) * stripe_size, dimension);

        if (start >= dimension) break;

        // 강도 계산 (0 ~ 255)
        int intensity = (255 * sequence_[i]) / (n_colors_ - 1);

        if (is_horizontal) {
            // 수평 스트라이프
            pattern(cv::Rect(start, 0, end - start, height_)) = intensity;
        } else {
            // 수직 스트라이프
            pattern(cv::Rect(0, start, width_, end - start)) = intensity;
        }
    }

    return pattern;
}

std::vector<int> DeBruijnGenerator::generateDeBruijnSequence(int k, int n) {
    std::vector<int> sequence;
    std::vector<int> a(k * n, 0);

    martinAlgorithm(sequence, 1, 1, a, k, n);

    return sequence;
}

void DeBruijnGenerator::martinAlgorithm(std::vector<int>& sequence,
                                       int t, int p,
                                       std::vector<int>& a,
                                       int k, int n) {
    if (t > n) {
        if (n % p == 0) {
            for (int j = 1; j <= p; ++j) {
                sequence.push_back(a[j]);
            }
        }
    } else {
        a[t] = a[t - p];
        martinAlgorithm(sequence, t + 1, p, a, k, n);

        for (int j = a[t - p] + 1; j < k; ++j) {
            a[t] = j;
            martinAlgorithm(sequence, t + 1, t, a, k, n);
        }
    }
}

void DeBruijnGenerator::buildWindowMap() {
    window_map_.clear();

    size_t seq_len = sequence_.size();

    for (size_t i = 0; i < seq_len; ++i) {
        std::vector<int> window;

        for (int j = 0; j < window_size_; ++j) {
            window.push_back(sequence_[(i + j) % seq_len]);
        }

        window_map_[window] = i;
    }
}

cv::Mat DeBruijnGenerator::decode(const cv::Mat& captured) {
    cv::Mat gray;

    // 그레이스케일 변환
    if (captured.channels() == 3) {
        cv::cvtColor(captured, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = captured.clone();
    }

    cv::Mat coord_map = cv::Mat::zeros(gray.rows, gray.cols, CV_32SC1);
    coord_map = -1; // 초기화

    int half_window = window_size_ / 2;

    // 각 픽셀에서 윈도우 추출 및 디코딩
    for (int y = 0; y < gray.rows; ++y) {
        for (int x = half_window; x < gray.cols - half_window; ++x) {
            // 윈도우 추출
            std::vector<int> window;
            std::vector<uint8_t> intensities;

            for (int i = -half_window; i <= half_window; ++i) {
                intensities.push_back(gray.at<uint8_t>(y, x + i));
            }

            // 이진화 (지역적 임계값)
            uint8_t threshold = 0;
            for (auto val : intensities) {
                threshold += val;
            }
            threshold /= intensities.size();

            for (auto val : intensities) {
                window.push_back(val > threshold ? 1 : 0);
            }

            // 윈도우 맵에서 위치 찾기
            auto it = window_map_.find(window);
            if (it != window_map_.end()) {
                coord_map.at<int>(y, x) = it->second;
            }
        }
    }

    return coord_map;
}

int DeBruijnGenerator::findWindowPosition(const std::vector<int>& window) {
    auto it = window_map_.find(window);
    if (it != window_map_.end()) {
        return it->second;
    }
    return -1;
}

bool DeBruijnGenerator::savePatterns(const std::vector<cv::Mat>& patterns,
                                    const std::string& output_dir,
                                    const std::string& prefix) {
    try {
        fs::create_directories(output_dir);

        std::vector<std::string> names = {"horizontal", "vertical", "color"};

        for (size_t i = 0; i < patterns.size() && i < names.size(); ++i) {
            std::ostringstream filename;
            filename << output_dir << "/" << prefix << "_" << names[i] << ".png";

            if (!cv::imwrite(filename.str(), patterns[i])) {
                std::cerr << "Failed to save: " << filename.str() << std::endl;
                return false;
            }
        }

        std::cout << "Saved " << patterns.size() << " patterns to " << output_dir << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error saving patterns: " << e.what() << std::endl;
        return false;
    }
}

int DeBruijnGenerator::getUniqueWindowCount() const {
    return std::pow(n_colors_, window_size_);
}

void DeBruijnGenerator::printInfo() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "De Bruijn Pattern Generator" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Resolution: " << width_ << "x" << height_ << std::endl;
    std::cout << "Window size: " << window_size_ << std::endl;
    std::cout << "Number of colors: " << n_colors_ << std::endl;
    std::cout << "Sequence length: " << sequence_.size() << std::endl;
    std::cout << "Unique windows: " << getUniqueWindowCount() << std::endl;
    std::cout << "Total patterns: 3 (H, V, Color)" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void DeBruijnGenerator::visualizeSequence(int max_length) const {
    int display_len = std::min(max_length, static_cast<int>(sequence_.size()));

    std::cout << "\nDe Bruijn Sequence (first " << display_len << " elements):" << std::endl;
    std::cout << "[";
    for (int i = 0; i < display_len; ++i) {
        std::cout << sequence_[i];
        if (i < display_len - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    // 히스토그램
    std::vector<int> histogram(n_colors_, 0);
    for (auto val : sequence_) {
        if (val >= 0 && val < n_colors_) {
            histogram[val]++;
        }
    }

    std::cout << "\nValue distribution:" << std::endl;
    for (int i = 0; i < n_colors_; ++i) {
        std::cout << "  " << i << ": " << histogram[i]
                  << " (" << (100.0 * histogram[i] / sequence_.size()) << "%)"
                  << std::endl;
    }
}

// 유틸리티 함수들

std::vector<int> generateDeBruijnSequence(int k, int n) {
    DeBruijnGenerator gen(1, 1, n, k);
    return gen.getSequence();
}

cv::Mat visualizeDeBruijnGraph(int k, int n) {
    // De Bruijn 그래프 시각화 (간단한 버전)
    auto sequence = generateDeBruijnSequence(k, n);

    int width = 800;
    int height = 200;
    cv::Mat graph = cv::Mat::zeros(height, width, CV_8UC3);

    if (sequence.empty()) return graph;

    // 시퀀스를 막대 그래프로 표시
    int bar_width = width / sequence.size();
    int max_val = k - 1;

    for (size_t i = 0; i < sequence.size(); ++i) {
        int x = i * bar_width;
        int bar_height = (height * sequence[i]) / max_val;

        cv::Scalar color;
        switch (sequence[i] % 3) {
            case 0: color = cv::Scalar(255, 0, 0); break;    // 파랑
            case 1: color = cv::Scalar(0, 255, 0); break;    // 초록
            case 2: color = cv::Scalar(0, 0, 255); break;    // 빨강
        }

        cv::rectangle(graph,
                     cv::Point(x, height - bar_height),
                     cv::Point(x + bar_width - 1, height),
                     color, -1);
    }

    // 제목 추가
    std::string title = "De Bruijn Sequence (k=" + std::to_string(k) +
                       ", n=" + std::to_string(n) + ")";
    cv::putText(graph, title, cv::Point(10, 30),
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    return graph;
}

} // namespace structured_light
