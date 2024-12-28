#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>

#include "mixsis.h"
#include "changewatcher.h"
extern const char* alsaDeviceName;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool event(QEvent *ev);

    int loadFrom(QString filen);
    int saveTo(QString filen);
    void setChangeWatcherMask(int n, int mask);

public slots:
    void setVal(int alsa_id, int value, int idx = 0);

signals:
    void maskVol(int num, bool mask);

private:
    ChangeWatcher *changeWatcher;
    Ui::MainWindow *ui;
    MixSis *mixer;
    MixSisCtrl mixerCtrl;

    QMenu *fileMenu;
    QAction *saveAct;
    QAction *loadAct;
    QAction *exitAct;
    QMenu *editMenu;
    QAction *clearMtxAct;
    QAction *bounceMtxAct;
    QAction *restoreMtxAct;

    int mtxRAM[18][9];

    void createMenu();
};

#endif // MAINWINDOW_H
