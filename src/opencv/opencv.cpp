#include "opencv.h"
#include "widget.h"
#include "framerate.h"

#include <thread>
#include <chrono>
#include <vector>

extern bool state; // for manage while state in frameBuffer

// start video camera
cv::VideoCapture OpenCV::startStream()
{
    cv::VideoCapture _cap;
    _cap.open(0);
    if(!_cap.isOpened()){ std::cout << "Camera is not open" << std::endl; }
    else { return _cap; };
    return 0;
}

// stop vidoe camera
void OpenCV::stopStream()
{
    cv::VideoCapture _cap;
    _cap.release();
    if(!_cap.isOpened()){ std::cout << "Camera is closed" << std::endl; }
}

// store camera frame in ringbuffer
void OpenCV::frameBuffer(cv::VideoCapture _cap)
{
    while(state) {
        _cap >> _frame;
        if(!_frame.empty()){ add(_frame); }
    }
}

// get color frame of ringbuffer
cv::Mat OpenCV::getFrameColor()
{
    _output = get();
    cv::cvtColor(_output, _output1, CV_BGR2RGB);
    return _output1;
}

// get gray frame of ringbuffer
cv::Mat OpenCV::getFrameGray()
{
    _output = get();
    cv::cvtColor(_output, _output1, CV_BGR2GRAY);
    return _output1;
}

// face detection on frame from ring buffer
cv::Mat OpenCV::getFaceFrame(float circle_dia)
{
    _frameFace = getFrameColor();
    circle_dia = circle_dia / 100;
    // Initialize the inbuilt Harr Cascade frontal face detection
    cv::CascadeClassifier face_cascade;
    face_cascade.load("haarcascade_frontalface_alt2.xml");

    // Container of faces
    std::vector<cv::Rect> faces;

    // Detect faces
    face_cascade.detectMultiScale(_frameFace, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE,
                                  cv::Size(120, 120));

    // Show the results
    for(uint i = 0; i < static_cast<uint>(faces.size()); i++)
    {
        cv::Point center(static_cast<int>(faces[i].x) +
                         static_cast<int>(faces[i].width*0.5),
                         static_cast<int>(faces[i].y + faces[i].height*0.5));

        ellipse(_frameFace, center,
                cv::Size(static_cast<int>(faces[i].width*circle_dia),
                         static_cast<int>(faces[i].height*circle_dia)),
                         0, 0, 360, cv::Scalar(0, 0, 0), 4, 8, 0);
    }
    return _frameFace;
}

cv::Mat OpenCV::getRGBFrame(int _chR, int _chG, int _chB)
{
    _frameRGB = getFrameColor();
    cv::Mat _fin_img;
    cv::Mat chR, chG, chB;
    std::vector<cv::Mat> channels(3);
    cv::split(_frameRGB, channels);

    chR = channels[1];
    chG = channels[1];
    chB = channels[2];

    chR = chR + _chR;
    chG = chG + _chG;
    chB = chB + _chB;
    cv::merge(channels, _fin_img);

    return _fin_img;
}
