#include "mytablewightitem.h"

mytablewightitem::mytablewightitem()
{

}

void mytablewightitem::setInfo(FileInfo &info)
{
    //触发浅拷贝，该结构体中的成员符合浅拷贝，可以这么写
    m_info = info;
    this->setText(info.name);
    //加载图标，资源文件路径以":/"开始
    if(info.type == "dir"){
        this->setIcon(QIcon(":/images/folder.png"));
    }else{
        this->setIcon(QIcon(":/images/file.png"));
    }

    //设置勾选框，并设置开始勾选状态为非勾选状态
    this->setCheckState( Qt::Unchecked );
}
