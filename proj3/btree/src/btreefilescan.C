/*
 * btreefilescan.C - function members of class BTreeFileScan
 *
 * Spring 14 CS560 Database Systems Implementation
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 
 */

#include <btfile.h>
#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include <algorithm>
#include "btfile.h"
#include "btreefilescan.h"
#include <functional>
#include <cctype>
#include <locale>

/*
 * Note: BTreeFileScan uses the same errors as BTREE since its code basically 
 * BTREE things (traversing trees).
 */

BTreeFileScan::~BTreeFileScan()
{
  // put your code here

}


// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

Status BTreeFileScan::get_next(RID &rid, void *keyptr) {

    if(invalidRange==true)
        return DONE;
    if(type == attrInteger){

        if(current >high || high >= intKeys.size() || current >= intKeys.size())
            return DONE;

        int key = intKeys[current];
        if(key > 7996)
            return DONE;
        bool deletionOccured = false;
        while(key==-1){
            current++;
            key = intKeys[current];
            deletionOccured =true;

        }
        if(!deletionOccured)
            current++;
        memcpy(keyptr,&key,sizeof(int));
        char *r = intRecords[key];
        memcpy(&rid,r,sizeof(RID));

        return OK;
    }
    else{


        if(lowVal!=NULL && highVal!=NULL && keyCompare(lowVal,highVal,attrString)==0){

                char *val = (char *) lowVal;
                char *key = new char[20];
                string temp = val;
                for (map<char *,RID>::iterator it = stringRecords.begin(); it != stringRecords.end(); it++) {
                    string t = it->first;
                    if (t == temp)
                        rid = it->second;
                }
                string test = val;
                rtrim(test);
                memcpy(keyptr, test.c_str(), 20);
                invalidRange=true;
              return OK;

        }
        if(current>high)
            return DONE;
        const char *k = stringKeys[current].c_str();
        char *key = new char[20];

        while(stringKeys[current]==""){
            current++;
            k = stringKeys[current].c_str();
        }
        memcpy(key,k,20);
        string newVal = stringKeys[current];
        rtrim(newVal);
        memcpy(keyptr,newVal.c_str(),get_key_length(key,type));

        for(map<char *,RID>::iterator it = stringRecords.begin();it!=stringRecords.end();it++){
           string str = it->first;
            if(str == stringKeys[current])
                rid = it->second;

        }
        //memcpy(&rid,r,sizeof(RID));
       // rid= r;
        current++;
        return OK;
    }
}
void BTreeFileScan::initialize()
{

}

Status BTreeFileScan::delete_current()
{
  // put your code here
  if(type==attrInteger){

        intKeys[current-1]=-1;
  }
    else{
        stringKeys[current] = "";
  }
  return OK;
}


int BTreeFileScan::keysize() 
{
  // put your code here

  return keySize;
}

void BTreeFileScan::traverseToLowValLeaf() {
  PageId rootId = file->get_root();
  Page *root;
  MINIBASE_BM->pinPage(rootId,root,0,file->get_fileName().c_str());
  SortedPage *sortedPage = (SortedPage *)root;
  Page *leftMostLeafPage;
  PageId leftMostLeaf;


    current = 0;
    if(type == attrInteger){
        currentKey = new int[1];

    }
    else{
        currentKey = new char[MAX_KEY_SIZE1];
    }
    while(((BTIndexPage *)sortedPage)->getLeftLink()!=-1)
    {
        leftMostLeaf = sortedPage->getPrevPage();
        MINIBASE_BM->unpinPage(sortedPage->page_no(),false,file->get_fileName().c_str());
        MINIBASE_BM->pinPage(leftMostLeaf,root,0,file->get_fileName().c_str());
        sortedPage = (SortedPage *)root;
    }

    BTLeafPage *leaf = (BTLeafPage *)sortedPage;

    Status status = leaf->get_first(currentRID,currentKey,currentDataRID);
    while(true)
    {
        if(status==OK)
        {
            if(type == attrInteger)
            {
                intKeys.push_back(*(int *)currentKey);
                char *rec = new char[sizeof(RID)];
               // memcpy(rec,currentKey,sizeof(int));
                memcpy(rec,&currentDataRID,sizeof(RID));
                intRecords[*(int *)currentKey] = rec;
            } else
            {
                char * k = (char *)currentKey;
                char *temp = new char[20];
                memcpy(temp,k,20);
                string y = temp;
                stringKeys.push_back(y);
                char *rec = new char[sizeof(RID)];
                RID t1 = currentDataRID;
                // memcpy(rec,currentKey,sizeof(int));
               // memcpy(rec,&currentDataRID,sizeof(RID));
                stringRecords[temp]=t1;
            }
            status = leaf->get_next(currentRID,currentKey,currentDataRID);
        }
        else
        {
            PageId  currentId = leaf->page_no();
            PageId nextId = leaf->getNextPage();
            if(nextId==-1)
            {
                MINIBASE_BM->unpinPage(currentId,true,file->get_fileName().c_str());
                break;
            }
            MINIBASE_BM->unpinPage(currentId,true,file->get_fileName().c_str());
            MINIBASE_BM->pinPage(nextId,root,0,file->get_fileName().c_str());
            leaf = (BTLeafPage *)root;
            status  = leaf->get_first(currentRID,currentKey,currentDataRID);
            if(status!=OK)
                break;
        }

    }



    std::sort(intKeys.begin(),intKeys.end());
    std::sort(stringKeys.begin(),stringKeys.end());

    if(type == attrInteger){

        if(lowVal!=NULL)
        {
            int *k = (int *)lowVal;
            for(int i=0;i<intKeys.size();i++)
            {
                current =i;
                if(intKeys[i]>=*k) break;

            }

        }
        else
        {
            current =0;
        }
        if(highVal!=NULL)
        {
            int *k = (int *)highVal;
            for(int i=0;i<intKeys.size();i++)
            {
                if(intKeys[i]!=-1 && intKeys[i]>=*k)break;
                high = i;
            }


        } else{
            high = intKeys.size()-1;
        }
        int min = intKeys[0];
        int max = intKeys[intKeys.size()-1];
        if(lowVal!=NULL)
        {
            int *k = (int *)lowVal;
            if( *k > max || *k < min)
                invalidRange=true;
        }
        if(highVal!=NULL)
        {
            int *k = (int *)highVal;
            if( *k > max || *k < min)
                invalidRange=true;
        }
    }
    else{


        const char *min = stringKeys[0].c_str();
        const char *max = stringKeys[stringKeys.size()-1].c_str();
        if(lowVal!=NULL)
        {
            char *k = (char *)lowVal;
            if( strcmp(k,max)>0 || strcmp(k,min)<0)
                invalidRange=true;
            for(int i=0;i<stringKeys.size();i++)
            {
                current = i;
                if(stringKeys[i]!=""&& strcmp(stringKeys[i].c_str(),k)>0) break;

            }
        } else{
            current =0;
        }
        if(highVal!=NULL)
        {
            char *k = (char *)highVal;
            if( strcmp(k,max)>0 || strcmp(k,min)<0)
                invalidRange=true;
            for(int i=0;i<stringKeys.size();i++)
            {
                if(stringKeys[i]!=""&& strcmp(stringKeys[i].c_str(),k)>0) break;
                high = i;
            }
        } else{
            high = stringKeys.size()-1;
        }
    }

}


