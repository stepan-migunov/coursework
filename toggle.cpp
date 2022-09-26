#include "toggle.h"
#include "mainwindow.h"
#include <complex>

Toggle::Toggle(QObject *parent)
    : QObject{parent}
{
    isLeft_ = true;
}

bool Toggle::isLeft() const
{
    return isLeft_;
}

void Toggle::setLeftRight(QRadioButton *left, QRadioButton *right)
{
    left_ = left;
    right_ = right;
}

void Toggle::change(bool checked)
{
    auto p_ = parent();
    while(p_->parent() != nullptr)
        p_ = dynamic_cast<QObject*>(p_->parent());

    auto p = dynamic_cast<MainWindow*>(p_);

    if(p->calc_mutex.try_lock()==false)
    {
        p->printMessage(tr("Вычисления всё ещё выполняются, невозможно изменить корень"));
        if(isLeft_)
            left_->setChecked(true);
        else
            right_->setChecked(true);
        return;
    }

    auto re = p->roots_[index].first.real();
    auto im = p->roots_[index].first.imag();


    auto module = re * re + im * im;
    if(checked)
        p->roots_[index].first = std::complex<double>(re/module, im/module),
        p->printMessage(tr("В качестве %1-го кореня выбрано число (%2)+i(%3)").arg(index+1).arg(re/module).arg(im/module)),
        isLeft_ = !isLeft_,
        p->makeCalcButtonBlue();
    p->calc_mutex.unlock();
}
