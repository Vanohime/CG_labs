#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QImage originalImage;
private slots:
    void loadImage();
    void applyFilter();
    QImage applyBernsenThreshold(const QImage &, int kernelSize, int contrastThreshold);
    QImage applyAdaptiveThreshold(const QImage &, int, double);
    QImage applyLocalMedianThreshold(const QImage &image, int kernelSize);
    QImage convertToGray(const QImage &image);
    QImage processInRGB(const QImage &image);
    QImage processInHSV(const QImage &image);
};
#endif // MAINWINDOW_H
