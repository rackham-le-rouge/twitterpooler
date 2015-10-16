# Project : twitterpooler
# A project to grab usefull info in twitter, specificated by a configuration file. Big data
# Author : Jerome GRARD
# Date 10/2015
# Content : just command lines used during debug, shortcut in order to avoid typing the same commands each time


#!/bin/bash

if [[ "$1" == "" ]]; then
    ./pooler 2>logcat
fi


if [ "$1" == "log" ]; then
    tail -f logcat
fi
