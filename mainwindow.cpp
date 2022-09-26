#include "mainwindow.h"
#include <thread>
#include <QKeyEvent>

#include "toggle.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mode_(Plotting::trajectory)

{
    ui->setupUi(this);

    defaultButtonPalette = ui->calcButton->palette();
    save_notes = false;
    setDefaultsT();
    setDefaultsX();
    setDefaultsY();
    setDefaultsDensity();
    setDefaultProdnumber();

    ui->actionTrajectoryMode->setChecked(true);
    ui->actionPlot_trig_polynome->setChecked(true);
    printMessage(tr("Добро пожаловать!"));

    fileopen = new QFileDialog(this);
    fileopen->setMimeTypeFilters({"application/json"});
    connect(fileopen, &QFileDialog::fileSelected, this, &MainWindow::openFile);

    filesave = new QFileDialog(this);
    filesave->setLabelText(QFileDialog::Accept,tr("Сохранить"));
    filesave->setMimeTypeFilters({"application/json"});
    connect(filesave, &QFileDialog::fileSelected, this, &MainWindow::saveFile);


    performCalculations();

    makeCalcButtonGray();
}


void MainWindow::printMessage(const QString &message)
{
    ui->statusBar->showMessage(message);
}

void MainWindow::printResults(QString message)
{
    ui->notes->setPlainText(message);
}

MainWindow::Plotting MainWindow::mode() const
{
    return mode_;
}

//
//Mathematical Part
//

std::complex<double> MainWindow::evalPolyAt(double x) const
{
    //while(x > 3.1415926535)
    //    x -= 2*3.1415926535;
    //while(x < -3.1415926535)
    //    x += 2*3.1415926535;

    auto divisor    = std::complex<double>(1,0);
    auto z          = std::complex<double>(cos(x),sin(x));
    auto result     = std::complex<double>(1,0);
    const auto unit       = std::complex<double>(1,0);

    for(auto i : roots_)
       result *= (z - i.first),
       divisor *= (unit - i.first);

    return result / divisor;
}

std::complex<double> MainWindow::evalPhiCapAt(double x) const
{
    std::complex<double> result = std::complex<double>(1,0);

    for(size_t i = 0; i < prodNumber; ++i)
        result *= evalPolyAt(x/2.0), x /= 2;

    return result;
}

std::complex<double> MainWindow::evalPolyPrimeAt(double x) const
{
    auto unit   = std::complex<double>(1.0,0);
    auto result = std::complex<double>(0,0);
    auto exp_ix = std::complex<double>(cos(x),sin(x));

    for(auto z : roots_)
        result += ( unit + z.first / (exp_ix - z.first) );

    result *= std::complex<double>(0,1.0);

    result *= evalPolyAt(x);

    return result;
}

std::complex<double> MainWindow::evalPhiAt(double x) const
{
    auto result = std::complex<double>(0,0);

    for(auto t = 0ul; t <= density; t++)

        result += evalPhiCapAt(linear(t,0,density,tmin,tmax))
                * std::complex<double>(cos(x * linear(t,0,density,tmin,tmax)),sin(x * linear(t,0,density,tmin,tmax)));

    return result;
}

const std::vector< std::pair< std::complex<double>, QObject* > > &MainWindow::roots() const
{
    return roots_;
}

void MainWindow::performCalculations() {


    if(calc_mutex.try_lock()==false)
    {
        printMessage(tr("Вычисления уже выполняются"));
        return;
    }
    auto start = std::chrono::system_clock::now();
    tmin = ui->drawing->tmin();
    tmax = ui->drawing->tmax();
    density = ui->drawing->density();

    printMessage(tr("Выполняются вычисления"));

    prod_numb = prodNumber;
    poly_cache.resize(density + 1);
    phiCap_cache.resize(density + 1);

    for (size_t i = 0; i < density + 1; i++) {
        auto temp = linear(i, 0, density, tmin, tmax);
        poly_cache[i] = evalPolyAt(temp);
        phiCap_cache[i] = evalPhiCapAt(temp);
    }

    printMessage(tr("Вычисления выполнены"));
    auto end = std::chrono::system_clock::now();

    time_amount = ((std::chrono::duration<double>)(end - start)).count();

    calc_mutex.unlock();
    makeCalcButtonGray();
}

//
//Defaults Setters
//

void MainWindow::setDefaultsT()
{
    ui->tmin->setText(QString::number(0)), tmin = 0;
    ui->tmax->setText(QString::number(2*3.1415926535)), tmax = 2*3.1415926535;
    ui->drawing->setTmin(0);
    ui->drawing->setTmax(2*3.1415926535);
}

void MainWindow::setDefaultsX()
{
    ui->xmin->setText(QString::number(-15));
    ui->xmax->setText(QString::number(15));
    ui->drawing->setXmin(-15);
    ui->drawing->setXmax(15);
}

void MainWindow::setDefaultsY()
{

    ui->ymin->setText(QString::number(-10));
    ui->ymax->setText(QString::number(10));
    ui->drawing->setYmin(-10);
    ui->drawing->setYmax(10);
}

void MainWindow::setDefaultsDensity()
{
    ui->drawing->setDensity(1000);
    density = 1000;
    ui->density->setText(QString::number(1000));
}

void MainWindow::setDefaultProdnumber()
{
    prodNumber = 30;
    ui->prodNumber->setText(QString::number(30));
}



MainWindow::~MainWindow()
{
    delete fileopen;
    delete filesave;
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    switch(ev->key())
    {
    case Qt::Key_F11:
    {
        if(this->isFullScreen())
            this->showNormal();
        else
            this->showFullScreen();
        ui->menuBar->update();
        break;
    }
    default:
    {

    }
    }
}

void MainWindow::makeCalcButtonBlue()
{
    QPalette pal = ui->calcButton->palette();
    pal.setColor(QPalette::Button, QColor(Qt::blue));
    ui->calcButton->setAutoFillBackground(true);
    ui->calcButton->setPalette(pal);
    ui->calcButton->update();

    need_recalc = true;
}

void MainWindow::makeCalcButtonGray()
{
    need_recalc = false;

    ui->calcButton->setPalette(defaultButtonPalette);

    ui->calcButton->update();
}

void MainWindow::makeWidthButtonBlue()
{
    QPalette pal = ui->calcWidthButton->palette();
    pal.setColor(QPalette::Button, QColor(Qt::blue));
    ui->calcWidthButton->setAutoFillBackground(true);
    ui->calcWidthButton->setPalette(pal);
    ui->calcWidthButton->update();
}

void MainWindow::makeWidthButtonGray()
{
    ui->calcWidthButton->setPalette(defaultButtonPalette);

    ui->calcWidthButton->update();
}

void MainWindow::checkBoundaries()
{
    if(ui->drawing->tmin() >= ui->drawing->tmax())
    {
        printMessage(tr("Ошибка, нижняя граница по оси времени больше верхней, установлены значения по умолчанию"));
        setDefaultsT();
    }

    if(ui->drawing->xmin() >= ui->drawing->xmax())
    {
        printMessage(tr("Ошибка, нижняя граница по оси абсцисс больше верхней, установлены значения по умолчанию"));
        setDefaultsX();
    }

    if(ui->drawing->ymin() >= ui->drawing->ymax())
    {
        printMessage(tr("Ошибка, нижняя граница по оси абсцисс больше верхней, установлены значения по умолчанию"));
        setDefaultsY();
    }

    if(ui->drawing->density() <= 0)
    {
        printMessage(tr("Ошибка, мощность множества точек дробления меньше 0, установлены значения по умолчанию"));
        setDefaultsDensity();
    }
}

void MainWindow::blockButtons()
{
    ui->rootsTable->setDisabled(true);
    ui->addRoots->setDisabled(true);
    ui->deleteRoots->setDisabled(true);

    ui->tmin->setDisabled(true);
    ui->tmax->setDisabled(true);

    ui->density->setDisabled(true);
    ui->prodNumber->setDisabled(true);

    ui->calcButton->setDisabled(true);
}

void MainWindow::unlockButtons()
{
    ui->rootsTable->setEnabled(true);
    ui->rootsTable->setPalette(defaultButtonPalette);

    ui->addRoots->setEnabled(true);
    ui->addRoots->setPalette(defaultButtonPalette);

    ui->deleteRoots->setEnabled(true);
    ui->deleteRoots->setDisabled(false);

    ui->tmin->setEnabled(true);
    ui->tmax->setEnabled(true);

    ui->density->setEnabled(true);
    ui->prodNumber->setEnabled(true);

    ui->calcButton->setEnabled(true);
}


//
//Buttons Pressing
//

void MainWindow::on_addRoots_clicked()
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя добавить новый корень"));
        return;
    }

    printMessage(tr("Добавляются корни"));
    double re, im;
    bool cast = true;
    if(ui->inputRe->text().isEmpty())
        re = 0.0;
    else
        re = ui->inputRe->text().toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная действительная часть"));
        calc_mutex.unlock();
        return;
    }

    if(ui->inputIm->text().isEmpty())
        im = 0.0;
    else
        im = ui->inputIm->text().toDouble(&cast);

    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная мнимая часть"));
        calc_mutex.unlock();
        return;
    }
    else
    {
        printMessage(tr("Добавление пары корней в список..."));
    }

    addRow(re,im);

    calc_mutex.unlock();
}

void MainWindow::on_deleteRoots_clicked()
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя удалить корень"));
        return;
    }
    auto index = ui->rootsTable->currentRow();
    if(index == -1){
        calc_mutex.unlock();
        return;
    }
    printMessage(tr("Удаляются корни"));
    roots_.erase(roots_.begin()+index);
    ui->rootsTable->removeRow(index);
    printMessage(tr("Удален корень с индексом %1").arg(QString::number(index+1)));

    for(int i = 0; i < ui->rootsTable->rowCount(); ++i)
        dynamic_cast<Toggle*>(roots_[i].second)->setIndex(i);

    ui->rootsTable->setCurrentCell(-1,-1);

    makeCalcButtonBlue();

    calc_mutex.unlock();
}

void MainWindow::on_drawButton_clicked()
{
    checkBoundaries();
    printMessage(tr("Отрисовка..."));

    if(need_recalc)
    {
        if(calc_mutex.try_lock())
        {
            performCalculations();
            calc_mutex.unlock();
        }
        else
        {
            printMessage(tr("Вычисления уже выполняются, повторите отрисовку позднее"));
            return;
        }
    }
    need_recalc = false;


    ui->drawing->draw(mode()
                      , ui->actionPlot_trig_polynome->isChecked()
                      | (ui->actionPlot->isChecked() << 1)
                      | (ui->actionPlot_2->isChecked() << 2)
                      );
    printMessage(tr("Отрисовка закончена в режиме %1").arg(mode()));
}

void MainWindow::addRow(double re, double im)
{
    double module = re*re + im*im;

    if(module < 0.000001)
    {
        printMessage(tr("Ошибка, нельзя добавить корень 0"));
        return;
    }
    if(abs(module-1) + abs(re-1) < 0.000001)
    {
        printMessage(tr("Ошибка, нельзя добавить корень 1"));
        return;
    }


    roots_.push_back(std::make_pair(std::complex<double>(re, im), new Toggle(this)));

    auto index = ui->rootsTable->rowCount();
    ui->rootsTable->insertRow(index);
    auto temp = new QWidget(this);

    QHBoxLayout* hbl = new QHBoxLayout(temp);
    QLabel* label = new QLabel(temp);
            label->setText(QString::number(re));
    hbl->addWidget(label);
    temp->setLayout(hbl);
    ui->rootsTable->setCellWidget(index,0,temp);


    temp = new QWidget(this);
    hbl = new QHBoxLayout(temp);
    label = new QLabel(temp);
    label->setText(QString::number(im));
    hbl->addWidget(label);
    temp->setLayout(hbl);
    ui->rootsTable->setCellWidget(index,1,temp);

    temp = new QWidget(this);
    hbl = new QHBoxLayout(temp);
    auto left = new QRadioButton(temp);
    auto right = new QRadioButton(temp);
    label = new QLabel(tr("Л/П"),temp);
    hbl->addWidget(left);
    hbl->addWidget(label);
    hbl->addWidget(right);
    left->setChecked(true);
    auto togg = dynamic_cast<Toggle*>(roots_[index].second);
    togg->setIndex(index);
    connect(left, &QRadioButton::clicked/*&QRadioButton::toggled*/, togg, &Toggle::change);
    connect(right, &QRadioButton::clicked/*&QRadioButton::toggled*/, togg, &Toggle::change);
    togg->setLeftRight(left,right);

    ui->rootsTable->setCellWidget(index,2,temp);

    temp = new QWidget(this);
    hbl = new QHBoxLayout(temp);
    label = new QLabel(temp);
    label->setText(QString::number(re/module));
    hbl->addWidget(label);
    temp->setLayout(hbl);
    ui->rootsTable->setCellWidget(index,3,temp);

    temp = new QWidget(this);
    hbl = new QHBoxLayout(temp);
    label = new QLabel(temp);
    label->setText(QString::number(im/module));
    hbl->addWidget(label);
    temp->setLayout(hbl);
    ui->rootsTable->setCellWidget(index,4,temp);

    printMessage(tr("Корни (%1)+i(%2), (%3)+i(%4) успешно добавлены").arg(re).arg(im).arg(re/module).arg(im/module));
    makeCalcButtonBlue();
}

void MainWindow::on_calcWidthButton_clicked()
{
    if(!calc_mutex.try_lock())
    {
        printMessage(tr("Невозможно произвести рассчет, кэш все еще вычисляется"));
        return;
    }
    std::complex<double> center_of_phi = 0.0, center_for_div = 0.0;
    std::complex<double> weight_of_phi = 0.0, weight_for_div = 0.0;
    std::complex<double> abs_phi_prime_for_div = 0.0;

    double delta_t = (tmax - tmin) / density;

    //Integrating with Newton's Sums
    for(auto z = 1u; z < phiCap_cache.size(); z++)
        center_of_phi   += std::conj(phiCap_cache[z]) * (phiCap_cache[z]-phiCap_cache[z - 1]),
                center_for_div  += std::conj(phiCap_cache[z]) * (phiCap_cache[z]-phiCap_cache[z - 1]) / delta_t,

                weight_of_phi   += phiCap_cache[z] * std::conj(phiCap_cache[z]) * delta_t,
                weight_for_div  += phiCap_cache[z] * std::conj(phiCap_cache[z]),
                abs_phi_prime_for_div   += (phiCap_cache[z] - phiCap_cache[z-1]) * std::conj(phiCap_cache[z] - phiCap_cache[z-1])
                                    / (delta_t * delta_t);
    std::complex<double> center_of_phiCap   = 0.0;
    std::complex<double> width_of_phiCap    = 0.0;

    for(auto z = 1u; z < phiCap_cache.size(); z++)
        center_of_phiCap += std::conj(phiCap_cache[z]) * (phiCap_cache[z]-phiCap_cache[z - 1]) * linear(z,1,density,tmin,tmax);

    center_of_phiCap /= weight_for_div;

    for(auto z = 1u; z < phiCap_cache.size(); z++)
    {
        auto t = linear(z, 1, density, tmin, tmax);
        width_of_phiCap += std::conj(phiCap_cache[z]-phiCap_cache[z - 1]) * (phiCap_cache[z]-phiCap_cache[z - 1]) * (t - center_of_phiCap) * (t - center_of_phiCap);
    }

    width_of_phiCap /= weight_for_div;

    center_of_phi *= std::complex<double>(0.0, 1.0)     / weight_of_phi;

    auto center_without_division = center_for_div       / weight_for_div;
    auto width_without_division = abs_phi_prime_for_div / weight_for_div - center_of_phi * center_of_phi;

    center_without_division *= std::complex<double>(0.0, 1.0);

    auto UC = width_of_phiCap * width_without_division;

    QString output(tr("Результаты вычислений:\n"
                      "\tt_мин:   %1\n"
                      "\tt_макс:   %2\n"
                      "\tМощность отрезка дробления: %3\n"
                      "\tПлотность дробления: %4\n\n"
                      "\tНорма L2 фи: %5\n"
                      "\tЦентр фи:  %6 + i(%7)\n"
                      "\tШирина фи: %8 + i(%9)\n\n"
                      "\tНорма L2 фи: %10\n"
                      "\tЦентр фи-с-шапочкой:  %11 + i(%12)\n"
                      "\tШирина фи-с-шапочкой: %13 + i(%14)\n\n"
                      "\tUC = %15 + i(%16)\n\n"
                      "\tЗатраченное время: %17 секунд.\n\n"
                      )
                   .arg(tmin)
                   .arg(tmax)
                   .arg(phiCap_cache.size())
                   .arg((tmax-tmin)/density)
                   .arg(weight_of_phi.real())
                   .arg(center_of_phi.real())
                   .arg(center_of_phi.imag())
                   .arg(width_without_division.real())
                   .arg(width_without_division.imag())
                   .arg(weight_of_phi.real())
                   .arg(center_of_phiCap.real())
                   .arg(center_of_phiCap.imag())
                   .arg(width_of_phiCap.real())
                   .arg(width_of_phiCap.imag())
                   .arg(UC.real())
                   .arg(UC.imag())
                   .arg(time_amount)
                   );
    printResults(output);
    makeWidthButtonGray();
    calc_mutex.unlock();
}

void MainWindow::on_calcButton_clicked()
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя начать новые"));
        return;
    }
    calc_mutex.unlock();

    std::thread([this](){

        printMessage(tr("Выполняются вычисления..."));

        performCalculations();

        makeWidthButtonBlue();
    }).detach();

}

//
//Text fields changing
//

void MainWindow::on_xmin_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить нижнюю границу по х"));
        return;
    }
    bool cast;
    double new_xmin = arg1.toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная нижняя граница по оси абсцисс"));
        calc_mutex.unlock();
        return;
    }
    ui->drawing->setXmin(new_xmin);
    printMessage(tr("Установлена новая нижняя граница по оси абсцисс: %1").arg(new_xmin));

    calc_mutex.unlock();
}

void MainWindow::on_xmax_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить верхнюю границу по х"));
        return;
    }
    bool cast;
    double new_xmax = arg1.toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная верхняя граница по оси абсцисс"));
        calc_mutex.unlock();
        return;
    }
    ui->drawing->setXmax(new_xmax);
    printMessage(tr("Установлена новая верхняя граница по оси абсцисс: %1").arg(new_xmax));

    calc_mutex.unlock();
}



void MainWindow::on_ymin_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить нижнюю границу по у"));
        return;
    }
    bool cast;
    double new_ymin = arg1.toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная нижняя граница по оси ординат"));
        calc_mutex.unlock();
        return;
    }
    ui->drawing->setYmin(new_ymin);
    printMessage(tr("Установлена новая нижняя граница по оси ординат: %1").arg(new_ymin));

    calc_mutex.unlock();
}

void MainWindow::on_ymax_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить верхнюю границу по у"));
        return;
    }
    ui->calcButton->setForegroundRole(QPalette::Highlight);
    bool cast;
    double new_ymax = arg1.toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная верхняя граница по оси ординат"));
        calc_mutex.unlock();
        return;
    }
    ui->drawing->setYmax(new_ymax);
    printMessage(tr("Установлена новая верхняя граница по оси ординат: %1").arg(new_ymax));

    calc_mutex.unlock();
}



void MainWindow::on_tmin_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить нижнюю границу по t"));
        return;
    }
    bool cast;
    double new_tmin = arg1.toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная нижняя граница по времени"));
        calc_mutex.unlock();
        return;
    }
    ui->drawing->setTmin(new_tmin);
    printMessage(tr("Установлена новая нижняя граница по времени: %1").arg(new_tmin));
    makeCalcButtonBlue();

    calc_mutex.unlock();
}

void MainWindow::on_tmax_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить верхнюю границу по t"));
        return;
    }
    ui->calcButton->setForegroundRole(QPalette::Highlight);
    bool cast;
    double new_tmax = arg1.toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная верхняя граница по оси времени"));
        calc_mutex.unlock();
        return;
    }
    ui->drawing->setTmax(new_tmax);
    printMessage(tr("Установлена новая верхняя граница по времени: %1").arg(new_tmax));
    makeCalcButtonBlue();

    calc_mutex.unlock();
}



void MainWindow::on_density_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить плотность разбиения отрезка времени"));
        return;
    }
    bool cast;
    double new_dens = arg1.toDouble(&cast);
    if(!cast)
    {
        printMessage(tr("Ошибка, некорректная мощность множества дробления временного отрезка"));
        calc_mutex.unlock();
        return;
    }
    ui->drawing->setDensity(new_dens);
    printMessage(tr("Установлена новая мощность множества дробления временного отрезка: %1").arg(new_dens));
    makeCalcButtonBlue();

    calc_mutex.unlock();
}

void MainWindow::on_prodNumber_textChanged(const QString &arg1)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя изменить мощность произведения"));
        return;
    }
    bool cast;
    double new_prodNumber = arg1.toDouble(&cast);
    if(!cast || new_prodNumber < 1)
    {
        printMessage(tr("Ошибка, некорректная нижняя граница по времени"));
        calc_mutex.unlock();
        return;
    }
    prodNumber = new_prodNumber;
    printMessage(tr("Установлена новая верхняя гранца произведения: %1").arg(new_prodNumber));
    makeCalcButtonBlue();

    calc_mutex.unlock();
}


//
//Menu Actions Triggering
//

void MainWindow::on_actionFunctionMode_changed()
{
    if(!ui->actionFunctionMode->isChecked())
        return;
    ui->actionTrajectoryMode->setChecked(false);
    ui->actionFunctionMode->setChecked(true);
    mode_ = Plotting::function;
}

void MainWindow::on_actionTrajectoryMode_changed()
{
    if(!ui->actionTrajectoryMode->isChecked())
        return;
    ui->actionFunctionMode->setChecked(false);
    ui->actionTrajectoryMode->setChecked(true);
    mode_ = Plotting::trajectory;
}

void MainWindow::on_actionOpen_triggered()
{
    if(calc_mutex.try_lock()==false)
    {
        printMessage(tr("Вычисления все еще выполняются, невозможно открыть файл"));
        return;
    }
    printMessage(tr("opening file"));
    fileopen->show();
    calc_mutex.unlock();
}

void MainWindow::on_actionSave_triggered()
{
    printMessage(tr("opening file"));
    filesave->show();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}



void MainWindow::on_rootsTable_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}



void MainWindow::openFile(const QString &fileName)
{
    if(!current_settings.exists(fileName))
    {
        printMessage(tr("Невозможно открыть файл: ") + fileName);
        return;
    }

    current_settings.setFileName(fileName);

    QFile temp(fileName, this);
    temp.open(QIODevice::ReadOnly);


    QJsonDocument doc = QJsonDocument::fromJson(temp.read(65536));

    if(doc.isNull())
    {
        printMessage(tr("Невозможно прочесть файл \"%1\"").arg(fileName));
        return;
    }

    roots_.clear();
    while(ui->rootsTable->rowCount()>0)
        ui->rootsTable->removeRow(0);
    update();

    auto json = doc.object();

    if(json.contains("roots") && json["roots"].isArray())
    {
        auto arr = json["roots"].toArray();
        for(auto r : arr)
            if(!r.isObject() || !r.toObject().contains("real") || !r.toObject().contains("imag"))
            {
                printMessage(tr("Некорректный формат файла"));
                continue;
            }
            else addRow(r.toObject()["real"].toDouble(), r.toObject()["imag"].toDouble());
    }
    else
    {
        printMessage(tr("Некорректный формат файла"));
        return;
    }

    if(json.contains("tmin") && json["tmin"].isDouble())
    {
        auto new_tmin = json["tmin"].toDouble();
        ui->drawing->setTmin(new_tmin);
        ui->tmin->setText(QString::number(new_tmin));
    }
    else
    {
        ui->drawing->setTmin(-10);
        ui->tmin->setText(QString::number(-10));
    }

    if(json.contains("tmax") && json["tmax"].isDouble())
    {
        auto new_tmax = json["tmax"].toDouble();
        ui->drawing->setTmax(new_tmax);
        ui->tmax->setText(QString::number(new_tmax));
    }
    else
    {
        ui->drawing->setTmax(10);
        ui->tmax->setText(QString::number(10));
    }

    if(json.contains("xmin") && json["xmin"].isDouble())
    {
        auto new_xmin = json["xmin"].toDouble();
        ui->drawing->setXmin(new_xmin);
        ui->xmin->setText(QString::number(new_xmin));
    }
    else
    {
        ui->drawing->setXmin(-15);
        ui->xmin->setText(QString::number(-15));
    }

    if(json.contains("xmax") && json["xmax"].isDouble())
    {
        auto new_xmax = json["xmax"].toDouble();
        ui->drawing->setXmax(new_xmax);
        ui->xmax->setText(QString::number(new_xmax));
    }
    else
    {
        ui->drawing->setXmax(15);
        ui->xmax->setText(QString::number(15));
    }

    if(json.contains("ymin") && json["ymin"].isDouble())
    {
        auto new_ymin = json["ymin"].toDouble();
        ui->drawing->setYmin(new_ymin);
        ui->ymin->setText(QString::number(new_ymin));
    }
    else
    {
        ui->drawing->setYmin(-10);
        ui->ymin->setText(QString::number(10));
    }

    if(json.contains("ymax") && json["ymax"].isDouble())
    {
        auto new_ymax = json["ymax"].toDouble();
        ui->drawing->setYmax(new_ymax);
        ui->ymax->setText(QString::number(new_ymax));
    }
    else
    {
        ui->drawing->setYmax(10);
        ui->ymax->setText(QString::number(10));
    }

    if(json.contains("density") && json["density"].isDouble())
    {
        auto new_dens = json["density"].toDouble();
        ui->drawing->setDensity(new_dens);
        ui->density->setText(QString::number(new_dens));
    }
    else
    {
        ui->drawing->setDensity(1000);
        ui->density->setText(QString::number(1000));
    }

    if(json.contains("prod_numb") && json["prod_numb"].isDouble())
    {
        auto new_prod = json["prod_numb"].toDouble();
        prodNumber = new_prod;
        ui->prodNumber->setText(QString::number(new_prod));
    }
    else
    {
        prodNumber = 30;
        ui->prodNumber->setText(QString::number(30));
    }

    if(json.contains("notes") && json["notes"].isString())
    {
        auto new_notes = json["notes"].toString();
        ui->notes->setText(new_notes);
    }
    else
    {
        ui->notes->setText(tr(""));
        save_notes = false;
    }
    makeCalcButtonBlue();
}

void MainWindow::saveFile(const QString &fileName)
{
    if(calc_mutex.try_lock() == false)
    {
        printMessage(tr("Вычисления еще выполняются, нельзя сохранить имеющиеся параметры"));
        return;
    }
    QJsonObject json_file;
    QJsonArray json_roots;
    QJsonObject json_root;

    for(auto r : roots_)
        json_root["real"] = (r.first.real()),
        json_root["imag"] = (r.first.imag()),
        json_roots.append(json_root);
    json_file["roots"] = json_roots;

    json_file["tmin"] = ui->tmin->text().toDouble();
    json_file["tmax"] = ui->tmax->text().toDouble();

    json_file["xmin"] = ui->xmin->text().toDouble();
    json_file["xmax"] = ui->xmax->text().toDouble();

    json_file["ymin"] = ui->ymin->text().toDouble();
    json_file["ymax"] = ui->ymax->text().toDouble();

    json_file["density"] = ui->density->text().toInt();
    json_file["prod_numb"] = ui->prodNumber->text().toInt();
    if(save_notes) json_file["notes"] = ui->notes->toPlainText();

    QJsonDocument doc(json_file);
    QFile file(fileName + tr(".json"), this);

    if(file.open(QIODevice::WriteOnly))
        file.write(doc.toJson());
    else
        printMessage(tr("Невозможно открыть файл \"%1\" для записи").arg(fileName));
    calc_mutex.unlock();
}




double linear(double val, double a_old, double b_old, double a_new, double b_new)
{
    return a_new + (val - a_old) * (b_new - a_new) / (b_old - a_old);
}

void MainWindow::on_actionInfo_triggered()
{
    QString information =
            tr("Программа предназначена для грубого вычисления центра "
               "и ширины функции фи-с-шапочкой. В таблицу вводятся корни "
               "тригонометрического многочлена, после чего на данном отрезке "
               "[t_мин,t_макс] с заданной мелкостью дробления считается функция "
               "фи-с-шапочкой, которая кэшируется в оперативную память. После чего, "
               "по нажатию кнопки \"Нарисовать\" производится отрисовка графика функции "
               "в соответствующем режиме. Для удобства восприятия, график рисуется градиентным. "
               "Кнопка \"Посчитать центр и ширину\" производит подсчёты, соответственно, центра и ширины.");
    printResults(information);
}

