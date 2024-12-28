#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>

void help();
void version();

//static const char * filename = "cfg.6i6";
const char* alsaDeviceName = "hw:USB";

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    QApplication::setApplicationName("Qt Sixisix Mixer");
    QApplication::setApplicationVersion("\nversion 0.2.6\n25 Feb 2021");
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Sixisix Mixer\nMixer GUI for controlling the Focusrite Scarlett 6i6");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
           // set alsa device handle
           {{"D","device"}, QApplication::translate("main","Use alsa device handle <device> (default hw:USB)"),
                            QApplication::translate("main","device")},
           {{"d","dump"}, QApplication::translate("main","Dump current configuration to <filename>"),
                            QApplication::translate("main","filename")},
           {{"l","load"}, QApplication::translate("main","Load configuration from <filename>"),
                            QApplication::translate("main","filename")},
                      });
    parser.process(a);

    // create lock file to make sure there is only one instance running
    QString lockFileName = QDir::tempPath() + "/qsismix.lock";
    QLockFile lockFile(lockFileName);
    if(!lockFile.tryLock(100)){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("There is probably another instance of qsismix running.\n"
                       "If you are sure that there is not, delete the file at\n" + lockFileName + " and invoke the program again.");
        msgBox.exec();
        return 1;
    }

    char * tmp_device = NULL;
    if(parser.isSet("device")){
        QByteArray userDevice = parser.value("device").toLocal8Bit();

        tmp_device = new char[userDevice.size() + 1];
        strcpy(tmp_device, userDevice.data());
        alsaDeviceName = tmp_device;
    }
    MainWindow w;
    if(parser.isSet("dump")){
        w.saveTo(parser.value("dump"));
    }
    if(parser.isSet("load")){
        w.loadFrom(parser.value("load"));
    }
    w.show();
    return a.exec();
}
