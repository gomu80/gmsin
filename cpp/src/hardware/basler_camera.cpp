#include "structured_light/hardware/basler_camera.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace structured_light {

BaslerCamera::BaslerCamera(int camera_index)
    : camera_index_(camera_index)
    , is_open_(false)
    , image_width_(0)
    , image_height_(0)
{
#ifdef HAVE_PYLON
    PylonInitialize();
    format_converter_.OutputPixelFormat = PixelType_BGR8packed;
#endif
}

BaslerCamera::~BaslerCamera() {
    close();
#ifdef HAVE_PYLON
    PylonTerminate();
#endif
}

bool BaslerCamera::open() {
#ifdef HAVE_PYLON
    try {
        // 카메라 장치 찾기
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        DeviceInfoList_t devices;

        if (tlFactory.EnumerateDevices(devices) == 0) {
            std::cerr << "No Basler cameras found!" << std::endl;
            return false;
        }

        if (camera_index_ >= static_cast<int>(devices.size())) {
            std::cerr << "Camera index " << camera_index_ << " out of range!" << std::endl;
            return false;
        }

        // 카메라 연결
        camera_.Attach(tlFactory.CreateDevice(devices[camera_index_]));
        camera_.Open();

        // 기본 설정
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();

        // 픽셀 포맷 설정
        CEnumerationPtr pixelFormat(nodemap.GetNode("PixelFormat"));
        if (IsAvailable(pixelFormat)) {
            pixelFormat->FromString("Mono8"); // 또는 "BayerRG8" 등
        }

        // 이미지 크기 가져오기
        CIntegerPtr width(nodemap.GetNode("Width"));
        CIntegerPtr height(nodemap.GetNode("Height"));
        if (IsReadable(width) && IsReadable(height)) {
            image_width_ = width->GetValue();
            image_height_ = height->GetValue();
        }

        is_open_ = true;
        std::cout << "Basler camera opened successfully" << std::endl;
        std::cout << "Resolution: " << image_width_ << "x" << image_height_ << std::endl;

        return true;

    } catch (const GenericException &e) {
        std::cerr << "Pylon error: " << e.GetDescription() << std::endl;
        return false;
    }
#else
    std::cerr << "Pylon SDK not available!" << std::endl;
    return false;
#endif
}

void BaslerCamera::close() {
#ifdef HAVE_PYLON
    if (is_open_) {
        try {
            if (camera_.IsGrabbing()) {
                camera_.StopGrabbing();
            }
            camera_.Close();
            is_open_ = false;
            std::cout << "Basler camera closed" << std::endl;
        } catch (const GenericException &e) {
            std::cerr << "Error closing camera: " << e.GetDescription() << std::endl;
        }
    }
#endif
}

bool BaslerCamera::isOpen() const {
    return is_open_;
}

bool BaslerCamera::grabImage(cv::Mat& image, int timeout_ms) {
#ifdef HAVE_PYLON
    if (!is_open_) {
        std::cerr << "Camera not open!" << std::endl;
        return false;
    }

    try {
        camera_.StartGrabbing(1, GrabStrategy_LatestImageOnly);

        if (camera_.RetrieveResult(timeout_ms, grab_result_, TimeoutHandling_ThrowException)) {
            if (grab_result_->GrabSucceeded()) {
                return convertToMat(image);
            } else {
                std::cerr << "Grab failed: " << grab_result_->GetErrorDescription() << std::endl;
                return false;
            }
        }

    } catch (const GenericException &e) {
        std::cerr << "Grab error: " << e.GetDescription() << std::endl;
        return false;
    }
#endif
    return false;
}

bool BaslerCamera::grabSequence(int num_images, std::vector<cv::Mat>& images, int delay_ms) {
    images.clear();
    images.reserve(num_images);

    for (int i = 0; i < num_images; ++i) {
        cv::Mat image;
        if (grabImage(image)) {
            images.push_back(image.clone());
            std::cout << "Captured image " << (i+1) << "/" << num_images << std::endl;

            if (i < num_images - 1 && delay_ms > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            }
        } else {
            std::cerr << "Failed to capture image " << (i+1) << std::endl;
            return false;
        }
    }

    return true;
}

bool BaslerCamera::startGrabbing() {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        camera_.StartGrabbing(GrabStrategy_LatestImageOnly);
        return true;
    } catch (const GenericException &e) {
        std::cerr << "Start grabbing error: " << e.GetDescription() << std::endl;
        return false;
    }
#endif
    return false;
}

void BaslerCamera::stopGrabbing() {
#ifdef HAVE_PYLON
    if (is_open_ && camera_.IsGrabbing()) {
        camera_.StopGrabbing();
    }
#endif
}

bool BaslerCamera::setImageSize(int width, int height) {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();

        CIntegerPtr widthNode(nodemap.GetNode("Width"));
        CIntegerPtr heightNode(nodemap.GetNode("Height"));

        if (IsWritable(widthNode) && IsWritable(heightNode)) {
            widthNode->SetValue(width);
            heightNode->SetValue(height);
            image_width_ = width;
            image_height_ = height;
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Set image size error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

void BaslerCamera::getImageSize(int& width, int& height) const {
    width = image_width_;
    height = image_height_;
}

bool BaslerCamera::setExposureTime(double exposure_us) {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CFloatPtr exposureTime(nodemap.GetNode("ExposureTime"));

        if (IsWritable(exposureTime)) {
            exposureTime->SetValue(exposure_us);
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Set exposure error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

double BaslerCamera::getExposureTime() const {
#ifdef HAVE_PYLON
    if (!is_open_) return 0.0;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CFloatPtr exposureTime(nodemap.GetNode("ExposureTime"));

        if (IsReadable(exposureTime)) {
            return exposureTime->GetValue();
        }
    } catch (const GenericException &e) {
        std::cerr << "Get exposure error: " << e.GetDescription() << std::endl;
    }
#endif
    return 0.0;
}

bool BaslerCamera::setAutoExposure(bool enable) {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CEnumerationPtr exposureAuto(nodemap.GetNode("ExposureAuto"));

        if (IsWritable(exposureAuto)) {
            exposureAuto->FromString(enable ? "Continuous" : "Off");
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Set auto exposure error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

bool BaslerCamera::setGain(double gain) {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CFloatPtr gainNode(nodemap.GetNode("Gain"));

        if (IsWritable(gainNode)) {
            gainNode->SetValue(gain);
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Set gain error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

double BaslerCamera::getGain() const {
#ifdef HAVE_PYLON
    if (!is_open_) return 0.0;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CFloatPtr gainNode(nodemap.GetNode("Gain"));

        if (IsReadable(gainNode)) {
            return gainNode->GetValue();
        }
    } catch (const GenericException &e) {
        std::cerr << "Get gain error: " << e.GetDescription() << std::endl;
    }
#endif
    return 0.0;
}

bool BaslerCamera::setAutoGain(bool enable) {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CEnumerationPtr gainAuto(nodemap.GetNode("GainAuto"));

        if (IsWritable(gainAuto)) {
            gainAuto->FromString(enable ? "Continuous" : "Off");
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Set auto gain error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

bool BaslerCamera::setFrameRate(double fps) {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();

        // Frame rate enable
        CBooleanPtr frameRateEnable(nodemap.GetNode("AcquisitionFrameRateEnable"));
        if (IsWritable(frameRateEnable)) {
            frameRateEnable->SetValue(true);
        }

        // Set frame rate
        CFloatPtr frameRate(nodemap.GetNode("AcquisitionFrameRate"));
        if (IsWritable(frameRate)) {
            frameRate->SetValue(fps);
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Set frame rate error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

double BaslerCamera::getFrameRate() const {
#ifdef HAVE_PYLON
    if (!is_open_) return 0.0;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CFloatPtr frameRate(nodemap.GetNode("AcquisitionFrameRate"));

        if (IsReadable(frameRate)) {
            return frameRate->GetValue();
        }
    } catch (const GenericException &e) {
        std::cerr << "Get frame rate error: " << e.GetDescription() << std::endl;
    }
#endif
    return 0.0;
}

bool BaslerCamera::setTriggerMode(bool enable) {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CEnumerationPtr triggerMode(nodemap.GetNode("TriggerMode"));

        if (IsWritable(triggerMode)) {
            triggerMode->FromString(enable ? "On" : "Off");

            if (enable) {
                // Set trigger source to Software
                CEnumerationPtr triggerSource(nodemap.GetNode("TriggerSource"));
                if (IsWritable(triggerSource)) {
                    triggerSource->FromString("Software");
                }
            }
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Set trigger mode error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

bool BaslerCamera::executeSoftwareTrigger() {
#ifdef HAVE_PYLON
    if (!is_open_) return false;

    try {
        GenApi::INodeMap& nodemap = camera_.GetNodeMap();
        CCommandPtr triggerSoftware(nodemap.GetNode("TriggerSoftware"));

        if (IsWritable(triggerSoftware)) {
            triggerSoftware->Execute();
            return true;
        }
    } catch (const GenericException &e) {
        std::cerr << "Execute software trigger error: " << e.GetDescription() << std::endl;
    }
#endif
    return false;
}

std::string BaslerCamera::getDeviceInfo() const {
#ifdef HAVE_PYLON
    if (!is_open_) return "Camera not open";

    try {
        std::stringstream ss;
        ss << "Device Info:" << std::endl;
        ss << "  Vendor: " << camera_.GetDeviceInfo().GetVendorName() << std::endl;
        ss << "  Model: " << camera_.GetDeviceInfo().GetModelName() << std::endl;
        ss << "  Serial: " << camera_.GetDeviceInfo().GetSerialNumber() << std::endl;
        return ss.str();
    } catch (const GenericException &e) {
        return std::string("Error: ") + e.GetDescription();
    }
#endif
    return "Pylon SDK not available";
}

void BaslerCamera::printCameraParameters() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Basler Camera Parameters" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << getDeviceInfo() << std::endl;
    std::cout << "Image Size: " << image_width_ << "x" << image_height_ << std::endl;
    std::cout << "Exposure Time: " << getExposureTime() << " us" << std::endl;
    std::cout << "Gain: " << getGain() << " dB" << std::endl;
    std::cout << "Frame Rate: " << getFrameRate() << " fps" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

bool BaslerCamera::convertToMat(cv::Mat& image) {
#ifdef HAVE_PYLON
    try {
        CPylonImage pylonImage;
        format_converter_.Convert(pylonImage, grab_result_);

        image = cv::Mat(grab_result_->GetHeight(), grab_result_->GetWidth(),
                       CV_8UC1, (uint8_t*)pylonImage.GetBuffer());

        // BGR인 경우
        if (pylonImage.GetPixelType() == PixelType_BGR8packed) {
            image = cv::Mat(grab_result_->GetHeight(), grab_result_->GetWidth(),
                           CV_8UC3, (uint8_t*)pylonImage.GetBuffer());
        }

        image = image.clone(); // 데이터 복사
        return true;

    } catch (const GenericException &e) {
        std::cerr << "Convert error: " << e.GetDescription() << std::endl;
        return false;
    }
#endif
    return false;
}

void listBaslerCameras() {
#ifdef HAVE_PYLON
    try {
        PylonInitialize();

        CTlFactory& tlFactory = CTlFactory::GetInstance();
        DeviceInfoList_t devices;

        int count = tlFactory.EnumerateDevices(devices);

        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Available Basler Cameras: " << count << std::endl;
        std::cout << std::string(60, '=') << std::endl;

        for (size_t i = 0; i < devices.size(); ++i) {
            std::cout << "Camera " << i << ":" << std::endl;
            std::cout << "  Vendor: " << devices[i].GetVendorName() << std::endl;
            std::cout << "  Model: " << devices[i].GetModelName() << std::endl;
            std::cout << "  Serial: " << devices[i].GetSerialNumber() << std::endl;
            std::cout << "  Interface: " << devices[i].GetDeviceClass() << std::endl;
            std::cout << std::endl;
        }

        PylonTerminate();

    } catch (const GenericException &e) {
        std::cerr << "Error: " << e.GetDescription() << std::endl;
    }
#else
    std::cout << "Pylon SDK not available!" << std::endl;
#endif
}

} // namespace structured_light
