#include "drawingwidget.h"
#include "mainwindow.h"

DrawingWidget::DrawingWidget(QWidget *parent)
    : QWidget{parent}
{
}

void DrawingWidget::setDensity(int new_density){density_ = new_density;}

void DrawingWidget::draw(int mode, int functions)
{
    mode_ = mode;
    draw_poly = functions & 1;
    draw_abs_poly_sqr = (functions >> 1) & 1;
    draw_phiCap = (functions >> 2) & 1;
    repaint();
}

void DrawingWidget::paintEvent(QPaintEvent *event)
{
    int width = this->width(), height = this->height();
    int hor_lines = 20;
    int ver_lines = 30;

    QPainter painter(this);
    Q_UNUSED(event);

    //###########################################
    //Drawing grid
    //###########################################

    painter.setBrush(QColor(50,50,60));
    painter.drawRect(0,0,width,height);

    painter.setPen(Qt::darkGray);
    painter.drawLine(0, height / 2, width, height / 2);
    painter.drawLine(width / 2, 0, width / 2, height);

    //Horizontal Grid
    for(int i = 1; i < hor_lines; ++i)
    {
        int y = i * height / hor_lines;

        painter.setPen(Qt::darkGray),
        painter.drawLine(0, y, width, y);

        auto s = QString::number(linear(y, height, 0, ymin_, ymax_));

        if(s[0] == '-')
            s.chop(s.length() - 4);
        else
            s.chop(s.length() - 3);

        painter.setPen(Qt::white),
        painter.drawText(width / 2, y, s);
    }

    //Vertical Grid
    for(int i = 1; i < ver_lines; ++i)
    {
        int x = i * width / ver_lines;

        painter.setPen(Qt::darkGray),
        painter.drawLine(x, 0, x, height);

        auto s = QString::number(linear(x, 0, width, xmin_, xmax_));

        if(s[0] == '-')
            s.chop(s.length() - 4);
        else
            s.chop(s.length() - 3);

        painter.setPen(Qt::white),
        painter.drawText(x, height / 2, s);
    }



    //###########################################
    //Drawing graph
    //###########################################

    auto p_ = dynamic_cast<QWidget*>(parent());
    while(p_->parent() != nullptr)
        p_ = dynamic_cast<QWidget*>(p_->parent());

    auto p = dynamic_cast<MainWindow*>(p_);

    if(p->calc_mutex.try_lock() == false)
    {
        p->printMessage(tr("Вычисления еще выполняются"));
        return;
    }
    switch(mode_)
    {
    case MainWindow::Plotting::trajectory:
        for(int i = 1; i < width; i++)
            painter.setPen(QColor::fromHsv(linear(i, 0, width, 0, 359), 255, 255)),
            painter.drawLine(i - 1, height - 1, i, height - 1);

        if(draw_poly)
        {
            painter.setPen(QColor::fromHsv(0, 255, 255));

            const auto& roots = p->roots();

            std::complex<double> curr_z = p->poly_cache[0];

            auto curr_x = curr_z.real();
            auto curr_y = curr_z.imag();

            int curr_pix = linear(curr_x, xmin_, xmax_, 0, width),
                curr_piy = linear(curr_y, ymin_, ymax_, height, 0);

            for(int curr_t = 1; curr_t <= density_; curr_t++)
            {
                std::complex<double> new_z = p->poly_cache[curr_t];


                int new_pix = linear(new_z.real(), xmin_, xmax_, 0, width),
                    new_piy = linear(new_z.imag(), ymin_, ymax_, height, 0);

                painter.drawLine(curr_pix, curr_piy, new_pix, new_piy);

                curr_pix = new_pix,
                curr_piy = new_piy;

                painter.setPen(QColor::fromHsv(linear(curr_t, 0, density_, 0, 359), 255, 255));
            }
            for(auto root : roots)
            {
                painter.setPen(Qt::white);

                curr_pix = linear(root.first.real(), xmin_, xmax_, 0, width);
                curr_piy = linear(root.first.imag(), ymin_, ymax_, height, 0);

                painter.drawEllipse(curr_pix - 2, curr_piy - 2, 4, 4);
            }
        }
        if(draw_phiCap)
        {
            painter.setPen(QColor::fromHsv(0, 255, 255));

            const auto& roots = p->roots();

            std::complex<double> curr_z = p->phiCap_cache[0];

            auto curr_x = curr_z.real();
            auto curr_y = curr_z.imag();

            int curr_pix = linear(curr_x, xmin_, xmax_, 0, width),
                curr_piy = linear(curr_y, ymin_, ymax_, height, 0);

            for(int curr_t = 1; curr_t <= density_; curr_t++)
            {
                std::complex<double> new_z = p->phiCap_cache[curr_t];

                int new_pix = linear(new_z.real(), xmin_, xmax_, 0, width),
                    new_piy = linear(new_z.imag(), ymin_, ymax_, height, 0);

                painter.drawLine(curr_pix, curr_piy, new_pix, new_piy);

                curr_pix = new_pix,
                curr_piy = new_piy;

                painter.setPen(QColor::fromHsv(linear(curr_t, 0, density_, 0, 359), 255, 255));
            }
            for(auto root : roots)
            {
                painter.setPen(Qt::white);

                curr_pix = linear(root.first.real(), xmin_, xmax_, 0, width);
                curr_piy = linear(root.first.imag(), ymin_, ymax_, height, 0);

                painter.drawEllipse(curr_pix - 2, curr_piy - 2, 4, 4);
            }
        }
        if(draw_abs_poly_sqr)
        {
            painter.setPen(QColor::fromHsv(0, 255, 255));

            const auto& roots = p->roots();

            std::complex<double> curr_z = p->poly_cache[0];
            curr_z *= std::conj(curr_z);

            auto curr_x = curr_z.real();
            auto curr_y = curr_z.imag();

            int curr_pix = linear(curr_x, xmin_, xmax_, 0, width),
                curr_piy = linear(curr_y, ymin_, ymax_, height, 0);

            for(int curr_t = 1; curr_t <= density_; curr_t++)
            {
                std::complex<double> new_z = p->poly_cache[curr_t];
                new_z *= std::conj(new_z);

                int new_pix = linear(new_z.real(), xmin_, xmax_, 0, width),
                    new_piy = linear(new_z.imag(), ymin_, ymax_, height, 0);

                painter.drawLine(curr_pix, curr_piy, new_pix, new_piy);

                curr_pix = new_pix,
                curr_piy = new_piy;

                painter.setPen(QColor::fromHsv(linear(curr_t, 0, density_, 0, 359), 255, 255));
            }
            for(auto root : roots)
            {
                painter.setPen(Qt::white);

                curr_pix = linear(root.first.real(), xmin_, xmax_, 0, width);
                curr_piy = linear(root.first.imag(), ymin_, ymax_, height, 0);

                painter.drawEllipse(curr_pix - 2, curr_piy - 2, 4, 4);
            }
        }
        break;

    case MainWindow::Plotting::function:
        for(int i = 1; i < width; i++)
            painter.setPen(QColor::fromHsv(linear(i, 0, width, 0, 359), 255, 255)),
            painter.drawLine(i - 1, height / 2, i, height / 2);

        if(draw_poly)
        {
            painter.setPen(QColor::fromHsv(0, 255, 255));

            std::complex<double> curr_x = p->poly_cache[0];

            auto curr_y_real = curr_x.real();
            auto curr_y_imag = curr_x.imag();

            int curr_pix        = linear(tmin_,         xmin_, xmax_, 0, width),
                curr_piy_real   = linear(curr_y_real,   ymin_, ymax_, height, 0),
                curr_piy_imag   = linear(curr_y_imag,   ymin_, ymax_, height, 0);

            int min_cache_index = (int)linear(std::fmax(xmin_,tmin_),tmin_,tmax_,0,density_) >= 0
                                ? (int)linear(std::fmax(xmin_,tmin_),tmin_,tmax_,0,density_)
                                : 0;

            int max_cache_index = (int)linear(std::fmin(xmax_,tmax_),tmin_,tmax_,0,density_) <= density_
                                ? (int)linear(std::fmin(xmax_,tmax_),tmin_,tmax_,0,density_)
                                : density_;

            for(int curr_t = min_cache_index; curr_t <= max_cache_index; curr_t++)
            {
                std::complex<double> new_x = p->poly_cache[curr_t];

                int new_pix         = linear(tmin_ + curr_t * (tmax_ - tmin_) / density_, xmin_, xmax_, 0, width),
                    new_piy_real    = linear(new_x.real(), ymin_, ymax_, height, 0),
                    new_piy_imag    = linear(new_x.imag(), ymin_, ymax_, height, 0);

                painter.drawLine(curr_pix, curr_piy_real, new_pix, new_piy_real);
                painter.drawLine(curr_pix, curr_piy_imag, new_pix, new_piy_imag);

                curr_pix = new_pix,
                curr_piy_real = new_piy_real,
                curr_piy_imag = new_piy_imag;

                painter.setPen(QColor::fromHsv(linear(curr_t, 0, density_, 0, 359), 255, 255));
            }
        }
        if(draw_phiCap)
        {
            painter.setPen(QColor::fromHsv(0, 255, 255));

            std::complex<double> curr_x = p->phiCap_cache[0];

            auto curr_y_real = curr_x.real();
            auto curr_y_imag = curr_x.imag();

            int curr_pix        = linear(tmin_,         xmin_, xmax_, 0, width),
                curr_piy_real   = linear(curr_y_real,   ymin_, ymax_, height, 0),
                curr_piy_imag   = linear(curr_y_imag,   ymin_, ymax_, height, 0);

            int min_cache_index = (int)linear(std::fmax(xmin_,tmin_),tmin_,tmax_,0,density_) >= 0
                                ? (int)linear(std::fmax(xmin_,tmin_),tmin_,tmax_,0,density_)
                                : 0;

            int max_cache_index = (int)linear(std::fmin(xmax_,tmax_),tmin_,tmax_,0,density_) <= density_
                                ? (int)linear(std::fmin(xmax_,tmax_),tmin_,tmax_,0,density_)
                                : density_;

            for(int curr_t = min_cache_index; curr_t <= max_cache_index; curr_t++)
            {
                std::complex<double> new_x = p->phiCap_cache[curr_t];

                int new_pix         = linear(tmin_ + curr_t * (tmax_ - tmin_) / density_, xmin_, xmax_, 0, width),
                    new_piy_real    = linear(new_x.real(), ymin_, ymax_, height, 0),
                    new_piy_imag    = linear(new_x.imag(), ymin_, ymax_, height, 0);

                painter.drawLine(curr_pix,curr_piy_real,new_pix,new_piy_real);
                painter.drawLine(curr_pix,curr_piy_imag,new_pix,new_piy_imag);

                curr_pix = new_pix,
                curr_piy_real = new_piy_real,
                curr_piy_imag = new_piy_imag;

                painter.setPen(QColor::fromHsv(linear(curr_t, 0, density_, 0, 359), 255, 255));
            }

        }
        if(draw_abs_poly_sqr)
        {
            painter.setPen(QColor::fromHsv(0, 255, 255));

            std::complex<double> curr_x = p->poly_cache[0];

            curr_x *= std::conj(curr_x);

            auto curr_y_real = curr_x.real();
            auto curr_y_imag = curr_x.imag();

            int curr_pix        = linear(tmin_,         xmin_, xmax_, 0, width),
                curr_piy_real   = linear(curr_y_real,   ymin_, ymax_, height, 0),
                curr_piy_imag   = linear(curr_y_imag,   ymin_, ymax_, height, 0);

            int min_cache_index = (int)linear(std::fmax(xmin_,tmin_),tmin_,tmax_,0,density_) >= 0
                                ? (int)linear(std::fmax(xmin_,tmin_),tmin_,tmax_,0,density_)
                                : 0;

            int max_cache_index = (int)linear(std::fmin(xmax_,tmax_),tmin_,tmax_,0,density_) <= density_
                                ? (int)linear(std::fmin(xmax_,tmax_),tmin_,tmax_,0,density_)
                                : density_;

            for(int curr_t = min_cache_index; curr_t <= max_cache_index; curr_t++)
            {
                std::complex<double> new_x = p->poly_cache[curr_t];
                new_x *= std::conj(new_x);

                int new_pix         = linear(tmin_ + curr_t * (tmax_ - tmin_) / density_, xmin_, xmax_, 0, width),
                    new_piy_real    = linear(new_x.real(), ymin_, ymax_, height, 0),
                    new_piy_imag    = linear(new_x.imag(), ymin_, ymax_, height, 0);

                painter.drawLine(curr_pix,curr_piy_real,new_pix,new_piy_real);
                painter.drawLine(curr_pix,curr_piy_imag,new_pix,new_piy_imag);

                curr_pix = new_pix,
                curr_piy_real = new_piy_real,
                curr_piy_imag = new_piy_imag;

                painter.setPen(QColor::fromHsv(linear(curr_t, 0, density_, 0, 359), 255, 255));
            }

        }
        break;
    case MainWindow::Plotting::no_graph:
    default:{}
    }
    p->calc_mutex.unlock();
}
