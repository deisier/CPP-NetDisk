#ifndef MYTABLEWIGHTITEM_H
#define MYTABLEWIGHTITEM_H

#include<QTableWidgetItem>
#include"common.h"

class mytablewightitem : public QTableWidgetItem
{
 //   Q_OBJECT 继承Qobject类这个要注释掉，不然会报错
public:
    explicit mytablewightitem();

signals:

public slots:
    //保存文件信息到m_info,并设置该控件
    void setInfo(FileInfo & info);

public:
    FileInfo m_info;

};

#endif // MYTABLEWIGHTITEM_H
