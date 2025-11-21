#ifndef STRUCTURED_LIGHT_LIGHTCRAFTER_PROJECTOR_H
#define STRUCTURED_LIGHT_LIGHTCRAFTER_PROJECTOR_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

namespace structured_light {

/**
 * @brief Pattern Sequence 설정
 */
struct PatternSequence {
    int bit_depth;              // 비트 깊이 (1, 2, 3, 4, 5, 6, 7, 8)
    int pattern_count;          // 패턴 개수
    int exposure_us;            // 노출 시간 (마이크로초)
    int frame_period_us;        // 프레임 주기
    bool trigger_out;           // 트리거 출력 활성화
    std::vector<cv::Mat> patterns;  // 패턴 이미지들
};

/**
 * @brief TI DLP LightCrafter 4500 프로젝터 컨트롤러
 *
 * TI DLP LightCrafter 4500 프로젝터 제어
 * - 패턴 시퀀스 모드
 * - 고속 투영 (최대 4kHz)
 * - USB/네트워크 제어
 */
class LightCrafterProjector {
public:
    /**
     * @brief 프로젝터 연결 타입
     */
    enum class ConnectionType {
        USB,
        NETWORK
    };

    /**
     * @brief 디스플레이 모드
     */
    enum class DisplayMode {
        VIDEO,              // 비디오 모드 (HDMI)
        PATTERN_SEQUENCE    // 패턴 시퀀스 모드
    };

    /**
     * @brief 생성자
     */
    LightCrafterProjector();

    /**
     * @brief 소멸자
     */
    ~LightCrafterProjector();

    /**
     * @brief 프로젝터 연결
     * @param type 연결 타입 (USB 또는 Network)
     * @param address 네트워크 주소 (네트워크 연결시)
     * @return 성공 여부
     */
    bool connect(ConnectionType type = ConnectionType::USB,
                const std::string& address = "192.168.1.100");

    /**
     * @brief 프로젝터 연결 해제
     */
    void disconnect();

    /**
     * @brief 연결 여부 확인
     * @return 연결 여부
     */
    bool isConnected() const;

    /**
     * @brief 디스플레이 모드 설정
     * @param mode 디스플레이 모드
     * @return 성공 여부
     */
    bool setDisplayMode(DisplayMode mode);

    /**
     * @brief 현재 디스플레이 모드 가져오기
     * @return 디스플레이 모드
     */
    DisplayMode getDisplayMode() const;

    /**
     * @brief LED 전원 설정
     * @param red 레드 LED 전류 (0-255)
     * @param green 그린 LED 전류 (0-255)
     * @param blue 블루 LED 전류 (0-255)
     * @return 성공 여부
     */
    bool setLEDCurrent(int red, int green, int blue);

    /**
     * @brief LED 활성화/비활성화
     * @param red 레드 LED
     * @param green 그린 LED
     * @param blue 블루 LED
     * @return 성공 여부
     */
    bool setLEDEnable(bool red, bool green, bool blue);

    /**
     * @brief 비디오 모드로 단일 이미지 투영
     * @param image 투영할 이미지
     * @param display_time_ms 표시 시간 (밀리초)
     * @return 성공 여부
     */
    bool projectImage(const cv::Mat& image, int display_time_ms = 0);

    /**
     * @brief 패턴 시퀀스 업로드
     * @param sequence 패턴 시퀀스 설정
     * @return 성공 여부
     */
    bool uploadPatternSequence(const PatternSequence& sequence);

    /**
     * @brief 패턴 시퀀스 시작
     * @return 성공 여부
     */
    bool startPatternSequence();

    /**
     * @brief 패턴 시퀀스 중지
     * @return 성공 여부
     */
    bool stopPatternSequence();

    /**
     * @brief 패턴 시퀀스 일시 정지
     * @return 성공 여부
     */
    bool pausePatternSequence();

    /**
     * @brief 트리거 모드 설정
     * @param internal true: 내부 트리거, false: 외부 트리거
     * @return 성공 여부
     */
    bool setTriggerMode(bool internal);

    /**
     * @brief 트리거 출력 설정
     * @param enable 트리거 출력 활성화
     * @param polarity true: Positive, false: Negative
     * @return 성공 여부
     */
    bool setTriggerOutput(bool enable, bool polarity = true);

    /**
     * @brief 프로젝터 초기화 및 기본 설정
     * @return 성공 여부
     */
    bool initialize();

    /**
     * @brief 프로젝터 정보 가져오기
     * @return 프로젝터 정보 문자열
     */
    std::string getDeviceInfo() const;

    /**
     * @brief 프로젝터 상태 출력
     */
    void printStatus() const;

    /**
     * @brief 펌웨어 버전 가져오기
     * @return 펌웨어 버전
     */
    std::string getFirmwareVersion() const;

    /**
     * @brief 하드웨어 상태 가져오기
     * @return 상태 정보
     */
    std::string getHardwareStatus() const;

private:
    bool is_connected_;
    ConnectionType connection_type_;
    DisplayMode display_mode_;
    std::string device_address_;

    // 패턴 시퀀스 상태
    bool sequence_running_;
    int current_pattern_index_;

    /**
     * @brief HDMI를 통해 이미지 표시 (비디오 모드)
     * @param image 이미지
     * @return 성공 여부
     */
    bool displayViaHDMI(const cv::Mat& image);

    /**
     * @brief 패턴을 내부 메모리에 저장
     * @param pattern 패턴 이미지
     * @param index 패턴 인덱스
     * @return 성공 여부
     */
    bool storePattern(const cv::Mat& pattern, int index);

    /**
     * @brief USB 명령 전송
     * @param command 명령 바이트
     * @param data 데이터
     * @return 성공 여부
     */
    bool sendUSBCommand(uint8_t command, const std::vector<uint8_t>& data);

    /**
     * @brief 네트워크 명령 전송
     * @param command 명령 문자열
     * @return 응답 문자열
     */
    std::string sendNetworkCommand(const std::string& command);
};

/**
 * @brief 패턴을 LightCrafter 형식으로 변환
 * @param pattern 입력 패턴 (CV_8UC1 또는 CV_8UC3)
 * @param bit_depth 비트 깊이
 * @return 변환된 패턴
 */
cv::Mat convertToLightCrafterFormat(const cv::Mat& pattern, int bit_depth);

/**
 * @brief 사용 가능한 LightCrafter 장치 검색
 */
void listLightCrafterDevices();

} // namespace structured_light

#endif // STRUCTURED_LIGHT_LIGHTCRAFTER_PROJECTOR_H
