#include "widget.h"
#include "ui_widget.h"
#include "opencv.h"

bool state = true; // initiale state for frame buffer = true

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    timer = new QTimer(this);
}


Widget::~Widget() {
    delete ui;
}

// button: start camera stream
void Widget::on_pushButton_open_Webcam_clicked()
{
    if (stateThread) start_thread();

    ui->horizontalSlider_CannyEdge->setValue(0);
    ui->horizontalSlider_FaceDetection->setValue(50);

    ui->checkBox_CannyEdge->setChecked(false);
    ui->checkBox_FaceDetection->setChecked(false);
    ui->checkBox_RGB_Modifier->setChecked(false);
    ui->checkBox_GOA_Mode->setChecked(false);

    if(!cap_w.isOpened())
    {
        std::cout << "camera is not open" << std::endl;
    }
    else
    {
        if (state_NormalStream) {
        std::cout << "camera is open" << std::endl;
        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(1);
        }
    }
}

// button: stop camera stream
void Widget::on_pushButton_close_Webcam_clicked()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(goa_mode()));

    ui->horizontalSlider_CannyEdge->setValue(0);
    ui->horizontalSlider_FaceDetection->setValue(50);

    ui->checkBox_CannyEdge->setChecked(false);
    ui->checkBox_FaceDetection->setChecked(false);
    ui->checkBox_RGB_Modifier->setChecked(false);
    ui->checkBox_GOA_Mode->setChecked(false);

    state = false;
    cap_w.release();
    if (thread_buffer.joinable()) { thread_buffer.join(); }
    cl_o.stopStream();

    cv::Mat image = cv::Mat::zeros(frame_w.size(), CV_8UC3);
    qt_image = QImage(static_cast<unsigned char*> (image.data), image.cols,
                      image.rows, QImage::Format_RGB888);
    ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
    ui->gui_window->resize(ui->gui_window->pixmap()->size());

    stateThread = true;
    state_NormalStream = true;
}

// start framebuffer in thread
void Widget::start_thread()
{
    state = true;
    cap_w = cl_o.startStream();
    std::thread thread_buffer(&OpenCV::frameBuffer, cl_o, cap_w);
    if (thread_buffer.joinable()) thread_buffer.detach();
    stateThread = false;
}

// show frame in window
void Widget::update_window()
{
    if (state_NormalStream) state_NormalStream = false;

    frame_w = cl_o.getFrameColor();

    if(!frame_w.empty())
    {
        qt_image = QImage(static_cast<unsigned char*> (frame_w.data),
                          frame_w.cols, frame_w.rows, QImage::Format_RGB888);
        ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
        ui->gui_window->resize(ui->gui_window->pixmap()->size());
    }
}

// checkbox: canny edge
void Widget::on_checkBox_CannyEdge_clicked(bool checked)
{
    if(!cap_w.isOpened())
     {
         ui->checkBox_CannyEdge->setChecked(false);
     }
     else
     {
         if(checked)
         {
             ui->checkBox_FaceDetection->setChecked(false);
             ui->checkBox_RGB_Modifier->setChecked(false);
             ui->checkBox_GOA_Mode->setChecked(false);
             ui->horizontalSlider_FaceDetection->setValue(50);
             disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(goa_mode()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             connect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             timer->start(1);
         }
         else
         {
             disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             timer->start(1);
         }
     }
}

// show: canny edge in window
void Widget::canny_edge()
{
    frame_w = cl_o.getFrameGray();

    GaussianBlur(frame_w, frame_w, cv::Size(3,3), 0);
    Canny(frame_w, frame_w, 0, ui->horizontalSlider_CannyEdge->value());

    qt_image = QImage(static_cast<unsigned char*> (frame_w.data), frame_w.cols,
                      frame_w.rows, QImage::Format_Grayscale8);
    ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
    ui->gui_window->resize(ui->gui_window->pixmap()->size());
}

// get state of slider for canny edge threshhold
void Widget::on_horizontalSlider_CannyEdge_valueChanged()
{
    slider = new QSlider(Qt::Horizontal);
}

// checkbox: face detection
void Widget::on_checkBox_FaceDetection_clicked(bool checked)
{
    if(!cap_w.isOpened())
    {
        ui->checkBox_FaceDetection->setChecked(false);
    }
    else
    {
        if(checked)
        {
            ui->checkBox_CannyEdge->setChecked(false);
            ui->checkBox_RGB_Modifier->setChecked(false);
            ui->checkBox_GOA_Mode->setChecked(false);
            ui->horizontalSlider_CannyEdge->setValue(0);
            disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
            disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
            disconnect(timer, SIGNAL(timeout()), this, SLOT(goa_mode()));
            disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
            connect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
            timer->start(1);
        }
        else
        {
            disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
            connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
            timer->start(1);
        }
    }
}

// function for face detection
void Widget::face_detector()
{
    frame_w = cl_o.getFrameColor();
    float circle_dia = ui->horizontalSlider_FaceDetection->value();
    frame_w = cl_o.getFaceFrame(frame_w, circle_dia);
    qt_image = QImage(static_cast<unsigned char*> (frame_w.data), frame_w.cols,
                      frame_w.rows, QImage::Format_RGB888);
    ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
    ui->gui_window->resize(ui->gui_window->pixmap()->size());
}

// get state of slider for face detection circle diameter
void Widget::on_horizontalSlider_FaceDetection_valueChanged()
{
    slider = new QSlider(Qt::Horizontal);
}

/**
 * @brief Widget::on_checkBox_RGB_Modifier_clicked
 * @param checked
 */
void Widget::on_checkBox_RGB_Modifier_clicked(bool checked)
{
    if(!cap_w.isOpened())
    {
        ui->checkBox_RGB_Modifier->setChecked(false);
    }
    else
     {
        if(checked)
        {
             ui->checkBox_CannyEdge->setChecked(false);
             ui->checkBox_FaceDetection->setChecked(false);
             ui->checkBox_GOA_Mode->setChecked(false);
             ui->horizontalSlider_CannyEdge->setValue(0);
             ui->horizontalSlider_FaceDetection->setValue(50);
             disconnect(timer, SIGNAL(timeout()), this, SLOT(goa_mode()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             connect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             timer->start(1);
         }
         else
         {
             disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             timer->start(1);
         }
    }
}

/**
 * @brief Widget::rgb_modifier
 */
void Widget::rgb_modifier()
{
    frame_w = cl_o.getFrameColor();

    cv::Mat channel[3];
    cv::split(frame_w, channel);

    float channel_r = ui->verticalSlider_R->value();
    float channel_g = ui->verticalSlider_G->value();
    float channel_b = ui->verticalSlider_B->value();



//   channel[0] = cv::Mat::zeros(frame_w.rows, frame_w.cols, CV_8UC1);
//   channel[1] = cv::Mat::zeros(frame_w.rows, frame_w.cols, CV_8UC1);
//   channel[2] = cv::Mat::zeros(frame_w.rows, frame_w.cols, CV_8UC1);

   std::cout << channel << std::endl;

    cv::merge(channel,1, frame_w);

    qt_image = QImage(static_cast<unsigned char*> (frame_w.data), frame_w.cols,
                      frame_w.rows, QImage::Format_RGB888);
    ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
    ui->gui_window->resize(ui->gui_window->pixmap()->size());
}

// slider: adjust color R
void Widget::on_verticalSlider_R_valueChanged()
{
    slider = new QSlider(Qt::Vertical);
}

// slider: adjust color G
void Widget::on_verticalSlider_G_valueChanged()
{
    slider = new QSlider(Qt::Vertical);
}

// slider: adjust color B
void Widget::on_verticalSlider_B_valueChanged()
{
    slider = new QSlider(Qt::Vertical);
}

// not done yet
void Widget::on_checkBox_GOA_Mode_clicked(bool checked)
{
    if(!cap_w.isOpened())
     {
         ui->checkBox_GOA_Mode->setChecked(false);
     }
     else
     {
         if(checked)
         {
             ui->checkBox_CannyEdge->setChecked(false);
             ui->checkBox_FaceDetection->setChecked(false);
             ui->checkBox_RGB_Modifier->setChecked(false);
             ui->horizontalSlider_CannyEdge->setValue(0);
             ui->horizontalSlider_FaceDetection->setValue(50);
             disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             connect(timer, SIGNAL(timeout()), this, SLOT(goa_mode()));
             timer->start(1);
         }
         else
         {
             disconnect(timer, SIGNAL(timeout()), this, SLOT(goa_mode()));
             connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             timer->start(1);
         }
    }
}

// show frame in goa mode
void Widget::goa_mode()
{
    frame_w = cl_o.getFrameColor();

    if(!frame_w.empty())
    {
        qt_image = QImage(static_cast<unsigned char*> (frame_w.data),
                          frame_w.cols, frame_w.rows, QImage::Format_RGB666);
        ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
        ui->gui_window->resize(ui->gui_window->pixmap()->size());
    }
}

