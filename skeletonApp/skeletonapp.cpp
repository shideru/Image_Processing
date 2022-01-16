#include <QToolBar>
#include <QIcon>
#include <QClipboard>
#include <QColorSpace>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardPaths>
#include <QStatusBar>
#include <iostream>
#include <fstream>
#include <cmath>


#include "skeletonApp.h"

skeletonApp::skeletonApp(QWidget* parent)
    : QMainWindow(parent), imageLabel(new QLabel)
    , histogramLabel(new QLabel)
    , scrollArea(new QScrollArea)
    , hBoxLayout(new QHBoxLayout(scrollArea))
{
    ui.setupUi(this);

    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    histogramLabel->setBackgroundRole(QPalette::Base);
    histogramLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    histogramLabel->setScaledContents(true);

    hBoxLayout->addWidget(imageLabel);
    hBoxLayout->addWidget(histogramLabel);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setLayout(hBoxLayout);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);
    // setCentralWidget(hBoxLayout);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

}




bool skeletonApp::loadFile(const QString& fileName)
{

    data.clear();

    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot load %1: %2")
            .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    //testFilter->setEnabled(true);
    filtersMenu->setDisabled(false);
    histogramMenu->setDisabled(false);
    filtrySplotoweMenu->setDisabled(false);
    binaryzacjaMenu->setDisabled(false);
    doWyboryMenu->setDisabled(false);

    setImage(newImage);



    width = image.width();
    height = image.height();
    int bitPlanes = image.bitPlaneCount();
    int depth = image.depth();
    QImage::Format format = image.format();
    int bytesPerLine = image.bytesPerLine();//niezależnie od formatu( pbm, pgm, ppm), wyrównanie do 4B


    nbChannels = 1;
    if (image.depth() > 1)
        nbChannels = image.depth() / 8;

    nbRealChannels = 1;
    if (image.bitPlaneCount() > 1)
        nbRealChannels = image.bitPlaneCount() / 8;

    std::ofstream file("test_zapisu.txt", std::ios::out);

    if (file.good())
    {

        for (int i = 0; i < image.height(); i++)
        {
            std::vector<std::vector<int>> row;

            for (int j = 0; j < image.width(); j++)
            {
                QRgb impix = image.pixel(j, i);//pixel() działa w logice pixel(x,y) - najpierw specyfikuje kolumnę

                std::vector<int> pixel;

                switch (nbChannels)
                {
                case 1:
                    pixel = { qRed(impix) };
                    file << pixel[0] << " ";
                    break;
                case 3:
                    pixel = { qRed(impix), qGreen(impix), (int)qBlue(impix) };
                    file << pixel[0] << " " << pixel[1] << " " << pixel[2];
                    break;
                case 4:
                    pixel = { qRed(impix), qGreen(impix), qBlue(impix), qAlpha(impix) };
                    file << pixel[0] << " " << pixel[1] << " " << pixel[2] << " " << pixel[3];
                    break;
                }
                file << "\n";
                row.push_back(pixel);
            }
            // file << "\n";
            data.push_back(row);
            //std::cout << "\n";
        }

        file.close();
    }
    setWindowFilePath(fileName);


    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4, Bit planes: %5")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth()).arg(image.bitPlaneCount());
    statusBar()->showMessage(message);
    return true;
}

void skeletonApp::setImage(const QImage& newImage)
{
    image = newImage;
    if (image.colorSpace().isValid())
        image.convertToColorSpace(QColorSpace::SRgb);
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    //    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);

    normalSize();
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();
}


void skeletonApp::scaleImage(double factor)
{
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap(Qt::ReturnByValue).size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}


void skeletonApp::adjustScrollBar(QScrollBar* scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
        + ((factor - 1) * scrollBar->pageStep() / 2)));
}


void skeletonApp::about()
{
    QMessageBox::about(this, tr("About skeletonApp"),
        tr("<p>Aplikacja<b> szkieletowa</b> do kursu <i>Przetwarzanie obrazu</i></p>"));
}



bool skeletonApp::saveFile(const QString& fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(image)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot write %1: %2")
            .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

//! [1]

static void initializeImageFileDialog(QFileDialog& dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray& mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();

    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/x-portable-pixmap");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("ppm");
}

void skeletonApp::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}
//! [1]

void skeletonApp::saveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}



void skeletonApp::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData* mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD


void skeletonApp::paste()
{
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    }
    else {
        setImage(newImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);

        filtersMenu->setDisabled(false);
        histogramMenu->setDisabled(false);
        filtrySplotoweMenu->setDisabled(false);
    }
#endif // !QT_NO_CLIPBOARD
}

void skeletonApp::zoomIn()
{
    scaleImage(1.25);
}

void skeletonApp::zoomOut()
{
    scaleImage(0.8);
}

void skeletonApp::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void skeletonApp::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}

//Lab 2
void skeletonApp::applyDesaturacja()
{
    dataResult.clear();

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b, gray;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel);
                g = (double)qGreen(pixel);
                b = (double)qBlue(pixel);

                gray = ((r * 0.3) + (g * 0.6) + (b * 0.1));

                imageResult.setPixel(i, j, QColor(gray, gray, gray).rgb());
            }
    }
    setImage(imageResult);
}

void skeletonApp::applyNegatyw()//negative
{
    dataResult.clear();

    for (int i = 0; i < height; i++)
    {
        std::vector<std::vector<int>> row;
        for (int j = 0; j < width; j++)
        {
            std::vector<int> pixel;

            for (int k = 0; k < nbRealChannels; k++)
            {
                pixel.push_back(255 - data[i][j][k]);
            }//k

            if (nbRealChannels < nbChannels)
                pixel.push_back(255);

            row.push_back(pixel);
        }//j
        dataResult.push_back(row);
    }//i

    imageResult = QImage(width, height, image.format());

    if (nbChannels > 1)
        for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
            for (int j = 0; j < image.width(); j++)
                imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][1], dataResult[i][j][2], dataResult[i][j][3]));
    else
        if (image.bitPlaneCount() == 8)
            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                    imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][0], dataResult[i][j][0], 255));
        else
        {
            imageResult.setColorCount(2);
            imageResult.setColor(0, qRgba(0, 0, 0, 255)); // Index #0 = Red
            imageResult.setColor(1, qRgba(255, 0, 0, 0));

            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                {
                    if (dataResult[i][j][0] == 0)
                        imageResult.setPixel(j, i, 0);
                    else
                        imageResult.setPixel(j, i, 1);
                }

        }
    //imageResult =  QImage(img,width,height,image.format());
    setImage(imageResult);
}

void skeletonApp::applyKontrastLinUp()
{
    dataResult.clear();

    for (int i = 0; i < height; i++)
    {
        std::vector<std::vector<int>> row;
        for (int j = 0; j < width; j++)
        {
            std::vector<int> pixel;

            for (int k = 0; k < nbRealChannels; k++)
            {

                int temp = 1.5 * data[i][j][k];
                if (temp > 255)
                {
                    temp = 255;
                }
                pixel.push_back(temp);
            }//k

            if (nbRealChannels < nbChannels)
                pixel.push_back(255);

            row.push_back(pixel);
        }//j
        dataResult.push_back(row);
    }//i

    imageResult = QImage(width, height, image.format());

    if (nbChannels > 1)
        for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
            for (int j = 0; j < image.width(); j++)
                imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][1], dataResult[i][j][2], dataResult[i][j][3]));
    else
        if (image.bitPlaneCount() == 8)
            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                    imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][0], dataResult[i][j][0], 255));
        else
        {
            imageResult.setColorCount(2);
            imageResult.setColor(0, qRgba(0, 0, 0, 255)); // Index #0 = Red
            imageResult.setColor(1, qRgba(255, 0, 0, 0));

            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                {
                    if (dataResult[i][j][0] == 0)
                        imageResult.setPixel(j, i, 0);
                    else
                        imageResult.setPixel(j, i, 1);
                }

        }
    setImage(imageResult);
}
void skeletonApp::applyKontrastLogUp()
{
    dataResult.clear();

    for (int i = 0; i < height; i++)
    {
        std::vector<std::vector<int>> row;
        for (int j = 0; j < width; j++)
        {
            std::vector<int> pixel;

            for (int k = 0; k < nbRealChannels; k++)
            {

                int temp = data[i][j][k] * log(data[i][j][k] + 1);
                if (temp > 255)
                {
                    temp = 255;
                }


                pixel.push_back(temp);
            }//k

            if (nbRealChannels < nbChannels)
                pixel.push_back(255);

            row.push_back(pixel);
        }//j
        dataResult.push_back(row);
    }//i

    imageResult = QImage(width, height, image.format());

    if (nbChannels > 1)
        for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
            for (int j = 0; j < image.width(); j++)
                imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][1], dataResult[i][j][2], dataResult[i][j][3]));
    else
        if (image.bitPlaneCount() == 8)
            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                    imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][0], dataResult[i][j][0], 255));
        else
        {
            imageResult.setColorCount(2);
            imageResult.setColor(0, qRgba(0, 0, 0, 255)); // Index #0 = Red
            imageResult.setColor(1, qRgba(255, 0, 0, 0));

            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                {
                    if (dataResult[i][j][0] == 0)
                        imageResult.setPixel(j, i, 0);
                    else
                        imageResult.setPixel(j, i, 1);
                }

        }
    setImage(imageResult);
}
void skeletonApp::applyKontrastPowUp()
{
    dataResult.clear();

    for (int i = 0; i < height; i++)
    {
        std::vector<std::vector<int>> row;
        for (int j = 0; j < width; j++)
        {
            std::vector<int> pixel;

            for (int k = 0; k < nbRealChannels; k++)
            {

                int temp = data[i][j][k] * pow(data[i][j][k], 1.5);
                if (temp > 255)
                {
                    temp = 255;
                }
                pixel.push_back(temp);
            }//k

            if (nbRealChannels < nbChannels)
                pixel.push_back(255);

            row.push_back(pixel);
        }//j
        dataResult.push_back(row);
    }//i

    imageResult = QImage(width, height, image.format());

    if (nbChannels > 1)
        for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
            for (int j = 0; j < image.width(); j++)
                imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][1], dataResult[i][j][2], dataResult[i][j][3]));
    else
        if (image.bitPlaneCount() == 8)
            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                    imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][0], dataResult[i][j][0], 255));
        else
        {
            imageResult.setColorCount(2);
            imageResult.setColor(0, qRgba(0, 0, 0, 255)); // Index #0 = Red
            imageResult.setColor(1, qRgba(255, 0, 0, 0));

            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                {
                    if (dataResult[i][j][0] == 0)
                        imageResult.setPixel(j, i, 0);
                    else
                        imageResult.setPixel(j, i, 1);
                }

        }
    setImage(imageResult);
}

void skeletonApp::applyKontrastLinDown()
{
    dataResult.clear();

    for (int i = 0; i < height; i++)
    {
        std::vector<std::vector<int>> row;
        for (int j = 0; j < width; j++)
        {
            std::vector<int> pixel;

            for (int k = 0; k < nbRealChannels; k++)
            {

                int temp = -((data[i][j][k] * -0.5));
                if (temp < 0)
                {
                    temp = 0;
                }
                pixel.push_back(temp);
            }//k

            if (nbRealChannels < nbChannels)
                pixel.push_back(255);

            row.push_back(pixel);
        }//j
        dataResult.push_back(row);
    }//i

    imageResult = QImage(width, height, image.format());

    if (nbChannels > 1)
        for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
            for (int j = 0; j < image.width(); j++)
                imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][1], dataResult[i][j][2], dataResult[i][j][3]));
    else
        if (image.bitPlaneCount() == 8)
            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                    imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][0], dataResult[i][j][0], 255));
        else
        {
            imageResult.setColorCount(2);
            imageResult.setColor(0, qRgba(0, 0, 0, 255)); // Index #0 = Red
            imageResult.setColor(1, qRgba(255, 0, 0, 0));

            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                {
                    if (dataResult[i][j][0] == 0)
                        imageResult.setPixel(j, i, 0);
                    else
                        imageResult.setPixel(j, i, 1);
                }

        }
    setImage(imageResult);
}
void skeletonApp::applyKontrastLogDown()
{
    dataResult.clear();

    for (int i = 0; i < height; i++)
    {
        std::vector<std::vector<int>> row;
        for (int j = 0; j < width; j++)
        {
            std::vector<int> pixel;

            for (int k = 0; k < nbRealChannels; k++)
            {
                int temp = data[i][j][k] * (1 / log(data[i][j][k] + 1));
                if (temp < 0)
                {
                    temp = 0;
                }


                pixel.push_back(temp);
            }//k

            if (nbRealChannels < nbChannels)
                pixel.push_back(255);

            row.push_back(pixel);
        }//j
        dataResult.push_back(row);
    }//i

    imageResult = QImage(width, height, image.format());

    if (nbChannels > 1)
        for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
            for (int j = 0; j < image.width(); j++)
                imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][1], dataResult[i][j][2], dataResult[i][j][3]));
    else
        if (image.bitPlaneCount() == 8)
            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                    imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][0], dataResult[i][j][0], 255));
        else
        {
            imageResult.setColorCount(2);
            imageResult.setColor(0, qRgba(0, 0, 0, 255)); // Index #0 = Red
            imageResult.setColor(1, qRgba(255, 0, 0, 0));

            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                {
                    if (dataResult[i][j][0] == 0)
                        imageResult.setPixel(j, i, 0);
                    else
                        imageResult.setPixel(j, i, 1);
                }

        }
    setImage(imageResult);
}
void skeletonApp::applyKontrastPowDown()
{
    dataResult.clear();

    for (int i = 0; i < height; i++)
    {
        std::vector<std::vector<int>> row;
        for (int j = 0; j < width; j++)
        {
            std::vector<int> pixel;

            for (int k = 0; k < nbRealChannels; k++)
            {
                int temp = data[i][j][k] * (pow(data[i][j][k], -0.1));

                if (temp < 0)
                {
                    temp = 0;
                }
                pixel.push_back(temp);
            }//k

            if (nbRealChannels < nbChannels)
                pixel.push_back(255);

            row.push_back(pixel);
        }//j
        dataResult.push_back(row);
    }//i

    imageResult = QImage(width, height, image.format());

    if (nbChannels > 1)
        for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
            for (int j = 0; j < image.width(); j++)
                imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][1], dataResult[i][j][2], dataResult[i][j][3]));
    else
        if (image.bitPlaneCount() == 8)
            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                    imageResult.setPixel(j, i, qRgba(dataResult[i][j][0], dataResult[i][j][0], dataResult[i][j][0], 255));
        else
        {
            imageResult.setColorCount(2);
            imageResult.setColor(0, qRgba(0, 0, 0, 255)); // Index #0 = Red
            imageResult.setColor(1, qRgba(255, 0, 0, 0));

            for (int i = 0; i < image.height(); i++)//obraz jest w pliku zapisany 
                for (int j = 0; j < image.width(); j++)
                {
                    if (dataResult[i][j][0] == 0)
                        imageResult.setPixel(j, i, 0);
                    else
                        imageResult.setPixel(j, i, 1);
                }

        }
    setImage(imageResult);
}

void skeletonApp::applyJasnoscUp()
{
    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {

                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {

                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel) + 20;
                g = (double)qGreen(pixel) + 20;
                b = (double)qBlue(pixel) + 20;
                if (r > 255)
                {
                    r = 255;
                }
                if (g > 255)
                {
                    g = 255;
                }
                if (b > 255)
                {
                    b = 255;
                }
                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}
void skeletonApp::applyJasnoscDown()
{
    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {

                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {

                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel) - 20;
                g = (double)qGreen(pixel) - 20;
                b = (double)qBlue(pixel) - 20;
                if (r < 0)
                {
                    r = 0;
                }
                if (g < 0)
                {
                    g = 0;
                }
                if (b < 0)
                {
                    b = 0;
                }
                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}

void skeletonApp::applyNasycenie()
{
    dataResult.clear();

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel) * 1.1;
                if (r > 255)
                {
                    r = 255;
                }
                g = (double)qGreen(pixel) * 1.1;
                if (g > 255)
                {
                    g = 255;
                }
                b = (double)qBlue(pixel) * 1.1;
                if (b > 255)
                {
                    b = 255;
                }

                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}

void skeletonApp::applyMonoRedUp()
{

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel) * 1.1;
                if (r > 255)
                {
                    r = 255;
                }
                g = (double)qGreen(pixel);
                b = (double)qBlue(pixel);

                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}
void skeletonApp::applyMonoGreenUp()
{

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel);
                g = (double)qGreen(pixel) * 1.1;
                b = (double)qBlue(pixel);
                if (g > 255)
                {
                    g = 255;
                }
                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}
void skeletonApp::applyMonoBlueUp()
{

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel);
                g = (double)qGreen(pixel);
                b = (double)qBlue(pixel) * 1.1;
                if (b > 255)
                {
                    b = 255;
                }
                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}

void skeletonApp::applyMonoRedDown()
{

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel) * 0.9;
                if (r > 255)
                {
                    r = 255;
                }
                g = (double)qGreen(pixel);
                b = (double)qBlue(pixel);

                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}
void skeletonApp::applyMonoGreenDown()
{

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel);
                g = (double)qGreen(pixel) * 0.9;
                b = (double)qBlue(pixel);
                if (g > 255)
                {
                    g = 255;
                }
                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}
void skeletonApp::applyMonoBlueDown()
{

    int w = image.width();
    int h = image.height();

    imageResult = QImage(w, h, image.format());

    if (image.format() == QImage::Format_Mono)
    {
        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);
                int p = qGray(pixel);

                if (p > 128)
                {
                    p = 255;
                }
                else
                {
                    p = 0;
                }
                imageResult.setPixel(i, j, QColor(p, p, p).rgb());
            }
    }
    else
    {
        double r, g, b;

        for (int i = 0; i < w; i++)
            for (int j = 0; j < h; j++)
            {
                QRgb pixel = image.pixel(i, j);

                r = (double)qRed(pixel);
                g = (double)qGreen(pixel);
                b = (double)qBlue(pixel) * 0.9;
                if (b > 255)
                {
                    b = 255;
                }
                imageResult.setPixel(i, j, QColor(r, g, b).rgb());
            }
    }
    setImage(imageResult);
}

//Lab 3
void skeletonApp::generateHistograms()
{

    int w = image.width();
    int h = image.height();
    int TableAvg[256], TableRed[256], TableGreen[256], TableBlue[256];
    int maksAvg = 0, maksRed = 0, maksGreen = 0, maksBlue = 0;
    for (int i = 0; i < 256; i++)
    {
        TableAvg[i] = 0;
        TableRed[i] = 0;
        TableGreen[i] = 0;
        TableBlue[i] = 0;
    }

    int r, g, b, gray;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);

            r = (int)qRed(pixel);
            g = (int)qGreen(pixel);
            b = (int)qBlue(pixel);
            gray = (int)((r * 0.3) + (g * 0.6) + (b * 0.1));

            TableRed[r]++;
            TableGreen[g]++;
            TableBlue[b]++;
            TableAvg[gray]++;
        }

    for (int i = 0; i < 256; i++)
    {
        if (maksAvg < TableAvg[i]) {
            maksAvg = TableAvg[i];
        }
        if (maksRed < TableRed[i]) {
            maksRed = TableRed[i];
        }
        if (maksGreen < TableGreen[i]) {
            maksGreen = TableGreen[i];
        }
        if (maksBlue < TableBlue[i]) {
            maksBlue = TableBlue[i];
        }

    }
    for (int i = 0; i < 256; i++)
    {
        TableAvg[i] = floor((TableAvg[i] * 100) / maksAvg);
        TableRed[i] = floor((TableRed[i] * 100) / maksRed);
        TableGreen[i] = floor((TableGreen[i] * 100) / maksGreen);
        TableBlue[i] = floor((TableBlue[i] * 100) / maksBlue);
    }

    imageResult = QImage(256, 400, image.format());
    for (int i = 0; i < 256; i++)
    {
        for (int j = 400; j >= 0; j--)
        {
            if (j <= 100)
            {
                if (TableAvg[i] > 0)
                {
                    imageResult.setPixel(i, j, QColor(0, 0, 0).rgb());
                    TableAvg[i]--;
                }
                else
                {
                    imageResult.setPixel(i, j, QColor(255, 255, 255).rgb());

                }
            }
            else if (j <= 200)
            {
                if (TableRed[i] > 0)
                {
                    imageResult.setPixel(i, j, QColor(0, 0, 0).rgb());
                    TableRed[i]--;
                }
                else
                {
                    imageResult.setPixel(i, j, QColor(255, 0, 0).rgb());
                }

            }
            else if (j <= 300)
            {
                if (TableGreen[i] > 0)
                {
                    imageResult.setPixel(i, j, QColor(0, 0, 0).rgb());
                    TableGreen[i]--;
                }
                else
                {
                    imageResult.setPixel(i, j, QColor(0, 255, 0).rgb());
                }
            }
            else
            {
                if (TableBlue[i] > 0)
                {
                    imageResult.setPixel(i, j, QColor(0, 0, 0).rgb());
                    TableBlue[i]--;
                }
                else
                {
                    imageResult.setPixel(i, j, QColor(0, 0, 255).rgb());
                }
            }
        }

    }


    if (imageResult.colorSpace().isValid())
        imageResult.convertToColorSpace(QColorSpace::SRgb);
    histogramLabel->setPixmap(QPixmap::fromImage(imageResult));
    scaleFactor = 1.0;
    updateActions();
    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();


}
void skeletonApp::applyRozciaganieHistogram()
{

    int w = image.width();
    int h = image.height();
    int TableAvg[256], TableRed[256], TableGreen[256], TableBlue[256];
    int temp = 0, rMin = 0, rMax = 255, gMin = 0, gMax = 255, bMin = 0, bMax = 255;
    for (int i = 0; i < 256; i++)
    {
        TableAvg[i] = 0;
        TableRed[i] = 0;
        TableGreen[i] = 0;
        TableBlue[i] = 0;
    }

    int r, g, b, gray;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);

            r = (int)qRed(pixel);
            g = (int)qGreen(pixel);
            b = (int)qBlue(pixel);
            gray = (int)((r * 0.3) + (g * 0.6) + (b * 0.1));

            TableRed[r]++;
            TableGreen[g]++;
            TableBlue[b]++;
            TableAvg[gray]++;
        }
    temp = 0;
    do {
        rMin = temp;
        temp++;
    } while (TableRed[temp] == 0);
    temp = 255;
    do {
        rMax = temp;
        temp--;
    } while (TableRed[temp] == 0);

    temp = 0;
    do {
        gMin = temp;
        temp++;
    } while (TableGreen[temp] == 0);
    temp = 255;
    do {
        gMax = temp;
        temp--;
    } while (TableGreen[temp] == 0);

    temp = 0;
    do {
        bMin = temp;
        temp++;
    } while (TableBlue[temp] == 0);
    temp = 255;
    do {
        bMax = temp;
        temp--;
    } while (TableBlue[temp] == 0);

    imageResult = QImage(w, h, image.format());
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);

            r = (255 * ((int)qRed(pixel) - rMin)) / (rMax - rMin);
            g = (255 * ((int)qGreen(pixel) - gMin)) / (gMax - gMin);
            b = (255 * ((int)qBlue(pixel) - bMin)) / (bMax - bMin);

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());
        }

    setImage(imageResult);
    skeletonApp::generateHistograms();

}
void skeletonApp::applyWyrownanieHistogram()
{

    int w = image.width();
    int h = image.height();
    int  TableRed[256], TableGreen[256], TableBlue[256];
    double probRed[256], probGreen[256], probBlue[256], dysRed[256], dysGreen[256], dysBlue[256];
    for (int i = 0; i < 256; i++)
    {
        probRed[i] = 0.0;
        probGreen[i] = 0.0;
        probBlue[i] = 0.0;

        TableRed[i] = 0;
        TableGreen[i] = 0;
        TableBlue[i] = 0;
    }

    int r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);

            r = (int)qRed(pixel);
            g = (int)qGreen(pixel);
            b = (int)qBlue(pixel);

            TableRed[r]++;
            TableGreen[g]++;
            TableBlue[b]++;
        }
    for (int i = 0; i < 256; i++)
    {
        probRed[i] = (double)TableRed[i] / (double)(w * h);
        probGreen[i] = (double)TableGreen[i] / (double)(w * h);
        probBlue[i] = (double)TableBlue[i] / (double)(w * h);
    }

    for (int i = 0; i < 256; i++)
    {
        dysRed[i] = probRed[0];
        dysGreen[i] = probGreen[0];
        dysBlue[i] = probBlue[0];
    }

    for (int i = 1; i < 256; i++)
    {
        dysRed[i] = dysRed[i - 1] + probRed[i];
        dysGreen[i] = dysGreen[i - 1] + probGreen[i];
        dysBlue[i] = dysBlue[i - 1] + probBlue[i];
    }
    for (int i = 0; i < 256; i++)
    {
        dysRed[i] = dysRed[i] * 255;
        dysGreen[i] = dysGreen[i] * 255;
        dysBlue[i] = dysBlue[i] * 255;
    }

    imageResult = QImage(w, h, image.format());
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);

            r = (int)dysRed[qRed(pixel)];
            g = (int)dysGreen[qGreen(pixel)];
            b = (int)dysBlue[qBlue(pixel)];

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());
        }

    setImage(imageResult);
    skeletonApp::generateHistograms();

}

//Lab 4

void skeletonApp::applyWygladzanieJednostajne3x3()
{

    double tabFiltr[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tabFiltr[i][j] = 1;
        }
    }
    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 1][l + 1]) / 9;
                        g += ((double)qGreen(pixel) * tabFiltr[k + 1][l + 1]) / 9;
                        b += ((double)qBlue(pixel) * tabFiltr[k + 1][l + 1]) / 9;

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}

void skeletonApp::applyWygladzanieJednostajne5x5()
{
    int size;
    double tabFiltr[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            tabFiltr[i][j] = 1;
        }
    }

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -2; k < 3; k++)
                for (int l = -2; l < 3; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 2][l + 2]) / 25;
                        g += ((double)qGreen(pixel) * tabFiltr[k + 2][l + 2]) / 25;
                        b += ((double)qBlue(pixel) * tabFiltr[k + 2][l + 2]) / 25;

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}



void skeletonApp::applyWygladzanieGaussowskie5x5()
{
    int size;
    double tabFiltr[5][5];

    tabFiltr[0][0] = 1;
    tabFiltr[0][1] = 4;
    tabFiltr[0][2] = 7;
    tabFiltr[0][3] = 4;
    tabFiltr[0][4] = 1;

    tabFiltr[1][0] = 4;
    tabFiltr[1][1] = 16;
    tabFiltr[1][2] = 26;
    tabFiltr[1][3] = 16;
    tabFiltr[1][4] = 4;

    tabFiltr[2][0] = 7;
    tabFiltr[2][1] = 26;
    tabFiltr[2][2] = 41;
    tabFiltr[2][3] = 26;
    tabFiltr[2][4] = 7;

    tabFiltr[3][0] = 4;
    tabFiltr[3][1] = 16;
    tabFiltr[3][2] = 26;
    tabFiltr[3][3] = 16;
    tabFiltr[3][4] = 4;

    tabFiltr[4][0] = 1;
    tabFiltr[4][1] = 4;
    tabFiltr[4][2] = 7;
    tabFiltr[4][3] = 4;
    tabFiltr[4][4] = 1;

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -2; k < 3; k++)
                for (int l = -2; l < 3; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 2][l + 2]) / 273;
                        g += ((double)qGreen(pixel) * tabFiltr[k + 2][l + 2]) / 273;
                        b += ((double)qBlue(pixel) * tabFiltr[k + 2][l + 2]) / 273;

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}

void skeletonApp::applyKrawedziowyRoberts() // Nie zaimplementowano
{
    int size;
    double r1[2][2], r2[2][2];

    r1[0][0] = 0.0;
    r1[0][1] = -1.0;
    r1[1][0] = 1.0;
    r1[1][1] = 0.0;

    r2[0][0] = 1.0;
    r2[0][1] = 0.0;
    r2[1][0] = 0.0;
    r2[1][1] = -1.0;

}

void skeletonApp::applyKrawedziowyPrewit() // Nie zaimplementowano
{
    int size;
    double p1[3][3], p2[3][3];

    p1[0][0] = -1.0;
    p1[0][1] = -1.0;
    p1[0][2] = -1.0;

    p1[1][0] = 0.0;
    p1[1][1] = 0.0;
    p1[1][2] = 0.0;

    p1[2][0] = 1.0;
    p1[2][1] = 1.0;
    p1[2][2] = 1.0;

    p2[0][0] = 1.0;
    p2[0][1] = 0.0;
    p2[0][2] = -1.0;

    p2[1][0] = 1.0;
    p2[1][1] = 0.0;
    p2[1][2] = -1.0;

    p2[2][0] = 1.0;
    p2[2][1] = 0.0;
    p2[2][2] = -1.0;
}

void skeletonApp::applyKrawedziowySobel() // Nie zaimplementowano
{
    int size;
    double s1[3][3], s2[3][3];

    s1[0][0] = -1.0;
    s1[0][1] = -2.0;
    s1[0][2] = -1.0;

    s1[1][0] = 0.0;
    s1[1][1] = 0.0;
    s1[1][2] = 0.0;

    s1[2][0] = 1.0;
    s1[2][1] = 2.0;
    s1[2][2] = 1.0;

    s2[0][0] = 1.0;
    s2[0][1] = 0.0;
    s2[0][2] = -1.0;

    s2[1][0] = 2.0;
    s2[1][1] = 0.0;
    s2[1][2] = -2.0;

    s2[2][0] = 1.0;
    s2[2][1] = 0.0;
    s2[2][2] = -1.0;

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double x, y;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            x = 0;
            y = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        x += ((double)qRed(pixel) * s2[k + 1][l + 1]);
                        y += ((double)qRed(pixel) * s1[k + 1][l + 1]);

                    }

                }

            imageResult.setPixel(i, j, QColor(sqrt(pow(x, 2) + pow(y, 2)), sqrt(pow(x, 2) + pow(y, 2)), sqrt(pow(x, 2) + pow(y, 2))).rgb());
        }
    setImage(imageResult);
}

void skeletonApp::applyKrawedziowyKirsch1()
{
    int size;
    double tabFiltr[3][3];

    tabFiltr[0][0] = 5.0;
    tabFiltr[0][1] = -3.0;
    tabFiltr[0][2] = -3.0;

    tabFiltr[1][0] = 5.0;
    tabFiltr[1][1] = 0.0;
    tabFiltr[1][2] = -3.0;

    tabFiltr[2][0] = 5.0;
    tabFiltr[2][1] = -3.0;
    tabFiltr[2][2] = -3.0;

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 1][l + 1]);
                        g += ((double)qGreen(pixel) * tabFiltr[k + 1][l + 1]);
                        b += ((double)qBlue(pixel) * tabFiltr[k + 1][l + 1]);

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}

void skeletonApp::applyKrawedziowyKirsch2()
{
    int size;
    double tabFiltr[3][3];

    tabFiltr[0][0] = 5.0;
    tabFiltr[0][1] = 5.0;
    tabFiltr[0][2] = -3.0;

    tabFiltr[1][0] = 5.0;
    tabFiltr[1][1] = 0.0;
    tabFiltr[1][2] = -3.0;

    tabFiltr[2][0] = -3.0;
    tabFiltr[2][1] = -3.0;
    tabFiltr[2][2] = -3.0;

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 1][l + 1]);
                        g += ((double)qGreen(pixel) * tabFiltr[k + 1][l + 1]);
                        b += ((double)qBlue(pixel) * tabFiltr[k + 1][l + 1]);

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}

void skeletonApp::applyKrawedziowyKirsch3()
{
    int size;
    double tabFiltr[3][3];

    tabFiltr[0][0] = 5.0;
    tabFiltr[0][1] = 5.0;
    tabFiltr[0][2] = 5.0;

    tabFiltr[1][0] = -3.0;
    tabFiltr[1][1] = 0.0;
    tabFiltr[1][2] = -3.0;

    tabFiltr[2][0] = -3.0;
    tabFiltr[2][1] = -3.0;
    tabFiltr[2][2] = -3.0;

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 1][l + 1]);
                        g += ((double)qGreen(pixel) * tabFiltr[k + 1][l + 1]);
                        b += ((double)qBlue(pixel) * tabFiltr[k + 1][l + 1]);

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}

void skeletonApp::applyLaplasjanKlasyczny()
{
    int size;
    double tabFiltr[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tabFiltr[i][j] = -1.0;
        }
    }
    tabFiltr[1][1] = 8.0;

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 1][l + 1]);
                        g += ((double)qGreen(pixel) * tabFiltr[k + 1][l + 1]);
                        b += ((double)qBlue(pixel) * tabFiltr[k + 1][l + 1]);

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}

void skeletonApp::applyLaplasjanLog()
{
    int size;
    double tabFiltr[9][9];

    tabFiltr[0][0] = 0.0;
    tabFiltr[0][1] = 1.0;
    tabFiltr[0][2] = 1.0;
    tabFiltr[0][3] = 2.0;
    tabFiltr[0][4] = 2.0;
    tabFiltr[0][5] = 2.0;
    tabFiltr[0][6] = 1.0;
    tabFiltr[0][7] = 1.0;
    tabFiltr[0][8] = 0.0;

    tabFiltr[1][0] = 1.0;
    tabFiltr[1][1] = 2.0;
    tabFiltr[1][2] = 4.0;
    tabFiltr[1][3] = 5.0;
    tabFiltr[1][4] = 5.0;
    tabFiltr[1][5] = 5.0;
    tabFiltr[1][6] = 4.0;
    tabFiltr[1][7] = 2.0;
    tabFiltr[1][8] = 1.0;

    tabFiltr[2][0] = 1.0;
    tabFiltr[2][1] = 4.0;
    tabFiltr[2][2] = 5.0;
    tabFiltr[2][3] = 3.0;
    tabFiltr[2][4] = 0.0;
    tabFiltr[2][5] = 3.0;
    tabFiltr[2][6] = 5.0;
    tabFiltr[2][7] = 4.0;
    tabFiltr[2][8] = 1.0;

    tabFiltr[3][0] = 2.0;
    tabFiltr[3][1] = 5.0;
    tabFiltr[3][2] = 3.0;
    tabFiltr[3][3] = -12.0;
    tabFiltr[3][4] = -24.0;
    tabFiltr[3][5] = -12.0;
    tabFiltr[3][6] = 3.0;
    tabFiltr[3][7] = 5.0;
    tabFiltr[3][8] = 2.0;

    tabFiltr[4][0] = 2.0;
    tabFiltr[4][1] = 5.0;
    tabFiltr[4][2] = 0.0;
    tabFiltr[4][3] = -24.0;
    tabFiltr[4][4] = -50.0;
    tabFiltr[4][5] = -24.0;
    tabFiltr[4][6] = 0.0;
    tabFiltr[4][7] = 5.0;
    tabFiltr[4][8] = 2.0;

    tabFiltr[5][0] = 2.0;
    tabFiltr[5][1] = 5.0;
    tabFiltr[5][2] = 3.0;
    tabFiltr[5][3] = -12.0;
    tabFiltr[5][4] = -24.0;
    tabFiltr[5][5] = -12.0;
    tabFiltr[5][6] = 3.0;
    tabFiltr[5][7] = 5.0;
    tabFiltr[5][8] = 2.0;

    tabFiltr[6][0] = 1.0;
    tabFiltr[6][1] = 4.0;
    tabFiltr[6][2] = 5.0;
    tabFiltr[6][3] = 3.0;
    tabFiltr[6][4] = 0.0;
    tabFiltr[6][5] = 3.0;
    tabFiltr[6][6] = 5.0;
    tabFiltr[6][7] = 4.0;
    tabFiltr[6][8] = 1.0;

    tabFiltr[7][0] = 1.0;
    tabFiltr[7][1] = 2.0;
    tabFiltr[7][2] = 4.0;
    tabFiltr[7][3] = 5.0;
    tabFiltr[7][4] = 5.0;
    tabFiltr[7][5] = 5.0;
    tabFiltr[7][6] = 4.0;
    tabFiltr[7][7] = 2.0;
    tabFiltr[7][8] = 1.0;

    tabFiltr[8][0] = 0.0;
    tabFiltr[8][1] = 0.0;
    tabFiltr[8][2] = 1.0;
    tabFiltr[8][3] = 2.0;
    tabFiltr[8][4] = 2.0;
    tabFiltr[8][5] = 2.0;
    tabFiltr[8][6] = 1.0;
    tabFiltr[8][7] = 0.0;
    tabFiltr[8][8] = 0.0;

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -4; k < 5; k++)
                for (int l = -4; l < 5; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 4][l + 4]);
                        g += ((double)qGreen(pixel) * tabFiltr[k + 4][l + 4]);
                        b += ((double)qBlue(pixel) * tabFiltr[k + 4][l + 4]);

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    setImage(imageResult);
}

void skeletonApp::applyLaplasjanDog()
{
    // Nie zaimplementowano
}

void skeletonApp::applyWyostrzanie3x3()
{
    int size;
    double tabFiltr[3][3];

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            tabFiltr[i][j] = -1.0;
        }
    }
    tabFiltr[1][1] = 9.0;

    int w = image.width();
    int h = image.height();

    imageSmooth = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 1][l + 1]) / 9;
                        g += ((double)qGreen(pixel) * tabFiltr[k + 1][l + 1]) / 9;
                        b += ((double)qBlue(pixel) * tabFiltr[k + 1][l + 1]) / 9;

                    }

                }

            imageSmooth.setPixel(i, j, QColor(r, g, b).rgb());

        }

    imageResult = QImage(w, h, image.format());

    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++) {
            QRgb pixel = image.pixel(i, j);
            QRgb pixelSmooth = imageSmooth.pixel(i, j);

            r = (double)qRed(pixel) + (0.2 * ((double)qRed(pixel) - (double)qRed(pixelSmooth)));
            g = (double)qGreen(pixel) + (0.2 * ((double)qGreen(pixel) - (double)qGreen(pixelSmooth)));
            b = (double)qBlue(pixel) + (0.2 * ((double)qBlue(pixel) - (double)qBlue(pixelSmooth)));

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());
        }

    setImage(imageResult);
}

void skeletonApp::applyWyostrzanie5x5()
{
    int size;
    double tabFiltr[5][5];

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            tabFiltr[i][j] = -1.0;
        }
    }
    tabFiltr[2][2] = 25.0;

    int w = image.width();
    int h = image.height();

    imageSmooth = QImage(w, h, image.format());

    double r, g, b;
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -2; k < 3; k++)
                for (int l = -2; l < 3; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 2][l + 2]) / 25;
                        g += ((double)qGreen(pixel) * tabFiltr[k + 2][l + 2]) / 25;
                        b += ((double)qBlue(pixel) * tabFiltr[k + 2][l + 2]) / 25;

                    }

                }

            imageSmooth.setPixel(i, j, QColor(r, g, b).rgb());

        }

    imageResult = QImage(w, h, image.format());

    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++) {
            QRgb pixel = image.pixel(i, j);
            QRgb pixelSmooth = imageSmooth.pixel(i, j);

            r = (double)qRed(pixel) + (0.2 * ((double)qRed(pixel) - (double)qRed(pixelSmooth)));
            g = (double)qGreen(pixel) + (0.2 * ((double)qGreen(pixel) - (double)qGreen(pixelSmooth)));
            b = (double)qBlue(pixel) + (0.2 * ((double)qBlue(pixel) - (double)qBlue(pixelSmooth)));

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());
        }

    setImage(imageResult);
}

//Lab 5

void skeletonApp::applyOtsu2() {
    bool is_grayscale = true;
    int w = image.width();
    int h = image.height();
    int r;
    double u = 0.0, ut = 0.0, ot = 0.0, wynik = 0.0, q = 0.0;
    int  t = 0;
    imageResult = QImage(w, h, image.format());
    float TableAvg[256];
    for (int i = 0; i < 256; i++) 
    {
        TableAvg[i] = 0.0;
    }
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);

            r = (int)qRed(pixel);

            TableAvg[r] = TableAvg[r] + 1.0;
        }
    for (int i = 0; i < 256; i++) 
    {
        TableAvg[i] = TableAvg[i]/(w*h);
    }

    for (int i = 0; i < 256; i++) 
    {
        u = u + (i*TableAvg[i]) ;
    }

    for (int i = 0; i < 256; i++) 
    {
        ut = 0.0;
        ot = 0.0;

        if (i > 0)
        {
            for (int j = 1; j <= i; j++) 
            {
                ut = ut + (j * TableAvg[j]);
                ot = ot + TableAvg[j];
            }
        }
        else
        {
            ut = ut + (i * TableAvg[i]);
            ot = ot + TableAvg[i];
        }
        q = (pow((ut-(u*ot)),2)) / (ot*(1-ot));
        if (q > wynik) {
            wynik = (double)(q);
            t = i;
        }
    }
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);
            int p = qGray(pixel);

            if (p > t)
            {
                p = 255;// WHITE
            }
            else
            {
                p = 0;// BLACK
            }
            imageResult.setPixel(i, j, QColor(p, p, p).rgb());
        }

    setImage(imageResult);

}

//Lab 6-9

void skeletonApp::applyCanny() {

    int w = image.width();
    int h = image.height();
    imageResult = QImage(w, h, image.format());
    imageResult2 = QImage(w, h, image.format());
    imageDir = QImage(w, h, image.format());


    double r, g, b, x, y;
    int size;
    double tabFiltr[5][5];
    double s1[3][3], s2[3][3];

    tabFiltr[0][0] = 1;
    tabFiltr[0][1] = 4;
    tabFiltr[0][2] = 7;
    tabFiltr[0][3] = 4;
    tabFiltr[0][4] = 1;

    tabFiltr[1][0] = 4;
    tabFiltr[1][1] = 16;
    tabFiltr[1][2] = 26;
    tabFiltr[1][3] = 16;
    tabFiltr[1][4] = 4;

    tabFiltr[2][0] = 7;
    tabFiltr[2][1] = 26;
    tabFiltr[2][2] = 41;
    tabFiltr[2][3] = 26;
    tabFiltr[2][4] = 7;

    tabFiltr[3][0] = 4;
    tabFiltr[3][1] = 16;
    tabFiltr[3][2] = 26;
    tabFiltr[3][3] = 16;
    tabFiltr[3][4] = 4;

    tabFiltr[4][0] = 1;
    tabFiltr[4][1] = 4;
    tabFiltr[4][2] = 7;
    tabFiltr[4][3] = 4;
    tabFiltr[4][4] = 1;

    s1[0][0] = -1.0;
    s1[0][1] = -2.0;
    s1[0][2] = -1.0;

    s1[1][0] = 0.0;
    s1[1][1] = 0.0;
    s1[1][2] = 0.0;

    s1[2][0] = 1.0;
    s1[2][1] = 2.0;
    s1[2][2] = 1.0;

    s2[0][0] = 1.0;
    s2[0][1] = 0.0;
    s2[0][2] = -1.0;

    s2[1][0] = 2.0;
    s2[1][1] = 0.0;
    s2[1][2] = -2.0;

    s2[2][0] = 1.0;
    s2[2][1] = 0.0;
    s2[2][2] = -1.0;

   

    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            r = 0;
            g = 0;
            b = 0;
            for (int k = -2; k < 3; k++)
                for (int l = -2; l < 3; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = image.pixel(i + k, j + l);

                        r += ((double)qRed(pixel) * tabFiltr[k + 2][l + 2]) / 273;
                        g += ((double)qGreen(pixel) * tabFiltr[k + 2][l + 2]) / 273;
                        b += ((double)qBlue(pixel) * tabFiltr[k + 2][l + 2]) / 273;

                    }

                }

            imageResult.setPixel(i, j, QColor(r, g, b).rgb());

        }
    
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            x = 0;
            y = 0;
            for (int k = -1; k < 2; k++)
                for (int l = -1; l < 2; l++)
                {
                    if (i + k >= 0 && j + l >= 0 && i + k < w && j + l < h) {

                        QRgb pixel = imageResult.pixel(i + k, j + l);

                        x += ((double)qRed(pixel) * s2[k + 1][l + 1]);
                        y += ((double)qRed(pixel) * s1[k + 1][l + 1]);

                    }

                }

            imageResult2.setPixel(i, j, QColor(sqrt(pow(x, 2) + pow(y, 2)), sqrt(pow(x, 2) + pow(y, 2)), sqrt(pow(x, 2) + pow(y, 2))).rgb());
            
        }


    setImage(imageResult2);

}

void skeletonApp::applyHough() {
    bool is_grayscale = true;
    int w = image.width();
    int h = image.height();
    int r;
    double u = 0.0, ut = 0.0, ot = 0.0, wynik = 0.0, q = 0.0;
    int  t = 0;
    imageResult = QImage(w, h, image.format());
    float TableAvg[256];
    for (int i = 0; i < 256; i++)
    {
        TableAvg[i] = 0.0;
    }
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);

            r = (int)qRed(pixel);

            TableAvg[r] = TableAvg[r] + 1.0;
        }
    for (int i = 0; i < 256; i++)
    {
        TableAvg[i] = TableAvg[i] / (w * h);
    }

    for (int i = 0; i < 256; i++)
    {
        u = u + (i * TableAvg[i]);
    }

    for (int i = 0; i < 256; i++)
    {
        ut = 0.0;
        ot = 0.0;

        if (i > 0)
        {
            for (int j = 1; j <= i; j++)
            {
                ut = ut + (j * TableAvg[j]);
                ot = ot + TableAvg[j];
            }
        }
        else
        {
            ut = ut + (i * TableAvg[i]);
            ot = ot + TableAvg[i];
        }
        q = (pow((ut - (u * ot)), 2)) / (ot * (1 - ot));
        if (q > wynik) {
            wynik = (double)(q);
            t = i;
        }
    }
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
        {
            QRgb pixel = image.pixel(i, j);
            int p = qGray(pixel);

            if (p > t)
            {
                p = 255;// WHITE
            }
            else
            {
                p = 0;// BLACK
            }
            imageResult.setPixel(i, j, QColor(p, p, p).rgb());
        }

    setImage(imageResult);

}

void skeletonApp::updateActions()
{
    saveAsAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void skeletonApp::createActions()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openAct = fileMenu->addAction(tr("&Open..."), this, &skeletonApp::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &skeletonApp::saveAs);
    saveAsAct->setEnabled(false);


    fileMenu->addSeparator();

    QAction* exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &skeletonApp::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction* pasteAct = editMenu->addAction(tr("&Paste"), this, &skeletonApp::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &skeletonApp::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &skeletonApp::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &skeletonApp::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &skeletonApp::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    filtersMenu = menuBar()->addMenu(tr("&Filters"));
    filtersMenu->setDisabled(true);

    desaturacja = filtersMenu->addAction(tr("&Desaturacja"), this, &skeletonApp::applyDesaturacja);
    negatyw = filtersMenu->addAction(tr("&Negatyw"), this, &skeletonApp::applyNegatyw);
    filtersMenu->addSeparator();
    kontrast_lin_up = filtersMenu->addAction(tr("&Kontrast liniowy +"), this, &skeletonApp::applyKontrastLinUp);
    kontrast_lin_down = filtersMenu->addAction(tr("&Kontrast liniowy -"), this, &skeletonApp::applyKontrastLinDown);
    kontrast_log_up = filtersMenu->addAction(tr("&Kontrast logarytmiczny +"), this, &skeletonApp::applyKontrastLogUp);
    kontrast_log_down = filtersMenu->addAction(tr("&Kontrast logarytmiczny -"), this, &skeletonApp::applyKontrastLogDown);
    kontrast_pow_up = filtersMenu->addAction(tr("&Kontrast potegowy +"), this, &skeletonApp::applyKontrastPowUp);
    kontrast_pow_down = filtersMenu->addAction(tr("&Kontrast potegowy -"), this, &skeletonApp::applyKontrastPowDown);
    filtersMenu->addSeparator();
    jasnosc_up = filtersMenu->addAction(tr("&Jasnosc +"), this, &skeletonApp::applyJasnoscUp);
    jasnosc_down = filtersMenu->addAction(tr("&Jasnosc -"), this, &skeletonApp::applyJasnoscDown);
    filtersMenu->addSeparator();
    nasycenie_up = filtersMenu->addAction(tr("&Nasycenie"), this, &skeletonApp::applyNasycenie);
    filtersMenu->addSeparator();
    //suma = filtersMenu->addAction(tr("&Suma obrazow"), this, &skeletonApp::applyTestFilter);
    //roznica = filtersMenu->addAction(tr("&Roznica obrazow"), this, &skeletonApp::applyTestFilter);
    //iloczyn = filtersMenu->addAction(tr("&Iloczyn obrazow"), this, &skeletonApp::applyTestFilter);
    monoRed_up = filtersMenu->addAction(tr("&Monochromatyczny czerwony +"), this, &skeletonApp::applyMonoRedUp);
    monoRed_down = filtersMenu->addAction(tr("&Monochromatyczny czerwony -"), this, &skeletonApp::applyMonoRedDown);
    monoGreen_up = filtersMenu->addAction(tr("&Monochromatyczny zielony +"), this, &skeletonApp::applyMonoGreenUp);
    monoGreen_down = filtersMenu->addAction(tr("&Monochromatyczny zielony -"), this, &skeletonApp::applyMonoGreenDown);
    monoBlue_up = filtersMenu->addAction(tr("&Monochromatyczny niebieski +"), this, &skeletonApp::applyMonoBlueUp);
    monoBlue_down = filtersMenu->addAction(tr("&Monochromatyczny niebieski -"), this, &skeletonApp::applyMonoBlueDown);

    histogramMenu = menuBar()->addMenu(tr("&Histogram"));
    histogramMenu->setDisabled(true);
    generuj_histogram = histogramMenu->addAction(tr("&Generuj histogram"), this, &skeletonApp::generateHistograms);
    rozciagnij_histogram = histogramMenu->addAction(tr("&Rozcjagnij histogram"), this, &skeletonApp::applyRozciaganieHistogram);
    wyrownaj_histogram = histogramMenu->addAction(tr("&Wyrownaj histogram"), this, &skeletonApp::applyWyrownanieHistogram);

    //testFilter->setDisabled(true);
    filtrySplotoweMenu = menuBar()->addMenu(tr("&Filtry Splotowe"));
    filtrySplotoweMenu->setDisabled(true);
    jednostajny_wygladzajacy_3x3 = filtrySplotoweMenu->addAction(tr("&Filtr wygładzający jednostajny 3x3"), this, &skeletonApp::applyWygladzanieJednostajne3x3);
    jednostajny_wygladzajacy_5x5 = filtrySplotoweMenu->addAction(tr("&Filtr wygładzający jednostajny 5x5"), this, &skeletonApp::applyWygladzanieJednostajne5x5);

    gaussowski_wygladzajacy_5x5 = filtrySplotoweMenu->addAction(tr("&Filtr wygładzający gaussowski 5x5"), this, &skeletonApp::applyWygladzanieGaussowskie5x5);

    filtrySplotoweMenu->addSeparator();

    krawedziowe_roberts = filtrySplotoweMenu->addAction(tr("&Filtr krawędziowy : Roberts"), this, &skeletonApp::applyKrawedziowyRoberts);
    krawedziowe_roberts->setDisabled(true);
    krawedziowe_prewit = filtrySplotoweMenu->addAction(tr("&Filtr krawędziowy : Prewit"), this, &skeletonApp::applyKrawedziowyPrewit);
    krawedziowe_prewit->setDisabled(true);
    krawedziowe_sobel = filtrySplotoweMenu->addAction(tr("&Filtr krawędziowy : Sobel"), this, &skeletonApp::applyKrawedziowySobel);
    

    krawedziowe_kirsch_1 = filtrySplotoweMenu->addAction(tr("&Filtr krawędziowy : Kirsch 1"), this, &skeletonApp::applyKrawedziowyKirsch1);
    krawedziowe_kirsch_2 = filtrySplotoweMenu->addAction(tr("&Filtr krawędziowy : Kirsch 2"), this, &skeletonApp::applyKrawedziowyKirsch2);
    krawedziowe_kirsch_3 = filtrySplotoweMenu->addAction(tr("&Filtr krawędziowy : Kirsch 3"), this, &skeletonApp::applyKrawedziowyKirsch3);

    laplasjan_klasyczny = filtrySplotoweMenu->addAction(tr("&Filtr Laplasjan klasyczny"), this, &skeletonApp::applyLaplasjanKlasyczny);
    laplasjan_log = filtrySplotoweMenu->addAction(tr("&Filtr Laplasjan LoG"), this, &skeletonApp::applyLaplasjanLog);
    laplasjan_dog = filtrySplotoweMenu->addAction(tr("&Filtr Laplasjan Dog"), this, &skeletonApp::applyLaplasjanDog);
    laplasjan_dog->setDisabled(true);
    filtrySplotoweMenu->addSeparator();
    wyostrzanie_3x3 = filtrySplotoweMenu->addAction(tr("&Filtr wyostrzający 3x3"), this, &skeletonApp::applyWyostrzanie3x3);
    wyostrzanie_5x5 = filtrySplotoweMenu->addAction(tr("&Filtr wyostrzający 5x5"), this, &skeletonApp::applyWyostrzanie5x5);

    binaryzacjaMenu = menuBar()->addMenu(tr("&Algorytmy binaryzacji"));
    binaryzacjaMenu->setDisabled(true);
    otsu = binaryzacjaMenu->addAction(tr("&Algorytm Otsu"), this, &skeletonApp::applyOtsu2);
    doWyboryMenu = menuBar()->addMenu(tr("&Algorytmy do wyboru"));
    doWyboryMenu->setDisabled(true);
    canny = doWyboryMenu->addAction(tr("&Algorytm Canny'ego"), this, &skeletonApp::applyCanny);
    hough = doWyboryMenu->addAction(tr("&Algorytm Hough'ego"), this, &skeletonApp::applyHough);

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &skeletonApp::about);


}

