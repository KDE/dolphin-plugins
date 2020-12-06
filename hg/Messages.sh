#! /bin/sh
$EXTRACTRC *.kcfg >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/fileviewhgplugin.pot
