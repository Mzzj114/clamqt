#include "header.h"
#include <QMouseEvent>

Header::Header(QWidget *parent)
    : QWidget{parent}
{

    mWidParent = parent;
}

void Header::setWidgetToMove(QWidget *w)
{
    mWidParent = w;
}

// 重写mouseMoveEvent
void Header::mouseMoveEvent(QMouseEvent *event)
{
    // 持续按住才做对应事件
    if (mKeepPressed)
    {
        QPoint move = event->globalPos()-mPntStart;
        // 将父窗体移动到父窗体之前的位置加上鼠标移动的位置【event->globalPos()- mPntStart】
        mWidParent->move(mWidParent->pos()+move);
        // 将鼠标在屏幕中的位置替换为新的位置
        mPntStart = event->globalPos();
    }
}

// 重写mousePressEvent
void Header::mousePressEvent(QMouseEvent *event)
{
    // 鼠标左键按下事件
    if (event->button() == Qt::LeftButton)
    {
        // 记录鼠标状态
        mKeepPressed = true;
        // 记录鼠标在屏幕中的位置
        mPntStart = event->globalPos();
    }
}

// 重写mouseReleaseEvent
void Header::mouseReleaseEvent(QMouseEvent *event)
{
    // 鼠标左键释放
    if (event->button() == Qt::LeftButton)
    {
        // 记录鼠标状态
        mKeepPressed = false;
    }
}
