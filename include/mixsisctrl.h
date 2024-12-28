#ifndef MIXSISCTRL
#define MIXSISCTRL

#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QDataStream>
#include <QHash>

#include <alsa/version.h>
/*
// where alsa >= 1.1.2, the numid's start with USB_SYNC=5. where alsa < 1.1.2, they start with USB_SYNC=3. Either way, they sequentially increase in the same order.
// note that as a result, if you upgrade alsa from a version < 1.1.2 to a newer version, you must recompile qsismix so that it will be compatible.

int constexpr kUsbSync() {
    return (SND_LIB_VERSION >= 0x010102) ? (SND_LIB_VERSION >= 0x010107) ? 8 : 5 : 3;
}

enum alsa_numid {
  #ifdef NEW_ALSA_CTRL_SCHEME
        USB_SYNC = kUsbSync(), LINE4_SWITCH, MSTR_SWITCH, MSTR_VOL, OUT_SWITCH_12, OUT_VOL_12, OUT_1_SRC, OUT_2_SRC,
	#else
	USB_SYNC = kUsbSync(), MSTR_SWITCH, MSTR_VOL, OUT_SWITCH_12, OUT_VOL_12, OUT_1_SRC, OUT_2_SRC,
	#endif
        OUT_SWITCH_34, OUT_VOL_34, OUT_3_SRC, OUT_4_SRC, OUT_SWITCH_56, OUT_VOL_56,
        OUT_5_SRC, OUT_6_SRC, IN_IMP_1, IN_PAD_1, IN_IMP_2, IN_PAD_2, IN_PAD_3, IN_PAD_4,
        MATRIX_ROUTE_1, MATRIX_1_A, MATRIX_1_B, MATRIX_1_C, MATRIX_1_D, MATRIX_1_E, MATRIX_1_F, MATRIX_1_G, MATRIX_1_H,
        MATRIX_ROUTE_2, MATRIX_2_A, MATRIX_2_B, MATRIX_2_C, MATRIX_2_D, MATRIX_2_E, MATRIX_2_F, MATRIX_2_G, MATRIX_2_H,
        MATRIX_ROUTE_3, MATRIX_3_A, MATRIX_3_B, MATRIX_3_C, MATRIX_3_D, MATRIX_3_E, MATRIX_3_F, MATRIX_3_G, MATRIX_3_H,
        MATRIX_ROUTE_4, MATRIX_4_A, MATRIX_4_B, MATRIX_4_C, MATRIX_4_D, MATRIX_4_E, MATRIX_4_F, MATRIX_4_G, MATRIX_4_H,
        MATRIX_ROUTE_5, MATRIX_5_A, MATRIX_5_B, MATRIX_5_C, MATRIX_5_D, MATRIX_5_E, MATRIX_5_F, MATRIX_5_G, MATRIX_5_H,
        MATRIX_ROUTE_6, MATRIX_6_A, MATRIX_6_B, MATRIX_6_C, MATRIX_6_D, MATRIX_6_E, MATRIX_6_F, MATRIX_6_G, MATRIX_6_H,
        MATRIX_ROUTE_7, MATRIX_7_A, MATRIX_7_B, MATRIX_7_C, MATRIX_7_D, MATRIX_7_E, MATRIX_7_F, MATRIX_7_G, MATRIX_7_H,
        MATRIX_ROUTE_8, MATRIX_8_A, MATRIX_8_B, MATRIX_8_C, MATRIX_8_D, MATRIX_8_E, MATRIX_8_F, MATRIX_8_G, MATRIX_8_H,
        MATRIX_ROUTE_9, MATRIX_9_A, MATRIX_9_B, MATRIX_9_C, MATRIX_9_D, MATRIX_9_E, MATRIX_9_F, MATRIX_9_G, MATRIX_9_H,
        MATRIX_ROUTE_10, MATRIX_10_A, MATRIX_10_B, MATRIX_10_C, MATRIX_10_D, MATRIX_10_E, MATRIX_10_F, MATRIX_10_G, MATRIX_10_H,
        MATRIX_ROUTE_11, MATRIX_11_A, MATRIX_11_B, MATRIX_11_C, MATRIX_11_D, MATRIX_11_E, MATRIX_11_F, MATRIX_11_G, MATRIX_11_H,
        MATRIX_ROUTE_12, MATRIX_12_A, MATRIX_12_B, MATRIX_12_C, MATRIX_12_D, MATRIX_12_E, MATRIX_12_F, MATRIX_12_G, MATRIX_12_H,
        MATRIX_ROUTE_13, MATRIX_13_A, MATRIX_13_B, MATRIX_13_C, MATRIX_13_D, MATRIX_13_E, MATRIX_13_F, MATRIX_13_G, MATRIX_13_H,
        MATRIX_ROUTE_14, MATRIX_14_A, MATRIX_14_B, MATRIX_14_C, MATRIX_14_D, MATRIX_14_E, MATRIX_14_F, MATRIX_14_G, MATRIX_14_H,
        MATRIX_ROUTE_15, MATRIX_15_A, MATRIX_15_B, MATRIX_15_C, MATRIX_15_D, MATRIX_15_E, MATRIX_15_F, MATRIX_15_G, MATRIX_15_H,
        MATRIX_ROUTE_16, MATRIX_16_A, MATRIX_16_B, MATRIX_16_C, MATRIX_16_D, MATRIX_16_E, MATRIX_16_F, MATRIX_16_G, MATRIX_16_H,
        MATRIX_ROUTE_17, MATRIX_17_A, MATRIX_17_B, MATRIX_17_C, MATRIX_17_D, MATRIX_17_E, MATRIX_17_F, MATRIX_17_G, MATRIX_17_H,
        MATRIX_ROUTE_18, MATRIX_18_A, MATRIX_18_B, MATRIX_18_C, MATRIX_18_D, MATRIX_18_E, MATRIX_18_F, MATRIX_18_G, MATRIX_18_H,
        INPUT_ROUTE_1, INPUT_ROUTE_2, INPUT_ROUTE_3, INPUT_ROUTE_4, INPUT_ROUTE_5, INPUT_ROUTE_6

};*/

const QString USB_SYNC = "Scarlett 6i6 USB-Sync Clock Source",
                MSTR_SWITCH = "Master Playback Switch",
                MSTR_VOL = "Master Playback Volume",
                OUT_SWITCH_12 = "Master 1 (Monitor) Playback Switch",
                OUT_VOL_12 = "Master 1 (Monitor) Playback Volume",
                OUT_1_SRC = "Master 1L (Monitor) Source Playback Enum",
                OUT_2_SRC = "Master 1R (Monitor) Source Playback Enum",
                OUT_SWITCH_34 = "Master 2 (Headphone) Playback Switch",
                OUT_VOL_34 = "Master 2 (Headphone) Playback Volume",
                OUT_3_SRC = "Master 2L (Headphone) Source Playback Enum",
                OUT_4_SRC = "Master 2R (Headphone) Source Playback Enum",
                OUT_SWITCH_56 = "Master 3 (SPDIF) Playback Switch",
                OUT_VOL_56 = "Master 3 (SPDIF) Playback Volume",
                OUT_5_SRC = "Master 3L (SPDIF) Source Playback Enum",
                OUT_6_SRC = "Master 3R (SPDIF) Source Playback Enum",
                IN_IMP_1 = "Input 1 Impedance Switch",
                IN_PAD_1 = "Input 1 Pad Switch",
                IN_IMP_2 = "Input 2 Impedance Switch",
                IN_PAD_2 = "Input 2 Pad Switch",
                IN_PAD_3 = "Input 3 Pad Switch",
                IN_PAD_4 = "Input 4 Pad Switch",
                MATRIX_ROUTE_1 = "Matrix 01 Input Playback Route",
                MATRIX_1_A = "Matrix 01 Mix A Playback Volume", MATRIX_1_B = "Matrix 01 Mix B Playback Volume", MATRIX_1_C = "Matrix 01 Mix C Playback Volume",
                MATRIX_1_D = "Matrix 01 Mix D Playback Volume", MATRIX_1_E = "Matrix 01 Mix E Playback Volume", MATRIX_1_F = "Matrix 01 Mix F Playback Volume",
                MATRIX_1_G = "Matrix 01 Mix G Playback Volume", MATRIX_1_H = "Matrix 01 Mix H Playback Volume",
                MATRIX_ROUTE_2 = "Matrix 02 Input Playback Route",
                MATRIX_2_A = "Matrix 02 Mix A Playback Volume", MATRIX_2_B = "Matrix 02 Mix B Playback Volume", MATRIX_2_C = "Matrix 02 Mix C Playback Volume",
                MATRIX_2_D = "Matrix 02 Mix D Playback Volume", MATRIX_2_E = "Matrix 02 Mix E Playback Volume", MATRIX_2_F = "Matrix 02 Mix F Playback Volume",
                MATRIX_2_G = "Matrix 02 Mix G Playback Volume", MATRIX_2_H = "Matrix 02 Mix H Playback Volume",
                MATRIX_ROUTE_3 = "Matrix 03 Input Playback Route",
                MATRIX_3_A = "Matrix 03 Mix A Playback Volume", MATRIX_3_B = "Matrix 03 Mix B Playback Volume", MATRIX_3_C = "Matrix 03 Mix C Playback Volume",
                MATRIX_3_D = "Matrix 03 Mix D Playback Volume", MATRIX_3_E = "Matrix 03 Mix E Playback Volume", MATRIX_3_F = "Matrix 03 Mix F Playback Volume",
                MATRIX_3_G = "Matrix 03 Mix G Playback Volume", MATRIX_3_H = "Matrix 03 Mix H Playback Volume",
                MATRIX_ROUTE_4 = "Matrix 04 Input Playback Route",
                MATRIX_4_A = "Matrix 04 Mix A Playback Volume", MATRIX_4_B = "Matrix 04 Mix B Playback Volume", MATRIX_4_C = "Matrix 04 Mix C Playback Volume",
                MATRIX_4_D = "Matrix 04 Mix D Playback Volume", MATRIX_4_E = "Matrix 04 Mix E Playback Volume", MATRIX_4_F = "Matrix 04 Mix F Playback Volume",
                MATRIX_4_G = "Matrix 04 Mix G Playback Volume", MATRIX_4_H = "Matrix 04 Mix H Playback Volume",
                MATRIX_ROUTE_5 = "Matrix 05 Input Playback Route",
                MATRIX_5_A = "Matrix 05 Mix A Playback Volume", MATRIX_5_B = "Matrix 05 Mix B Playback Volume", MATRIX_5_C = "Matrix 05 Mix C Playback Volume",
                MATRIX_5_D = "Matrix 05 Mix D Playback Volume", MATRIX_5_E = "Matrix 05 Mix E Playback Volume", MATRIX_5_F = "Matrix 05 Mix F Playback Volume",
                MATRIX_5_G = "Matrix 05 Mix G Playback Volume", MATRIX_5_H = "Matrix 05 Mix H Playback Volume",
                MATRIX_ROUTE_6 = "Matrix 06 Input Playback Route",
                MATRIX_6_A = "Matrix 06 Mix A Playback Volume", MATRIX_6_B = "Matrix 06 Mix B Playback Volume", MATRIX_6_C = "Matrix 06 Mix C Playback Volume",
                MATRIX_6_D = "Matrix 06 Mix D Playback Volume", MATRIX_6_E = "Matrix 06 Mix E Playback Volume", MATRIX_6_F = "Matrix 06 Mix F Playback Volume",
                MATRIX_6_G = "Matrix 06 Mix G Playback Volume", MATRIX_6_H = "Matrix 06 Mix H Playback Volume",
                MATRIX_ROUTE_7 = "Matrix 07 Input Playback Route",
                MATRIX_7_A = "Matrix 07 Mix A Playback Volume", MATRIX_7_B = "Matrix 07 Mix B Playback Volume", MATRIX_7_C = "Matrix 07 Mix C Playback Volume",
                MATRIX_7_D = "Matrix 07 Mix D Playback Volume", MATRIX_7_E = "Matrix 07 Mix E Playback Volume", MATRIX_7_F = "Matrix 07 Mix F Playback Volume",
                MATRIX_7_G = "Matrix 07 Mix G Playback Volume", MATRIX_7_H = "Matrix 07 Mix H Playback Volume",
                MATRIX_ROUTE_8 = "Matrix 08 Input Playback Route",
                MATRIX_8_A = "Matrix 08 Mix A Playback Volume", MATRIX_8_B = "Matrix 08 Mix B Playback Volume", MATRIX_8_C = "Matrix 08 Mix C Playback Volume",
                MATRIX_8_D = "Matrix 08 Mix D Playback Volume", MATRIX_8_E = "Matrix 08 Mix E Playback Volume", MATRIX_8_F = "Matrix 08 Mix F Playback Volume",
                MATRIX_8_G = "Matrix 08 Mix G Playback Volume", MATRIX_8_H = "Matrix 08 Mix H Playback Volume",
                MATRIX_ROUTE_9 = "Matrix 09 Input Playback Route",
                MATRIX_9_A = "Matrix 09 Mix A Playback Volume", MATRIX_9_B = "Matrix 09 Mix B Playback Volume", MATRIX_9_C = "Matrix 09 Mix C Playback Volume",
                MATRIX_9_D = "Matrix 09 Mix D Playback Volume", MATRIX_9_E = "Matrix 09 Mix E Playback Volume", MATRIX_9_F = "Matrix 09 Mix F Playback Volume",
                MATRIX_9_G = "Matrix 09 Mix G Playback Volume", MATRIX_9_H = "Matrix 09 Mix H Playback Volume",
                MATRIX_ROUTE_10 = "Matrix 10 Input Playback Route",
                MATRIX_10_A = "Matrix 10 Mix A Playback Volume", MATRIX_10_B = "Matrix 10 Mix B Playback Volume", MATRIX_10_C = "Matrix 10 Mix C Playback Volume",
                MATRIX_10_D = "Matrix 10 Mix D Playback Volume", MATRIX_10_E = "Matrix 10 Mix E Playback Volume", MATRIX_10_F = "Matrix 10 Mix F Playback Volume",
                MATRIX_10_G = "Matrix 10 Mix G Playback Volume", MATRIX_10_H = "Matrix 10 Mix H Playback Volume",
                MATRIX_ROUTE_11 = "Matrix 11 Input Playback Route",
                MATRIX_11_A = "Matrix 11 Mix A Playback Volume", MATRIX_11_B = "Matrix 11 Mix B Playback Volume", MATRIX_11_C = "Matrix 11 Mix C Playback Volume",
                MATRIX_11_D = "Matrix 11 Mix D Playback Volume", MATRIX_11_E = "Matrix 11 Mix E Playback Volume", MATRIX_11_F = "Matrix 11 Mix F Playback Volume",
                MATRIX_11_G = "Matrix 11 Mix G Playback Volume", MATRIX_11_H = "Matrix 11 Mix H Playback Volume",
                MATRIX_ROUTE_12 = "Matrix 12 Input Playback Route",
                MATRIX_12_A = "Matrix 12 Mix A Playback Volume", MATRIX_12_B = "Matrix 12 Mix B Playback Volume", MATRIX_12_C = "Matrix 12 Mix C Playback Volume",
                MATRIX_12_D = "Matrix 12 Mix D Playback Volume", MATRIX_12_E = "Matrix 12 Mix E Playback Volume", MATRIX_12_F = "Matrix 12 Mix F Playback Volume",
                MATRIX_12_G = "Matrix 12 Mix G Playback Volume", MATRIX_12_H = "Matrix 12 Mix H Playback Volume",
                MATRIX_ROUTE_13 = "Matrix 13 Input Playback Route",
                MATRIX_13_A = "Matrix 13 Mix A Playback Volume", MATRIX_13_B = "Matrix 13 Mix B Playback Volume", MATRIX_13_C = "Matrix 13 Mix C Playback Volume",
                MATRIX_13_D = "Matrix 13 Mix D Playback Volume", MATRIX_13_E = "Matrix 13 Mix E Playback Volume", MATRIX_13_F = "Matrix 13 Mix F Playback Volume",
                MATRIX_13_G = "Matrix 13 Mix G Playback Volume", MATRIX_13_H = "Matrix 13 Mix H Playback Volume",
                MATRIX_ROUTE_14 = "Matrix 14 Input Playback Route",
                MATRIX_14_A = "Matrix 14 Mix A Playback Volume", MATRIX_14_B = "Matrix 14 Mix B Playback Volume", MATRIX_14_C = "Matrix 14 Mix C Playback Volume",
                MATRIX_14_D = "Matrix 14 Mix D Playback Volume", MATRIX_14_E = "Matrix 14 Mix E Playback Volume", MATRIX_14_F = "Matrix 14 Mix F Playback Volume",
                MATRIX_14_G = "Matrix 14 Mix G Playback Volume", MATRIX_14_H = "Matrix 14 Mix H Playback Volume",
                MATRIX_ROUTE_15 = "Matrix 15 Input Playback Route",
                MATRIX_15_A = "Matrix 15 Mix A Playback Volume", MATRIX_15_B = "Matrix 15 Mix B Playback Volume", MATRIX_15_C = "Matrix 15 Mix C Playback Volume",
                MATRIX_15_D = "Matrix 15 Mix D Playback Volume", MATRIX_15_E = "Matrix 15 Mix E Playback Volume", MATRIX_15_F = "Matrix 15 Mix F Playback Volume",
                MATRIX_15_G = "Matrix 15 Mix G Playback Volume", MATRIX_15_H = "Matrix 15 Mix H Playback Volume",
                MATRIX_ROUTE_16 = "Matrix 16 Input Playback Route",
                MATRIX_16_A = "Matrix 16 Mix A Playback Volume", MATRIX_16_B = "Matrix 16 Mix B Playback Volume", MATRIX_16_C = "Matrix 16 Mix C Playback Volume",
                MATRIX_16_D = "Matrix 16 Mix D Playback Volume", MATRIX_16_E = "Matrix 16 Mix E Playback Volume", MATRIX_16_F = "Matrix 16 Mix F Playback Volume",
                MATRIX_16_G = "Matrix 16 Mix G Playback Volume", MATRIX_16_H = "Matrix 16 Mix H Playback Volume",
                MATRIX_ROUTE_17 = "Matrix 17 Input Playback Route",
                MATRIX_17_A = "Matrix 17 Mix A Playback Volume", MATRIX_17_B = "Matrix 17 Mix B Playback Volume", MATRIX_17_C = "Matrix 17 Mix C Playback Volume",
                MATRIX_17_D = "Matrix 17 Mix D Playback Volume", MATRIX_17_E = "Matrix 17 Mix E Playback Volume", MATRIX_17_F = "Matrix 17 Mix F Playback Volume",
                MATRIX_17_G = "Matrix 17 Mix G Playback Volume", MATRIX_17_H = "Matrix 17 Mix H Playback Volume",
                MATRIX_ROUTE_18 = "Matrix 18 Input Playback Route",
                MATRIX_18_A = "Matrix 18 Mix A Playback Volume", MATRIX_18_B = "Matrix 18 Mix B Playback Volume", MATRIX_18_C = "Matrix 18 Mix C Playback Volume",
                MATRIX_18_D = "Matrix 18 Mix D Playback Volume", MATRIX_18_E = "Matrix 18 Mix E Playback Volume", MATRIX_18_F = "Matrix 18 Mix F Playback Volume",
                MATRIX_18_G = "Matrix 18 Mix G Playback Volume", MATRIX_18_H = "Matrix 18 Mix H Playback Volume",
                INPUT_ROUTE_1 = "Input Source 01 Capture Route", INPUT_ROUTE_2 = "Input Source 02 Capture Route", INPUT_ROUTE_3 = "Input Source 03 Capture Route",
                INPUT_ROUTE_4 = "Input Source 04 Capture Route", INPUT_ROUTE_5 = "Input Source 05 Capture Route", INPUT_ROUTE_6 = "Input Source 06 Capture Route";



struct MixSisCtrl
{

    static QHash<QString, int> get_numid;

    static bool numidIsVolume(int test);

    QSlider *vol_master[2];
    QCheckBox *vol_master_mute;
    QCheckBox *vol_master_link;
    QSlider *vol_out[6];
    QCheckBox *vol_out_mute[6];
    QCheckBox *vol_out_link[3];
    QComboBox *out_src[6];
    QComboBox *in_src[6];
    QRadioButton *in_imp[4];
    QRadioButton *in_pad[8];
    QComboBox *mtx_src[18];
    QSlider *mtx_vol[18][8];
    QPushButton *mtx_clear[18];


 //   int save_to_dialog(QWidget *context);
 //   int load_from_dialog(QWidget *context);

    /// gets control values from file
    int load_from(const QString &filename);

    /// dumps control values to file
    int save_to(const QString &filename);

    /// sets the qt control corresponding to alsa_id,idx to a certain value
    void set(int alsa_id, int value, int idx = 0);
};

#endif // MIXSISCTRL

