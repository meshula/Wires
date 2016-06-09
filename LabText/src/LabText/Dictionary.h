//
//  Dictionary.h
//  LabText
//
//  Created by Nick Porcino on 2014 03/6.
//
//

#pragma once
#include "LabText/dict.h"
#include <string>

dictType* privateDictionarySetupTable();

// simple wrapper on dict for the case where keys are duplicated for the
// dict, and values are not owned by the dict. A dict is relatively light weight
// when empty, on the order of 100 bytes.

template <typename DataType>
class RefDictionary {
public:

    RefDictionary() {
        _dict = dictCreate(privateDictionarySetupTable(), 0);
    }
    ~RefDictionary() {
        delete _dict;
    }

    void insert(char const*const name, DataType* value) {
        dictReplace(_dict, (void*) name, reinterpret_cast<void*>(value));
    }
    void insert(const std::string& name, DataType* value) {
        dictReplace(_dict, (void*) name.c_str(), reinterpret_cast<void*>(value));
    }

    DataType* find(char const*const name) {
        return reinterpret_cast<DataType*>(dictFetchValue(_dict, name));
    }
    DataType* find(const std::string& name) {
        return reinterpret_cast<DataType*>(dictFetchValue(_dict, name.c_str()));
    }

    void erase(char const*const name) {
        dictDelete_(_dict, name);
    }
    void erase(const std::string& name) {
        dictDelete_(_dict, name.c_str());
    }

private:
    dict* _dict;
};

template <typename DataType>
class CopyDictionary {
public:

    struct Adaptor {
        DataType data;
        static void* dictMakeAdaptor(void *privdata, const void *obj)
        {
            Adaptor* data = new Adaptor;
            void* ncobj = const_cast<void*>(obj);
            data->data = *(reinterpret_cast<DataType*>(ncobj));
            return data;
        }
        static void dictFreeAdaptor(void *privdata, void *obj)
        {
            Adaptor* data = reinterpret_cast<Adaptor*>(obj);
            delete data;
        }
    };

    CopyDictionary() {
        memcpy(&setup, privateDictionarySetupTable(), sizeof(dictType));
        setup.valDup = Adaptor::dictMakeAdaptor;
        setup.valDestructor = Adaptor::dictFreeAdaptor;
        _dict = dictCreate(&setup, 0);
    }
    ~CopyDictionary() {
        delete _dict;
    }

    void insert(char const*const name, DataType value) {
        dictReplace(_dict, (void*) name, reinterpret_cast<void*>(&value));
    }
    void insert(const std::string& name, DataType value) {
        dictReplace(_dict, (void*) name.c_str(), reinterpret_cast<void*>(&value));
    }

    DataType *find(char const*const name) {
        Adaptor* adaptor = reinterpret_cast<Adaptor*>(dictFetchValue(_dict, name));
        if (adaptor)
            return &adaptor->data;
        return 0;
    }
    DataType *find(const std::string& name) {
        Adaptor* adaptor = reinterpret_cast<Adaptor*>(dictFetchValue(_dict, name.c_str()));
        if (adaptor)
            return &adaptor->data;
        return 0;
    }

    void erase(char const*const name) {
        dictDelete_(_dict, name);
    }
    void erase(const std::string& name) {
        dictDelete_(_dict, name.c_str());
    }

private:
    dict* _dict;
    dictType setup;
};




