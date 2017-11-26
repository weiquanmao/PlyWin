#include "pannelsetting.h"

#include <QLabel>
#include <QDoubleSpinBox>
#include <QSLider>
#include <QGridLayout>
PanelSetting::PanelSetting(QWidget *parent)
    : QDockWidget(parent)
{
    setupme();
    connect(fSpin_RX, SIGNAL(valueChanged(double)), this, SLOT(sendSignal()));
    connect(fSpin_RY, SIGNAL(valueChanged(double)), this, SLOT(sendSignal()));
    connect(fSpin_RZ, SIGNAL(valueChanged(double)), this, SLOT(sendSignal()));
    connect(fSpin_TX, SIGNAL(valueChanged(double)), this, SLOT(sendSignal()));
    connect(fSpin_TY, SIGNAL(valueChanged(double)), this, SLOT(sendSignal()));
    connect(fSpin_TZ, SIGNAL(valueChanged(double)), this, SLOT(sendSignal()));
    connect(fSpin_S, SIGNAL(valueChanged(double)), this, SLOT(sendSignal()));
    connect(Slider_Alpah, SIGNAL(valueChanged(int)), this, SLOT(sendSignal()));
}
PanelSetting::~PanelSetting()
{
}
void PanelSetting::setupme()
{
    QWidget* lTitleBar = titleBarWidget();
    QWidget* lEmptyWidget = new QWidget();
    setTitleBarWidget(lEmptyWidget);
    delete lTitleBar;

    QWidget *_container = new QWidget();

    QLabel *labetR = new QLabel(QString::fromLocal8Bit("旋转:"));
    labetR->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    fSpin_RX = new QDoubleSpinBox();
    fSpin_RY = new QDoubleSpinBox();
    fSpin_RZ = new QDoubleSpinBox();
    fSpin_RX->setMinimum(-360);
    fSpin_RX->setMaximum(360);
    fSpin_RX->setDecimals(8);
    fSpin_RY->setMinimum(-360);
    fSpin_RY->setMaximum(360);
    fSpin_RY->setDecimals(8);
    fSpin_RZ->setMinimum(-360);
    fSpin_RZ->setMaximum(360);
    fSpin_RZ->setDecimals(8);
    QLabel *labetT = new QLabel(QString::fromLocal8Bit("平移:"));
    labetT->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    fSpin_TX = new QDoubleSpinBox();
    fSpin_TY = new QDoubleSpinBox();
    fSpin_TZ = new QDoubleSpinBox();
    fSpin_TX->setMinimum(-1e9);
    fSpin_TX->setMaximum(1e9);
    fSpin_TX->setDecimals(8);
    fSpin_TY->setMinimum(-1e9);
    fSpin_TY->setMaximum(1e9);
    fSpin_TY->setDecimals(8);
    fSpin_TZ->setMinimum(-1e9);
    fSpin_TZ->setMaximum(1e9);
    fSpin_TZ->setDecimals(8);
    QLabel *labetS = new QLabel(QString::fromLocal8Bit("缩放:"));
    labetS->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    fSpin_S = new QDoubleSpinBox();
    QLabel *labelAlpha = new QLabel(QString::fromLocal8Bit("透明度:"));
    labelAlpha->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    Slider_Alpah = new QSlider(Qt::Horizontal);
    Slider_Alpah->setRange(0, 10);
    Slider_Alpah->setPageStep(1);

    QGridLayout *lay = new QGridLayout();
    lay->addWidget(labetR, 0, 0);
    lay->addWidget(fSpin_RX, 0, 1, 1, 2);
    lay->addWidget(fSpin_RY, 0, 3, 1, 2);
    lay->addWidget(fSpin_RZ, 0, 5, 1, 2);
    lay->addWidget(labetT, 1, 0);
    lay->addWidget(fSpin_TX, 1, 1, 1, 2);
    lay->addWidget(fSpin_TY, 1, 3, 1, 2);
    lay->addWidget(fSpin_TZ, 1, 5, 1, 2);
    lay->addWidget(labetS, 2, 0);
    lay->addWidget(fSpin_S, 2, 1, 1, 2);
    lay->addWidget(labelAlpha, 2, 3);
    lay->addWidget(Slider_Alpah, 2, 4, 1, 3);

    lay->setColumnStretch(0, 0);
    lay->setColumnStretch(1, 1);
    lay->setColumnStretch(2, 1);
    lay->setColumnStretch(3, 0);
    lay->setColumnStretch(4, 1);
    lay->setColumnStretch(5, 1);
    lay->setColumnStretch(6, 1);

    _container->setLayout(lay);
    setWidget(_container);

    setMaximumHeight(sizeHint().height());
}
void PanelSetting::sendSignal()
{
    if (!bUpdateByMe)
        emit signalUpdate();
}
void PanelSetting::setParams(float *rot, float *tra, float scale, int alpha)
{
    bUpdateByMe = true;
    fSpin_RX->setValue(rot[0] * 180 / 3.141592635);
    fSpin_RY->setValue(rot[1] * 180 / 3.141592635);
    fSpin_RZ->setValue(rot[2] * 180 / 3.141592635);
    fSpin_TX->setValue(tra[0]);
    fSpin_TY->setValue(tra[1]);
    fSpin_TZ->setValue(tra[2]);
    fSpin_S->setValue(scale);
    Slider_Alpah->setValue(alpha);
    bUpdateByMe = false;
}
void PanelSetting::getParams(float *rot, float *tra, float *scale, int *alpha)
{
    rot[0] = fSpin_RX->value() / 180.0 * 3.141592635;
    rot[1] = fSpin_RY->value() / 180.0 * 3.141592635;
    rot[2] = fSpin_RZ->value() / 180.0 * 3.141592635;
    tra[0] = fSpin_TX->value();
    tra[1] = fSpin_TY->value();
    tra[2] = fSpin_TZ->value();
    *scale = fSpin_S->value();
    *alpha = Slider_Alpah->value();
}