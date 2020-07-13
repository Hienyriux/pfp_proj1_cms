#ifndef CMS_H
#define CMS_H

#include"commodity.h"
#include<QDebug>
#include<QDialog>
#include<QDoubleSpinBox>
#include<QFile>
#include<QHeaderView>
#include<QHBoxLayout>
#include<QLabel>
#include<QLineEdit>
#include<QMessageBox>
#include<QPushButton>
#include<QGridLayout>
#include<QSpinBox>
#include<QTableWidget>
#include<QTableWidgetItem>
#include<QTextStream>
#include<QVBoxLayout>

class cms : public QDialog {
    Q_OBJECT
    // 0 = 管理员/1 = 用户
    int type;
    // 管理员/用户
    QString role;
    // admin1/user1
    QString id;
    // adminlist.txt/userlist.txt
    QString list_name;
    // 进入管理界面/进入购物
    QString leave_hint;
    // sold.txt/user1.txt
    QString cos_file_name;
    // 已售商品/查看购物车
    QString cos_button_name;
    // inventory.txt/user1.txt
    QString save_file_name;
    QVector<commodity> ivtr;
    QVector<commodity> cart_or_sold;
    QTableWidget *ivtr_widget;
    QTableWidget *cos_widget;
    QVector<QVector<QTableWidgetItem>> ivtr_item;
    QVector<QVector<QTableWidgetItem>> cos_item;
    int search_pos;
    int mymode;
public:
    cms(int _type, QWidget *parent = 0);
    ~cms() {}
    void login_check(QString, QString);
    void file_read();
    void row_making(QVector<QTableWidgetItem> &, QString, QString, QString, double, int);
    void table_proc(QVector<commodity> &,  QTableWidget *&, QVector<QVector<QTableWidgetItem>> &);
    void table_search(QVector<commodity> &, QTableWidget *&, QString);
    void aivtr_table();
    void insert_proc(int, QString, QString, QString, double, int);
    void asold_table();
    void table_buydel(QString, QString, QVector<commodity> &, QVector<commodity> &);
    void ivtr_cart_proc(int, int, QVector<commodity> &, QVector<commodity> &);
    void u_table(int);
    void file_checkout();
public slots:
    void file_save();
    void atable_insert();
    void atable_del();
    void atable_modify();
    void user_checkout();
};

#endif // CMS_H
