#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVector>
#include <QString>
#include <QRegularExpression>
#include <QPaintEvent>
#include <QColorDialog>
#include <QPalette>
#include <QDebug>

double roundToTwoDecimals(double value) {
    return std::round(value * 100) / 100;
}

QVector<double> string_to_vector(QString s, bool& is_valid) {
    QVector<double> res;
    if(s == "") {
        is_valid = false;
        return res;
    }
    for (auto c : s) {
        if((c > '9' || c < '0') && c != ' ' && c != ',' && c!= '.'){
            is_valid = false;
            return res;
        }
    }
    if((s[0] < '0' || s[0] > '9') && s[0] != ' '){
        is_valid = false;
        return res;
    }

    QString cleanedInput = s;
    cleanedInput.replace(QRegularExpression("[,\\s]+"), " ");
    cleanedInput = cleanedInput.trimmed();

    QStringList parts = cleanedInput.split(' ',  Qt::SkipEmptyParts);

    for (const QString &part : parts) {
        bool ok;
        double number = part.toDouble(&ok);
        if (ok) {
            res.push_back((double)number);
        } else {
            is_valid = false;
            return res;
        }
    }
    for (int i = 0; i < res.size(); i++) {
        res[i] = roundToTwoDecimals(res[i]);
    }
    if(res.empty())
        is_valid = false;
    return res;
}

QString vector_to_string(QVector<double> arr, bool& is_valid) {
    QString ans;
    if (arr.empty()){
        is_valid = false;
        return ans;
    }
    for (int i = 0; i < arr.size(); i++) {
        arr[i] = roundToTwoDecimals(arr[i]);
    }
    for(auto n : arr) {
        if(!(n - (int)n)){
            ans += QString::number((int)n);
        }
        else {
            ans += QString::number(n);
        }
        ans += ", ";
    }
    ans.truncate(ans.lastIndexOf(QChar(',')));
    return ans;
}

QVector <double> rgb_to_cmyk(const QVector <double>& rgb_arg) {
    QVector<double> cmyk_1(4, 0.0);
    double r = rgb_arg[0];
    double g = rgb_arg[1];
    double b = rgb_arg[2];
    if (r == 0 && g == 0 && b == 0) {
        cmyk_1[3] = 1;
        return cmyk_1;
    }
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;
    double k = 1 - std::max({r, g, b});
    cmyk_1[0] = (1 - r - k) / (1 - k);
    cmyk_1[1] = (1 - g - k) / (1 - k);
    cmyk_1[2] = (1 - b - k) / (1 - k);
    cmyk_1[3] = k;
    return cmyk_1;
}

QVector <double> cmyk_to_rgb(const QVector <double>& arg) {
    QVector<double> res(3);
    res[0] = std::round(255 * (1 - arg[0]) * (1 - arg[3]));
    res[1] = std::round(255 * (1 - arg[1]) * (1 - arg[3]));
    res[2] = std::round(255 * (1 - arg[2]) * (1 - arg[3]));
    return res;
}

QVector <double> rgb_to_hsl (const QVector <double>& arg){
    QVector<double> res(3, 0.0);
    double r = arg[0] / 255.0;
    double g = arg[1] / 255.0;
    double b = arg[2] / 255.0;

    double M = std::max({r, g, b});
    double m = std::min({r, g, b});

    double c = M - m;

    if(c)
    {
        double h_dash;
        if(M == r) {
            h_dash =(g - b) / c  + (g < b ? 6 : 0);
        }
        if (M == g) {
            h_dash = ((b - r)/ c) + 2;
        }
        if (M == b) {
            h_dash = ((r - g)/c) + 4;
        }
        res[0] = 60 * h_dash;
        if(res[0] < 0)
            res[0] += 360;
    }
    res[2] = (M + m) / 2;
    if(c)
        res[1] = c / (1 - abs(2*res[2] - 1));
    else
        res[1] = 0;
    return res;
}

QVector<double> hsl_to_rgb(const QVector<double>& hsl) {
    QVector<double> rgb(3, 0.0);

    double h = hsl[0];
    double s = hsl[1];
    double l = hsl[2];

    double c = (1 - std::abs(2 * l - 1)) * s;
    double h_prime = h / 60.0;
    double x = c * (1 - std::abs(fmod(h_prime, 2) - 1));

    double r1 = 0, g1 = 0, b1 = 0;

    if (0 <= h_prime && h_prime < 1) {
        r1 = c;
        g1 = x;
    } else if (1 <= h_prime && h_prime < 2) {
        r1 = x;
        g1 = c;
    } else if (2 <= h_prime && h_prime < 3) {
        g1 = c;
        b1 = x;
    } else if (3 <= h_prime && h_prime < 4) {
        g1 = x;
        b1 = c;
    } else if (4 <= h_prime && h_prime < 5) {
        r1 = x;
        b1 = c;
    } else if (5 <= h_prime && h_prime < 6) {
        r1 = c;
        b1 = x;
    }

    double m = l - c / 2;
    rgb[0] = (r1 + m) * 255;
    rgb[1] = (g1 + m) * 255;
    rgb[2] = (b1 + m) * 255;

    return rgb;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    current_color = Qt::white;
    rgb = QVector<double>(3, 0);
    hsl = QVector<double>(3, 0);
    cmyk = QVector<double>(4, 0);
    this->setAutoFillBackground(true);
    ui->cmyk_line->setAutoFillBackground(true);
    ui->rgb_line->setAutoFillBackground(true);
    ui->hsl_line->setAutoFillBackground(true);
    ui->red_sl_val->setText(QString::number(ui->red_slider->value()));
    ui->green_sl_val->setText(QString::number(ui->green_slider->value()));
    ui->blue_sl_val->setText(QString::number(ui->blue_slider->value()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_rgb()
{
    bool ok1 = true;
    QString c = vector_to_string(rgb, ok1);
    if (ok1)
        ui->rgb_line->setText(c);
    on_rgb_line_textEdited(c);
}

void MainWindow::update_sliders()
{
    ui->red_slider->setValue(rgb[0]);
    ui->green_slider->setValue(rgb[1]);
    ui->blue_slider->setValue(rgb[2]);

    ui->red_sl_val->setText(QString::number(ui->red_slider->value()));
    ui->green_sl_val->setText(QString::number(ui->green_slider->value()));
    ui->blue_sl_val->setText(QString::number(ui->blue_slider->value()));
}

void MainWindow::update_color()
{
    QPalette pal;
    pal.setColor(QPalette :: Window, current_color);
    this->setPalette(pal);
    QString colorStyleW = "color: rgb(255, 255, 255);";
    QString colorStyleB = "color: rgb(0, 0, 0);";
    if (cmyk[3] >= 0.50) {
        ui->label->setStyleSheet(colorStyleW);
        ui->label_2->setStyleSheet(colorStyleW);
        ui->label_3->setStyleSheet(colorStyleW);
        ui->message_label->setStyleSheet(colorStyleW);
        ui->red_sl_val->setStyleSheet(colorStyleW);
        ui->green_sl_val->setStyleSheet(colorStyleW);
        ui->blue_sl_val->setStyleSheet(colorStyleW);
        ui->label_4->setStyleSheet(colorStyleW);
        ui->label_5->setStyleSheet(colorStyleW);
        ui->label_6->setStyleSheet(colorStyleW);
    } else {
        ui->label->setStyleSheet(colorStyleB);
        ui->label_2->setStyleSheet(colorStyleB);
        ui->label_3->setStyleSheet(colorStyleB);
        ui->message_label->setStyleSheet(colorStyleB);
        ui->red_sl_val->setStyleSheet(colorStyleB);
        ui->green_sl_val->setStyleSheet(colorStyleB);
        ui->blue_sl_val->setStyleSheet(colorStyleB);
        ui->label_4->setStyleSheet(colorStyleB);
        ui->label_5->setStyleSheet(colorStyleB);
        ui->label_6->setStyleSheet(colorStyleB);
    }

}

void MainWindow::on_rgb_line_textEdited(const QString &arg1)
{
    ui->message_label->setText("");
    bool ok = true;
    QVector<double> nums(string_to_vector(arg1, ok));
    if(!ok)
        return;
    if(nums.size()!= 3)
        return;
    for(int i = 0; i < 3; i++){
        if(nums[i] > 255 || nums[i] < 0)
            return;
    }

    rgb = nums;
    cmyk = rgb_to_cmyk(rgb);
    bool ok1 = true;
    QString c = vector_to_string(cmyk, ok1);
    if (ok1)
        ui->cmyk_line->setText(c);
    current_color = QColor (nums[0], nums[1], nums[2]);

    hsl = rgb_to_hsl(rgb);
    bool ok2 = true;
    QString c1 = vector_to_string(hsl, ok2);
    if(ok2)
        ui->hsl_line->setText(c1);

    update_sliders();

    update_color();

}

void MainWindow::on_cmyk_line_textEdited(const QString &arg1)
{
    bool ok = true;
    QVector<double> nums(string_to_vector(arg1, ok));
    if(!ok)
        return;
    if(nums.size()!= 4)
        return;
    for(int i = 0; i < 4; i++){
        if(nums[i] > 1 || nums[i] < 0)
            return;
    }
    cmyk = nums;
    rgb = cmyk_to_rgb(cmyk);
    bool ok1 = true;
    QString c = vector_to_string(rgb, ok1);
    if (ok1)
        ui->rgb_line->setText(c);
    QColor n;
    n.setCmykF(cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
    current_color = n;

    hsl = rgb_to_hsl(rgb);
    bool ok2 = true;
    QString c1 = vector_to_string(hsl, ok2);
    if(ok2)
        ui->hsl_line->setText(c1);

    QVector<double> test(rgb_to_cmyk(rgb));
    for(int i = 0; i < 4; i++){
        if(abs(test[i] - cmyk[i]) > 0.01) {
            QFont f( "Arial", 18, QFont::Bold);
            ui->message_label->setFont( f);
            ui->message_label->setText("Внимание! Потеря цвета");
        }
        else
            ui->message_label->setText("");
    }

    update_sliders();
    update_color();
}

void MainWindow::on_hsl_line_textEdited(const QString &arg1)
{
    ui->message_label->setText("");
    bool ok = true;
    QVector<double> nums(string_to_vector(arg1, ok));
    if(!ok)
        return;
    if(nums.size()!= 3)
        return;
    if(nums[0] > 360 || nums[0] < 0)
        return;
    if(nums[1] > 1 || nums[1] < 0)
        return;
    if(nums[2] > 1 || nums[2] < 0)
        return;
    hsl = nums;
    current_color = QColor::fromHslF(hsl[0]/360, hsl[1], hsl[2]);

    rgb = hsl_to_rgb(hsl);
    bool ok1 = true;
    QString c = vector_to_string(rgb, ok1);
    if (ok1)
        ui->rgb_line->setText(c);

    cmyk = rgb_to_cmyk(rgb);
    bool ok2 = true;
    QString c1 = vector_to_string(cmyk, ok2);
    if (ok2)
        ui->cmyk_line->setText(c1);

    update_sliders();

    update_color();
}

void MainWindow::on_actionSelect_color_triggered()
{
    ui->message_label->setText("");
    current_color = QColorDialog::getColor(current_color, this, "Выберите новый цвет");
    rgb[0] = current_color.red();
    rgb[1] = current_color.green();
    rgb[2] = current_color.blue();

    update_rgb();

    update_sliders();

    update_color();
}

void MainWindow::on_red_slider_sliderMoved(int position)
{
    ui->red_sl_val->setText(QString::number(position));
    rgb[0] = position;
    update_rgb();
}

void MainWindow::on_green_slider_sliderMoved(int position)
{
    ui->green_sl_val->setText(QString::number(position));
    rgb[1] = position;
    update_rgb();
}

void MainWindow::on_blue_slider_sliderMoved(int position)
{
    ui->blue_sl_val->setText(QString::number(position));
    rgb[2] = position;
    update_rgb();
}

