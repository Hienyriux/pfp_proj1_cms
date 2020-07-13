#include "cms.h"
#define TB_SEARCH(i,str) ((src[i].id==str)||(src[i].name==str)||(src[i].brand==str))

QString attr_str[5] = {"ID", "名称", "品牌", "价格", "数量"};

cms::cms(int _type, QWidget *parent) : QDialog(parent) {
    type = _type;
    // 管理员界面
    if(type == 0) {
        role = "管理员";
        list_name = "adminlist.txt";
        leave_hint = "进入管理界面";
        cos_button_name = "已售商品";
    }
    // 用户界面
    else if(type == 1){
        role = "用户";
        list_name = "userlist.txt";
        leave_hint = "进入购物";
        cos_button_name = "查看购物车";
    }
    // 用户注册
    else {
        role = "用户";
        list_name = "userlist.txt";
    }
    // 窗口标题
    if(type != 2)
        setWindowTitle(role + "登录");
    else
        setWindowTitle("用户注册");
    QLabel *id_label = new QLabel(role + "名：");
    QLabel *pw_label = new QLabel("密码：");
    QLineEdit *id_edit = new QLineEdit;
    QLineEdit *pw_edit = new QLineEdit;
    id_edit->setPlaceholderText("请输入" + role + "名");
    pw_edit->setPlaceholderText("请输入" + role + "密码");
    QPushButton *login_button = new QPushButton("确定");
    QGridLayout *login_layout = new QGridLayout;
    login_layout->addWidget(id_label, 0, 0, 1, 1);
    login_layout->addWidget(id_edit, 0, 1, 1, 2);
    login_layout->addWidget(pw_label, 1, 0, 1, 1);
    login_layout->addWidget(pw_edit, 1, 1, 1, 2);
    login_layout->addWidget(login_button, 2, 1, 1, 1);
    setLayout(login_layout);
    // lambda表达式中的=表示传值
    connect(login_button, &QPushButton::clicked, [=]{
        login_check(id_edit->text(), pw_edit->text());
    });
    exec();
}

// 登录/注册
void cms::login_check(QString id_str, QString pw_str) {
    // 密码本文件
    QFile list_file(list_name);
    list_file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream list_stream(&list_file);
    bool login_succ = false;
    while(!list_stream.atEnd()) {
        QString id_read, pw_read;
        list_stream >> id_read >> pw_read;
        if(id_read == id_str && pw_read == pw_str) {
            login_succ = true;
            break;
        }
    }
    list_file.close();
    if(login_succ) {
        if(type != 2) {
            QMessageBox::information(this, "登录成功", leave_hint);
            id = id_str;
        }
        else
            QMessageBox::information(this, "注册失败", "该用户已存在");
        accept();
        // 登录成功, 读取数据文件
        if(type != 2)
            file_read();
    }
    else if(type != 2)
        QMessageBox::information(this, "登录失败", "返回登录界面");
    else {
        bool file_exists = list_file.exists();
        list_file.open(QIODevice::Append | QIODevice::Text);
        QTextStream cur_stream(&list_file);
        if(file_exists)
            cur_stream << '\n';
        cur_stream << id_str << ' ' << pw_str;
        list_file.close();
        QMessageBox::information(this, "注册成功", "请返回初始界面登录");
        accept();
    }
}

// 读取数据文件, 管理员读取库存清单(inventory.txt)和已售清单(sold.txt), 用户读取购物车文件
void cms::file_read() {
    // cos = cart_or_sold放一块处理了
    if(type == 0) {
        cos_file_name = "sold.txt";
        save_file_name = "inventory.txt";
    }
    else {
        cos_file_name = id + ".txt";
        save_file_name = cos_file_name;
    }
    QFile rec_file(cos_file_name);
    if(rec_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream rec_file_stream(&rec_file);
        while(!rec_file_stream.atEnd()) {
            commodity tmp_cmdt;
            rec_file_stream >> tmp_cmdt.id >> tmp_cmdt.name >> tmp_cmdt.brand
                    >> tmp_cmdt.price >> tmp_cmdt.num;
            // cart额外有一个user_name
            if(type == 0)
                rec_file_stream >> tmp_cmdt.uname;
            // 数量小于0的不读
            if(tmp_cmdt.num > 0)
                cart_or_sold.push_back(tmp_cmdt);
        }
        rec_file.close();
    }
    // inventory.txt是大家都要读的
    rec_file.setFileName("inventory.txt");
    if(rec_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream rec_file_stream(&rec_file);
        while(!rec_file_stream.atEnd()) {
            commodity tmp_cmdt;
            rec_file_stream >> tmp_cmdt.id >> tmp_cmdt.name >> tmp_cmdt.brand
                    >> tmp_cmdt.price >> tmp_cmdt.num;
            if((type == 0 && tmp_cmdt.num >= 0) || (type == 1 && tmp_cmdt.num > 0))
                ivtr.push_back(tmp_cmdt);
        }
        rec_file.close();
    }
    // 把ivtr_widget填充好
    table_proc(ivtr, ivtr_widget, ivtr_item);
    if(type == 0) {
        // 先排序, 再合并同id, 同价格的项
        std::sort(cart_or_sold.begin(), cart_or_sold.end(), [](const commodity &a, const commodity &b) {
            if(a.id != b.id)
                return a.id < b.id;
            return a.price < b.price;
        });
        QVector<commodity> tmp_vector;
        for(int i = 0; i < cart_or_sold.size(); i++) {
            int j = i + 1, num = cart_or_sold[i].num;
            for(; j < cart_or_sold.size(); j++) {
                if(cart_or_sold[j].id == cart_or_sold[i].id
                        && cart_or_sold[j].price == cart_or_sold[i].price) {
                    num += cart_or_sold[j].num;
                }
                else
                    break;
            }
            cart_or_sold[i].num = num;
            tmp_vector.push_back(cart_or_sold[i]);
            i = j - 1;
        }
        cart_or_sold.clear();
        for(int i = 0; i < tmp_vector.size(); i++)
            cart_or_sold.push_back(tmp_vector[i]);
        table_proc(cart_or_sold, cos_widget, cos_item);
    }
    else {
        for(int i = 0; i < cart_or_sold.size(); i++) {
            int j = 0;
            for(; j < ivtr.size() && cart_or_sold[i].id != ivtr[j].id; j++);
            // 库存中不存在的商品不会显示在购物车中
            if(j == ivtr.size())
                cart_or_sold.erase(cart_or_sold.begin() + i);
        }
        table_proc(cart_or_sold, cos_widget, cos_item);
    }
    QPushButton *ivtr_button = new QPushButton("查看商品");
    QPushButton *cos_button = new QPushButton(cos_button_name);
    QHBoxLayout *cur_layout = new QHBoxLayout;
    cur_layout->addWidget(ivtr_button);
    cur_layout->addWidget(cos_button);
    QDialog *cur_dialog = new QDialog;
    cur_dialog->setWindowTitle(id);
    cur_dialog->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    cur_dialog->setLayout(cur_layout);
    cur_dialog->setMinimumWidth(400);
    connect(ivtr_button, &QPushButton::clicked, [=]{
        // 管理员的库存清单界面
        if(type == 0)
            aivtr_table();
        // 用户的库存清单界面
        else
            u_table(2);
    });
    connect(cos_button, &QPushButton::clicked, [=]{
        // 管理员的已售清单界面
        if(type == 0)
            asold_table();
        // 用户的购物车界面
        else
            u_table(3);
    });
    // 退出前保存文件
    connect(cur_dialog, SIGNAL(rejected()), SLOT(file_save()));
    cur_dialog->exec();
}

void cms::file_save() {
    // 管理员/用户都需要保存inventory.txt
    QFile cur_file("inventory.txt");
    cur_file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream ivtr_stream(&cur_file);
    for(int i = 0; i < ivtr.size(); i++) {
        ivtr_stream << ivtr[i].id << ' ' << ivtr[i].name << ' ' << ivtr[i].brand << ' '
                     << ivtr[i].price << ' ' << ivtr[i].num;
        if(i < ivtr.size() - 1)
            ivtr_stream << '\n';
    }
    cur_file.close();
    // 用户还需要保存购物车数据
    if(type == 1) {
        cur_file.setFileName(save_file_name);
        cur_file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream cur_stream(&cur_file);
        for(int i = 0; i < cart_or_sold.size(); i++) {
            cur_stream << cart_or_sold[i].id << ' ' << cart_or_sold[i].name << ' '
                       << cart_or_sold[i].brand << ' ' << cart_or_sold[i].price << ' '
                       << cart_or_sold[i].num;
            if(i < cart_or_sold.size() - 1)
                cur_stream << '\n';
        }
        cur_file.close();
    }
}

// 行填充
void cms::row_making(QVector<QTableWidgetItem> &tmp_tbwi, QString _id, QString _name,
                     QString _brand, double _price, int _num) {
    QTableWidgetItem temp_tbwi(_id);
    tmp_tbwi.push_back(temp_tbwi);
    temp_tbwi.setText(_name);
    tmp_tbwi.push_back(temp_tbwi);
    temp_tbwi.setText(_brand);
    tmp_tbwi.push_back(temp_tbwi);
    temp_tbwi.setText(QString::number(_price));
    tmp_tbwi.push_back(temp_tbwi);
    temp_tbwi.setText(QString::number(_num));
    tmp_tbwi.push_back(temp_tbwi);
}

// 表格填充
void cms::table_proc(QVector<commodity> &src, QTableWidget *&cur_widget,
                     QVector<QVector<QTableWidgetItem>> &cur_item) {
    search_pos = -1;
    int tb_row = src.size() + 1;
    cur_widget = new QTableWidget(tb_row - 1, 5);
    QVector<QTableWidgetItem> tmp_tbwi;
    // 表头
    for(int i = 0; i < 5; i++) {
        QTableWidgetItem temp_tbwi(attr_str[i]);
        tmp_tbwi.push_back(temp_tbwi);
    }
    cur_item.push_back(tmp_tbwi);
    // 表头
    for(int i = 0; i < 5; i++)
        cur_widget->setHorizontalHeaderItem(i, &cur_item[0][i]);
    for(int i = 1; i < tb_row; i++) {
        QVector<QTableWidgetItem> tmp_tbwi;
        row_making(tmp_tbwi, src[i - 1].id, src[i - 1].name, src[i - 1].brand,
                src[i - 1].price, src[i - 1].num);
        cur_item.push_back(tmp_tbwi);
    }
    for(int i = 1; i < tb_row; i++) {
        for(int j = 0; j < 5; j++) {
            cur_item[i][j].setTextAlignment(Qt::AlignCenter);
            cur_widget->setItem(i - 1, j, &cur_item[i][j]);
        }
    }
    // 使行可选, 不可编辑
    cur_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cur_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
}

// 表格项搜索, 可以搜ID, 名称, 品牌
void cms::table_search(QVector<commodity> &src, QTableWidget *&src_widget, QString search_str) {
    // 确定没搜到时是否需要重新从开头处搜索, 有时, 搜索目标在当前选中位置的上方
    // research这个单词有歧义
    bool need_research = false;
    if(search_pos != -1 && search_pos < src.size() && TB_SEARCH(search_pos, search_str)) {
        search_pos++;
        if(search_pos == src.size())
            search_pos = 0;
        else
            need_research = true;
    }
    else
        search_pos = 0;
    for(; search_pos < src.size() && !TB_SEARCH(search_pos, search_str); search_pos++);
    if(search_pos == src.size()) {
        if(need_research) {
            for(search_pos = 0; search_pos < src.size() && !TB_SEARCH(search_pos, search_str);
                search_pos++);
            if(search_pos == src.size())
                QMessageBox::information(this, " ", "未找到该商品");
            else
                src_widget->setCurrentCell(search_pos, 0);
        }
        else
            QMessageBox::information(this, " ", "未找到该商品");
    }
    else
        src_widget->setCurrentCell(search_pos, 0);
}

// 管理员库存清单
void cms::aivtr_table() {
    QPushButton *leftup1_button = new QPushButton("添加");
    QPushButton *leftup2_button = new QPushButton("删除");
    QPushButton *leftup3_button = new QPushButton("修改");
    QLineEdit *search_edit = new QLineEdit;
    search_edit->setPlaceholderText("搜索商品");
    QPushButton *search_button = new QPushButton("搜索");
    //search_edit->setFixedHeight(32);
    QHBoxLayout *tool_layout = new QHBoxLayout;
    tool_layout->addWidget(leftup1_button);
    tool_layout->addWidget(leftup2_button);
    tool_layout->addWidget(leftup3_button);
    tool_layout->addSpacing(400);
    tool_layout->addWidget(search_edit);
    tool_layout->addWidget(search_button);
    QVBoxLayout *mytb_layout = new QVBoxLayout;
    mytb_layout->addLayout(tool_layout);
    mytb_layout->addWidget(ivtr_widget);
    QDialog *mytb_dialog = new QDialog;
    mytb_dialog->setWindowTitle("库存清单");
    mytb_dialog->setLayout(mytb_layout);
    mytb_dialog->setMinimumSize(ivtr_widget->horizontalHeader()->length() + 100,
                                ivtr_widget->verticalHeader()->length() + 150);
    // 需要传参的, 都用lambda表达式
    // 设置搜索功能
    connect(search_button, &QPushButton::clicked, [=]{
        table_search(ivtr, ivtr_widget, search_edit->text());
    });
    // 不需要传参的, 可以使用SLOT
    // 增删改
    connect(leftup1_button, SIGNAL(clicked(bool)), SLOT(atable_insert()));
    connect(leftup2_button, SIGNAL(clicked(bool)), SLOT(atable_del()));
    connect(leftup3_button, SIGNAL(clicked(bool)), SLOT(atable_modify()));
    mytb_dialog->exec();
}

// 管理员增加库存商品
void cms::atable_insert() {
    int cur_row = ivtr_widget->currentRow();
    if(cur_row < 0 || cur_row > ivtr.size())
        cur_row = ivtr.size() - 1;
    QDialog *insert_dialog = new QDialog;
    insert_dialog->setWindowTitle("添加商品");
    QLabel *insert_id_label = new QLabel("ID：");
    QLabel *insert_name_label = new QLabel("名称：");
    QLabel *insert_brand_label = new QLabel("品牌：");
    QLabel *insert_price_label = new QLabel("价格：");
    QLabel *insert_num_label = new QLabel("数量：");
    QLineEdit *insert_id_edit = new QLineEdit;
    QLineEdit *insert_name_edit = new QLineEdit;
    QLineEdit *insert_brand_edit = new QLineEdit;
    QDoubleSpinBox *insert_price_spin = new QDoubleSpinBox;
    insert_price_spin->setMaximum(10000.0);
    QSpinBox *insert_num_spin = new QSpinBox;
    insert_num_spin->setMaximum(10000);
    QPushButton *confirm1_button = new QPushButton("添加到选中位置前");
    QPushButton *confirm2_button = new QPushButton("添加到选中位置后");
    QGridLayout *insert_layout = new QGridLayout;
    insert_layout->addWidget(insert_id_label, 0, 0, 1, 2);
    insert_layout->addWidget(insert_id_edit, 0, 2, 1, 2);
    insert_layout->addWidget(insert_name_label, 1, 0, 1, 2);
    insert_layout->addWidget(insert_name_edit, 1, 2, 1, 2);
    insert_layout->addWidget(insert_brand_label, 2, 0, 1, 2);
    insert_layout->addWidget(insert_brand_edit, 2, 2, 1, 2);
    insert_layout->addWidget(insert_price_label, 3, 0, 1, 2);
    insert_layout->addWidget(insert_price_spin, 3, 2, 1, 2);
    insert_layout->addWidget(insert_num_label, 4, 0, 1, 2);
    insert_layout->addWidget(insert_num_spin, 4, 2, 1, 2);
    insert_layout->addWidget(confirm1_button, 5, 0, 1, 2);
    insert_layout->addWidget(confirm2_button, 5, 2, 1, 2);
    insert_dialog->setLayout(insert_layout);
    connect(confirm1_button, &QPushButton::clicked, [=]{
        insert_proc(cur_row, insert_id_edit->text(), insert_name_edit->text(),
                    insert_brand_edit->text(), insert_price_spin->value(), insert_num_spin->value());
        insert_dialog->accept();
    });
    connect(confirm2_button, &QPushButton::clicked, [=]{
        insert_proc(cur_row + 1, insert_id_edit->text(), insert_name_edit->text(),
                    insert_brand_edit->text(), insert_price_spin->value(), insert_num_spin->value());
        insert_dialog->accept();
    });
    insert_dialog->exec();
}

// 增加商品的表格填充操作
void cms::insert_proc(int cur_row, QString _id, QString _name, QString _brand, double _price, int _num) {
    commodity tmp_cmdt;
    tmp_cmdt.id = _id;
    tmp_cmdt.name = _name;
    tmp_cmdt.brand = _brand;
    tmp_cmdt.price = _price;
    tmp_cmdt.num = _num;
    ivtr.insert(cur_row, tmp_cmdt);
    ivtr_widget->insertRow(cur_row);
    QVector<QTableWidgetItem> tmp_tbwi;
    row_making(tmp_tbwi, ivtr[cur_row].id, ivtr[cur_row].name, ivtr[cur_row].brand,
               ivtr[cur_row].price, ivtr[cur_row].num);
    ivtr_item.insert(cur_row + 1, tmp_tbwi);
    for(int i = 0; i < 5; i++) {
        ivtr_item[cur_row + 1][i].setTextAlignment(Qt::AlignCenter);
        ivtr_widget->setItem(cur_row, i, &ivtr_item[cur_row + 1][i]);
    }
}

// 管理员删除库存商品
void cms::atable_del() {
    int cur_row = ivtr_widget->currentRow();
    if(cur_row < 0 || cur_row > ivtr.size())
        return;
    QDialog *del_dialog = new QDialog;
    del_dialog->setWindowTitle(" ");
    QLabel *del_label = new QLabel("确定要删除该商品吗");
    QPushButton *confirm_button = new QPushButton("确定");
    QPushButton *cancel_button = new QPushButton("取消");
    QGridLayout *del_layout = new QGridLayout;
    del_layout->addWidget(del_label, 0, 0, 1, 2);
    del_layout->addWidget(confirm_button, 1, 0, 1, 1);
    del_layout->addWidget(cancel_button, 1, 1, 1, 1);
    del_dialog->setLayout(del_layout);
    connect(confirm_button, &QPushButton::clicked, [=]{
        ivtr.erase(ivtr.begin() + cur_row);
        ivtr_widget->removeRow(cur_row);
        ivtr_item.erase(ivtr_item.begin() + cur_row + 1);
        del_dialog->accept();
    });
    connect(cancel_button, &QPushButton::clicked, [=]{
        del_dialog->accept();
    });
    del_dialog->exec();
}

// 管理员修改库存商品
void cms::atable_modify() {
    int cur_row = ivtr_widget->currentRow();
    if(cur_row < 0 || cur_row > ivtr.size())
        return;
    QDialog *modify_dialog = new QDialog;
    modify_dialog->setWindowTitle("修改商品");
    QLabel *modify_id_label = new QLabel("ID：");
    QLabel *modify_name_label = new QLabel("名称：");
    QLabel *modify_brand_label = new QLabel("品牌：");
    QLabel *modify_price_label = new QLabel("价格：");
    QLabel *modify_num_label = new QLabel("数量：");
    QLineEdit *modify_id_edit = new QLineEdit(ivtr[cur_row].id);
    QLineEdit *modify_name_edit = new QLineEdit(ivtr[cur_row].name);
    QLineEdit *modify_brand_edit = new QLineEdit(ivtr[cur_row].brand);
    QDoubleSpinBox *modify_price_spin = new QDoubleSpinBox;
    modify_price_spin->setMaximum(10000.0);
    modify_price_spin->setValue(ivtr[cur_row].price);
    QSpinBox *modify_num_spin = new QSpinBox;
    modify_num_spin->setMaximum(10000);
    modify_num_spin->setValue(ivtr[cur_row].num);
    QPushButton *confirm_button = new QPushButton("确定");
    QGridLayout *modify_layout = new QGridLayout;
    modify_layout->addWidget(modify_id_label, 0, 0, 1, 2);
    modify_layout->addWidget(modify_id_edit, 0, 2, 1, 2);
    modify_layout->addWidget(modify_name_label, 1, 0, 1, 2);
    modify_layout->addWidget(modify_name_edit, 1, 2, 1, 2);
    modify_layout->addWidget(modify_brand_label, 2, 0, 1, 2);
    modify_layout->addWidget(modify_brand_edit, 2, 2, 1, 2);
    modify_layout->addWidget(modify_price_label, 3, 0, 1, 2);
    modify_layout->addWidget(modify_price_spin, 3, 2, 1, 2);
    modify_layout->addWidget(modify_num_label, 4, 0, 1, 2);
    modify_layout->addWidget(modify_num_spin, 4, 2, 1, 2);
    modify_layout->addWidget(confirm_button, 5, 1, 1, 2);
    modify_dialog->setLayout(modify_layout);
    connect(confirm_button, &QPushButton::clicked, [=]{
        ivtr[cur_row].id = modify_id_edit->text();
        ivtr[cur_row].name = modify_name_edit->text();
        ivtr[cur_row].brand = modify_brand_edit->text();
        ivtr[cur_row].price = modify_price_spin->value();
        ivtr[cur_row].num = modify_num_spin->value();
        ivtr_item[cur_row + 1][0].setText(ivtr[cur_row].id);
        ivtr_item[cur_row + 1][1].setText(ivtr[cur_row].name);
        ivtr_item[cur_row + 1][2].setText(ivtr[cur_row].brand);
        ivtr_item[cur_row + 1][3].setText(QString::number(ivtr[cur_row].price));
        ivtr_item[cur_row + 1][4].setText(QString::number(ivtr[cur_row].num));
        modify_dialog->accept();
    });
    modify_dialog->exec();
}

// 管理员的已售清单, 有搜索功能, 不能增删改
void cms::asold_table() {
    QLineEdit *search_edit = new QLineEdit;
    search_edit->setPlaceholderText("搜索商品");
    search_edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QPushButton *search_button = new QPushButton("搜索");
    search_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //search_edit->setFixedHeight(32);
    QHBoxLayout *tool_layout = new QHBoxLayout;
    tool_layout->setDirection(QBoxLayout::RightToLeft);
    tool_layout->addWidget(search_button);
    tool_layout->addWidget(search_edit);
    tool_layout->addStretch();
    QVBoxLayout *mytb_layout = new QVBoxLayout;
    mytb_layout->addLayout(tool_layout);
    mytb_layout->addWidget(cos_widget);
    QDialog *mytb_dialog = new QDialog;
    mytb_dialog->setWindowTitle("已售商品");
    mytb_dialog->setLayout(mytb_layout);
    mytb_dialog->setMinimumSize(cos_widget->horizontalHeader()->length() + 100,
                                cos_widget->verticalHeader()->length() + 150);
    connect(search_button, &QPushButton::clicked, [=]{
        table_search(cart_or_sold, cos_widget, search_edit->text());
    });
    mytb_dialog->exec();
}

// 用户的库存清单/购物车界面
void cms::u_table(int mode) {
    QString action_str, title_str, hint_str;
    mymode = mode;
    if(mode == 2) {
        action_str = "购买";
        title_str = "库存清单";
        hint_str = "请选择购买数量";
    }
    else {
        action_str = "移除";
        title_str = "购物车";
        hint_str = "请选择移除数量";
    }
    // 引用类型的初始化, 如果要分情况讨论, 必须使用三目表达式
    // 库存清单与购物车中的商品守恒(在没有结账之前), 可以互为源/目的
    QVector<commodity> &src = (mode == 2) ? ivtr : cart_or_sold;
    QVector<commodity> &dst = (mode == 2) ? cart_or_sold : ivtr;
    QTableWidget *&src_widget = (mode == 2) ? ivtr_widget : cos_widget;
    QPushButton *leftup1_button = new QPushButton(action_str);
    QPushButton *leftup2_button = NULL;
    // 购物车界面多一个结账选项
    if(mode == 3)
        leftup2_button = new QPushButton("结账");
    QLineEdit *search_edit = new QLineEdit;
    search_edit->setPlaceholderText("搜索商品");
    QPushButton *search_button = new QPushButton("搜索");
    //search_edit->setFixedHeight(32);
    QHBoxLayout *tool_layout = new QHBoxLayout;
    tool_layout->addWidget(leftup1_button);
    if(mode == 3)
        tool_layout->addWidget(leftup2_button);
    tool_layout->addSpacing(400);
    tool_layout->addWidget(search_edit);
    tool_layout->addWidget(search_button);
    QVBoxLayout *mytb_layout = new QVBoxLayout;
    mytb_layout->addLayout(tool_layout);
    mytb_layout->addWidget(src_widget);
    QDialog *mytb_dialog = new QDialog;
    mytb_dialog->setWindowTitle(title_str);
    mytb_dialog->setLayout(mytb_layout);
    mytb_dialog->setMinimumSize(src_widget->horizontalHeader()->length() + 100,
                                src_widget->verticalHeader()->length() + 150);
    // 搜索功能
    connect(search_button, &QPushButton::clicked, [&]{
        table_search(src, src_widget, search_edit->text());
    });
    // 既处理库存清单->购物车, 又处理购物车->库存清单的商品转移
    connect(leftup1_button, &QPushButton::clicked, [&]{
        table_buydel(action_str, hint_str, src, dst);
    });
    // 结账
    if(mode == 3)
        connect(leftup2_button, SIGNAL(clicked(bool)), SLOT(user_checkout()));
    mytb_dialog->exec();
}

// 购买/移除询问界面
void cms::table_buydel(QString title_str, QString hint_str,
                       QVector<commodity> &src, QVector<commodity> &dst) {
    QTableWidget *&src_widget = (mymode == 2) ? ivtr_widget : cos_widget;
    int cur_row = src_widget->currentRow();
    if(cur_row < 0 || cur_row > src.size())
        return;
    QDialog *buydel_dialog = new QDialog;
    buydel_dialog->setWindowTitle(title_str);
    QGridLayout *buydel_layout = new QGridLayout;
    QLabel *buydel_label = new QLabel(hint_str);
    QSpinBox *buydel_spin = new QSpinBox;
    buydel_spin->setRange(0, src[cur_row].num);
    QPushButton *confirm_button = new QPushButton("确定");
    buydel_layout->addWidget(buydel_label, 0, 0, 1, 2);
    buydel_layout->addWidget(buydel_spin, 0, 2, 1, 1);
    buydel_layout->addWidget(confirm_button, 1, 1, 1, 1);
    buydel_dialog->setLayout(buydel_layout);
    buydel_dialog->setMinimumWidth(300);
    connect(confirm_button, &QPushButton::clicked, [&]{
        ivtr_cart_proc(cur_row, buydel_spin->value(), src, dst);
        buydel_dialog->accept();
    });
    buydel_dialog->exec();
}

// 处理商品转移
void cms::ivtr_cart_proc(int cur_row, int num, QVector<commodity> &src, QVector<commodity> &dst) {
    QTableWidget *&src_widget = (mymode == 2) ? ivtr_widget : cos_widget;
    QVector<QVector<QTableWidgetItem>> &src_item = (mymode == 2) ? ivtr_item : cos_item;
    QTableWidget *&dst_widget = (mymode == 2) ? cos_widget : ivtr_widget;
    QVector<QVector<QTableWidgetItem>> &dst_item = (mymode == 2) ? cos_item : ivtr_item;
    if(num <= 0)
        return;
    int i = 0;
    for(; i < dst.size() && src[cur_row].id != dst[i].id; i++);
    if(i == dst.size()) {
        commodity tmp_cmdt = src[cur_row];
        tmp_cmdt.num = num;
        dst.push_back(tmp_cmdt);
        dst_widget->insertRow(i);
        QVector<QTableWidgetItem> tmp_tbwi;
        row_making(tmp_tbwi, tmp_cmdt.id, tmp_cmdt.name, tmp_cmdt.brand, tmp_cmdt.price, tmp_cmdt.num);
        dst_item.push_back(tmp_tbwi);
        for(int j = 0; j < 5; j++) {
            dst_item[i + 1][j].setTextAlignment(Qt::AlignCenter);
            dst_widget->setItem(i, j, &dst_item[i + 1][j]);
        }
    }
    else {
        dst[i].num += num;
        dst_item[i + 1][4].setText(QString::number(dst[i].num));
    }
    src[cur_row].num -= num;
    if(src[cur_row].num == 0) {
        src.erase(src.begin() + cur_row);
        src_widget->removeRow(cur_row);
        src_item.erase(src_item.begin() + cur_row + 1);
    }
    else
        src_item[cur_row + 1][4].setText(QString::number(src[cur_row].num));
}

// 结账界面
void cms::user_checkout() {
    QDialog *checkout_dialog = new QDialog;
    checkout_dialog->setWindowTitle("结账");
    double sum = 0.0;
    for(int i = 0; i < cart_or_sold.size(); i++)
        sum += cart_or_sold[i].price * cart_or_sold[i].num;
    QString checkout_str = "总计：" + QString::number(sum) + "元";
    QLabel *checkout_label = new QLabel(checkout_str);
    QPushButton *checkout_confirm = new QPushButton("结账");
    QPushButton *checkout_cancel = new QPushButton("取消");
    QGridLayout *checkout_layout = new QGridLayout;
    checkout_layout->addWidget(checkout_label, 0, 0, 1, 4);
    checkout_layout->addWidget(checkout_confirm, 1, 0, 1, 1);
    checkout_layout->addWidget(checkout_cancel, 1, 3, 1, 1);
    checkout_dialog->setLayout(checkout_layout);
    connect(checkout_confirm, &QPushButton::clicked, [=]{
        file_checkout();
        checkout_dialog->accept();
    });
    connect(checkout_cancel, &QPushButton::clicked, [=]{
        checkout_dialog->accept();
    });
    checkout_dialog->exec();
}

// 写入已售清单文件, 清空购物车文件
void cms::file_checkout() {
    QFile cur_file("sold.txt");
    bool file_exists = cur_file.exists();
    cur_file.open(QIODevice::Append | QIODevice::Text);
    if(cart_or_sold.size() > 0) {
        QTextStream sold_stream(&cur_file);
        if(file_exists)
            sold_stream << '\n';
        for(int i = 0; i < cart_or_sold.size(); i++) {
            sold_stream << cart_or_sold[i].id << ' ' << cart_or_sold[i].name<< ' '
                        << cart_or_sold[i].brand << ' ' << cart_or_sold[i].price << ' '
                        << cart_or_sold[i].num << ' ' << id;
            if(i < cart_or_sold.size() - 1)
                sold_stream << '\n';
        }
    }
    cur_file.close();
    cur_file.setFileName(id + ".txt");
    if(cur_file.exists())
        cur_file.remove();
    cart_or_sold.clear();
    cos_widget->clearContents();
    cos_item.erase(cos_item.begin() + 1, cos_item.end());
}
