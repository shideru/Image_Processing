#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_skeletonApp.h"
#include <QImage>
#include <QLabel>
#include <QScrollArea>
#include <QHBoxLayout>


class skeletonApp : public QMainWindow
{
    Q_OBJECT

public:
    skeletonApp(QWidget* parent = Q_NULLPTR);

    bool loadFile(const QString& fileName);

private:
    Ui::skeletonAppClass ui;
    QAction* saveAsAct;
    QAction* copyAct;
    QAction* zoomInAct;
    QAction* zoomOutAct;
    QAction* normalSizeAct;
    QAction* fitToWindowAct;
    QAction* testFilter;
    //Lab 2
    QAction* desaturacja;
    QAction* negatyw;
    QAction* kontrast_lin_up;
    QAction* kontrast_log_up;
    QAction* kontrast_pow_up;
    QAction* kontrast_lin_down;
    QAction* kontrast_log_down;
    QAction* kontrast_pow_down;
    QAction* jasnosc_up;
    QAction* jasnosc_down;
    QAction* nasycenie_up;
    QAction* nasycenie_down;
    QAction* suma_up;
    QAction* roznica_up;
    QAction* iloczyn_up;
    QAction* suma_down;
    QAction* roznica_down;
    QAction* iloczyn_down;
    QAction* monoRed_up;
    QAction* monoGreen_up;
    QAction* monoBlue_up;
    QAction* monoRed_down;
    QAction* monoGreen_down;
    QAction* monoBlue_down;

    //Lab 3
    QAction* generuj_histogram;
    QAction* rozciagnij_histogram;
    QAction* wyrownaj_histogram;

    //Lab 4
    QAction* jednostajny_wygladzajacy_3x3;

    QAction* jednostajny_wygladzajacy_5x5;
    QAction* gaussowski_wygladzajacy_5x5;


    QAction* krawedziowe_roberts;
    QAction* krawedziowe_prewit;
    QAction* krawedziowe_sobel;
    QAction* krawedziowe_kirsch_1;
    QAction* krawedziowe_kirsch_2;
    QAction* krawedziowe_kirsch_3;

    QAction* laplasjan_klasyczny;
    QAction* laplasjan_log;
    QAction* laplasjan_dog;

    QAction* wyostrzanie_3x3;
    QAction* wyostrzanie_5x5;

    //Lab 5
    QAction* otsu;

    //Lab 6-9
    QAction* canny;
    QAction* hough;

    //Lab 2
    QMenu* filtersMenu;
    //Lab 3
    QMenu* histogramMenu;
    //Lab 4
    QMenu* filtrySplotoweMenu;
    //Lab 5
    QMenu* binaryzacjaMenu;
    //Lab 6-9
    QMenu* doWyboryMenu;

    QImage image;
    QImage imageResult;
    QImage imageResult2;
    QImage imageDir;
    QImage imageSmooth;
    QImage imageHistogram;
    QImage histogramAvg;
    QImage histogramRed;
    QImage histogramGreen;
    QImage histogramBlue;
    QLabel* imageLabel;
    QLabel* histogramLabel;
    QScrollArea* scrollArea;
    QHBoxLayout* hBoxLayout;
    double scaleFactor = 1;
    int width;
    int height;
    int nbChannels;
    int nbRealChannels;
    int maxValue;
    std::vector<std::vector<std::vector<int>>>  data;
    std::vector<std::vector<std::vector<int>>>  dataResult;


    void createActions();
    bool saveFile(const QString& fileName);
    void setImage(const QImage& newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar* scrollBar, double factor);
    void updateActions();


private slots:
    void open();
    void saveAs();
    //void print();
    void copy();
    void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();
    //void applyTestFilter();

    //Lab 2
    void applyDesaturacja();
    void applyNegatyw();
    void applyKontrastLinUp();
    void applyKontrastLogUp();
    void applyKontrastPowUp();
    void applyKontrastLinDown();
    void applyKontrastLogDown();
    void applyKontrastPowDown();
    void applyJasnoscUp();
    void applyJasnoscDown();
    void applyNasycenie();
    //  void applySuma();
    //  void applyRoznica();
    //  void applyIloczyn();
    void applyMonoRedUp();
    void applyMonoGreenUp();
    void applyMonoBlueUp();
    void applyMonoRedDown();
    void applyMonoGreenDown();
    void applyMonoBlueDown();

    //Lab 3
    void generateHistograms();
    void applyRozciaganieHistogram();
    void applyWyrownanieHistogram();

    //Lab 4
    void applyWygladzanieJednostajne3x3();
    void applyWygladzanieJednostajne5x5();

    void applyWygladzanieGaussowskie5x5();
    void applyKrawedziowyRoberts();
    void applyKrawedziowyPrewit();
    void applyKrawedziowySobel();
    void applyKrawedziowyKirsch1();
    void applyKrawedziowyKirsch2();
    void applyKrawedziowyKirsch3();
    void applyLaplasjanKlasyczny();
    void applyLaplasjanLog();
    void applyLaplasjanDog();
    void applyWyostrzanie3x3();
    void applyWyostrzanie5x5();

    //Lab 5
    void applyOtsu2();
    
    //Lab 6-9
    void applyCanny();
    void applyHough();

};
