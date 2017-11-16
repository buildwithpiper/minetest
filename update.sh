#!/bin/bash
echo Updating PiperCraft
rsync -vcaiz --no-o --no-g --delay-updates --exclude .git rsync://updater.playpiper.com/pipercraft-master/* ~/pipercraft
