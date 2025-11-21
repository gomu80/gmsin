#include "structured_light/pattern/phase_shift_generator.h"
#include <iostream>
#include <cmath>
#include <filesystem>
#include <queue>

namespace fs = std::filesystem;

namespace structured_light {

PhaseShiftGenerator::PhaseShiftGenerator(int width, int height,
                                       int n_steps, int n_periods,
                                       Direction direction)
    : width_(width)
    , height_(height)
    , n_steps_(n_steps)
    , n_periods_(n_periods)
    , direction_(direction)
{
}

std::vector<cv::Mat> PhaseShiftGenerator::generate() {
    std::vector<cv::Mat> patterns;

    if (direction_ == Direction::HORIZONTAL || direction_ == Direction::BOTH) {
        auto h_patterns = generateHorizontal();
        patterns.insert(patterns.end(), h_patterns.begin(), h_patterns.end());
    }

    if (direction_ == Direction::VERTICAL || direction_ == Direction::BOTH) {
        auto v_patterns = generateVertical();
        patterns.insert(patterns.end(), v_patterns.begin(), v_patterns.end());
    }

    std::cout << "Generated " << patterns.size() << " Phase Shifting patterns" << std::endl;
    return patterns;
}

std::vector<cv::Mat> PhaseShiftGenerator::generateHorizontal() {
    std::vector<cv::Mat> patterns;

    for (int step = 0; step < n_steps_; ++step) {
        double phase = 2.0 * M_PI * step / n_steps_;
        cv::Mat pattern = createSinusoidalPattern(n_periods_, phase, true);
        patterns.push_back(pattern);
    }

    return patterns;
}

std::vector<cv::Mat> PhaseShiftGenerator::generateVertical() {
    std::vector<cv::Mat> patterns;

    for (int step = 0; step < n_steps_; ++step) {
        double phase = 2.0 * M_PI * step / n_steps_;
        cv::Mat pattern = createSinusoidalPattern(n_periods_, phase, false);
        patterns.push_back(pattern);
    }

    return patterns;
}

cv::Mat PhaseShiftGenerator::createSinusoidalPattern(int n_periods, double phase,
                                                    bool is_horizontal) {
    cv::Mat pattern(height_, width_, CV_8UC1);

    if (is_horizontal) {
        // 수평 방향 정현파
        for (int x = 0; x < width_; ++x) {
            double value = 127.5 * (1.0 + std::sin(2.0 * M_PI * n_periods * x / width_ + phase));
            uint8_t intensity = static_cast<uint8_t>(std::round(value));

            for (int y = 0; y < height_; ++y) {
                pattern.at<uint8_t>(y, x) = intensity;
            }
        }
    } else {
        // 수직 방향 정현파
        for (int y = 0; y < height_; ++y) {
            double value = 127.5 * (1.0 + std::sin(2.0 * M_PI * n_periods * y / height_ + phase));
            uint8_t intensity = static_cast<uint8_t>(std::round(value));

            for (int x = 0; x < width_; ++x) {
                pattern.at<uint8_t>(y, x) = intensity;
            }
        }
    }

    return pattern;
}

std::vector<cv::Mat> PhaseShiftGenerator::generateMultiFrequency(
    const std::vector<int>& frequencies) {

    std::vector<cv::Mat> patterns;

    for (int freq : frequencies) {
        PhaseShiftGenerator gen(width_, height_, n_steps_, freq, direction_);
        auto freq_patterns = gen.generate();
        patterns.insert(patterns.end(), freq_patterns.begin(), freq_patterns.end());
    }

    std::cout << "Generated " << patterns.size()
              << " multi-frequency patterns" << std::endl;
    return patterns;
}

bool PhaseShiftGenerator::savePatterns(const std::vector<cv::Mat>& patterns,
                                      const std::string& output_dir,
                                      const std::string& prefix) {
    try {
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

cv::Mat PhaseShiftGenerator::computeWrappedPhase(const std::vector<cv::Mat>& images) {
    if (images.empty()) {
        throw std::runtime_error("No images provided");
    }

    int n_steps = images.size();
    cv::Mat wrapped_phase = cv::Mat::zeros(images[0].size(), CV_32F);

    // Float 이미지로 변환
    std::vector<cv::Mat> images_float;
    for (const auto& img : images) {
        cv::Mat gray, float_img;
        if (img.channels() == 3) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = img.clone();
        }
        gray.convertTo(float_img, CV_32F);
        images_float.push_back(float_img);
    }

    // N-step algorithm
    cv::Mat numerator = cv::Mat::zeros(images[0].size(), CV_32F);
    cv::Mat denominator = cv::Mat::zeros(images[0].size(), CV_32F);

    for (int k = 0; k < n_steps; ++k) {
        double phase = 2.0 * M_PI * k / n_steps;
        numerator += images_float[k] * std::sin(phase);
        denominator += images_float[k] * std::cos(phase);
    }

    // 위상 계산
    cv::phase(denominator, -numerator, wrapped_phase, false); // [-π, π]

    return wrapped_phase;
}

cv::Mat PhaseShiftGenerator::computeIntensity(const std::vector<cv::Mat>& images) {
    if (images.empty()) return cv::Mat();

    cv::Mat sum = cv::Mat::zeros(images[0].size(), CV_32F);

    for (const auto& img : images) {
        cv::Mat gray, float_img;
        if (img.channels() == 3) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = img.clone();
        }
        gray.convertTo(float_img, CV_32F);
        sum += float_img;
    }

    sum /= static_cast<float>(images.size());

    cv::Mat intensity;
    sum.convertTo(intensity, CV_8U);
    return intensity;
}

cv::Mat PhaseShiftGenerator::computeModulation(const std::vector<cv::Mat>& images) {
    if (images.empty()) return cv::Mat();

    std::vector<cv::Mat> images_float;
    for (const auto& img : images) {
        cv::Mat gray, float_img;
        if (img.channels() == 3) {
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = img.clone();
        }
        gray.convertTo(float_img, CV_32F);
        images_float.push_back(float_img);
    }

    cv::Mat I_max = images_float[0].clone();
    cv::Mat I_min = images_float[0].clone();

    for (size_t i = 1; i < images_float.size(); ++i) {
        I_max = cv::max(I_max, images_float[i]);
        I_min = cv::min(I_min, images_float[i]);
    }

    cv::Mat modulation = (I_max - I_min) / (I_max + I_min + 1e-6f);

    return modulation;
}

cv::Mat PhaseShiftGenerator::unwrapPhase(const cv::Mat& wrapped_phase,
                                        const cv::Mat& quality_map,
                                        float threshold) {
    cv::Mat unwrapped = wrapped_phase.clone();
    cv::Mat visited = cv::Mat::zeros(wrapped_phase.size(), CV_8U);

    // 품질이 높은 픽셀부터 처리
    cv::Mat mask = quality_map > threshold;
    std::vector<cv::Point> valid_pixels;

    for (int y = 0; y < mask.rows; ++y) {
        for (int x = 0; x < mask.cols; ++x) {
            if (mask.at<uint8_t>(y, x)) {
                valid_pixels.push_back(cv::Point(x, y));
            }
        }
    }

    if (valid_pixels.empty()) return unwrapped;

    // 중심 픽셀부터 시작
    cv::Point start = valid_pixels[valid_pixels.size() / 2];
    std::queue<cv::Point> queue;
    queue.push(start);
    visited.at<uint8_t>(start) = 1;

    // BFS로 전파
    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};

    while (!queue.empty()) {
        cv::Point p = queue.front();
        queue.pop();

        for (int i = 0; i < 4; ++i) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];

            if (nx >= 0 && nx < unwrapped.cols &&
                ny >= 0 && ny < unwrapped.rows &&
                !visited.at<uint8_t>(ny, nx) &&
                mask.at<uint8_t>(ny, nx)) {

                // 위상 차이 계산
                float phase_diff = wrapped_phase.at<float>(ny, nx) -
                                  unwrapped.at<float>(p.y, p.x);

                // 2π 단위로 조정
                int k = std::round(phase_diff / (2.0f * M_PI));
                unwrapped.at<float>(ny, nx) =
                    wrapped_phase.at<float>(ny, nx) - k * 2.0f * M_PI;

                visited.at<uint8_t>(ny, nx) = 1;
                queue.push(cv::Point(nx, ny));
            }
        }
    }

    return unwrapped;
}

cv::Mat PhaseShiftGenerator::unwrapPhaseMultiFreq(
    const std::vector<cv::Mat>& wrapped_phases,
    const std::vector<int>& frequencies) {

    if (wrapped_phases.size() != frequencies.size()) {
        throw std::runtime_error("Number of phases and frequencies must match");
    }

    // 저주파부터 고주파로 언래핑
    cv::Mat absolute_phase = wrapped_phases[0].clone();

    for (size_t i = 1; i < wrapped_phases.size(); ++i) {
        float ratio = static_cast<float>(frequencies[i]) / frequencies[i-1];

        cv::Mat k = (absolute_phase * ratio - wrapped_phases[i]) / (2.0f * M_PI);
        cv::Mat k_rounded;
        k.convertTo(k_rounded, CV_32F);

        for (int y = 0; y < k.rows; ++y) {
            for (int x = 0; x < k.cols; ++x) {
                k_rounded.at<float>(y, x) = std::round(k.at<float>(y, x));
            }
        }

        absolute_phase = (wrapped_phases[i] + k_rounded * 2.0f * M_PI) / ratio;
    }

    return absolute_phase;
}

int PhaseShiftGenerator::getTotalPatternCount() const {
    int count = n_steps_;

    if (direction_ == Direction::BOTH) {
        count *= 2;
    }

    return count;
}

void PhaseShiftGenerator::printInfo() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Phase Shifting Pattern Generator" << std::endl;
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

    std::cout << "Phase steps: " << n_steps_ << std::endl;
    std::cout << "Periods: " << n_periods_ << std::endl;
    std::cout << "Total patterns: " << getTotalPatternCount() << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

} // namespace structured_light
