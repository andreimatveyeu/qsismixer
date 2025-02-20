#ifndef CHANGEWATCHER_H
#define CHANGEWATCHER_H

#include "qthread.h"
#include "alsa/asoundlib.h"
#include "mixsisctrl.h"

#define CLR_BIT(X,Y) ((X) &= ~(1 << (Y)))
#define SET_BIT(X,Y) ((X) |= (1 << (Y)))
#define CLR_MASK(X,Y) ((X) &= ~(Y))
#define SET_MASK(X,Y) ((X) |= (Y))

const int MASK_VOL = 0x80000;
const int MASK_MUTE = MASK_VOL << 1;
const int MASK_MTX_VOL = MASK_MUTE << 1;

class ChangeWatcher : public QThread{
Q_OBJECT
public:
    ChangeWatcher(snd_ctl_t *ctl, QObject *parent);
    void run();
    snd_ctl_t *ctl;

    public slots:

    void setMask(int num, int mask);

signals:
    void changeVal(int numid, int val, int idx);

private:
    bool isVolMasked[3];
    bool isMuteMasked[3];
    uint8_t isMatrixVolMasked[18];
    bool isNumidMasked(int numid);
};

#endif // CHANGEWATCHER_H

