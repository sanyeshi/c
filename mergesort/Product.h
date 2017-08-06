//
//  Product.h
//  MultiWayMergeSort
//
//  Created by 孙硕磊 on 11/1/14.
//  Copyright (c) 2014 dhu.cst. All rights reserved.
//

#ifndef __MultiWayMergeSort__Product__
#define __MultiWayMergeSort__Product__

#include <iostream>

class Product
{
    /*
        when objects are written to file,properties are separated by blank ' ' and objects are separated by ‘\n’,so every object will take 100 bytes.
     */
public:
    char Id[9];      //product id,8 Bytes and '\0'
    int price;     //product price, 4 Bytes
    char name[86];   //product name,  85 Bytes and '\0'
    
public:
    Product();
    Product(unsigned int Id,int price);
    void setId(unsigned int Id);
    void setId(Product product);
    void setName();
    std::string getProductInStrFormat();
    
};

#endif /* defined(__MultiWayMergeSort__Product__) */
