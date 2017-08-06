//
//  Tool.cpp
//  MultiWayMergeSort
//
//  Created by 孙硕磊 on 11/2/14.
//  Copyright (c) 2014 dhu.cst. All rights reserved.
//

#include "Tool.h"

/**
 *  convert int to string
 */
string int2String(int ival)
{
    string str;
    ostringstream ostream;
    ostream<<ival;
    istringstream istream(ostream.str());
    istream>>str;
    return str;
}
