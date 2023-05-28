#ifndef COMMON_H
#define COMMON_H
#include <QMetaType>
#include<QString>

struct FileInfo{

    FileInfo():fileid(0),size(0),pFile(nullptr),pos(0),isPause(0){

    }

    int fileid;
    QString name;
    QString dir;
    QString time;
    int size;
    QString md5;
    QString type;
    QString absolutePath;

    int pos;

    int isPause; // 暂停 1 0

    //文件指针
    FILE *pFile;

    //通过字节数，获取kb，mb大小字符串
static QString getSize(int size){

        QString res;

        int tmp = size;
        int count = 0;

        while(tmp != 0){
            tmp /= 1024;
            if(tmp != 0) count++;
        }

        switch (count) {
        case 0:
            res = QString("0.%1KB").arg((int)(size/1024.0*100),2,10,QChar('0'));
            break;
        case 1:
            res = QString("%1.%2KB").arg(size/1024).
                    arg(size%1024,2,10,QChar('0'));
            break;
        default:
            res = QString("%1.%2MB").arg(size/1024/1024).
                    arg(size/1024%1024,2,10,QChar('0'));
            break;
        }

        return res;

    }
};
Q_DECLARE_METATYPE(FileInfo);



#endif // COMMON_H




