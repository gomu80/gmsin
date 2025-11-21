#include "structured_light/pattern/gray_code_generator.h"
#include <iostream>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

namespace structured_light {

GrayCodeGenerator::GrayCodeGenerator(int width, int height,
                                   Direction direction, bool use_inverse)
    : width_(width)
    , height_(height)
    , direction_(direction)
    , use_inverse_(use_inverse)
{
    n_bits_horizontal_ = static_cast<int>(std::ceil(std::log2(width)));
    n_bits_vertical_ = static_cast<int>(std::ceil(std::log2(height)));
}

std::vector<cv::Mat> GrayCodeGenerator::generate() {
    std::vector<cv::Mat> patterns;

    if (direction_ == Direction::HORIZONTAL || direction_ == Direction::BOTH) {
        auto h_patterns = generateHorizontal();
        patterns.insert(patterns.end(), h_patterns.begin(), h_patterns.end());
    }

    if (direction_ == Direction::VERTICAL || direction_ == Direction::BOTH) {
        auto v_patterns = generateVertical();
        patterns.insert(patterns.end(), v_patterns.begin(), v_patterns.end());
    }

    // 전체 흰색/검은색 패턴 추가 (앰비언트 라이트 측정용)
    patterns.push_back(cv::Mat(height_, width_, CV_8UC1, cv::Scalar(255)));
    patterns.push_back(cv::Mat(height_, width_, CV_8UC1, cv::Scalar(0)));

    std::cout << "Generated " << patterns.size() << " Gray Code patterns" << std::endl;
    return patterns;
}

std::vector<cv::Mat> GrayCodeGenerator::generateHorizontal() {
    std::vector<cv::Mat> patterns;

    for (int bit = 0; bit < n_bits_horizontal_; ++bit) {
        cv::Mat pattern = createStripePattern(bit, true);
        patterns.push_back(pattern);

        if (use_inverse_) {
            cv::Mat inverse = 255 - pattern;
            patterns.push_back(inverse);
        }
    }

    return patterns;
}

std::vector<cv::Mat> GrayCodeGenerator::generateVertical() {
    std::vector<cv::Mat> patterns;

    for (int bit = 0; bit < n_bits_vertical_; ++bit) {
        cv::Mat pattern = createStripePattern(bit, false);
        patterns.push_back(pattern);

        if (use_inverse_) {
            cv::Mat inverse = 255 - pattern;
            patterns.push_back(inverse);
        }
    }

    return patterns;
}

cv::Mat GrayCodeGenerator::createStripePattern(int bit, bool is_horizontal) {
    cv::Mat pattern = cv::Mat::zeros(height_, width_, CV_8UC1);

    if (is_horizontal) {
        // 수평 스트라이프
        for (int x = 0; x < width_; ++x) {
            int gray_code = binaryToGray(x);
            if ((gray_code >> bit) & 1) {
                pattern.col(x) = 255;
            }
        }
    } else {
        // 수직 스트라이프
        for (int y = 0; y < height_; ++y) {
            int gray_code = binaryToGray(y);
            if ((gray_code >> bit) & 1) {
                pattern.row(y) = 255;
            }
        }
    }

    return pattern;
}

bool GrayCodeGenerator::savePatterns(const std::vector<cv::Mat>& patterns,
                                    const std::string& output_dir,
                                    const std::string& prefix) {
    try {
        // 디렉토리 생성
        fs::create_directories(output_dir);

        for (size_t i = 0; i < patterns.size(); ++i) {
            std::ostringstream filename;
            filename << output_dir << "/" << prefix << "_"
                    << std::setw(3) << std::setfill('0') << i << ".png";

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

int GrayCodeGenerator::getRequiredBits() const {
    int bits = 0;
    if (direction_ == Direction::HORIZONTAL || direction_ == Direction::BOTH) {
        bits += n_bits_horizontal_;
    }
    if (direction_ == Direction::VERTICAL || direction_ == Direction::BOTH) {
        bits += n_bits_vertical_;
    }
    return bits;
}

int GrayCodeGenerator::getTotalPatternCount() const {
    int count = 0;

    if (direction_ == Direction::HORIZONTAL || direction_ == Direction::BOTH) {
        count += n_bits_horizontal_ * (use_inverse_ ? 2 : 1);
    }

    if (direction_ == Direction::VERTICAL || direction_ == Direction::BOTH) {
        count += n_bits_vertical_ * (use_inverse_ ? 2 : 1);
    }

    count += 2; // 흰색/검은색 패턴
    return count;
}

void GrayCodeGenerator::printInfo() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Gray Code Pattern Generator" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Resolution: " << width_ << "x" << height_ << std::endl;
    std::cout << "Direction: ";

    switch (direction_) {
        case Direction::HORIZONTAL:
            std::cout << "Horizontal" << std::endl;
            break;
        case Direction::VERTICAL:
            std::cout << "Vertical" << std::endl;
            break;
        case Direction::BOTH:
            std::cout << "Both" << std::endl;
            break;
    }

    std::cout << "Horizontal bits: " << n_bits_horizontal_ << std::endl;
    std::cout << "Vertical bits: " << n_bits_vertical_ << std::endl;
    std::cout << "Use inverse: " << (use_inverse_ ? "Yes" : "No") << std::endl;
    std::cout << "Total patterns: " << getTotalPatternCount() << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

int GrayCodeGenerator::binaryToGray(int n) {
    return n ^ (n >> 1);
}

int GrayCodeGenerator::grayToBinary(int n) {
    int mask = n;
    while (mask) {
        mask >>= 1;
        n ^= mask;
    }
    return n;
}

} // namespace structured_light
