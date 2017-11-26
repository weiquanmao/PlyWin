#ifndef _PANNEL_SETTING_H_FILE_
#define _PANNEL_SETTING_H_FILE_

#include <QDockWidget>

class QDoubleSpinBox;
class QSlider;

class PanelSetting : public QDockWidget
{
    Q_OBJECT

public:
    PanelSetting(QWidget *parent = 0);
    ~PanelSetting();

    void setParams(float *rot, float *tra, float scale, int alpha);
    void getParams(float *rot, float *tra, float *scale, int *alpha);
private:
    QDoubleSpinBox *fSpin_RX;
    QDoubleSpinBox *fSpin_RY;
    QDoubleSpinBox *fSpin_RZ;
    QDoubleSpinBox *fSpin_TX;
    QDoubleSpinBox *fSpin_TY;
    QDoubleSpinBox *fSpin_TZ;
    QDoubleSpinBox *fSpin_S;
    QSlider *Slider_Alpah;
    bool bUpdateByMe;
private:
    void setupme();
private slots:
    void sendSignal();
signals:
    void signalUpdate();
};



#endif // !_PANNEL_SETTING_H_FILE_
