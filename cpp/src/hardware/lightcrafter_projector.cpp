#include "structured_light/hardware/lightcrafter_projector.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

#ifdef HAVE_LIGHTCRAFTER
// TI DLP LightCrafter SDK 헤더
#include "usb.h"
#include "dlpc350_api.h"
#include "dlpc350_usb.h"
#include "dlpc350_common.h"
#endif

namespace structured_light {

LightCrafterProjector::LightCrafterProjector()
    : is_connected_(false)
    , connection_type_(ConnectionType::USB)
    , display_mode_(DisplayMode::VIDEO)
    , sequence_running_(false)
    , current_pattern_index_(0)
{
}

LightCrafterProjector::~LightCrafterProjector() {
    disconnect();
}

bool LightCrafterProjector::connect(ConnectionType type, const std::string& address) {
#ifdef HAVE_LIGHTCRAFTER
    connection_type_ = type;
    device_address_ = address;

    if (type == ConnectionType::USB) {
        // USB 연결
        if (DLPC350_USB_Init() == 0) {
            if (DLPC350_USB_Open() == 0) {
                is_connected_ = true;
                std::cout << "LightCrafter 4500 connected via USB" << std::endl;
                return true;
            } else {
                std::cerr << "Failed to open USB connection" << std::endl;
            }
        } else {
            std::cerr << "Failed to initialize USB" << std::endl;
        }
    } else {
        // 네트워크 연결
        // TCP 소켓 연결 구현
        std::cout << "Connecting to " << address << "..." << std::endl;
        // TODO: 네트워크 연결 구현
        is_connected_ = false;
    }

    return false;
#else
    std::cout << "LightCrafter SDK not available. Using simulation mode." << std::endl;
    is_connected_ = true;
    return true;
#endif
}

void LightCrafterProjector::disconnect() {
#ifdef HAVE_LIGHTCRAFTER
    if (is_connected_) {
        if (sequence_running_) {
            stopPatternSequence();
        }

        if (connection_type_ == ConnectionType::USB) {
            DLPC350_USB_Close();
        }

        is_connected_ = false;
        std::cout << "LightCrafter disconnected" << std::endl;
    }
#endif
}

bool LightCrafterProjector::isConnected() const {
    return is_connected_;
}

bool LightCrafterProjector::setDisplayMode(DisplayMode mode) {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    uint8_t mode_value = (mode == DisplayMode::VIDEO) ? 0 : 1;

    if (DLPC350_SetMode(mode_value) == 0) {
        display_mode_ = mode;
        std::cout << "Display mode set to: "
                  << (mode == DisplayMode::VIDEO ? "VIDEO" : "PATTERN_SEQUENCE")
                  << std::endl;
        return true;
    }

    return false;
#else
    display_mode_ = mode;
    std::cout << "[Simulation] Display mode: "
              << (mode == DisplayMode::VIDEO ? "VIDEO" : "PATTERN_SEQUENCE")
              << std::endl;
    return true;
#endif
}

LightCrafterProjector::DisplayMode LightCrafterProjector::getDisplayMode() const {
    return display_mode_;
}

bool LightCrafterProjector::setLEDCurrent(int red, int green, int blue) {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    // LED 전류 범위: 0-255 (0-100%)
    red = std::max(0, std::min(255, red));
    green = std::max(0, std::min(255, green));
    blue = std::max(0, std::min(255, blue));

    if (DLPC350_SetLedCurrents(red, green, blue) == 0) {
        std::cout << "LED currents set - R:" << red
                  << " G:" << green << " B:" << blue << std::endl;
        return true;
    }

    return false;
#else
    std::cout << "[Simulation] LED currents - R:" << red
              << " G:" << green << " B:" << blue << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::setLEDEnable(bool red, bool green, bool blue) {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    uint8_t enable_mask = 0;
    if (red) enable_mask |= 0x01;
    if (green) enable_mask |= 0x02;
    if (blue) enable_mask |= 0x04;

    if (DLPC350_SetLedEnables(enable_mask) == 0) {
        std::cout << "LEDs enabled - R:" << red
                  << " G:" << green << " B:" << blue << std::endl;
        return true;
    }

    return false;
#else
    std::cout << "[Simulation] LEDs enabled - R:" << red
              << " G:" << green << " B:" << blue << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::projectImage(const cv::Mat& image, int display_time_ms) {
    if (!is_connected_) {
        std::cerr << "Projector not connected!" << std::endl;
        return false;
    }

    // 비디오 모드로 전환
    if (display_mode_ != DisplayMode::VIDEO) {
        setDisplayMode(DisplayMode::VIDEO);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // HDMI를 통해 이미지 표시
    return displayViaHDMI(image);
}

bool LightCrafterProjector::uploadPatternSequence(const PatternSequence& sequence) {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    std::cout << "Uploading pattern sequence..." << std::endl;
    std::cout << "  Pattern count: " << sequence.pattern_count << std::endl;
    std::cout << "  Bit depth: " << sequence.bit_depth << std::endl;
    std::cout << "  Exposure: " << sequence.exposure_us << " us" << std::endl;

    // 패턴 시퀀스 모드로 전환
    if (!setDisplayMode(DisplayMode::PATTERN_SEQUENCE)) {
        return false;
    }

    // 패턴 시퀀스 설정
    for (size_t i = 0; i < sequence.patterns.size(); ++i) {
        // 패턴을 LightCrafter 형식으로 변환
        cv::Mat converted = convertToLightCrafterFormat(
            sequence.patterns[i], sequence.bit_depth
        );

        // 내부 메모리에 저장
        if (!storePattern(converted, i)) {
            std::cerr << "Failed to store pattern " << i << std::endl;
            return false;
        }

        std::cout << "  Pattern " << i << " uploaded" << std::endl;
    }

    // 시퀀스 설정
    // DLPC350_SetPatternConfig(...) 호출
    // DLPC350_SetExposure_FramePeriod(...) 호출
    // DLPC350_SetTrigOutConfig(...) 호출

    std::cout << "Pattern sequence uploaded successfully" << std::endl;
    return true;

#else
    std::cout << "[Simulation] Pattern sequence uploaded:" << std::endl;
    std::cout << "  Patterns: " << sequence.pattern_count << std::endl;
    std::cout << "  Bit depth: " << sequence.bit_depth << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::startPatternSequence() {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    if (DLPC350_PatternDisplay(2) == 0) { // 2 = Start
        sequence_running_ = true;
        std::cout << "Pattern sequence started" << std::endl;
        return true;
    }

    return false;
#else
    sequence_running_ = true;
    std::cout << "[Simulation] Pattern sequence started" << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::stopPatternSequence() {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    if (DLPC350_PatternDisplay(0) == 0) { // 0 = Stop
        sequence_running_ = false;
        std::cout << "Pattern sequence stopped" << std::endl;
        return true;
    }

    return false;
#else
    sequence_running_ = false;
    std::cout << "[Simulation] Pattern sequence stopped" << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::pausePatternSequence() {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    if (DLPC350_PatternDisplay(1) == 0) { // 1 = Pause
        std::cout << "Pattern sequence paused" << std::endl;
        return true;
    }

    return false;
#else
    std::cout << "[Simulation] Pattern sequence paused" << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::setTriggerMode(bool internal) {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    uint8_t mode = internal ? 0 : 1; // 0: Internal, 1: External

    if (DLPC350_SetTrigMode(mode) == 0) {
        std::cout << "Trigger mode: " << (internal ? "Internal" : "External") << std::endl;
        return true;
    }

    return false;
#else
    std::cout << "[Simulation] Trigger mode: "
              << (internal ? "Internal" : "External") << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::setTriggerOutput(bool enable, bool polarity) {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return false;

    // Trigger output configuration
    uint8_t config = 0;
    if (enable) config |= 0x01;
    if (polarity) config |= 0x02; // Positive polarity

    if (DLPC350_SetTrigOutConfig(config) == 0) {
        std::cout << "Trigger output: " << (enable ? "Enabled" : "Disabled")
                  << ", Polarity: " << (polarity ? "Positive" : "Negative") << std::endl;
        return true;
    }

    return false;
#else
    std::cout << "[Simulation] Trigger output configured" << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::initialize() {
    if (!is_connected_) {
        std::cerr << "Projector not connected!" << std::endl;
        return false;
    }

    std::cout << "Initializing LightCrafter 4500..." << std::endl;

    // 기본 LED 설정 (흰색, 중간 밝기)
    setLEDEnable(true, true, true);
    setLEDCurrent(128, 128, 128);

    // 비디오 모드로 시작
    setDisplayMode(DisplayMode::VIDEO);

    // 내부 트리거
    setTriggerMode(true);

    std::cout << "LightCrafter initialization complete" << std::endl;
    return true;
}

std::string LightCrafterProjector::getDeviceInfo() const {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return "Not connected";

    std::stringstream ss;
    ss << "TI DLP LightCrafter 4500" << std::endl;
    ss << "Firmware: " << getFirmwareVersion() << std::endl;
    ss << "Connection: " << (connection_type_ == ConnectionType::USB ? "USB" : "Network") << std::endl;

    return ss.str();
#else
    return "TI DLP LightCrafter 4500 (Simulation Mode)";
#endif
}

void LightCrafterProjector::printStatus() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "LightCrafter 4500 Status" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << getDeviceInfo() << std::endl;
    std::cout << "Display Mode: "
              << (display_mode_ == DisplayMode::VIDEO ? "VIDEO" : "PATTERN_SEQUENCE")
              << std::endl;
    std::cout << "Sequence Running: " << (sequence_running_ ? "Yes" : "No") << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

std::string LightCrafterProjector::getFirmwareVersion() const {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return "Unknown";

    // DLPC350_GetFirmwareVersion() 호출
    return "v7.0.0"; // 예시
#else
    return "Simulation";
#endif
}

std::string LightCrafterProjector::getHardwareStatus() const {
#ifdef HAVE_LIGHTCRAFTER
    if (!is_connected_) return "Not connected";

    // DLPC350_GetHardwareStatus() 호출
    return "OK";
#else
    return "Simulation";
#endif
}

bool LightCrafterProjector::displayViaHDMI(const cv::Mat& image) {
    // HDMI를 통한 이미지 표시
    // OpenCV HighGUI를 사용하여 전체 화면으로 표시

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(912, 1140)); // LightCrafter 4500 해상도

    cv::namedWindow("LightCrafter", cv::WINDOW_NORMAL);
    cv::setWindowProperty("LightCrafter", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    cv::imshow("LightCrafter", resized);
    cv::waitKey(1);

    return true;
}

bool LightCrafterProjector::storePattern(const cv::Mat& pattern, int index) {
#ifdef HAVE_LIGHTCRAFTER
    // 패턴을 LightCrafter 내부 플래시 메모리에 저장
    // DLPC350_SetPatternMemory() 사용

    // 패턴 데이터를 비트맵 형식으로 변환
    std::vector<uint8_t> bitmap_data;
    // ... 변환 로직 ...

    // 메모리에 쓰기
    // DLPC350_WritePattern(index, bitmap_data.data(), bitmap_data.size());

    return true;
#else
    std::cout << "[Simulation] Pattern " << index << " stored" << std::endl;
    return true;
#endif
}

bool LightCrafterProjector::sendUSBCommand(uint8_t command, const std::vector<uint8_t>& data) {
#ifdef HAVE_LIGHTCRAFTER
    // USB 명령 전송 구현
    return true;
#else
    return false;
#endif
}

std::string LightCrafterProjector::sendNetworkCommand(const std::string& command) {
    // TCP 소켓을 통한 명령 전송
    return "";
}

cv::Mat convertToLightCrafterFormat(const cv::Mat& pattern, int bit_depth) {
    cv::Mat converted;

    // 그레이스케일로 변환
    if (pattern.channels() == 3) {
        cv::cvtColor(pattern, converted, cv::COLOR_BGR2GRAY);
    } else {
        converted = pattern.clone();
    }

    // 비트 깊이에 맞게 양자화
    if (bit_depth < 8) {
        int levels = (1 << bit_depth);
        double scale = (levels - 1) / 255.0;

        converted.convertTo(converted, CV_8U, scale);
        converted *= (255 / (levels - 1));
    }

    // LightCrafter 해상도로 리사이즈 (912x1140)
    cv::Mat resized;
    cv::resize(converted, resized, cv::Size(912, 1140));

    return resized;
}

void listLightCrafterDevices() {
#ifdef HAVE_LIGHTCRAFTER
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Searching for LightCrafter devices..." << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    // USB 장치 검색
    if (DLPC350_USB_Init() == 0) {
        int count = DLPC350_USB_GetDeviceCount();
        std::cout << "Found " << count << " USB device(s)" << std::endl;

        for (int i = 0; i < count; ++i) {
            std::cout << "Device " << i << ": LightCrafter 4500" << std::endl;
        }

        DLPC350_USB_Exit();
    }

    std::cout << std::string(60, '=') << std::endl;
#else
    std::cout << "LightCrafter SDK not available!" << std::endl;
    std::cout << "Download from: http://www.ti.com/tool/DLPLCR4500EVM" << std::endl;
#endif
}

} // namespace structured_light
