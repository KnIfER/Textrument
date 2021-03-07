#!/usr/bin/env python
# -*-coding=utf-8 -*-
import os
from os.path import *
import codecs
from shutil import copyfile

#Purpose: Sync Mergeable Commits From The Upstream Branch.

#    (Just copying and deleting files. )

#Usage: Git clone the upstream repo. 
#    put this script there.     
#    Modify targetRepPath to this repo.
#    then run this script, and follow it's prompts.

def main():
    targetRepPath = r'D:\Code\FigureOut\notepad-plus-plus'
    
    execRepPath = targetRepPath+r'\PowerEditor\bin64\Textrument.exe'
    
    formOldCommitSHA = str(input(""))
    toNewCommitSHA = str(input(""))
    
    #formOldCommitSHA='ddd2e5fd23412d5e8410c2a821461f1f578bca0f'
    #toNewCommitSHA='47bb08ee6c3bd18171585e095229a71d9b71e08b'
    
    print()
        
    root = split(realpath(__file__))[0]

    os.system("del modlist.user")
    
    os.system("git diff-tree -r --no-commit-id --name-only "+formOldCommitSHA+" "+toNewCommitSHA+" >> modlist.user")
    
    #读入
    f = codecs.open('modlist.user', 'r')
    AllPaths = [];
    delCount=0;
    for filename in f:
        filename = filename.strip().replace("/", "\\")
        pathname = join(root, filename)
        if len(filename)>0:
            isDel = not isfile(pathname)
            if isDel:
                filename = '(-)'+filename # to make it more readable
                delCount+=1
            AllPaths.append(filename)
            print(filename)
    f.close()

    #写入
    f = codecs.open('modlist.user', 'w')    
    f.writelines([line+'\n' for line in AllPaths])
    f.close()
    
    # at this stage you can modify the list as it'll be reload again.
    
    hint = "\r\n====================================\r\nAre You Sure To Continue with "
    if delCount>0:
        hint += str(delCount)+" files deleted ,"
    
    openInsteadOfExec = str(input(hint+str(len(AllPaths)-delCount)+" files Modified?"))
    
    openInsteadOfExec = len(openInsteadOfExec)>0
    
    if openInsteadOfExec:
        print("Open modified Files In Textrument!")
    
    #读入操作
    AllPaths = codecs.open('modlist.user', 'r').readlines();
    for filename in AllPaths:
        filename = filename.strip()
        isDel = filename[0]=='('
        if isDel:
            filename=filename[3:]
        if len(filename)>0:
            targetpathname = join(targetRepPath, filename)
            pathname = join(root, filename)
            if openInsteadOfExec:
                print(">>> 📂: "+targetpathname)
                if isfile(targetpathname):
                    os.system(execRepPath + " " + targetpathname)
            elif isDel:
                print("(---): "+filename)
                if isfile(targetpathname):
                    os.remove(targetpathname)
                else:
                    print("---(Error None Exist)")
            else:
                print(">>>: "+filename)
                if isfile(pathname):
                    if not exists(os.path.dirname(targetpathname)):
                        os.makedirs(os.path.dirname(targetpathname))
                    copyfile(pathname, targetpathname)

if __name__ == '__main__':
    main()
    os.system('pause')
    #_exit(0)








