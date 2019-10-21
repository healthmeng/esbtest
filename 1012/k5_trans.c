#include <stdio.h>
#include <string.h>

//#include "k5_primitive.h"
struct trans{
	int code ;
	char str[100];
};
struct trans trans_list[] = 
{
	{0x0011, "sys_reboot"},
	{0x0012, "sys_halt"},
	{0x0013, "sys_start_vmm"},
	{0x0014, "sys_stop_vmm"},
	{0x0015, "sys_mount_fs"},
	{0x0016, "sys_unmount_fs"},
	{0x0017, "sys_login"},
	{0x0018, "sys_logout"},
	{0x0019, "sys_svc_grp_reg"},
	{0x001a, "sys_svc_grp_del"},
	{0x001b, "sys_svc_reg"},
	{0x001c, "sys_svc_del"},
	{0x001d, "sys_svc_list"},
	{0x0021, "prc_start"},
	{0x0022, "prc_stop"},
	{0x0023, "prc_fork"},
	{0x0024, "prc_exit"},
	{0x0025, "prc_pause"},
	{0x0026, "prc_resume"},
	{0x0027, "prc_sync"},
	{0x0028, "prc_wait"},
	{0x0031, "thr_start"},
	{0x0032, "thr_stop"},
	{0x0033, "thr_exit"},
	{0x0034, "thr_pause"},
	{0x0035, "thr_resume"},
	{0x0036, "thr_sync"},
	{0x0037, "thr_wait"},
	{0x0041, "file_create"},
	{0x0042, "file_delete"},
	{0x0043, "file_open"},
	{0x0044, "file_close"},
	{0x0045, "file_read"},
	{0x0046, "file_write"},
	{0x0047, "file_seek"},
	{0x0048, "file_rewind"},
	{0x0049, "file_state"},
	{0x004a, "file_rename"},
	{0x004b, "file_link"},
	{0x004c, "file_unlink"},
	{0x004d, "file_set_mode"},
	{0x004e, "file_get_mode"},
	{0x004f, "file_copy"},
	{0x0051, "dir_make"},
	{0x0052, "dir_remove"},
	{0x0053, "dir_list"},
	{0x0054, "dir_change"},
	{0x0055, "dir_rename"},
	{0x0056, "dir_link"},
	{0x0057, "dir_unlink"},
	{0x0058, "dir_set_owner"},
	{0x0059, "dir_get_owner"},
	{0x005a, "dir_set_mode"},
	{0x005b, "dir_get_mode"},
	{0x0061, "usr_create"},
	{0x0062, "usr_delete"},
	{0x0063, "usr_get"},
	{0x0064, "usr_set"},
	{0x0065, "usr_get_uid"},
	{0x0066, "usr_set_uid"},
	{0x0067, "usr_check"},
	{0x0071, "grp_create"},
	{0x0072, "grp_delete"},
	{0x0073, "grp_get"},
	{0x0074, "grp_set"},
	{0x0075, "grp_get_gid"},
	{0x0076, "grp_set_gid"},
	{0x0077, "grp_check"},
	{0x0081, "mem_map"},
	{0x0082, "mem_unmap"},
	{0x0083, "mem_alloc"},
	{0x0084, "mem_free"},
	{0x0085, "mem_share"},
	{0x0086, "mem_unshare"},
	{0x0087, "mem_lock"},
	{0x0088, "mem_unlock"},
	{0x0089, "mem_sem_get"},
	{0x008a, "mem_sem_set"},
	{0x008b, "mem_sem_on"},
	{0x008c, "mem_sem_off"},
	{0x008d, "mem_control"},
	{0x0091, "time_get"},
	{0x0092, "time_set"},
	{0x0093, "time_sleep"},
	{0x0094, "time_wait"},
	{0x0096, "time_alarm"},
	{0x0097, "time_clear"},
	{0x0098, "time_sync"},
	{0x00a1, "net_socket"},
	{0x00a2, "net_close"},
	{0x00a3, "net_bind"},
	{0x00a4, "net_control"},
	{0x00a5, "net_connect"},
	{0x00a6, "net_accept"},
	{0x00a7, "net_send"},
	{0x00a8, "net_recv"},
	{0x00a9, "net_send_to"},
	{0x00aa, "net_recv_from"},
	{0x00ab, "net_select"},
	{0x00b1, "dev_start"},
	{0x00b2, "dev_stop"},
	{0x00b3, "dev_open"},
	{0x00b4, "dev_close"},
	{0x00b5, "dev_read"},
	{0x00b6, "dev_write"},
	{0x00b7, "dev_state"},
	{0x00b8, "dev_control"},
	{0x00b9, "dev_reset"},
	{0x00ba, "dev_sleep"},
	{0x00bb, "dev_set"},
	{0x00bc, "dev_get"}
};

int trans_c2s(int code, char *str)
{
    int i = 0;

    for ( i = 0; i < 0x00bc+1; i++ )
    {   
        if ( trans_list[i].code == code )
        {
            strcpy( str, trans_list[i].str);
            //memcpy( str, trans_list[i].str, strlen(trans_list[i].str) );
			return 0;
        }
    }   
}

int trans_s2c(char *str)
{
    int i = 0;

    for ( i = 0; i < 0x00bc+1; i++ )
	{
		if ( strcmp( str, trans_list[i].str ) == 0 )
			return trans_list[i].code;
	}
}

