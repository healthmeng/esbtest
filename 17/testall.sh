#!/bin/bash

BINPATH="/usr/bin"
GREP="/usr/bin/grep"
LS="/usr/bin/ls"
MKDIR="/usr/bin/mkdir"
MOUNT="/usr/bin/mount"

CMD=./k5_test
DIP="127.0.0.1"
DPORT="3535"

SVC="test_s_svc"
PROC="test_proc"
THR="test_thr"
PATH="/tmp"
FILE="test_f_file"
DIR="test_d_dir"
DIR_RNM="test_d_rename"
USR="testk5"
GRP="testgrpk5"
MEM="memk5"
SKTID1=`${CMD} ${DIP} ${DPORT} net_socket 192.168.1.1 10086 tcp`
SKTID2=`${CMD} ${DIP} ${DPORT} net_socket 192.168.1.1 10086 tcp`
SKTID3=`${CMD} ${DIP} ${DPORT} net_socket`
DEV="/dev/sdb1"


	# 系统
	${CMD} ${DIP} ${DPORT} sys_start_vmm test_vm
	${CMD} ${DIP} ${DPORT} sys_stop_vmm test_vm

	${MKDIR} -p /media/test
	${CMD} ${DIP} ${DPORT} sys_mount_fs /dev/sda8 /media/test
	${MOUNT} | ${GREP} test
	${CMD} ${DIP} ${DPORT} sys_unmount_fs /dev/sda8
	${MOUNT} | ${GREP} test

	${CMD} ${DIP} ${DPORT} sys_svc_grp_reg ${SVC}
	${CMD} ${DIP} ${DPORT} sys_svc_grp_del ${SVC}
	${CMD} ${DIP} ${DPORT} sys_svc_reg ${SVC}
	${CMD} ${DIP} ${DPORT} sys_svc_del ${SVC}
	${CMD} ${DIP} ${DPORT} sys_svc_list

	# 进程
	${CMD} ${DIP} ${DPORT} prc_start ${PROC}
	${CMD} ${DIP} ${DPORT} prc_pause ${PROC}
	${CMD} ${DIP} ${DPORT} prc_resume ${PROC}
	${CMD} ${DIP} ${DPORT} prc_fork ${PROC}
	${CMD} ${DIP} ${DPORT} prc_sync ${PROC}
	${CMD} ${DIP} ${DPORT} prc_exit ${PROC}
	${CMD} ${DIP} ${DPORT} prc_wait ${PROC} 3
	${CMD} ${DIP} ${DPORT} prc_stop ${PROC}

	# 线程
	${CMD} ${DIP} ${DPORT} thr_start ${THR}
	${CMD} ${DIP} ${DPORT} thr_pause ${THR}
	${CMD} ${DIP} ${DPORT} thr_resume ${THR}
	${CMD} ${DIP} ${DPORT} thr_sync ${THR}
	${CMD} ${DIP} ${DPORT} thr_exit ${THR}
	${CMD} ${DIP} ${DPORT} thr_stop ${THR}

	# 文件
	${LS} -l ${PATH}
	${CMD} ${DIP} ${DPORT} file_create ${PATH}/${FILE}
	${LS} -l ${PATH}
	${CMD} ${DIP} ${DPORT} file_open ${PATH}/${FILE} 
	${CMD} ${DIP} ${DPORT} file_write 12345678901234567890
	${CMD} ${DIP} ${DPORT} file_rewind ${PATH}/${FILE}
	${CMD} ${DIP} ${DPORT} file_read ${PATH}/${FILE} 8
	${CMD} ${DIP} ${DPORT} file_seek ${PATH}/${FILE} 5
	${CMD} ${DIP} ${DPORT} file_read ${PATH}/${FILE} 10
	${CMD} ${DIP} ${DPORT} file_close ${PATH}/${FILE}
	${CMD} ${DIP} ${DPORT} file_state ${PATH}/${FILE}
	${CMD} ${DIP} ${DPORT} file_get_mode ${PATH}/${FILE}
	${CMD} ${DIP} ${DPORT} file_set_mode ${PATH}/${FILE} 777
	${LS} -l ${PATH}/${FILE}
	${CMD} ${DIP} ${DPORT} file_get_mode ${PATH}/${FILE}
	${CMD} ${DIP} ${DPORT} file_copy ${PATH}/${FILE} ${PATH}/test_f_copy
	${CMD} ${DIP} ${DPORT} file_rename ${PATH}/test_f_copy ${PATH}/test_f_rename
	${CMD} ${DIP} ${DPORT} file_link ${PATH}/${FILE} ${PATH}/test_f_link
	${CMD} ${DIP} ${DPORT} file_unlink ${PATH}/test_f_link
	${CMD} ${DIP} ${DPORT} file_delete ${PATH}/${FILE}
	${CMD} ${DIP} ${DPORT} file_delete ${PATH}/test_f_rename

	# 目录
	${CMD} ${DIP} ${DPORT} dir_make ${PATH}/${DIR}
	${CMD} ${DIP} ${DPORT} dir_list ${PATH}
	${CMD} ${DIP} ${DPORT} dir_change /opt/
	${CMD} ${DIP} ${DPORT} dir_rename ${PATH}/${DIR} ${PATH}/${DIR_RNM}
	${CMD} ${DIP} ${DPORT} dir_link ${PATH}/${DIR_RNM} ${PATH}/test_d_ln
	${CMD} ${DIP} ${DPORT} dir_unlink ${PATH}/test_d_ln
	${CMD} ${DIP} ${DPORT} dir_get_owner ${PATH}/${DIR_RNM}
	${CMD} ${DIP} ${DPORT} dir_set_owner ${PATH}/${DIR_RNM} sys
	${CMD} ${DIP} ${DPORT} dir_get_owner ${PATH}/${DIR_RNM}
	${CMD} ${DIP} ${DPORT} dir_get_mode ${PATH}/${DIR_RNM}
	${CMD} ${DIP} ${DPORT} dir_set_mode ${PATH}/${DIR_RNM} 766
	${CMD} ${DIP} ${DPORT} dir_get_mode ${PATH}/${DIR_RNM}
	${CMD} ${DIP} ${DPORT} dir_remove ${PATH}/${DIR_RNM}

	# 用户
	${CMD} ${DIP} ${DPORT} usr_create ${USR}
	${CMD} ${DIP} ${DPORT} usr_get_uid ${USR}
	${CMD} ${DIP} ${DPORT} usr_set_uid ${USR} 1010
	${CMD} ${DIP} ${DPORT} usr_get_uid ${USR}
#	${CMD} ${DIP} ${DPORT} usr_get ${USR}
#	${CMD} ${DIP} ${DPORT} usr_set ${USR}
#	${CMD} ${DIP} ${DPORT} usr_get ${USR}
	${CMD} ${DIP} ${DPORT} usr_check ${USR}

	# 用户组
	${CMD} ${DIP} ${DPORT} grp_create ${GRP}
	${CMD} ${DIP} ${DPORT} grp_get_gid ${GRP}
	${CMD} ${DIP} ${DPORT} grp_set_gid ${GRP} 1020
	${CMD} ${DIP} ${DPORT} grp_get_gid ${GRP}
#	${CMD} ${DIP} ${DPORT} grp_get ${GRP}
#	${CMD} ${DIP} ${DPORT} grp_set ${GRP}
#	${CMD} ${DIP} ${DPORT} grp_get ${GRP}
	${CMD} ${DIP} ${DPORT} grp_delete ${GRP}

	# 内存（尚未实现）
	${CMD} ${DIP} ${DPORT} mem_map ${MEM}
	${CMD} ${DIP} ${DPORT} mem_unmap ${MEM}
	${CMD} ${DIP} ${DPORT} mem_alloc ${MEM}
	${CMD} ${DIP} ${DPORT} mem_free ${MEM}
	${CMD} ${DIP} ${DPORT} mem_share ${MEM}
	${CMD} ${DIP} ${DPORT} mem_unshare ${MEM}
	${CMD} ${DIP} ${DPORT} mem_lock ${MEM}
	${CMD} ${DIP} ${DPORT} mem_unlock ${MEM}
	${CMD} ${DIP} ${DPORT} mem_sem_get ${MEM}
	${CMD} ${DIP} ${DPORT} mem_sem_set ${MEM}
	${CMD} ${DIP} ${DPORT} mem_sem_get ${MEM}
	${CMD} ${DIP} ${DPORT} mem_sem_on ${MEM}
	${CMD} ${DIP} ${DPORT} mem_sem_off ${MEM}
	${CMD} ${DIP} ${DPORT} mem_control ${MEM}

	# 时间
	${CMD} ${DIP} ${DPORT} time_get
	${CMD} ${DIP} ${DPORT} time_set 20191018090000
	${CMD} ${DIP} ${DPORT} time_get
	${CMD} ${DIP} ${DPORT} time_sync
	${CMD} ${DIP} ${DPORT} time_sleep 5
	${CMD} ${DIP} ${DPORT} time_wait 3
	${CMD} ${DIP} ${DPORT} time_alarm
	${CMD} ${DIP} ${DPORT} time_clear

	# 网络
	## TCP发送
	${CMD} ${DIP} ${DPORT} net_connect ${SKTID}
	${CMD} ${DIP} ${DPORT} net_send ${SKTID} "test_send_message"
	${CMD} ${DIP} ${DPORT} net_close ${SKTID}
	## TCP接收
	${CMD} ${DIP} ${DPORT} net_bind ${SKTID}
	${CMD} ${DIP} ${DPORT} net_accept ${SKTID}
	${CMD} ${DIP} ${DPORT} net_recv ${SKTID}
	${CMD} ${DIP} ${DPORT} net_close ${SKTID}
	## UDP收发
	${CMD} ${DIP} ${DPORT} net_bind ${SKTID}
	${CMD} ${DIP} ${DPORT} net_recv_from ${SKTID} 192.168.1.1 10086
	${CMD} ${DIP} ${DPORT} net_send_to ${SKTID} 192.168.1.1 10086 udp_send_to_message
	${CMD} ${DIP} ${DPORT} net_close ${SKTID}
#	${CMD} ${DIP} ${DPORT} net_control ${SKTID}

	# 设备
	${CMD} ${DIP} ${DPORT} dev_start ${DEV}
	${CMD} ${DIP} ${DPORT} dev_open ${DEV}
	${CMD} ${DIP} ${DPORT} dev_write ${DEV}
	${CMD} ${DIP} ${DPORT} dev_read ${DEV}
	${CMD} ${DIP} ${DPORT} dev_close ${DEV}
	${CMD} ${DIP} ${DPORT} dev_get ${DEV}
	${CMD} ${DIP} ${DPORT} dev_set ${DEV} 1
	${CMD} ${DIP} ${DPORT} dev_get ${DEV}
	${CMD} ${DIP} ${DPORT} dev_state ${DEV}
	${CMD} ${DIP} ${DPORT} dev_control ${DEV}
	${CMD} ${DIP} ${DPORT} dev_reset ${DEV}
	${CMD} ${DIP} ${DPORT} dev_sleep ${DEV} 3
	${CMD} ${DIP} ${DPORT} dev_stop ${DEV}

