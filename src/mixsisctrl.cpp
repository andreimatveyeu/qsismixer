#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QObject>
#include <QTemporaryFile>
#include "mixsisctrl.h"

const char *magicn = "QSIS";

QHash<QString, int> MixSisCtrl::get_numid;

bool MixSisCtrl::numidIsVolume(int test){
    if(test == get_numid[MSTR_VOL]) return true;
    if(test == get_numid[OUT_VOL_12]) return true;
    if(test == get_numid[OUT_VOL_34]) return true;
    if(test == get_numid[OUT_VOL_56]) return true;
    if(test >= get_numid[MATRIX_1_A] && test <= get_numid[MATRIX_1_H]) return true;
    if(test >= get_numid[MATRIX_2_A] && test <= get_numid[MATRIX_2_H]) return true;
    if(test >= get_numid[MATRIX_3_A] && test <= get_numid[MATRIX_3_H]) return true;
    if(test >= get_numid[MATRIX_4_A] && test <= get_numid[MATRIX_4_H]) return true;
    if(test >= get_numid[MATRIX_5_A] && test <= get_numid[MATRIX_5_H]) return true;
    if(test >= get_numid[MATRIX_6_A] && test <= get_numid[MATRIX_6_H]) return true;
    if(test >= get_numid[MATRIX_7_A] && test <= get_numid[MATRIX_7_H]) return true;
    if(test >= get_numid[MATRIX_8_A] && test <= get_numid[MATRIX_8_H]) return true;
    if(test >= get_numid[MATRIX_9_A] && test <= get_numid[MATRIX_9_H]) return true;
    if(test >= get_numid[MATRIX_10_A] && test <= get_numid[MATRIX_10_H]) return true;
    if(test >= get_numid[MATRIX_11_A] && test <= get_numid[MATRIX_11_H]) return true;
    if(test >= get_numid[MATRIX_12_A] && test <= get_numid[MATRIX_12_H]) return true;
    if(test >= get_numid[MATRIX_13_A] && test <= get_numid[MATRIX_13_H]) return true;
    if(test >= get_numid[MATRIX_14_A] && test <= get_numid[MATRIX_14_H]) return true;
    if(test >= get_numid[MATRIX_15_A] && test <= get_numid[MATRIX_15_H]) return true;
    if(test >= get_numid[MATRIX_16_A] && test <= get_numid[MATRIX_16_H]) return true;
    if(test >= get_numid[MATRIX_17_A] && test <= get_numid[MATRIX_17_H]) return true;
    if(test >= get_numid[MATRIX_18_A] && test <= get_numid[MATRIX_18_H]) return true;

    return false;
}

int MixSisCtrl::save_to(const QString &filename){
    QFile file(filename);
    if(!file.open( QIODevice::WriteOnly )){
        fprintf(stderr, "%s\n", file.errorString().toLocal8Bit().constData());
        fprintf(stderr,"unable to write config to file \"%s\"\n", filename.toLocal8Bit().constData());
        return -1;
    }
    else{
        fprintf(stderr, "successfully opened file \"%s\" for write\n", filename.toLocal8Bit().constData());
    }
    QDataStream cfg(&file);
    cfg.writeRawData(magicn, 4);
    for(int i=0; i<2; ++i){
        cfg << (qint32)vol_master[i]->value();
        cfg << (qint32)in_imp[i*2+1]->isChecked();
    }
    cfg << (qint32) vol_master_mute->isChecked();
    cfg << (qint32) vol_master_link->isChecked();
    for(int i=0; i<6; ++i){
        cfg << (qint32) vol_out[i]->value();
        cfg << (qint32) vol_out_mute[i]->isChecked();
        cfg << (qint32) out_src[i]->currentIndex();
        cfg << (qint32) in_src[i]->currentIndex();
    }
    for(int i=0; i<3; ++i){
        cfg << (qint32) vol_out_link[i]->isChecked();
    }
    for(int i=0; i<4; ++i){
        cfg << (qint32) in_pad[i*2+1]->isChecked();
    }
    for(int i=0; i<18; ++i){
        cfg << (qint32) mtx_src[i]->currentIndex();
        for(int j=0; j<8; ++j){
            cfg << (qint32) mtx_vol[i][j]->value();
        }
    }
    /// best case scenario would be to instead define a QExceptionDataStream : QDataStream that throws exceptions like a modern thing
    /// or just use std::fstream and flag it to throw exceptions instead
    if(cfg.status() != QDataStream::Ok){
        fprintf(stderr, "File output error! Probably corrupted data at %s.\n", filename.toLocal8Bit().constData());
        file.close();
        return -1;
    }
    file.close();
    return 0;
}

int MixSisCtrl::load_from(const QString &filename){
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if(!file.isOpen()){
        fprintf(stderr, "unable to read from config file \"%s\"\n",filename.toLocal8Bit().constData());
        return -1;
    }
    else{
        fprintf(stderr, "successfully opened file \"%s\" for read\n", filename.toLocal8Bit().constData());
    }
    QDataStream cfg(&file);
    for(int i=0;i<4;++i){
        qint8 n;
        cfg >> n;
        if(n != magicn[i]){
            fprintf(stderr, "\"%s\" is not a QSIS config file, aborting...\n", filename.toLocal8Bit().constData());
            return -1;
        }
    }
    // here simply setting the controls will work, since they are already connected to alsa in MixSis
    qint32 tmp;
    for(int i=0; i<2; ++i){
        cfg >> tmp;
        vol_master[i]->setValue(tmp);
        cfg >> tmp;
        in_imp[i*2+1]->setChecked(tmp);
    }
    cfg >> tmp;
    vol_master_mute->setChecked(tmp);
    cfg >> tmp;
    vol_master_link->setChecked(tmp);
    for(int i=0; i<6; ++i){
        cfg >> tmp;
        vol_out[i]->setValue(tmp);
        cfg >> tmp;
        vol_out_mute[i]->setChecked(tmp);
        cfg >> tmp;
        out_src[i]->setCurrentIndex(tmp);
        cfg >> tmp;
        in_src[i]->setCurrentIndex(tmp);
    }
    for(int i=0; i<3; ++i){
        cfg >> tmp;
        vol_out_link[i]->setChecked(tmp);
    }
    for(int i=0; i<4; ++i){
        cfg >> tmp;
        in_pad[i*2+1]->setChecked(tmp);
    }
    for(int i=0; i<18; ++i){
        cfg >> tmp;
        mtx_src[i]->setCurrentIndex(tmp);
        for(int j=0; j<8; ++j){
            cfg >> tmp;
            mtx_vol[i][j]->setValue(tmp);
        }
    }
    // unfortunately, QT *won't* throw an exception on trying to read past eof (as far as I can tell)
    if(cfg.status() != QDataStream::Ok){
        fprintf(stderr, "File input error! Probably corrupted data at %s.\n"
                "It is also very likely that qsis settings are corrupted.\n", filename.toLocal8Bit().constData());
        file.close();
        return -1;
    }
    file.close();
    return 0;
}

/// sets the qt control corresponding to alsa_id,idx to a certain value
/// should probably find a cleaner way to do this than the switch case fallthrough but this is simplish and it works
void MixSisCtrl::set(int alsa_id, int value, int idx){
    long db;

    if(alsa_id == get_numid[MSTR_SWITCH]){
        vol_master_mute->setChecked(!value);
    }

    else if(alsa_id == get_numid[MSTR_VOL]){
        db = value;
        vol_master[0]->setValue(db);
        vol_master[1]->setValue(db);
    }

    else if(alsa_id == get_numid[OUT_SWITCH_12]){
        vol_out_mute[idx]->setChecked(!value);
    }
    else if(alsa_id == get_numid[OUT_SWITCH_34]){
        vol_out_mute[2+idx]->setChecked(!value);
    }
    else if(alsa_id == get_numid[OUT_SWITCH_56]){
        vol_out_mute[4+idx]->setChecked(!value);
    }

    else if(alsa_id == get_numid[OUT_VOL_12]){
        vol_out[idx]->setValue(value);
    }
    else if(alsa_id == get_numid[OUT_VOL_34]){
        vol_out[2+idx]->setValue(value);
    }
    else if(alsa_id == get_numid[OUT_VOL_56]){
        vol_out[4+idx]->setValue(value);
    }

    else if(alsa_id == get_numid[OUT_1_SRC]){
        out_src[0]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[OUT_2_SRC]){
        out_src[1]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[OUT_3_SRC]){
        out_src[2]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[OUT_4_SRC]){
        out_src[3]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[OUT_5_SRC]){
        out_src[4]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[OUT_6_SRC]){
        out_src[5]->setCurrentIndex(value);
    }

    else if(alsa_id == get_numid[IN_IMP_1]){
        in_imp[value]->setChecked(1);
    }
    else if(alsa_id == get_numid[IN_IMP_2]){
        in_imp[2 + value]->setChecked(1);
    }

    else if(alsa_id == get_numid[IN_PAD_1]){
        in_pad[value]->setChecked(1);
    }
    else if(alsa_id == get_numid[IN_PAD_2]){
        in_pad[2 + value]->setChecked(1);
    }
    else if(alsa_id == get_numid[IN_PAD_3]){
        in_pad[4 + value]->setChecked(1);
    }
    else if(alsa_id == get_numid[IN_PAD_4]){
        in_pad[6 + value]->setChecked(1);
    }

    else if(alsa_id == get_numid[INPUT_ROUTE_1]){
        in_src[0]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[INPUT_ROUTE_2]){
        in_src[1]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[INPUT_ROUTE_3]){
        in_src[2]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[INPUT_ROUTE_4]){
        in_src[3]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[INPUT_ROUTE_5]){
        in_src[4]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[INPUT_ROUTE_6]){
        in_src[5]->setCurrentIndex(value);
    }

    else if(alsa_id == get_numid[MATRIX_ROUTE_1]){
        mtx_src[0]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_2]){
        mtx_src[1]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_3]){
        mtx_src[2]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_4]){
        mtx_src[3]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_5]){
        mtx_src[4]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_6]){
        mtx_src[5]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_7]){
        mtx_src[6]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_8]){
        mtx_src[7]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_9]){
        mtx_src[8]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_10]){
        mtx_src[9]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_11]){
        mtx_src[10]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_12]){
        mtx_src[11]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_13]){
        mtx_src[12]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_14]){
        mtx_src[13]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_15]){
        mtx_src[14]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_16]){
        mtx_src[15]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_17]){
        mtx_src[16]->setCurrentIndex(value);
    }
    else if(alsa_id == get_numid[MATRIX_ROUTE_18]){
        mtx_src[17]->setCurrentIndex(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_A]){
        mtx_vol[0][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_A]){
        mtx_vol[1][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_A]){
        mtx_vol[2][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_A]){
        mtx_vol[3][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_A]){
        mtx_vol[4][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_A]){
        mtx_vol[5][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_A]){
        mtx_vol[6][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_A]){
        mtx_vol[7][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_A]){
        mtx_vol[8][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_A]){
        mtx_vol[9][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_A]){
        mtx_vol[10][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_A]){
        mtx_vol[11][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_A]){
        mtx_vol[12][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_A]){
        mtx_vol[13][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_A]){
        mtx_vol[14][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_A]){
        mtx_vol[15][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_A]){
        mtx_vol[16][0]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_A]){
        mtx_vol[17][0]->setValue(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_B]){
        mtx_vol[0][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_B]){
        mtx_vol[1][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_B]){
        mtx_vol[2][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_B]){
        mtx_vol[3][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_B]){
        mtx_vol[4][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_B]){
        mtx_vol[5][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_B]){
        mtx_vol[6][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_B]){
        mtx_vol[7][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_B]){
        mtx_vol[8][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_B]){
        mtx_vol[9][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_B]){
        mtx_vol[10][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_B]){
        mtx_vol[11][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_B]){
        mtx_vol[12][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_B]){
        mtx_vol[13][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_B]){
        mtx_vol[14][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_B]){
        mtx_vol[15][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_B]){
        mtx_vol[16][1]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_B]){
        mtx_vol[17][1]->setValue(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_C]){
        mtx_vol[0][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_C]){
        mtx_vol[1][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_C]){
        mtx_vol[2][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_C]){
        mtx_vol[3][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_C]){
        mtx_vol[4][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_C]){
        mtx_vol[5][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_C]){
        mtx_vol[6][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_C]){
        mtx_vol[7][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_C]){
        mtx_vol[8][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_C]){
        mtx_vol[9][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_C]){
        mtx_vol[10][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_C]){
        mtx_vol[11][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_C]){
        mtx_vol[12][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_C]){
        mtx_vol[13][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_C]){
        mtx_vol[14][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_C]){
        mtx_vol[15][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_C]){
        mtx_vol[16][2]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_C]){
        mtx_vol[17][2]->setValue(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_D]){
        mtx_vol[0][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_D]){
        mtx_vol[1][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_D]){
        mtx_vol[2][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_D]){
        mtx_vol[3][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_D]){
        mtx_vol[4][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_D]){
        mtx_vol[5][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_D]){
        mtx_vol[6][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_D]){
        mtx_vol[7][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_D]){
        mtx_vol[8][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_D]){
        mtx_vol[9][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_D]){
        mtx_vol[10][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_D]){
        mtx_vol[11][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_D]){
        mtx_vol[12][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_D]){
        mtx_vol[13][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_D]){
        mtx_vol[14][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_D]){
        mtx_vol[15][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_D]){
        mtx_vol[16][3]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_D]){
        mtx_vol[17][3]->setValue(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_E]){
        mtx_vol[0][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_E]){
        mtx_vol[1][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_E]){
        mtx_vol[2][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_E]){
        mtx_vol[3][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_E]){
        mtx_vol[4][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_E]){
        mtx_vol[5][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_E]){
        mtx_vol[6][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_E]){
        mtx_vol[7][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_E]){
        mtx_vol[8][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_E]){
        mtx_vol[9][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_E]){
        mtx_vol[10][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_E]){
        mtx_vol[11][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_E]){
        mtx_vol[12][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_E]){
        mtx_vol[13][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_E]){
        mtx_vol[14][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_E]){
        mtx_vol[15][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_E]){
        mtx_vol[16][4]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_E]){
        mtx_vol[17][4]->setValue(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_F]){
        mtx_vol[0][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_F]){
        mtx_vol[1][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_F]){
        mtx_vol[2][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_F]){
        mtx_vol[3][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_F]){
        mtx_vol[4][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_F]){
        mtx_vol[5][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_F]){
        mtx_vol[6][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_F]){
        mtx_vol[7][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_F]){
        mtx_vol[8][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_F]){
        mtx_vol[9][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_F]){
        mtx_vol[10][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_F]){
        mtx_vol[11][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_F]){
        mtx_vol[12][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_F]){
        mtx_vol[13][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_F]){
        mtx_vol[14][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_F]){
        mtx_vol[15][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_F]){
        mtx_vol[16][5]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_F]){
        mtx_vol[17][5]->setValue(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_G]){
        mtx_vol[0][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_G]){
        mtx_vol[1][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_G]){
        mtx_vol[2][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_G]){
        mtx_vol[3][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_G]){
        mtx_vol[4][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_G]){
        mtx_vol[5][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_G]){
        mtx_vol[6][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_G]){
        mtx_vol[7][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_G]){
        mtx_vol[8][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_G]){
        mtx_vol[9][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_G]){
        mtx_vol[10][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_G]){
        mtx_vol[11][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_G]){
        mtx_vol[12][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_G]){
        mtx_vol[13][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_G]){
        mtx_vol[14][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_G]){
        mtx_vol[15][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_G]){
        mtx_vol[16][6]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_G]){
        mtx_vol[17][6]->setValue(value);
    }

    else if(alsa_id == get_numid[MATRIX_1_H]){
        mtx_vol[0][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_2_H]){
        mtx_vol[1][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_3_H]){
        mtx_vol[2][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_4_H]){
        mtx_vol[3][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_5_H]){
        mtx_vol[4][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_6_H]){
        mtx_vol[5][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_7_H]){
        mtx_vol[6][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_8_H]){
        mtx_vol[7][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_9_H]){
        mtx_vol[8][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_10_H]){
        mtx_vol[9][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_11_H]){
        mtx_vol[10][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_12_H]){
        mtx_vol[11][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_13_H]){
        mtx_vol[12][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_14_H]){
        mtx_vol[13][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_15_H]){
        mtx_vol[14][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_16_H]){
        mtx_vol[15][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_17_H]){
        mtx_vol[16][7]->setValue(value);
    }
    else if(alsa_id == get_numid[MATRIX_18_H]){
        mtx_vol[17][7]->setValue(value);
    }
    else{
        fprintf(stderr, "MixSisCtrl::set: invalid alsa ID: %d\n", alsa_id);
    }
}
