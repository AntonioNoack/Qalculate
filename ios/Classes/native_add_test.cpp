#ifndef ANNO_PROTECT_557443269
#define ANNO_PROTECT_557443269
//
// Created by antonio on 11/26/19.
//

#include <stdint.h>
// #include <android/log.h>
#include "config.h"
#include "libxml/parser.h"
#include "libxml/xmlmemory.h"
#include "libxml/elfgcchack.h"
#include "libqalculate/Calculator.h"

#include <fstream>
#include <iostream>
#include <sstream>

void* operator new(size_t x){
    return malloc(x);
}

void operator delete(void* arg){
    free(arg);
}

void internalTests(){

    // xml looks like its working

    xmlNodePtr n;
    xmlDocPtr doc;
    xmlChar *xmlbuff;
    int buffersize;

    /*
     * Create the document.
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    n = xmlNewNode(NULL, BAD_CAST "root");
    xmlNodeSetContent(n, BAD_CAST "content");
    xmlDocSetRootElement(doc, n);

    /*
     * Dump the document to a buffer and print it
     * for demonstration purposes.
     */
    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
    printf("XMLLIB %s\n", (char *) xmlbuff);

    /*
     * Free associated memory.
     */
    xmlFree(xmlbuff);
    xmlFreeDoc(doc);

}

bool isInited;

void init(){
    new Calculator();
    CALCULATOR->loadExchangeRates();
    CALCULATOR->loadGlobalDefinitions();
    CALCULATOR->loadLocalDefinitions();
    isInited = true;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int32_t native_add(int32_t x, int32_t y) {

    internalTests();

    if(!isInited){
        init();
    }

    // cout << CALCULATOR->calculateAndPrint("1 + 1", 2000) << endl;

    return 12;
}

std::stringstream fromC;
std::stringstream toC;

int resultIndex = 0;
int resultLength = 0;
std::string result;

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int32_t putChar(int32_t x) {

    if(x == 0){

        try {

            // execute
            if(!isInited){
                init();
            }

            std::string query = toC.str();

            fromC.clear();fromC.str("");
            fromC << CALCULATOR->calculateAndPrint(query, 2000);

            // fromC.seekg(fromCStart, std::ios::beg); // doesn't work for str
            result = fromC.str();
            resultLength = result.length();

        } catch(...){

            result = std::string("Error :(");
            resultIndex = 0;
            resultLength = 8;

        }

        toC.clear();toC.str("");


    } else {

        // collect
        toC << (char) x;

    }

    return 0;

}


extern "C" __attribute__((visibility("default"))) __attribute__((used))
int32_t getChar() {

    // return the letter,
    // return 0 if done

    if(resultIndex < resultLength){
        return result[resultIndex++];
    } else {
        return 0;
    }

}



#endif
