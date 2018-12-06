#include "widget.h"
#include "ui_widget.h"
#include "opencv.h"

bool state = true; // initiale state for frame buffer

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    timer = new QTimer(this);

    this->setWindowTitle("OpenCV Widget");
    this->setStyleSheet("color: #DDDDDD;"
                        "background: #191919;");

    ui->pushButton_open_Webcam->setStyleSheet("border: 1px solid #5A5A5A;");
    ui->pushButton_close_Webcam->setStyleSheet("border: 1px solid #5A5A5A;");
    ui->gui_window->setStyleSheet("background-color: black;");
}

Widget::~Widget() {
    delete ui;
}



/**
 * @brief Widget::on_pushButton_open_Webcam_clicked
 * start camera stream
 * call thread function to store each frame into the ringbuffer
 */
void Widget::on_pushButton_open_Webcam_clicked()
{
    if(stateThread) start_thread();

    ui->horizontalSlider_CannyEdge->setValue(0);
    ui->horizontalSlider_FaceDetection->setValue(50);
    ui->verticalSlider_R->setValue(0);
    ui->verticalSlider_G->setValue(0);
    ui->verticalSlider_B->setValue(0);

    ui->checkBox_CannyEdge->setChecked(false);
    ui->checkBox_FaceDetection->setChecked(false);
    ui->checkBox_RGB_Modifier->setChecked(false);
    ui->checkBox_FancyMode->setChecked(false);

    if(!cap_w.isOpened()){
        std::cout << "camera is not open" << std::endl;
    }
    else{
        if (stateStream){
        std::cout << "camera is open" << std::endl;
        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(1);
        }
    }
}

/**
 * @brief Widget::on_pushButton_close_Webcam_clicked
 * close camera stream
 * kill ringbuffer thread
 */
void Widget::on_pushButton_close_Webcam_clicked()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(fancy_mode()));

    ui->horizontalSlider_CannyEdge->setValue(0);
    ui->horizontalSlider_FaceDetection->setValue(50);
    ui->verticalSlider_R->setValue(0);
    ui->verticalSlider_G->setValue(0);
    ui->verticalSlider_B->setValue(0);

    ui->checkBox_CannyEdge->setChecked(false);
    ui->checkBox_FaceDetection->setChecked(false);
    ui->checkBox_RGB_Modifier->setChecked(false);
    ui->checkBox_FancyMode->setChecked(false);

    state = false;
    cap_w.release();
    if(thread_buffer.joinable()){ thread_buffer.join(); }
    cl_o.stopStream();

    cv::Mat image = cv::Mat::zeros(frame_w.size(), CV_8UC3);
    qt_image = QImage(static_cast<unsigned char*> (image.data), image.cols,
                      image.rows, QImage::Format_RGB888);
    ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
    ui->gui_window->resize(ui->gui_window->pixmap()->size());

    stateThread = true;
    stateStream = true;
}

/**
 * @brief Widget::start_thread
 * start thread function for the ringbuffer
 */
void Widget::start_thread()
{
    state = true;
    cap_w = cl_o.startStream();
    std::thread thread_buffer(&OpenCV::frameBuffer, cl_o, cap_w);
    if(thread_buffer.joinable()) thread_buffer.detach();
    stateThread = false;
}

/**
 * @brief Widget::update_window
 * get color frame from the ringbuffer and show it in widget window
 */
void Widget::update_window()
{
    if(stateStream) stateStream = false;

    frame_w = cl_o.getFrameColor();

    if(!frame_w.empty()){
        qt_image = QImage(static_cast<unsigned char*> (frame_w.data),
                          frame_w.cols, frame_w.rows, QImage::Format_RGB888);
        ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
        ui->gui_window->resize(ui->gui_window->pixmap()->size());
    }
}

/**
 * @brief Widget::on_checkBox_CannyEdge_clicked
 * @param checked:  true: calls canny_edge funtion
 *                  false: gets color frame to the widget window
 */
void Widget::on_checkBox_CannyEdge_clicked(bool checked)
{
    if(!cap_w.isOpened()){
         ui->checkBox_CannyEdge->setChecked(false);
     }
     else{
         if(checked){
             ui->checkBox_FaceDetection->setChecked(false);
             ui->checkBox_RGB_Modifier->setChecked(false);
             ui->checkBox_FancyMode->setChecked(false);
             ui->horizontalSlider_FaceDetection->setValue(50);
             ui->verticalSlider_R->setValue(0);
             ui->verticalSlider_G->setValue(0);
             ui->verticalSlider_B->setValue(0);
             disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(fancy_mode()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             connect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             timer->start(1);
         }
         else{
             disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             timer->start(1);
         }
     }
}

/**
 * @brief Widget::canny_edge
 * gets frame from the ringbuffer
 * applies canny edge detection and shows it in widget window
 */
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

/**
 * @brief Widget::on_horizontalSlider_CannyEdge_valueChanged
 * adjust the gradient threshold for the canny edge function
 * moving to the rightside -> bigger value -> shows only strong gradients
 */
void Widget::on_horizontalSlider_CannyEdge_valueChanged()
{
    slider = new QSlider(Qt::Horizontal);
}

/**
 * @brief Widget::on_checkBox_FaceDetection_clicked
 * @param checked:  true: calls face_detection funtion
 *                  false: gets color frame to the widget window
 */
void Widget::on_checkBox_FaceDetection_clicked(bool checked)
{
    if(!cap_w.isOpened()){
        ui->checkBox_FaceDetection->setChecked(false);
    }
    else{
        if(checked){
            ui->checkBox_CannyEdge->setChecked(false);
            ui->checkBox_RGB_Modifier->setChecked(false);
            ui->checkBox_FancyMode->setChecked(false);
            ui->horizontalSlider_CannyEdge->setValue(0);
            ui->verticalSlider_R->setValue(0);
            ui->verticalSlider_G->setValue(0);
            ui->verticalSlider_B->setValue(0);
            disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
            disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
            disconnect(timer, SIGNAL(timeout()), this, SLOT(fancy_mode()));
            disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
            connect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
            timer->start(1);
        }
        else{
            disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
            connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
            timer->start(1);
        }
    }
}

/**
 * @brief Widget::face_detector
 * gets frame from the ringbuffer
 * applies face detection and shows it in widget window
 */
void Widget::face_detector()
{
    float circle_dia = ui->horizontalSlider_FaceDetection->value();
    frame_w = cl_o.getFaceFrame(circle_dia);
    qt_image = QImage(static_cast<unsigned char*> (frame_w.data), frame_w.cols,
                      frame_w.rows, QImage::Format_RGB888);
    ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
    ui->gui_window->resize(ui->gui_window->pixmap()->size());
}

/**
 * @brief Widget::on_horizontalSlider_FaceDetection_valueChanged
 * adjust size of the detection circle
 */
void Widget::on_horizontalSlider_FaceDetection_valueChanged()
{
    slider = new QSlider(Qt::Horizontal);
}

/**
 * @brief Widget::on_checkBox_RGB_Modifier_clicked
 * @param checked   true: calls rgb_modifying funtion
 *                  false: gets color frame to the widget window
 */
void Widget::on_checkBox_RGB_Modifier_clicked(bool checked)
{
    if(!cap_w.isOpened()){
        ui->checkBox_RGB_Modifier->setChecked(false);
    }
    else{
        if(checked){
             ui->checkBox_CannyEdge->setChecked(false);
             ui->checkBox_FaceDetection->setChecked(false);
             ui->checkBox_FancyMode->setChecked(false);
             ui->horizontalSlider_CannyEdge->setValue(0);
             ui->horizontalSlider_FaceDetection->setValue(50);
             disconnect(timer, SIGNAL(timeout()), this, SLOT(fancy_mode()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             connect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             timer->start(1);
         }
         else{
             disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             timer->start(1);
         }
    }
}

/**
 * @brief Widget::rgb_modifier
 * gets frame from the ringbuffer
 * applies rgb changes and shows it in widget window
 */
void Widget::rgb_modifier()
{
    int chR = ui->verticalSlider_R->value();;
    int chG = ui->verticalSlider_G->value();
    int chB = ui->verticalSlider_B->value();

    cv::Mat fin_img = cl_o.getRGBFrame(chR, chG, chB);

    qt_image = QImage(static_cast<unsigned char*> (fin_img.data), fin_img.cols,
                      fin_img.rows, QImage::Format_RGB888);
    ui->gui_window->setPixmap(QPixmap::fromImage(qt_image));
    ui->gui_window->resize(ui->gui_window->pixmap()->size());
}

/**
 * @brief Widget::on_verticalSlider_R_valueChanged
 * change r values between -256 & 256
 */
void Widget::on_verticalSlider_R_valueChanged()
{
    slider = new QSlider(Qt::Vertical);
}

/**
 * @brief Widget::on_verticalSlider_G_valueChanged
 * change g values between -256 & 256
 */
void Widget::on_verticalSlider_G_valueChanged()
{
    slider = new QSlider(Qt::Vertical);
}

/**
 * @brief Widget::on_verticalSlider_B_valueChanged
 * change b values between -256 & 256
 */
void Widget::on_verticalSlider_B_valueChanged()
{
    slider = new QSlider(Qt::Vertical);
}

/**
 * @brief Widget::on_checkBox_FancyMode_clicked
 * @param checked:  true: calls fancy mode funtion
 *                  false: gets color frame to the widget window
 */
void Widget::on_checkBox_FancyMode_clicked(bool checked)
{
    if(!cap_w.isOpened()){
         ui->checkBox_FancyMode->setChecked(false);
     }
     else{
         if(checked){
             ui->checkBox_CannyEdge->setChecked(false);
             ui->checkBox_FaceDetection->setChecked(false);
             ui->checkBox_RGB_Modifier->setChecked(false);
             ui->horizontalSlider_CannyEdge->setValue(0);
             ui->horizontalSlider_FaceDetection->setValue(50);
             ui->verticalSlider_R->setValue(0);
             ui->verticalSlider_G->setValue(0);
             ui->verticalSlider_B->setValue(0);
             disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(face_detector()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(canny_edge()));
             disconnect(timer, SIGNAL(timeout()), this, SLOT(rgb_modifier()));
             connect(timer, SIGNAL(timeout()), this, SLOT(fancy_mode()));
             timer->start(1);
         }
         else{
             disconnect(timer, SIGNAL(timeout()), this, SLOT(fancy_mode()));
             connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
             timer->start(1);
         }
    }
}

/**
 * @brief Widget::fancy_mode
 * gets frame from the ringbuffer and converts it to color format_RGB666
 */
void Widget::fancy_mode()
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

/**
 * @brief Widget::closeEvent
 * @param event:    closes camera stream and kills thread by pressing 'X'
 */
void Widget::closeEvent(QCloseEvent *event)
{
    state = false;
    cl_o.stopStream();
    cap_w.release();
    if(thread_buffer.joinable()){ thread_buffer.join(); }
}
