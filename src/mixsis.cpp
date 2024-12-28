#include <cstring>
#include <memory>
#include <cstdio>
#include <QtCore>
#include <poll.h>
#include "mixsis.h"
#include "mainwindow.h"

MixSis::MixSis(MixSisCtrl *ctrls, const char* device, QObject *obj) : mixerCtrl(ctrls), parent(obj)
{
    int err;
    err = snd_hctl_open(&alsa_hctl, device, 0);
    if(err){
        printf("error: unable to access device %s. error %s\n", device, strerror(err));
        exit(err);
    }
    err = snd_hctl_load(alsa_hctl);
    if(err){
        printf("error: unable to load mixer hctl. error%d\n", err);
        exit(err);
    }
    // boot up alsa interface
    snd_ctl_open(&ctl, device, 0);

    // populate get_numid table
    {
        snd_ctl_elem_list_t *list;
        int count;

        snd_ctl_elem_list_malloc(&list);

        snd_ctl_elem_list(ctl, list);

        count = snd_ctl_elem_list_get_count(list);

        snd_ctl_elem_list_alloc_space(list, count);

        snd_ctl_elem_list(ctl, list);

        unsigned int n_elems = snd_ctl_elem_list_get_used(list);

        for(unsigned int i = 0; i < n_elems; ++i){
            unsigned int numid = snd_ctl_elem_list_get_numid(list, i);
            QString elem_name = QString(snd_ctl_elem_list_get_name(list, i));

            mixerCtrl->get_numid[elem_name] = numid;
        }


        snd_ctl_elem_list_free_space(list);
        snd_ctl_elem_list_free(list);
    }

    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);

    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_info_set_numid(info, mixerCtrl->get_numid[USB_SYNC]);
    snd_ctl_elem_info(ctl, info);

    // check whether this is a 6i6
    if(mixerCtrl->get_numid["Scarlett 6i6 USB-Sync Clock Source"] == 0){
        printf("your card at %s (or its driver) is not configured in the expected manner:\n"
               "could not find control matching string 'Scarlett 6i6 USB-Sync'\n", device);
        exit(1);
    }
    if(SND_LIB_VERSION < 0x010204){
        printf("This version of qsismixer has not been designed to run on alsa older than 1.2.4\n"
                "However, it was compiled against alsa version %.5s\n"
                "It is recommended to use an older version of qsismixer, compatible with older versions of alsa\n",
                SND_LIB_VERSION_STR);
    }

    // now that we know it's a scarlett, first, get all the values and set them
    snd_ctl_elem_value_t *value;
    snd_ctl_elem_value_alloca(&value);

    // need hctl elem for great justice
    snd_hctl_elem_t *helem;
    bool trackingp = true;

    // setup volume controls
    int minVol = 0, maxVol = 100;
    // min/max slider values
    for(int k=0;k<18;++k){
        for(int l=0;l<8;++l){
            ctrls->mtx_vol[k][l]->setMinimum((int)minVol);
            ctrls->mtx_vol[k][l]->setMaximum((int)maxVol);
            ctrls->mtx_vol[k][l]->setTracking(trackingp);
        }
        if(k>=6) continue; // only 6 vol_out[k]
        ctrls->vol_out[k]->setMinimum((int)minVol);
        ctrls->vol_out[k]->setMaximum((int)maxVol);
        ctrls->vol_out[k]->setTracking(trackingp);
        if(k>=2) continue; // only 2 vol_master[k]
        ctrls->vol_master[k]->setMinimum((int)minVol);
        ctrls->vol_master[k]->setMaximum((int)maxVol);
        ctrls->vol_master[k]->setTracking(trackingp);
    }
    // get control values from alsa and set widgets accordingly
    for(int i = mixerCtrl->get_numid[MSTR_SWITCH]; i <= mixerCtrl->get_numid[INPUT_ROUTE_6]; ++i){
        /// this seems redundant but is necessary due to an alsa bug
        snd_ctl_elem_id_set_numid(id, i);
        snd_ctl_elem_info_set_id(info, id);
        snd_ctl_elem_info(ctl, info);
        snd_ctl_elem_info_get_id(info, id);

        helem = snd_hctl_find_elem(alsa_hctl, id);
        snd_hctl_elem_info(helem, info);
        snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);
        snd_ctl_elem_value_set_numid(value, i);
        snd_hctl_elem_read(helem, value);

        int count = snd_ctl_elem_info_get_count(info);
        int val, raw;
        bool isVolume = mixerCtrl->numidIsVolume(i);
        for(int idx = 0; idx < count; ++idx){
            if(type == SND_CTL_ELEM_TYPE_INTEGER){
                raw = val = snd_ctl_elem_value_get_integer(value,idx);
                if(isVolume){
                    val = dB_from_volume(val, i, ctl);
                }
            }
            else if(type == SND_CTL_ELEM_TYPE_BOOLEAN){
                raw = val = snd_ctl_elem_value_get_boolean(value,idx);
            }
            else if(type == SND_CTL_ELEM_TYPE_ENUMERATED){
                raw = val = snd_ctl_elem_value_get_enumerated(value,idx);
            }
            else{
                fprintf(stderr, "invalid index type, i=%d, idx=%d\n", i, idx);
                continue;
            }
            ctrls->set(i,val,idx);
            // to thwart a bug where faithfully reported values aren't actually reflected in hardware, turn the control off and on like a light switch
            this->set(i, !raw, idx);
            this->set(i, raw, idx);
        }

    }
    // set volume link toggle intelligently -- based on whether l/r channels are set to the same volume
    for(int i=0; i<3; ++i){
        ctrls->vol_out_link[i]->setChecked(ctrls->vol_out[2*i]->value() == ctrls->vol_out[2*i+1]->value());
    }
    // hook up callback lambdas
    obj->connect(ctrls->vol_master[0], &QSlider::valueChanged,
            [=](){
        int sixiVol = volume_from_dB(ctrls->vol_master[0]->value(),mixerCtrl->get_numid[MSTR_VOL], ctl);
        this->set(mixerCtrl->get_numid[MSTR_VOL], sixiVol);
        bool blocked = ctrls->vol_master[0]->blockSignals(true);
        ctrls->vol_master[1]->setValue(ctrls->vol_master[0]->value());
        ctrls->vol_master[0]->blockSignals(blocked);
    });
    obj->connect(ctrls->vol_master[1], &QSlider::valueChanged,
            [=](){
        int sixiVol = volume_from_dB(ctrls->vol_master[1]->value(),mixerCtrl->get_numid[MSTR_VOL], ctl);
        this->set(mixerCtrl->get_numid[MSTR_VOL], sixiVol);
        bool blocked = ctrls->vol_master[1]->blockSignals(true);
        ctrls->vol_master[0]->setValue(ctrls->vol_master[1]->value());
        ctrls->vol_master[1]->blockSignals(blocked);
    });
    obj->connect(ctrls->vol_master_mute, &QCheckBox::stateChanged,
            [=](int checkstate){
        this->set(mixerCtrl->get_numid[MSTR_SWITCH], !(checkstate == Qt::Checked), 0);
    });
    int idx;
    // sanity: pass by value lambda statements let the if statements work okay inside the lambda
    // open question, does the preprocessor simply evaluate them when dynamically generating code for execution or what?
    for(idx=0; idx<6; ++idx){
        // volume out controls 1-6
        obj->connect(ctrls->vol_out[idx], &QSlider::valueChanged,
                [=](){
            int volume = ctrls->vol_out[idx]->value();
            int which_control = idx/2;
            int other_idx = (idx & 1) ? idx - 1 : idx + 1; // if idx = 4, other_idx = 5, and vice versa
            if(ctrls->vol_out_link[which_control]->isChecked()){
                if(ctrls->vol_out[other_idx]->value() != volume){
                    // this primitive mutex stops linked stereo volumes from setting each other back and forth simultaneously on mouse drags
                    bool old = ctrls->vol_out[idx]->blockSignals(true);
                    ctrls->vol_out[other_idx]->setValue(volume);
                    ctrls->vol_out[idx]->blockSignals(old);
                }
            }
            int control_id;
            if(which_control == 0){
                control_id = mixerCtrl->get_numid[OUT_VOL_12];
            }
            else if(which_control == 1){
                control_id = mixerCtrl->get_numid[OUT_VOL_34];
            }
            else if(which_control == 2){
                control_id = mixerCtrl->get_numid[OUT_VOL_56];
            }
            else return; // this will never happen
#ifdef DEBUG
            fprintf(stderr, "volume ctrl %d\n",n);
#endif
            int value = volume_from_dB(volume, control_id, ctl);
            this->set(control_id, value, idx&1);

        });
        /// these next two event handlers stop changes in alsa from feeding back with changes in qsismix (by blocking them until slider is released)
        obj->connect(ctrls->vol_out[idx], &QSlider::sliderPressed,
                     [=](){
            ((MainWindow*)parent)->setChangeWatcherMask(idx/2 | MASK_VOL, true);
        });
        obj->connect(ctrls->vol_out[idx], &QSlider::sliderReleased,
                     [=](){
            ((MainWindow*)parent)->setChangeWatcherMask(idx/2 | MASK_VOL, false);

        });
        obj->connect(ctrls->vol_out_mute[idx], &QCheckBox::stateChanged,
                [=](int checkstate){
            bool value = checkstate ? 1 : 0;
            int which_control = idx/2;
            int other_idx = (idx&1) ? idx-1 : idx+1;
#ifdef DEBUG
            fprintf(stderr, "signal: ctl: %d idx: %d val: %d\n", which_control, n&1, value);
#endif
            int control_id;
            if(which_control == 0){
                control_id = mixerCtrl->get_numid[OUT_SWITCH_12];
            }
            else if(which_control == 1){
                control_id = mixerCtrl->get_numid[OUT_SWITCH_34];
            }
            else if(which_control == 2){
                control_id = mixerCtrl->get_numid[OUT_SWITCH_56];
            }
            else return; // unreachable instruction

            ((MainWindow*)parent)->setChangeWatcherMask(idx/2 | MASK_MUTE, true);  // this stops feedback

            if(ctrls->vol_out_link[which_control]->isChecked()){
                if(ctrls->vol_out_mute[other_idx]->isChecked() != value){
                    bool old = ctrls->vol_out_mute[idx]->blockSignals(true);
                    ctrls->vol_out_mute[other_idx]->setChecked(value);
                    ctrls->vol_out_mute[idx]->blockSignals(old);
                }
            }
            this->set(control_id, !value, idx&1);
            // if the other idx is accepting signals, then it didn't call this idx
            if(! ctrls->vol_out_mute[other_idx]->signalsBlocked()){
                ((MainWindow*)parent)->setChangeWatcherMask(which_control | MASK_MUTE, false);
            }
        });
        obj->connect(ctrls->out_src[idx], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            int control_id;
            switch(idx){
            case 0:
                control_id = mixerCtrl->get_numid[OUT_1_SRC];
                break;
            case 1:
                control_id = mixerCtrl->get_numid[OUT_2_SRC];
                break;
            case 2:
                control_id = mixerCtrl->get_numid[OUT_3_SRC];
                break;
            case 3:
                control_id = mixerCtrl->get_numid[OUT_4_SRC];
                break;
            case 4:
                control_id = mixerCtrl->get_numid[OUT_5_SRC];
                break;
            case 5:
                control_id = mixerCtrl->get_numid[OUT_6_SRC];
                break;
            default:
                return; // unreachable
            }
            this->set(control_id, index, 0);
        });
        obj->connect(ctrls->in_src[idx], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                [=](int index){
            int control_id;
            switch(idx){
            case 0:
                control_id = mixerCtrl->get_numid[INPUT_ROUTE_1];
                break;
            case 1:
                control_id = mixerCtrl->get_numid[INPUT_ROUTE_2];
                break;
            case 2:
                control_id = mixerCtrl->get_numid[INPUT_ROUTE_3];
                break;
            case 3:
                control_id = mixerCtrl->get_numid[INPUT_ROUTE_4];
                break;
            case 4:
                control_id = mixerCtrl->get_numid[INPUT_ROUTE_5];
                break;
            case 5:
                control_id = mixerCtrl->get_numid[INPUT_ROUTE_6];
                break;
            default:
                return; // unreachable
            }
            this->set(control_id, index);
        });
    }
    for(idx=0; idx<8; idx+=2){

        obj->connect(ctrls->in_pad[idx], &QRadioButton::toggled,
                     [=](){
            bool value = ctrls->in_pad[idx+1]->isChecked();
            int control_id;
            switch(idx/2){
            case 0:
                control_id = mixerCtrl->get_numid[IN_PAD_1];
                break;
            case 1:
                control_id = mixerCtrl->get_numid[IN_PAD_2];
                break;
            case 2:
                control_id = mixerCtrl->get_numid[IN_PAD_3];
                break;
            case 3:
                control_id = mixerCtrl->get_numid[IN_PAD_4];
                break;
            default:
                return; // unreachable
            }
            this->set(control_id, value, 0);
        });
    }
    for(idx=0; idx<4; idx += 2){
        obj->connect(ctrls->in_imp[idx], &QRadioButton::toggled,
                     [=](){
           bool value = ctrls->in_imp[idx+1]->isChecked();
           int control_id;
           switch(idx/2){
           case 0:
               control_id = mixerCtrl->get_numid[IN_IMP_1];
               break;
           case 1:
               control_id = mixerCtrl->get_numid[IN_IMP_2];
               break;
           default:
               return; // unreachable
           }
           this->set(control_id, value, 0);
        });
    }
    int matrix_index = mixerCtrl->get_numid[MATRIX_ROUTE_1];
    for(idx=0;idx<18;++idx){
        obj->connect(ctrls->mtx_src[idx], static_cast< void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     [=](int index){
            this->set(matrix_index, index);
        });
        matrix_index += 1;
        for(int j=0; j<8; ++j){
            obj->connect(ctrls->mtx_vol[idx][j], &QSlider::valueChanged,
                         [=](){
                int volume = ctrls->mtx_vol[idx][j]->value();
                int value = volume_from_dB(volume, matrix_index, ctl);
                this->set(matrix_index, value);
            });
            matrix_index += 1;
        }


    }

    // now, set up the 'clear matrix' buttons
    // just set the controls; the controls will set the mixer itself
    for(idx=0;idx<18;++idx){
        obj->connect(ctrls->mtx_clear[idx], &QPushButton::clicked,
        [=](){
            for(int j=0;j<8;++j){
                ctrls->mtx_vol[idx][j]->setValue(0);
            }
            ctrls->mtx_src[idx]->setCurrentIndex(0);
        });
        ctrls->mtx_clear[idx]->setText(obj->tr("Clear"));
    }
}

MixSis::~MixSis(){

}

// converts to 6i6-internal volume from slider %
// this vaguely represents the way an audio taper pot scales volume
int MixSis::volume_from_dB(int value, int controlID, snd_ctl_t *control){
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);

    get_alsa_id(control, id, controlID);

    long mindB;
    snd_ctl_convert_to_dB(control, id, 0, &mindB);
    // algorithm: 0<x<=20 -> mindb<db<=-40, 20<x -> -40<db<=0
    if(value == 0) return 0;
    else if(value <= 20){
        long db = value * (-4000. - mindB)/20. + mindB;
        long volume;
        snd_ctl_convert_from_dB(control, id, db, &volume, 0);
#ifdef DEBUG
        fprintf(stderr, "#1 got six volume %ld from slider %d\n", volume, value);
#endif
        return volume;
    }
    else{
        long db = (value-20) * 4000./80. + -4000.;
        long volume;
        snd_ctl_convert_from_dB(control, id, db, &volume, 0);
#ifdef DEBUG
        fprintf(stderr, "#2 got six volume %ld from slider %d\n", volume, value);
#endif
        return volume;
    }
}

// converts to slider % from 6i6-internal volume
// this isn't 1:1 for some reason and causes problems.
int MixSis::dB_from_volume(int value, int controlID, snd_ctl_t *control){
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);

    get_alsa_id(control, id, controlID);

    // algorithm: mindB<dB<-40 -> 0<x<20, -40<dB<0+ -> 20<x<=100
    long mindB;
    snd_ctl_convert_to_dB(control, id, 0, &mindB);
    long dB;
    snd_ctl_convert_to_dB(control, id, value, &dB);
    if(dB<-4000){
        int slider_vol = (dB - mindB) * 20./(-4000. - mindB);
#ifdef DEBUG
        fprintf(stderr, "#1: got slider %d from six vol %d; db=%ld\n", slider_vol, value, dB);
#endif
        return slider_vol;
    }
    else{
        int slider_vol = (dB + 4000.) * 80./4000. + 20;
        if(slider_vol > 100) slider_vol = 100;
#ifdef DEBUG
        fprintf(stderr, "#2: got slider %d from six vol %d\n", slider_vol, value);
#endif
        return slider_vol;
    }
}

void MixSis::set(int Nid, int val, int idx){
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);

    get_alsa_id(ctl, id, Nid);
    snd_ctl_elem_id_set_numid(id, Nid);
    snd_ctl_elem_info_set_id(info, id);
    snd_ctl_elem_info(ctl, info);
    snd_ctl_elem_value_t *value;
    snd_ctl_elem_value_alloca(&value);

    snd_ctl_elem_value_set_id(value, id);
    snd_ctl_elem_read(ctl, value);
    snd_ctl_elem_type_t type = snd_ctl_elem_info_get_type(info);
    if(type == SND_CTL_ELEM_TYPE_BOOLEAN){
        snd_ctl_elem_value_set_boolean(value,idx, val);
    }
    else if(type == SND_CTL_ELEM_TYPE_INTEGER){
        snd_ctl_elem_value_set_integer(value,idx,val);
    }
    else if(type == SND_CTL_ELEM_TYPE_ENUMERATED){
        snd_ctl_elem_value_set_enumerated(value,idx,val);
    }
    else{
#ifdef DEBUG
        fprintf(stderr,"MixSis::set: wrong type %d numid %d \n",type, Nid);
#endif
    }
    snd_ctl_elem_write(ctl, value);
    return;
}

// gets valid snd_id_t corresponding to alsa_numid
// before calling: id must be allocated, and this->ctl must be open (should be open during lifetime of MixSis)
void MixSis::get_alsa_id(snd_ctl_t *ctl, snd_ctl_elem_id_t*& id, int numid){
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_info_alloca(&info);
    snd_ctl_elem_id_set_numid(id, numid);
    snd_ctl_elem_info_set_id(info, id);
    snd_ctl_elem_info(ctl, info);
    snd_ctl_elem_info_get_id(info, id);
}
