#!/bin/bash
rm -rf /tmp/test_*
touch /tmp/test_f_cre
touch /tmp/test_f_del
touch /tmp/test_f_file
touch /tmp/test_f_rename_src
touch /tmp/test_f_link
touch /tmp/test_f_setmode
dd if=/dev/zero of=/tmp/test_f_copy bs=1024 count=1
mkdir -p /tmp/test_d_rename_src
mkdir -p /tmp/test_del_dir
mkdir -p /tmp/test_d_link
mkdir -p /tmp/test_d_unln
mkdir -p /tmp/test_d_setowner
mkdir -p /tmp/test_d_setmode

