#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QVector>
#include <algorithm>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(700, 700);
    connect(ui->loadButton, SIGNAL(clicked()), this, SLOT(loadImage()));
    connect(ui->ocuMet, SIGNAL(clicked()), this, SLOT(applyFilter()));
    connect(ui->adaptObr, SIGNAL(clicked()), this, SLOT(applyFilter()));
    connect(ui->procGrey, SIGNAL(clicked()), this, SLOT(applyFilter()));
    connect(ui->procHSV, SIGNAL(clicked()), this, SLOT(applyFilter()));
    connect(ui->procRGB, SIGNAL(clicked()), this, SLOT(applyFilter()));
    connect(ui->median, SIGNAL(clicked()), this, SLOT(applyFilter()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::loadImage() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp *.jfif)"));
    if (!fileName.isEmpty()) {
        originalImage.load(fileName);
        ui->oImageLabel->setPixmap(QPixmap::fromImage(originalImage).scaled(
            ui->oImageLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }
}

QImage MainWindow::applyBernsenThreshold(const QImage &image, int kernelSize, int contrastThreshold) {
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);
    QImage result = grayImage;
    int halfSize = kernelSize / 2;

    for (int y = halfSize; y < grayImage.height() - halfSize; ++y) {
        for (int x = halfSize; x < grayImage.width() - halfSize; ++x) {
            int sum = 0;
            int minIntensity = 255;
            int maxIntensity = 0;

            for (int ky = -halfSize; ky <= halfSize; ++ky) {
                for (int kx = -halfSize; kx <= halfSize; ++kx) {
                    int intensity = qGray(grayImage.pixel(x + kx, y + ky));
                    sum += intensity;
                    minIntensity = std::min(minIntensity, intensity);
                    maxIntensity = std::max(maxIntensity, intensity);
                }
            }

            int area = kernelSize * kernelSize;
            int mean = sum / area;
            int localContrast = maxIntensity - minIntensity;
            int threshold = (localContrast < contrastThreshold) ? 128 : mean;

            int currentIntensity = qGray(grayImage.pixel(x, y));
            result.setPixel(x, y, currentIntensity > threshold ? qRgb(255, 255, 255) : qRgb(0, 0, 0));
        }
    }

    return result;
}

QImage MainWindow::applyLocalMedianThreshold(const QImage &image, int kernelSize) {
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);
    QImage result = grayImage;
    int halfSize = kernelSize / 2;

    for (int y = halfSize; y < grayImage.height() - halfSize; ++y) {
        for (int x = halfSize; x < grayImage.width() - halfSize; ++x) {
            QVector<int> intensities;

            for (int ky = -halfSize; ky <= halfSize; ++ky) {
                for (int kx = -halfSize; kx <= halfSize; ++kx) {
                    int intensity = qGray(grayImage.pixel(x + kx, y + ky));
                    intensities.append(intensity);
                }
            }

            std::sort(intensities.begin(), intensities.end());
            int median = intensities[intensities.size() / 2];

            int currentIntensity = qGray(grayImage.pixel(x, y));
            result.setPixel(x, y, currentIntensity > median ? qRgb(255, 255, 255) : qRgb(0, 0, 0));
        }
    }

    return result;
}

QImage MainWindow::applyAdaptiveThreshold(const QImage &image, int kernelSize, double C) {
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);
    QImage result = grayImage;
    int halfSize = kernelSize / 2;

    for (int y = halfSize; y < grayImage.height() - halfSize; ++y) {
        for (int x = halfSize; x < grayImage.width() - halfSize; ++x) {
            int sum = 0;
            for (int ky = -halfSize; ky <= halfSize; ++ky) {
                for (int kx = -halfSize; kx <= halfSize; ++kx) {
                    sum += qGray(grayImage.pixel(x + kx, y + ky));
                }
            }

            int area = kernelSize * kernelSize;
            int mean = sum / area;
            int intensity = qGray(grayImage.pixel(x, y));

            result.setPixel(x, y, intensity > mean - C ? qRgb(255, 255, 255) : qRgb(0, 0, 0));
        }
    }

    return result;
}

void MainWindow::applyFilter() {
    if (originalImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("Load an image first!"));
        return;
    }
    QImage processedImage;
    if (sender() == ui->procGrey) {
        processedImage = convertToGray(originalImage);
    }
    if (sender() == ui->procHSV) {
        processedImage = processInHSV(originalImage);
    }
    if (sender() == ui->procRGB) {
        processedImage = processInRGB(originalImage);
    }
    if (sender() == ui->ocuMet) {
        processedImage = applyBernsenThreshold(originalImage, 15, 15);
    }
    if (sender() == ui->adaptObr) {
        processedImage = applyAdaptiveThreshold(originalImage, 15, 0.6);
    }
    if (sender() == ui->median) {
        processedImage = applyLocalMedianThreshold(originalImage, 15);
    }
    ui->nImageLabel->setPixmap(QPixmap::fromImage(processedImage).scaled(
        ui->nImageLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));
}

QImage MainWindow::processInRGB(const QImage &image) {
    QImage resultImage = image;
    for (int y = 0; y < resultImage.height(); ++y) {
        for (int x = 0; x < resultImage.width(); ++x) {
            QColor color = resultImage.pixelColor(x, y);

            int red = 255 - color.red();
            int green = 255 - color.green();
            int blue = 255 - color.blue();

            resultImage.setPixelColor(x, y, QColor(red, green, blue));
        }
    }
    return resultImage;
}

QImage MainWindow::processInHSV(const QImage &image) {
    QImage hsvImage = image.convertToFormat(QImage::Format_RGB32);

    for (int y = 0; y < hsvImage.height(); ++y) {
        for (int x = 0; x < hsvImage.width(); ++x) {
            QColor color = QColor(hsvImage.pixel(x, y));

            int h, s, v;
            color.getHsv(&h, &s, &v);

            v = std::min(v + 100, 255);

            color.setHsv(h, s, v);
            hsvImage.setPixel(x, y, color.rgb());
        }
    }

    return hsvImage;
}


QImage MainWindow::convertToGray(const QImage &image) {
    QImage grayImage(image.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);

            int gray = qGray(color.rgb());
            grayImage.setPixelColor(x, y, QColor(gray, gray, gray));
        }
    }
    return grayImage;
}

