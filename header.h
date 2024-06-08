#ifndef HEADER_H
#define HEADER_H

#include <QWidget>

class Header : public QWidget
{
    Q_OBJECT
public:
    explicit Header(QWidget *parent = nullptr);
    void setWidgetToMove(QWidget* w);

private:
    QPoint mPntStart;
    // 鼠标是否持续按下
    bool mKeepPressed;

    //要拖拽的
    QWidget *mWidParent;
signals:
protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // HEADER_H
