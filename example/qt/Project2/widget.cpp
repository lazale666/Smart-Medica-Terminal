#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent), opClicked(false)
{
    setFixedSize(300, 420);
    setWindowTitle("计算器");

    // ---------------------- 两个显示框（纯代码创建）----------------------
    formulaLine = new QLineEdit(this);
    formulaLine->setFont(QFont("Arial", 16));
    formulaLine->setAlignment(Qt::AlignRight);
    formulaLine->setReadOnly(true);

    resultLine = new QLineEdit(this);
    resultLine->setFont(QFont("Arial", 28));
    resultLine->setAlignment(Qt::AlignRight);
    resultLine->setReadOnly(true);
    resultLine->setText("0");

    // ---------------------- 按钮创建 ----------------------
    QPushButton *btn0 = new QPushButton("0", this);
    QPushButton *btn1 = new QPushButton("1", this);
    QPushButton *btn2 = new QPushButton("2", this);
    QPushButton *btn3 = new QPushButton("3", this);
    QPushButton *btn4 = new QPushButton("4", this);
    QPushButton *btn5 = new QPushButton("5", this);
    QPushButton *btn6 = new QPushButton("6", this);
    QPushButton *btn7 = new QPushButton("7", this);
    QPushButton *btn8 = new QPushButton("8", this);
    QPushButton *btn9 = new QPushButton("9", this);
    QPushButton *btnPoint = new QPushButton(".", this);
    QPushButton *btnSign = new QPushButton("+/-", this);

    QPushButton *btnAdd = new QPushButton("+", this);
    QPushButton *btnSub = new QPushButton("-", this);
    QPushButton *btnMul = new QPushButton("*", this);
    QPushButton *btnDiv = new QPushButton("/", this);
    QPushButton *btnEqual = new QPushButton("=", this);
    QPushButton *btnClear = new QPushButton("C", this);
    QPushButton *btnBack = new QPushButton("←", this);

    // ---------------------- 按钮样式 ----------------------
    for (auto btn : findChildren<QPushButton*>()) {
        btn->setMinimumSize(60, 50);
        btn->setFont(QFont("Arial", 14));
    }

    // ---------------------- 布局 ----------------------
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(formulaLine);
    mainLayout->addWidget(resultLine);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(btnClear, 0, 0);
    gridLayout->addWidget(btnBack, 0, 1);
    gridLayout->addWidget(btnSign, 0, 2);
    gridLayout->addWidget(btnDiv, 0, 3);

    gridLayout->addWidget(btn7, 1, 0);
    gridLayout->addWidget(btn8, 1, 1);
    gridLayout->addWidget(btn9, 1, 2);
    gridLayout->addWidget(btnMul, 1, 3);

    gridLayout->addWidget(btn4, 2, 0);
    gridLayout->addWidget(btn5, 2, 1);
    gridLayout->addWidget(btn6, 2, 2);
    gridLayout->addWidget(btnSub, 2, 3);

    gridLayout->addWidget(btn1, 3, 0);
    gridLayout->addWidget(btn2, 3, 1);
    gridLayout->addWidget(btn3, 3, 2);
    gridLayout->addWidget(btnAdd, 3, 3);

    gridLayout->addWidget(btn0, 4, 0);
    gridLayout->addWidget(btnPoint, 4, 1);
    gridLayout->addWidget(btnEqual, 4, 2, 1, 2);

    mainLayout->addLayout(gridLayout);
    setLayout(mainLayout);

    // ---------------------- 信号绑定 ----------------------
    QList<QPushButton*> numBtns = {btn0,btn1,btn2,btn3,btn4,btn5,btn6,btn7,btn8,btn9,btnPoint};
    for (auto b : numBtns) connect(b, &QPushButton::clicked, this, &Widget::numClick);

    connect(btnAdd, &QPushButton::clicked, this, &Widget::opClick);
    connect(btnSub, &QPushButton::clicked, this, &Widget::opClick);
    connect(btnMul, &QPushButton::clicked, this, &Widget::opClick);
    connect(btnDiv, &QPushButton::clicked, this, &Widget::opClick);

    connect(btnEqual, &QPushButton::clicked, this, &Widget::eqClick);
    connect(btnClear, &QPushButton::clicked, this, &Widget::clearClick);
    connect(btnBack, &QPushButton::clicked, this, &Widget::backClick);
    connect(btnSign, &QPushButton::clicked, this, &Widget::signClick);
}

Widget::~Widget() {}

void Widget::numClick() {
    QPushButton *b = qobject_cast<QPushButton*>(sender());
    QString t = b->text();

    if (t == "." && currentNum.contains(".")) return;
    if (opClicked) { currentNum.clear(); opClicked = false; }

    currentNum += t;
    resultLine->setText(currentNum);
}

void Widget::opClick() {
    if (currentNum.isEmpty()) return;
    QPushButton *b = qobject_cast<QPushButton*>(sender());

    lastNum = currentNum;
    op = b->text();
    formulaLine->setText(lastNum + " " + op);
    opClicked = true;
}

void Widget::eqClick() {
    if (lastNum.isEmpty() || currentNum.isEmpty()) return;

    double n1 = lastNum.toDouble();
    double n2 = currentNum.toDouble();
    double res = 0;
    bool err = false;

    if (op == "+") res = n1 + n2;
    else if (op == "-") res = n1 - n2;
    else if (op == "*") res = n1 * n2;
    else if (op == "/") {
        if (n2 == 0) err = true;
        else res = n1 / n2;
    }

    if (err) {
        resultLine->setText("错误");
        formulaLine->clear();
        lastNum.clear(); currentNum.clear(); op.clear();
        return;
    }

    formulaLine->setText(lastNum + " " + op + " " + currentNum + " =");
    resultLine->setText(QString::number(res, 'g', 12));
    currentNum = QString::number(res);
    lastNum.clear();
}

void Widget::clearClick() {
    currentNum.clear(); lastNum.clear(); op.clear(); opClicked = false;
    formulaLine->clear();
    resultLine->setText("0");
}

void Widget::backClick() {
    if (currentNum.isEmpty()) return;
    currentNum.chop(1);
    resultLine->setText(currentNum.isEmpty() ? "0" : currentNum);
}

void Widget::signClick() {
    if (currentNum.isEmpty()) return;
    currentNum.startsWith("-") ? currentNum.remove(0,1) : currentNum.prepend("-");
    resultLine->setText(currentNum);
}