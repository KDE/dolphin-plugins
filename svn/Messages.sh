#! /bin/sh
$EXTRACTRC *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/fileviewsvnplugin.pot
