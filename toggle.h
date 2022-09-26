#ifndef TOGGLE_H
#define TOGGLE_H

#include <QObject>
#include <QRadioButton>

class Toggle : public QObject
{
    Q_OBJECT
    int index ;
    bool isLeft_;
    void* parent_;
    QRadioButton* left_;
    QRadioButton* right_;
public:
    explicit Toggle(QObject *parent = nullptr);
    void setIndex(int new_index) {index = new_index;}
    int getIndex() const {return index;}
    bool isLeft() const;
    void setLeftRight(QRadioButton* left, QRadioButton* right);

signals:

public slots:
    void change(bool);

};

#endif // TOGGLE_H
