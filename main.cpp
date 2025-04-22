#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPixmap>
#include <QImage>
#include <QWidget>
#include <opencv2/opencv.hpp>

// Преобразование из Mat в QImage
QImage MatToQImage(const cv::Mat &mat) {
    if (mat.channels() == 3) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888).rgbSwapped();
    } else if (mat.channels() == 1) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    } else {
        return QImage();
    }
}

// Фильтр "Мозаика"
cv::Mat applyMosaic(const cv::Mat &src, int blockSize = 10) {
    cv::Mat dst = src.clone();
    for (int y = 0; y < src.rows; y += blockSize) {
        for (int x = 0; x < src.cols; x += blockSize) {
            int w = std::min(blockSize, src.cols - x);
            int h = std::min(blockSize, src.rows - y);
            cv::Rect rect(x, y, w, h);
            cv::Mat block = src(rect);
            cv::Scalar color = cv::mean(block);
            cv::rectangle(dst, rect, color, cv::FILLED);
        }
    }
    return dst;
}

// Оператор Шарра
cv::Mat applyScharr(const cv::Mat &src) {
    cv::Mat gray, grad_x, grad_y, abs_grad_x, abs_grad_y, result;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::Scharr(gray, grad_x, CV_16S, 1, 0);
    cv::Scharr(gray, grad_y, CV_16S, 0, 1);
    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, result);
    return result;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Image Filter App");

    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *imageLabel = new QLabel();
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel);

    QPushButton *loadButton = new QPushButton("Загрузить изображение");
    layout->addWidget(loadButton);

    QComboBox *filterSelector = new QComboBox();
    filterSelector->addItem("Мозаика");
    filterSelector->addItem("Медианный фильтр");
    filterSelector->addItem("Оператор Шарра");
    layout->addWidget(filterSelector);

    QPushButton *applyButton = new QPushButton("Применить фильтр");
    layout->addWidget(applyButton);

    cv::Mat currentImage;

    QObject::connect(loadButton, &QPushButton::clicked, [&]() {
        QString fileName = QFileDialog::getOpenFileName(&window, "Выберите изображение", "", "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!fileName.isEmpty()) {
            currentImage = cv::imread(fileName.toStdString());
            imageLabel->setPixmap(QPixmap::fromImage(MatToQImage(currentImage)).scaled(400, 400, Qt::KeepAspectRatio));
        }
    });

    QObject::connect(applyButton, &QPushButton::clicked, [&]() {
        if (currentImage.empty()) return;

        cv::Mat filteredImage;
        QString filter = filterSelector->currentText();

        if (filter == "Мозаика") {
            filteredImage = applyMosaic(currentImage);
        } else if (filter == "Медианный фильтр") {
            cv::medianBlur(currentImage, filteredImage, 5);
        } else if (filter == "Оператор Шарра") {
            filteredImage = applyScharr(currentImage);
        }

        imageLabel->setPixmap(QPixmap::fromImage(MatToQImage(filteredImage)).scaled(400, 400, Qt::KeepAspectRatio));
    });

    window.setLayout(layout);
    window.resize(600, 600);
    window.show();

    return app.exec();
}
