/*
 * key.C - implementation of <key,data> abstraction for BT*Page and 
 *         BTreeFile code.
 *
 * Gideon Glass & Johannes Gehrke  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 */

#include <string.h>
#include <assert.h>

#include "bt.h"

/*
 * See bt.h for more comments on the functions defined below.
 */

/*
 * Reminder: keyCompare compares two keys, key1 and key2
 * Return values:
 *   - key1  < key2 : negative
 *   - key1 == key2 : 0
 *   - key1  > key2 : positive
 */
int keyCompare(const void *key1, const void *key2, AttrType t)
{
    if(t == attrString)
    {
        char *k1 = (char *)key1;
        char *k2 = (char *)key2;
        return strcmp(k1,k2);
    } else
    {
        int *k1 = (int *)key1;
        int *k2 = (int *)key2;
        return *k1-*k2;
    }
}

/*
 * make_entry: write a <key,data> pair to a blob of memory (*target) big
 * enough to hold it.  Return length of data in the blob via *pentry_len.
 *
 * Ensures that <data> part begins at an offset which is an even 
 * multiple of sizeof(PageNo) for alignment purposes.
 */
void make_entry(KeyDataEntry *target,
                AttrType key_type, const void *key,
                nodetype ndtype, Datatype data,
                int *pentry_len)
{
  // put your code here

    if(ndtype == INDEX)
    {
        if(key_type == attrInteger)
        {
            target = new KeyDataEntry();
            memcpy(&target->key.intkey,key,sizeof(int));
            memcpy(&target->data.pageNo,&data.pageNo,sizeof(PageId));
            *pentry_len = sizeof(int) + sizeof(PageId);
        }
        else
        {
            target = new KeyDataEntry();
            memcpy(&target->key.charkey,key,get_key_length(key,key_type));
            memcpy(&target->data.pageNo,&data.pageNo,sizeof(PageId));
            *pentry_len = get_key_length(key,key_type) + sizeof(PageId);
        }
    }
    else if(ndtype == LEAF)
    {
        if(key_type == attrInteger)
        {
            target = new KeyDataEntry();
            memcpy(&target->key.intkey,key,sizeof(int));
            memcpy((&target->data.rid),&data.rid,sizeof(RID));
            *pentry_len = sizeof(int) + sizeof(RID);
        }
        else
        {
            target = new KeyDataEntry();
            memcpy(&target->key.charkey,key,get_key_length(key,key_type));
            memcpy((&target->data.rid),&data.rid,sizeof(RID));
            *pentry_len = get_key_length(key,key_type) + sizeof(RID);
        }
    }
  return;
}


/*
 * get_key_data: unpack a <key,data> pair into pointers to respective parts.
 * Needs a) memory chunk holding the pair (*psource) and, b) the length
 * of the data chunk (to calculate data start of the <data> part).
 */
void get_key_data(void *targetkey, Datatype *targetdata,
                  KeyDataEntry *psource, int entry_len, nodetype ndtype)
{
   // put your code here
    if(ndtype == INDEX)
    {
        if(entry_len == sizeof(int)+sizeof(PageId))
        {
            memcpy(targetkey,&psource->key.intkey,sizeof(int));
            memcpy(&targetdata->pageNo,&psource->data.pageNo,sizeof(PageId));
        }
        else
        {
            memcpy(targetkey,&psource->key.charkey,get_key_length(&psource->key.charkey,attrString));
            memcpy(&targetdata->pageNo,&psource->data.pageNo,sizeof(PageId));
        }
    }
    else if(ndtype == LEAF)
    {
        if(entry_len == sizeof(int) + sizeof(PageId))
        {
            memcpy(targetkey,&psource->key.intkey,sizeof(int));
            memcpy(&targetdata->rid,&psource->data.rid,sizeof(RID));
        }
        else
        {
            memcpy(targetkey,&psource->key.charkey,get_key_length(&psource->key.charkey,attrString));
            memcpy(&targetdata->rid,&psource->data.rid,sizeof(RID));
        }
    }
   return;
}

/*
 * get_key_length: return key length in given key_type
 */
int get_key_length(const void *key, const AttrType key_type)
{
    if(key_type==attrString)
    {
        char *rec = (char *)key;
        return 20;
    }
    else
    {
        return sizeof(int);
    }
}
 
/*
 * get_key_data_length: return (key+data) length in given key_type
 */   
int get_key_data_length(const void *key, const AttrType key_type, 
                        const nodetype ndtype)
{

    if(key_type == attrString)
    {
        char *str = (char *)key;
        if(ndtype == LEAF)
            return get_key_length(key,key_type) + sizeof(RID);
        else if(ndtype == INDEX)
            return get_key_length(key,key_type) + sizeof(PageId);
    } else{
        int *num = (int *)key;
        if(ndtype == LEAF)
            return sizeof(int)+sizeof(RID);
        else if(ndtype == INDEX)
            return sizeof(int) + sizeof(PageId);
    }
 // put your code here
 return 0;
}
