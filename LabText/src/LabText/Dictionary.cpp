//
//  Dictionary.cpp
//  LabText
//
//  Created by Nick Porcino on 2014 03/6.
//
//

#include "Dictionary.h"
#include <stdlib.h>

static unsigned int _dictStringCopyHTHashFunction(const void *key)
{
    return dictGenHashFunction((const unsigned char*) key, strlen((const char*) key));
}

static void *_dictStringDup(void *privdata, const void *key)
{
    int len = strlen((const char*) key);
    char *copy = (char*) malloc(len+1);
    DICT_NOTUSED(privdata);

    memcpy(copy, key, len);
    copy[len] = '\0';
    return copy;
}

static int _dictStringCopyHTKeyCompare(void *privdata, const void *key1,
                                       const void *key2)
{
    DICT_NOTUSED(privdata);
    return strcmp((const char*) key1, (const char*) key2) == 0;
}

static void _dictStringDestructor(void *privdata, void *key)
{
    DICT_NOTUSED(privdata);
    free(key);
}

dictType dictTypeHeapStringCopyKey = {
    _dictStringCopyHTHashFunction, /* hash function */
    _dictStringDup,                /* key dup */
    NULL,                          /* val dup */
    _dictStringCopyHTKeyCompare,   /* key compare */
    _dictStringDestructor,         /* key destructor */
    NULL                           /* val destructor */
};

/* This is like StringCopy but does not auto-duplicate the key.
 * It's used for intepreter's shared strings. */
dictType dictTypeStaticStrings = {
    _dictStringCopyHTHashFunction, /* hash function */
    NULL,                          /* key dup */
    NULL,                          /* val dup */
    _dictStringCopyHTKeyCompare,   /* key compare */
    NULL,                          /* key destructor */
    NULL                           /* val destructor */
};

/* This is like StringCopy but also automatically handle dynamic
 * allocated C strings as values. */
dictType dictTypeHeapStringCopyKeyValue = {
    _dictStringCopyHTHashFunction, /* hash function */
    _dictStringDup,                /* key dup */
    _dictStringDup,                /* val dup */
    _dictStringCopyHTKeyCompare,   /* key compare */
    _dictStringDestructor,         /* key destructor */
    _dictStringDestructor,         /* val destructor */
};

dictType* privateDictionarySetupTable() {
    return &dictTypeHeapStringCopyKey;
}

