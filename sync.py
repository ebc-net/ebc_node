#!/usr/bin/python

#This file is used to synchronize code on linux and windows by github

import sys
import time
import datetime
import os
import shutil


cmd = 'pull'

if len(sys.argv) > 1:
    cmd = 'push'


if __name__ == '__main__':
    fileList = os.listdir('./src')
    for i in fileList:
        if os.path.exists('../ebc_node/'+i):
            rFile = '../ebc_node/'+i
            lFile = './src/'+i
            if os.path.getmtime(rFile) > os.path.getmtime(lFile):
                shutil.copyfile(rFile, lFile)
            else:
                shutil.copyfile(lFile, rFile)

    fileList = os.listdir('./include')
    ignoreList = ['google', 'msg.pb.h', 'udt.h']
    for i in fileList:
        if i in ignoreList:
            continue

        if os.path.exists('../ebc_node/include/'+i):
            rFile = '../ebc_node/include/'+i
            lFile = './include/'+i
            if os.path.getmtime(rFile) > os.path.getmtime(lFile):
                shutil.copyfile(rFile, lFile)
            else:
                shutil.copyfile(lFile, rFile)
            




