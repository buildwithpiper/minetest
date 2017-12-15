#!/bin/bash
echo Updating PiperOnline
rsync -vcaiz --no-o --no-g --delay-updates --exclude .git rsync://updater.playpiper.com/piperonline-master/* ~/piperonline
