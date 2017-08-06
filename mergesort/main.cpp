//
//  main.cpp
//  MultiWayMergeSort
//
//  Created by 孙硕磊 on 11/1/14.
//  Copyright (c) 2014 dhu.cst. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include "Product.h"
#include "Tool.h"

using std::string;
using std::ofstream;
using std::ifstream;
using std::istringstream;
using std::ostringstream;

/*
 Assume that RAM only has 50MB(50 * 1000 * 1000 bytes) and every object takes 100 bytes,
 so we can load 50*1000*10 objects to RAM at a time.There are 10 *1000*1000 objects in data file,
 so we must load 20 times.
 
 we also assume that every input buffer takes 2M(2*1000*1000 bytes=500pages,there are totally 20 input buffers),
 and the only one output buffer takes 10M(10 *1000*1000 bytes).
 */
#define kPageSize                4000
#define kRecordCount             10000000
#define kMaxRAMRecordCount       500000
#define kLoadTimes               20
#define kInputBufferRecordCount  20000
#define kOutputBufferRecordCount 100000
#define random() (rand()%9000+1000) //generate random number 1000-9999


struct InputBuffer
{
    ifstream infile;         //associate file
    Product * data;          //data buffer
    int count;               //max record count
    int currentIndex;        //data current index
    bool isSubfileReadOver;  //indicate the associate file is read completely
    InputBuffer * next;
};

//declare function
int  readDataToInputBuffer(ifstream &infile,int count,Product * data);

/**
 * free an input buffer
 */
void freeInputBuffer(InputBuffer * inputBuffer)
{
    if (inputBuffer==NULL)
    {
        return;
    }
    inputBuffer->infile.close();
    if (inputBuffer->data!=NULL)
    {
        delete [] inputBuffer->data;
    }
    inputBuffer->next=NULL;
    delete inputBuffer;
}

/**
 *  if head is null,return YES,otherwise true
 **/
bool isInputBuffersEmpty(InputBuffer * head)
{
    if(head==NULL)
    {
        return true;
    }
    if (head->next==NULL)
    {
        return true;
    }
    return false;
}

/**
 *  delete node form the head pointed linkedlist
 */
void deleteInputBufferFromInpuBuffers(InputBuffer * head,InputBuffer * inputbuffer)
{
    if (head->next==NULL||inputbuffer==NULL)
    {
        return;
    }
    InputBuffer * p=head->next;
    InputBuffer * q=head;
    while(p!=inputbuffer&&p!=NULL)
    {
        q=p;
        p=p->next;
    }
    q->next=p->next;
    p->next=NULL;
    freeInputBuffer(p);
}

/**
 * insert an inputbuffer after head
 */
void insertInputBufferToInputBuffers(InputBuffer * head,InputBuffer * inputbuffer)
{
    if (head==NULL||inputbuffer==NULL)
    {
        return;
    }
    inputbuffer->next=head->next;
    head->next=inputbuffer;
}


/**
 * get minimum price associated product form inputbuffers
 */
Product getMinProduct(InputBuffer * head)
{
    Product product;
    if (head==NULL||head->next==NULL)
    {
        product.price=-1;
        return product;
    }
    InputBuffer * p=head->next;
    InputBuffer * q=p;
    float minPrice=p->data[p->currentIndex].price;
    while (p!=NULL)
    {
        float price=p->data[p->currentIndex].price;
        if (minPrice>price)
        {
            q=p;
            minPrice=price;
        }
        p=p->next;
    }
    product.setId(q->data[q->currentIndex]);
    product.price=minPrice;
    product.setName();
    q->currentIndex++;
    if (q->currentIndex>=q->count)           //retrieve data
    {
        if(readDataToInputBuffer(q->infile, q->count, q->data))
        {
            q->currentIndex=0;
        }
        else                                // file is readed completely
        {
            deleteInputBufferFromInpuBuffers(head, q);
        }
    }
    return  product;
}

/**
 *  create kRecordCont product objects,and write them to file
 */
void generateDataFile(const std::string filename)
{
    ofstream outfile(filename);
    srand((int)time(0));
    for(unsigned int i=0;i<kRecordCount; i++)
    {
        //generate random number 1000-9999 as product price
        Product product(i,random());
        //write to file
        outfile<<product.Id<<" "<<product.price<<" "<<product.name<<"\n";
    }
    outfile.close();
}

/**
 *  compare product object by price
 */
int qcmp(const void * a,const void * b)
{
    return (*(Product *)a).price-(*(Product *)b).price;
}


/*
 *
 *  @param infile file to read
 *  @param count  record count to read
 *  @param data   save records

 *  @return  real readed counts
 */
int  readDataToInputBuffer(ifstream &infile,int count,Product * data)
{
    //read by page 4KB(4*1000 bytes),40 records take one page,read time=(count-1)/40+1
    char buffer[kPageSize+1];
    buffer[kPageSize]='\0';
    int  readTimes=(count-1)/40+1;
    int i=0,j=0;
    string line;
    for (i=0; i<readTimes; i++)
    {
        infile.read(buffer, kPageSize);
        if (infile.gcount()==0)     //文件结束
        {
            return 0;
        }
        istringstream istrstreamOfBuffer(buffer);
        while(getline(istrstreamOfBuffer,line))
        {
            istringstream istrstream(line);
            int  ID;
            float  price;
            istrstream>>ID>>price;                                    //get ID,price
            data[j].setId(ID);
            data[j].price=price;
            j++;
        }
    }
       return j;
}


/**
 *
 *  @param outfile file to write
 *  @param count   recount to write
 *  @param data    records data
 */
void writeOutputBufferDataToFile(ofstream &outfile,int count,Product * data)
{
    
    //write by page 4KB(4*1000 bytes),40 records take one page
    /*
    char buffer[kPageSize+1];
    buffer[kPageSize]='\0';
    int currentIndex=0;
    for (int i=1; i<=count; i++)
    {
        string info=data[i].getProductInStrFormat();
        for(int j=0; j<info.length()&&currentIndex<kPageSize; j++)
        {
            buffer[currentIndex]=info[j];
            currentIndex++;
        }
        if (i%40==0)
        {
            outfile.write(buffer, kPageSize);
            currentIndex=0;
        }
    }
     */
    for (int i=0;i<count;i++)
    {
        outfile<<data[i].Id<<" "<<data[i].price<<" "<<data[i].name<<"\n";
    }
    
}

void  multiwayMergeSort(const string srcDataFilename,const string dstDataFilename)
{
    //phase 1,generate ordered sublists
    Product *products=new Product[kMaxRAMRecordCount];
    string tempFilenameBasic="/Users/ssl/temp";                         //temporary file name
    ifstream infile(srcDataFilename);                                   //read data file
    for (int i=0; i<kLoadTimes; i++)
    {
        readDataToInputBuffer(infile, kMaxRAMRecordCount, products);
        qsort(products,kMaxRAMRecordCount, sizeof(products[0]), qcmp); //quick sort
        ofstream outfile(tempFilenameBasic+int2String(i)+".txt");      //write ordered sublist to temp file
        writeOutputBufferDataToFile(outfile, kMaxRAMRecordCount, products);
        outfile.close();
    }
    infile.close();
    delete [] products;
    
    // phase 2,multiway sort
    //init inputbuffers
    InputBuffer * head= new InputBuffer;
    for (int i=0; i<kLoadTimes; i++)
    {
        InputBuffer * inputbuffer=new InputBuffer;
        inputbuffer->infile.open(tempFilenameBasic+int2String(i)+".txt");
        inputbuffer->data=new Product[kInputBufferRecordCount];
        inputbuffer->count=kInputBufferRecordCount;
        inputbuffer->currentIndex=0;
        inputbuffer->isSubfileReadOver=false;
        inputbuffer->next=NULL;
        int count=readDataToInputBuffer(inputbuffer->infile, kInputBufferRecordCount, inputbuffer->data);
        if(count==0)    //the file is empty
        {
            freeInputBuffer(inputbuffer);
        }
        else
        {
            insertInputBufferToInputBuffers(head,inputbuffer);
        }
    }
    // outputInputBuffers(head);
    //init output buffer,when the inputbuffer associated file read completely,the inputbuffer will be delete form inputbuffers
    
    Product * outputBuffer=new Product[kOutputBufferRecordCount];
    int  outputBufferCurrentIndex=0;
    ofstream targetFile(dstDataFilename);
    //multiway sort
    while (!isInputBuffersEmpty(head))
    {
        Product product=getMinProduct(head);
        outputBuffer[outputBufferCurrentIndex].setId(product);
        outputBuffer[outputBufferCurrentIndex].price=product.price;
        outputBuffer[outputBufferCurrentIndex].setName();
        outputBufferCurrentIndex++;
        if (outputBufferCurrentIndex>kOutputBufferRecordCount)
        {
            outputBufferCurrentIndex=0;
            writeOutputBufferDataToFile(targetFile, kOutputBufferRecordCount, outputBuffer);
        }
    }
    writeOutputBufferDataToFile(targetFile, outputBufferCurrentIndex+1, outputBuffer);
    
    //free resources
    targetFile.close();
    delete [] outputBuffer;
    //delete temp files
    string deletFileExpression="rm -f /Users/ssl/temp";
    for (int i=0; i<kLoadTimes; i++)
    {
        system((deletFileExpression+int2String(i)+".txt").c_str());
    }
}


int main(int argc, const char * argv[])
{
     string srcDataFilename="/Users/ssl/data.txt";
     string dstDataFilename="/Users/ssl/orderedData.txt";
     //generateDataFile(srcDataFilename);
     clock_t start=clock();
     multiwayMergeSort(srcDataFilename,dstDataFilename);
     clock_t end=clock();
     std::cout<<"multiway merge sort takes "<<(end-start)/CLOCKS_PER_SEC<<"s"<<"\n";
     return 0;
}
