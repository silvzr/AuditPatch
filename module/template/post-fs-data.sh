#!/system/bin/sh
MODDIR=${0%/*}

# Inject into already-running logd (no restart needed)
$MODDIR/bin/linjector-cli -p $(pidof logd) -f $MODDIR/lib/libauditpatch.so
