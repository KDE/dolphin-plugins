#! /bin/sh
$EXTRACTRC *.kcfg >> rc.cpp
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/fileviewsvnplugin.pot
