#ifndef MIX6I6_H
#define MIX6I6_H

#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>

#include <alsa/asoundlib.h>

#include "mixsisctrl.h"
#include "changewatcher.h"


class MixSis
{
    MixSisCtrl *mixerCtrl;
    snd_hctl_t *alsa_hctl;
    QObject *parent;
    //long mindB, maxdB;
    snd_mixer_t *alsa_mixer;

    void set_vol_mute(int which, bool);

public:

    MixSis(MixSisCtrl* ctrls, const char *device, QObject *obj);
    ~MixSis();

    snd_ctl_t *ctl;

    void set(int, int, int = 0);
    static int dB_from_volume(int value, int controlID, snd_ctl_t *control);
    static int volume_from_dB(int value, int controlID, snd_ctl_t *control);
    static void get_alsa_id(snd_ctl_t *ctl, snd_ctl_elem_id_t*& id, int numid);

};

#endif // MIX6I6_H
