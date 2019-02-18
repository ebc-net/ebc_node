#! /usr/bin/python
#-*- coding: utf-8 -*- 
import os
import sys
import pdb

def doReplace(fpath,src,dst):
    newConent = ""
    bFlag = False
    other_srcStr = "error"
    other_srcStr1 = "fail"
    other_dstStr = "LOG_ERROR()"
    endStr = "<<std::endl"
    with open(fpath,"rb") as fin:
        for line in fin :
            if line.find(src) == -1 :
                newLine = line
            else:
                bFlag = True
                if line.find(other_srcStr) == -1 and line.find(other_srcStr1) == -1 :
                    newLine = line.replace(src,dst)
                else:
                    newLine = line.replace(src, other_dstStr)

                if newLine.find(endStr) != -1:
                    newLine = newLine.replace(endStr, "")

            newConent += newLine

    if not bFlag : return None
    print fpath
    with open(fpath,"wb") as fout:
        fout.write(newConent)
        return None

def replaceMain(dirName,src,dst):
    for root, dirs, files in os.walk(dirName):
        for name in files:
            if name == "re.py":
                continue
            fpath = os.path.join(root, name)
            doReplace(fpath,src,dst)
    return None

if __name__ == "__main__":
    srcStr = "std::cout"
    dstStr = "LOG_INFO()"
    print srcStr, dstStr
    dirName = "."
    dirName = os.path.realpath(dirName)
    print "working dir :",dirName
    replaceMain(dirName,srcStr,dstStr)
