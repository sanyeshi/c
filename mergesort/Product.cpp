//
//  Product.cpp
//  MultiWayMergeSort
//
//  Created by 孙硕磊 on 11/1/14.
//  Copyright (c) 2014 dhu.cst. All rights reserved.
//

#include "Product.h"
#include <string>
#include "Tool.h"
using std::string;

/**
 *  constructor
 */

Product::Product()
{
   this->setName();
}

Product::Product(unsigned int Id,int price)
{
    this->setId(Id);
    this->price=price;
    this->setName();
}

/**
 *  set Product id
 */
void Product::setId(unsigned int Id)
{
    this->Id[8]='\0';
    int i=7;
    while (Id>0&&i>=0)
    {
        this->Id[i--]=Id%10+48;
        Id=Id/10;
    }
    while(i>=0)
    {
        this->Id[i--]=48;
    }
}

void Product::setId(Product product)
{
    int i=0;
    for (i=0; i<8; i++)
    {
        this->Id[i]=product.Id[i];
    }
    this->Id[i]='\0';
}

/**
 *  set Product  default name xxx...xxx
 */
void Product::setName()
{
    for (int i=0; i<85; i++)
    {
        this->name[i]='x';
    }
    this->name[85]='\0';
}

string Product::getProductInStrFormat()
{
    return string(this->Id)+" "+int2String(this->price)+" "+string(this->name)+"\n";
}