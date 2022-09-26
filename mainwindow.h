#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <complex>
#include <mutex>
#include <chrono>

#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>

#include <QWidget>
#include <QPainter>

#include <QFile>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "drawingwidget.h"
#include "toggle.h"

double linear(double val, double a_old, double b_old, double a_new, double b_new);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class Toggle;
    friend class DrawingWidget;


public:
    MainWindow(QWidget *parent = nullptr);

    void printMessage(const QString& message);
    void printResults(const QString message);

    void addRow(double re, double im);

    void setDefaultsT();
    void setDefaultsX();
    void setDefaultsY();
    void setDefaultsDensity();
    void setDefaultProdnumber();


    enum Plotting
    {
        no_graph = 0,
        trajectory,
        function
    };

    std::complex<double> evalPolyAt(double x) const;
    std::complex<double> evalPhiCapAt(double x) const;
    std::complex<double> evalPolyPrimeAt(double x) const;
    std::complex<double> evalPhiAt(double x) const;

    Plotting mode() const;

    const std::vector<std::pair<std::complex<double>,QObject*>>& roots() const;

    ~MainWindow();

private slots:

    void keyPressEvent(QKeyEvent* ev) override;

    void blockButtons();
    void unlockButtons();

    void makeCalcButtonBlue();
    void makeCalcButtonGray();

    void makeWidthButtonBlue();
    void makeWidthButtonGray();

    void performCalculations();
    void checkBoundaries();

    void on_addRoots_clicked();

    void on_deleteRoots_clicked();

    void on_drawButton_clicked();


    void on_xmin_textChanged(const QString &arg1);
    void on_xmax_textChanged(const QString &arg1);

    void on_ymin_textChanged(const QString &arg1);
    void on_ymax_textChanged(const QString &arg1);

    void on_tmin_textChanged(const QString &arg1);
    void on_tmax_textChanged(const QString &arg1);

    void on_density_textChanged(const QString &arg1);
    void on_prodNumber_textChanged(const QString &arg1);

    void on_actionFunctionMode_changed();
    void on_actionTrajectoryMode_changed();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionExit_triggered();

    void on_rootsTable_cellDoubleClicked(int row, int column);

    void openFile(const QString& fileName);
    void saveFile(const QString& fileName);

    void on_calcWidthButton_clicked();

    void on_calcButton_clicked();

    void on_actionInfo_triggered();

private:
    Ui::MainWindow *ui;


    Plotting mode_;

    size_t prodNumber;

    std::vector<std::pair<std::complex<double>,QObject*>> roots_;

    QFile current_settings;

    QFileDialog* fileopen;
    QFileDialog* filesave;

    bool save_notes;



    //##############################################
    //For Memoization
    //##############################################

    std::vector< std::complex<double> > poly_cache;
    std::vector< std::complex<double> > phiCap_cache;

    double tmin, tmax;
    size_t density, prod_numb;

    bool need_recalc = false;

    QPalette defaultButtonPalette;

    std::mutex calc_mutex;
    double time_amount;

};
#endif // MAINWINDOW_H
