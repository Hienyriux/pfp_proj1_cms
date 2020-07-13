#ifndef COMMODITY_H
#define COMMODITY_H

#include<QString>

struct commodity {
    // 商品ID
    QString id;
    // 商品名
    QString name;
    // 商品品牌
    QString brand;
    // 商品价格
    double price;
    // 商品数量
    int num;
    // 用户名（可选）
    QString uname;
};

#endif // COMMODITY_H
