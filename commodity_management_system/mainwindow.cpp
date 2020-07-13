#include"mainwindow.h"
#include"cms.h"

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent) {
    setWindowTitle("初始界面");
    setMinimumSize(370, 50);
    QPushButton *alogin_button = new QPushButton("管理员登录");
    QPushButton *ulogin_button = new QPushButton("用户登录");
    QPushButton *ureg_button = new QPushButton("用户注册");
    QHBoxLayout *mwin_layout = new QHBoxLayout;
    mwin_layout->addWidget(alogin_button);
    mwin_layout->addWidget(ulogin_button);
    mwin_layout->addWidget(ureg_button);
    QWidget *mwin_widget = new QWidget;
    mwin_widget->setLayout(mwin_layout);
    setCentralWidget(mwin_widget);
    // lambda表达式, 还有一种方式是SLOT(函数名), 但是不能传参
    connect(alogin_button, &QPushButton::clicked, []{
        cms *alogin = new cms(0);
        delete alogin;
    });
    connect(ulogin_button, &QPushButton::clicked, []{
        cms *ulogin = new cms(1);
        delete ulogin;
    });
    connect(ureg_button, &QPushButton::clicked, []{
        cms *ureg = new cms(2);
        delete ureg;
    });
}

MainWindow::~MainWindow() {}
