#include "outputdialog.h"
#include "QScrollBar"
#include "ui_outputdialog.h"

#include <QAbstractButton>


OutputDialog::OutputDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OutputDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &QDialog::hide);
    //connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &QDialog::show);
}

OutputDialog::~OutputDialog()
{
    delete ui;
}

void OutputDialog::textAdd(const QString &str)
{
    if (str.isEmpty())
        return;
    ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText()+str);

    //滚动到底部
    QScrollBar *scrollbar = ui->plainTextEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());

    this->show();
}

void OutputDialog::clear()
{
    ui->plainTextEdit->clear();
}
