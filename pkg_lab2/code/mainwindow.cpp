#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QStringList>
#include <QVector>
#include <QDebug>
#include <QDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <QMessageBox>
#include <QImage>

QVector<QString> getFilesAndFolders(const QString& path) {
    QVector<QString> items;
    QFileInfo fileInfo(path);

    if (!fileInfo.exists()) {
        return items;
    }

    if (fileInfo.isFile()) {
        items.append(fileInfo.absoluteFilePath());
        return items;
    }

    if (fileInfo.isDir()) {
        QDir dir(path);
        QStringList filters;
        filters << "*";
        dir.setNameFilters(filters);
        dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

        QFileInfoList fileInfoList = dir.entryInfoList();
        for (const QFileInfo& info : fileInfoList) {
            items.append(info.absoluteFilePath());
        }
    }

    return items;
}

bool hasValidImageExtension(const QString& filePath) {
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    QString extension = fileInfo.suffix().toLower();
    return (extension == "jpg" || extension == "jpeg" ||
            extension == "gif" || extension == "tif" ||
            extension == "bmp" || extension == "png" ||
            extension == "pcx");
}

QVector<QString> getImageProperties(const QString& filePath) {
    QVector<QString> properties(5);
    QImage image(filePath);

    if (image.isNull()) {
        properties.fill("Нет сжатия");
        return properties;
    }

    QFileInfo fileInfo(filePath);
    properties[0] = fileInfo.fileName();
    properties[1] = QString("%1 x %2").arg(image.width()).arg(image.height());
    properties[2] = QString::number(image.physicalDpiX());
    properties[3] = QString::number(image.depth());

    QString extension = fileInfo.suffix().toLower();
    if (extension == "jpg" || extension == "jpeg") {
        properties[4] = "С потерями";
    } else if (extension == "png") {
        properties[4] = "Без потерь";
    } else if (extension == "gif") {
        properties[4] = "Без потерь";
    } else if (extension == "tif" || extension == "tiff") {
        properties[4] = "Без потерь";
    } else if (extension == "bmp") {
        properties[4] = "Нет сжатия";
    } else if (extension == "pcx") {
        properties[4] = "Нет сжатия";
    } else {
        properties[4] = "Нет сжатия";
    }

    return properties;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Image analyser");
    model = new QFileSystemModel(this);
    model->setFilter(QDir::QDir::AllEntries);
    model->setRootPath("");
    ui->listView->setModel(model);
    current_clicked = "";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEdit_editingFinished()
{
    current_clicked = ui->lineEdit->text();
    on_open_button_clicked();
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    QListView* listView = (QListView*)sender();
    QFileInfo fileInfo = model->fileInfo(index);
    ui->lineEdit->setText(model->filePath(index));
    if (fileInfo.fileName() == "..") {
        QDir dir = fileInfo.dir();
        dir.cdUp();
        model->index(dir.absolutePath());
        listView->setRootIndex(model->index(dir.absolutePath()));
        ui->lineEdit->setText(dir.absolutePath());
    } else if (fileInfo.fileName() == ".") {
        listView->setRootIndex(model->index(""));
        ui->lineEdit->setText("");
    } else if (fileInfo.isDir()) {
        listView->setRootIndex(index);
    }
    current_clicked = "";
}

void MainWindow::on_open_button_clicked()
{
    QVector<QString> files = getFilesAndFolders(current_clicked);
    if (files.empty()) {
        current_clicked = "";
        return;
    }
    QDialog *table = new QDialog();
    table->setWindowTitle("Table");
    QGridLayout *tableLayout = new QGridLayout(table);
    table->setMinimumSize(700, 700);
    tableInfo = new QTableWidget(table);
    tableLayout->addWidget(tableInfo);
    tableInfo->setColumnCount(5);
    tableInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableInfo->setRowCount(files.size());
    tableInfo->setHorizontalHeaderItem(0, new QTableWidgetItem("Имя файла"));
    tableInfo->setHorizontalHeaderItem(1, new QTableWidgetItem("Размер"));
    tableInfo->setHorizontalHeaderItem(2, new QTableWidgetItem("Разрешение"));
    tableInfo->setHorizontalHeaderItem(3, new QTableWidgetItem("Глубина цвета"));
    tableInfo->setHorizontalHeaderItem(4, new QTableWidgetItem("Сжатие"));
    tableInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < files.size(); i++) {
        if (hasValidImageExtension(files[i])) {
            QVector<QString> properties = getImageProperties(files[i]);
            for (int j = 0; j < 5; j++) {
                tableInfo->setItem(i, j, new QTableWidgetItem(properties[j]));
            }
        } else {
            current_clicked = "";
            QMessageBox::warning(this, "Wrong file", "Directory contains not only pictures");
            return;
        }
    }
    table->show();
    current_clicked = "";
}

void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    QFileInfo fileInfo = model->fileInfo(index);

    if (fileInfo.fileName() == "..") {
        current_clicked = "";
    } else if (fileInfo.fileName() == ".") {
        current_clicked = "";
    } else {
        current_clicked = model->filePath(index);
        ui->lineEdit->setText(model->filePath(index));
    }
}
