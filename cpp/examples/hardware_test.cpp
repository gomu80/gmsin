/**
 * @file hardware_test.cpp
 * @brief 하드웨어 테스트 예제
 *
 * Basler 카메라와 LightCrafter 프로젝터 테스트
 */

#include "structured_light/hardware/basler_camera.h"
#include "structured_light/hardware/lightcrafter_projector.h"
#include <iostream>

using namespace structured_light;

void testBaslerCamera() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Basler Camera Test" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // 사용 가능한 카메라 나열
    listBaslerCameras();

    // 카메라 연결
    BaslerCamera camera(0);

    if (!camera.open()) {
        std::cerr << "Failed to open camera!" << std::endl;
        return;
    }

    // 카메라 설정
    camera.setExposureTime(10000); // 10ms
    camera.setGain(0);
    camera.setFrameRate(30);

    // 파라미터 출력
    camera.printCameraParameters();

    // 이미지 캡처
    cv::Mat image;
    if (camera.grabImage(image)) {
        std::cout << "\nImage captured: " << image.cols << "x" << image.rows << std::endl;

        // 이미지 표시
        cv::imshow("Basler Camera", image);
        std::cout << "Press any key to continue..." << std::endl;
        cv::waitKey(0);

        // 이미지 저장
        cv::imwrite("data/captured/test_image.png", image);
        std::cout << "Image saved to: data/captured/test_image.png" << std::endl;
    }

    // 연속 캡처 테스트
    std::cout << "\nCapturing sequence of 5 images..." << std::endl;
    std::vector<cv::Mat> images;
    if (camera.grabSequence(5, images, 200)) {
        std::cout << "Captured " << images.size() << " images" << std::endl;
    }

    camera.close();
}

void testLightCrafterProjector() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "LightCrafter Projector Test" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // 프로젝터 연결
    LightCrafterProjector projector;

    if (!projector.connect(LightCrafterProjector::ConnectionType::USB)) {
        std::cerr << "Failed to connect to projector!" << std::endl;
        return;
    }

    // 프로젝터 초기화
    if (!projector.initialize()) {
        std::cerr << "Failed to initialize projector!" << std::endl;
        return;
    }

    // 상태 출력
    projector.printStatus();

    // 단일 이미지 투영 테스트
    std::cout << "\nTesting image projection..." << std::endl;

    // 테스트 패턴 생성 (체스보드)
    cv::Mat test_pattern(1140, 912, CV_8UC1);
    int square_size = 114; // 10x8 체스보드

    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 8; ++x) {
            int color = ((x + y) % 2) ? 255 : 0;
            cv::rectangle(test_pattern,
                         cv::Point(x * square_size, y * square_size),
                         cv::Point((x + 1) * square_size, (y + 1) * square_size),
                         cv::Scalar(color), -1);
        }
    }

    projector.projectImage(test_pattern, 2000);
    std::cout << "Test pattern projected for 2 seconds" << std::endl;

    // LED 설정 테스트
    std::cout << "\nTesting LED control..." << std::endl;
    projector.setLEDEnable(true, false, false); // 레드만
    std::this_thread::sleep_for(std::chrono::seconds(1));

    projector.setLEDEnable(false, true, false); // 그린만
    std::this_thread::sleep_for(std::chrono::seconds(1));

    projector.setLEDEnable(false, false, true); // 블루만
    std::this_thread::sleep_for(std::chrono::seconds(1));

    projector.setLEDEnable(true, true, true); // 모두 켜기

    projector.disconnect();
}

void testSynchronizedCapture() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Synchronized Camera-Projector Test" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // 카메라와 프로젝터 초기화
    BaslerCamera camera(0);
    LightCrafterProjector projector;

    if (!camera.open() || !projector.connect()) {
        std::cerr << "Failed to initialize hardware!" << std::endl;
        return;
    }

    projector.initialize();
    camera.setExposureTime(5000); // 5ms

    // 간단한 스트라이프 패턴 생성
    cv::Mat stripe_pattern(1140, 912, CV_8UC1, cv::Scalar(0));
    for (int x = 0; x < 912; x += 20) {
        cv::rectangle(stripe_pattern, cv::Point(x, 0), cv::Point(x + 10, 1140),
                     cv::Scalar(255), -1);
    }

    // 패턴 투영 및 캡처
    projector.projectImage(stripe_pattern);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    cv::Mat captured;
    if (camera.grabImage(captured)) {
        std::cout << "Synchronized capture successful!" << std::endl;
        cv::imshow("Captured Pattern", captured);
        cv::waitKey(2000);

        cv::imwrite("data/captured/stripe_pattern.png", captured);
    }

    camera.close();
    projector.disconnect();
}

int main(int argc, char** argv) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Structured Light Hardware Test\n";
    std::cout << "========================================\n";

    try {
        // 1. Basler 카메라 테스트
        testBaslerCamera();

        // 2. LightCrafter 프로젝터 테스트
        testLightCrafterProjector();

        // 3. 동기화 캡처 테스트
        testSynchronizedCapture();

        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "All tests completed!" << std::endl;
        std::cout << std::string(70, '=') << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
