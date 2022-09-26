#ifndef DRAWINGWIDGET_H
#define DRAWINGWIDGET_H

#include <QWidget>
#include <QPainter>
#include <complex>

class DrawingWidget : public QWidget
{
    Q_OBJECT
    friend class MainWindow;
public:
    explicit DrawingWidget(QWidget *parent = nullptr);

    //Getters
    double xmin() const {return xmin_;}
    double xmax() const {return xmax_;}

    double ymin() const {return ymin_;}
    double ymax() const {return ymax_;}

    double tmin() const {return tmin_;}
    double tmax() const {return tmax_;}

    size_t density() const {return density_;}


    //Setters
    void setXmin(double new_xmin){xmin_ = new_xmin;}
    void setXmax(double new_xmax){xmax_ = new_xmax;}

    void setYmin(double new_ymin){ymin_ = new_ymin;}
    void setYmax(double new_ymax){ymax_ = new_ymax;}

    void setTmin(double new_tmin){tmin_ = new_tmin;}
    void setTmax(double new_tmax){tmax_ = new_tmax;}

    void setDensity(int new_density);

signals:

public slots:
    void paintEvent(QPaintEvent *event) override;
    void draw(int mode, int functions);
private:
    int mode_ = 0;

    double xmin_ = -15, xmax_ = 15;

    double ymin_ = -10, ymax_ = 10;

    double tmin_ = -10, tmax_ = 10;

    int density_ = 1000;

    bool draw_abs_poly_sqr = false, draw_phiCap = false, draw_poly = false;
};

#endif // DRAWINGWIDGET_H
