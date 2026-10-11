#pragma once
#include <QWidget>

namespace Ui { class Livraisons; }

class Livraisons : public QWidget
{
    Q_OBJECT
public:
    explicit Livraisons(QWidget *parent = nullptr);
    ~Livraisons();

private:
    Ui::Livraisons *ui;
};
