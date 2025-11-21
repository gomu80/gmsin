#ifndef STRUCTURED_LIGHT_BASLER_CAMERA_H
#define STRUCTURED_LIGHT_BASLER_CAMERA_H

#include <opencv2/opencv.hpp>
#include <string>
#include <memory>
#include <vector>

#ifdef HAVE_PYLON
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
using namespace Pylon;
#endif

namespace structured_light {

/**
 * @brief Basler Pylon 카메라 컨트롤러
 *
 * Basler USB/GigE 카메라 제어 및 이미지 획득
 */
class BaslerCamera {
public:
    /**
     * @brief 생성자
     * @param camera_index 카메라 인덱스 (0부터 시작)
     */
    explicit BaslerCamera(int camera_index = 0);

    /**
     * @brief 소멸자
     */
    ~BaslerCamera();

    /**
     * @brief 카메라 초기화 및 연결
     * @return 성공 여부
     */
    bool open();

    /**
     * @brief 카메라 연결 해제
     */
    void close();

    /**
     * @brief 카메라가 열려있는지 확인
     * @return 열림 여부
     */
    bool isOpen() const;

    /**
     * @brief 단일 이미지 캡처
     * @param image 출력 이미지
     * @param timeout_ms 타임아웃 (밀리초)
     * @return 성공 여부
     */
    bool grabImage(cv::Mat& image, int timeout_ms = 5000);

    /**
     * @brief 연속 이미지 캡처
     * @param num_images 캡처할 이미지 수
     * @param images 출력 이미지 벡터
     * @param delay_ms 이미지 간 지연 시간
     * @return 성공 여부
     */
    bool grabSequence(int num_images, std::vector<cv::Mat>& images, int delay_ms = 100);

    /**
     * @brief 연속 캡처 시작
     * @return 성공 여부
     */
    bool startGrabbing();

    /**
     * @brief 연속 캡처 중지
     */
    void stopGrabbing();

    /**
     * @brief 이미지 크기 설정
     * @param width 폭
     * @param height 높이
     * @return 성공 여부
     */
    bool setImageSize(int width, int height);

    /**
     * @brief 이미지 크기 가져오기
     * @param width 출력 폭
     * @param height 출력 높이
     */
    void getImageSize(int& width, int& height) const;

    /**
     * @brief 노출 시간 설정 (마이크로초)
     * @param exposure_us 노출 시간
     * @return 성공 여부
     */
    bool setExposureTime(double exposure_us);

    /**
     * @brief 노출 시간 가져오기
     * @return 노출 시간 (마이크로초)
     */
    double getExposureTime() const;

    /**
     * @brief 자동 노출 설정
     * @param enable 활성화 여부
     * @return 성공 여부
     */
    bool setAutoExposure(bool enable);

    /**
     * @brief 게인 설정
     * @param gain 게인 값 (dB)
     * @return 성공 여부
     */
    bool setGain(double gain);

    /**
     * @brief 게인 가져오기
     * @return 게인 값 (dB)
     */
    double getGain() const;

    /**
     * @brief 자동 게인 설정
     * @param enable 활성화 여부
     * @return 성공 여부
     */
    bool setAutoGain(bool enable);

    /**
     * @brief 프레임 레이트 설정
     * @param fps 프레임 레이트
     * @return 성공 여부
     */
    bool setFrameRate(double fps);

    /**
     * @brief 프레임 레이트 가져오기
     * @return 프레임 레이트
     */
    double getFrameRate() const;

    /**
     * @brief 트리거 모드 설정
     * @param enable 활성화 여부
     * @return 성공 여부
     */
    bool setTriggerMode(bool enable);

    /**
     * @brief 소프트웨어 트리거 실행
     * @return 성공 여부
     */
    bool executeSoftwareTrigger();

    /**
     * @brief 카메라 정보 가져오기
     * @return 카메라 정보 문자열
     */
    std::string getDeviceInfo() const;

    /**
     * @brief 카메라 파라미터 출력
     */
    void printCameraParameters() const;

private:
#ifdef HAVE_PYLON
    CInstantCamera camera_;
    CGrabResultPtr grab_result_;
    CImageFormatConverter format_converter_;
#endif

    int camera_index_;
    bool is_open_;
    int image_width_;
    int image_height_;

    /**
     * @brief Pylon 이미지를 OpenCV Mat으로 변환
     */
    bool convertToMat(cv::Mat& image);
};

/**
 * @brief 사용 가능한 Basler 카메라 목록 출력
 */
void listBaslerCameras();

} // namespace structured_light

#endif // STRUCTURED_LIGHT_BASLER_CAMERA_H
