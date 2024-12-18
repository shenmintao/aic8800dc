#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include<stdlib.h>
#include <errno.h>
#define WIFI_DRIVER_FW_PATH "sta"
#define WIFI_DRIVER_MODULE_NAME "rwnx_fdrv"
#define WIFI_DRIVER_MODULE_PATH "/vendor/modules/rwnx_fdrv.ko"
#define MAX_DRV_CMD_SIZE 1536
#define TXRX_PARA								SIOCDEVPRIVATE+1

static const char IFACE_DIR[]           = "";
static const char DRIVER_MODULE_NAME[]  = "rwnx_fdrv";
static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME " ";
static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_ARG[]   = "";
static const char FIRMWARE_LOADER[]     = "";
static const char DRIVER_PROP_NAME[]    = "wlan.driver.status";

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
  (__extension__                                                              \
    ({ long int __result;                                                     \
       do __result = (long int) (expression);                                 \
       while (__result == -1L && errno == EINTR);                             \
       __result; }))
#endif

typedef struct android_wifi_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} android_wifi_priv_cmd;

int wifi_send_cmd_to_net_interface(const char* if_name, int argC, char *argV[])
{
	int sock;
	struct ifreq ifr;
	int ret = 0;
	int i = 0;
	char buf[MAX_DRV_CMD_SIZE];
	struct android_wifi_priv_cmd priv_cmd;
	char is_param_err = 0;
	int buf_len = 0;

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		printf("bad sock!\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, if_name);

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) {
		printf("%s Could not read interface %s flags: %s",__func__, if_name, strerror(errno));
		return -1;
	}

	if (!(ifr.ifr_flags & IFF_UP)) {
		printf("%s is not up!\n",if_name);
		return -1;
	}

//	printf("ifr.ifr_name = %s\n", ifr.ifr_name);
	memset(&priv_cmd, 0, sizeof(priv_cmd));
	memset(buf, 0, sizeof(buf));

	for(i=2; i<argC; i++){
		strcat(buf, argV[i]);
		strcat(buf, " ");
	}

	priv_cmd.buf = buf;
	priv_cmd.used_len = strlen(buf);
	priv_cmd.total_len = sizeof(buf);
	ifr.ifr_data = (void*)&priv_cmd;

    printf("%s:\n", argV[2]);
    if (strcasecmp(argV[2], "SET_TX") == 0) {
        if (argC < 8) {
            is_param_err = 1;
        }
    } else if (strcasecmp(argV[2], "SET_TXTONE") == 0) {
        if (((argC == 4) && (argV[3][0] != '0'))
            || ((argC == 5) && (argV[3][0] == '0'))) {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "SET_RX") == 0)
            || (strcasecmp(argV[2], "SET_POWER") == 0)) {
        if (argC < 5) {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "SET_XTAL_CAP") == 0)
            || (strcasecmp(argV[2], "SET_XTAL_CAP_FINE") == 0)) {
        if (argC < 4) {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "GET_EFUSE_BLOCK") == 0)
            || (strcasecmp(argV[2], "SET_FREQ_CAL") == 0)) {
        if (argC < 4) {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "SET_MAC_ADDR") == 0)
            || (strcasecmp(argV[2], "SET_BT_MAC_ADDR") == 0)) {
        if (argC < 8) {
            is_param_err = 1;
        }
    } else if (strcasecmp(argV[2], "SET_VENDOR_INFO") == 0) {
        if (argC < 4) {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "RDWR_PWRIDX") == 0)
            || (strcasecmp(argV[2], "RDWR_PWROFST") == 0)
            || (strcasecmp(argV[2], "RDWR_EFUSE_PWROFST") == 0)) {
        if (((argC == 4) && (argV[3][0] != '0'))
            || (argC == 5)
            || ((argC == 6) && (argV[3][0] == '0'))) {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "RDWR_DRVIBIT") == 0)
            || (strcasecmp(argV[2], "RDWR_EFUSE_DRVIBIT") == 0)) {
        if (((argC == 4) && (argV[3][0] != '0'))
            || ((argC == 5) && (argV[3][0] == '0'))) {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "BT_RESET") == 0)) {
        char bt_reset_hci_cmd[32] = "01 03 0c 00";
        if (argC == 3) {
            buf_len = priv_cmd.used_len;
            memcpy(&priv_cmd.buf[buf_len], &bt_reset_hci_cmd[0], strlen(bt_reset_hci_cmd));
            buf_len += strlen(bt_reset_hci_cmd);
            priv_cmd.used_len = buf_len;
        } else {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "BT_TXDH") == 0)) {
        char bt_txdh_hci_cmd[255] = "01 06 18 0e ";
        if (argC == 17) {
            buf_len = priv_cmd.used_len;
            int arg_len = strlen(argV[2]);
            int txdh_cmd_len = strlen(bt_txdh_hci_cmd);
            memcpy(&bt_txdh_hci_cmd[txdh_cmd_len], &priv_cmd.buf[arg_len+1], buf_len - arg_len - 1);
            memcpy(&priv_cmd.buf[arg_len+1], &bt_txdh_hci_cmd[0], strlen(bt_txdh_hci_cmd));
            buf_len += strlen(bt_txdh_hci_cmd);
            priv_cmd.used_len = buf_len;
        } else {
            is_param_err = 1;
        }
    } else if ((strcasecmp(argV[2], "BT_RXDH") == 0)) {
        if (argC == 16) {
            char bt_rxdh_hci_cmd[255] = "01 0b 18 0d ";
            buf_len = priv_cmd.used_len;
            int arg_len = strlen(argV[2]);
            int rxdh_cmd_len = strlen(bt_rxdh_hci_cmd);
            memcpy(&bt_rxdh_hci_cmd[rxdh_cmd_len], &priv_cmd.buf[arg_len+1], buf_len - arg_len - 1);
            memcpy(&priv_cmd.buf[arg_len+1], &bt_rxdh_hci_cmd[0], strlen(bt_rxdh_hci_cmd));
            buf_len += strlen(bt_rxdh_hci_cmd);
            priv_cmd.used_len = buf_len;
        } else {
            is_param_err = 1;
        }
    }  else if ((strcasecmp(argV[2], "BT_STOP") == 0)) {
        char bt_stop_hci_cmd[255] = "01 0C 18 01 ";
        if (argC == 4) {
            buf_len = priv_cmd.used_len;
            int arg_len = strlen(argV[2]);
            int stop_cmd_len = strlen(bt_stop_hci_cmd);
            memcpy(&bt_stop_hci_cmd[stop_cmd_len], &priv_cmd.buf[arg_len+1], buf_len - arg_len - 1);
            memcpy(&priv_cmd.buf[arg_len+1], &bt_stop_hci_cmd[0], strlen(bt_stop_hci_cmd));
            buf_len += strlen(bt_stop_hci_cmd);
            priv_cmd.used_len = buf_len;
        } else {
            is_param_err = 1;
        }
    } else {
        is_param_err = 0;
    }

    if (is_param_err) {
        printf("param error!!!\n");
        return 0;
    }

    if ((ret = ioctl(sock, TXRX_PARA, &ifr)) < 0) {
        printf("%s: error ioctl[TX_PARA] ret= %d\n", __func__, ret);
        return ret;
    }

    memcpy(&priv_cmd, ifr.ifr_data, sizeof(struct android_wifi_priv_cmd));
    if (strcasecmp(argV[2], "SET_FREQ_CAL") == 0)
        printf("done: freq_cal: 0x%8x\n", *(unsigned int *)priv_cmd.buf);
    else if (strcasecmp(argV[2], "GET_EFUSE_BLOCK") == 0)
        printf("done:efuse: 0x%8x\n", *(unsigned int *)priv_cmd.buf);
    else if (strcasecmp(argV[2], "SET_XTAL_CAP") == 0)
        printf("done:xtal cap: 0x%x\n", *(unsigned int *)priv_cmd.buf);
    else if (strcasecmp(argV[2], "SET_XTAL_CAP_FINE") == 0)
        printf("done:xtal cap fine: 0x%x\n", *(unsigned int *)priv_cmd.buf);
    else if (strcasecmp(argV[2], "GET_RX_RESULT") == 0)
        printf("done: getrx fcsok=%d, total=%d\n", *(unsigned int *)priv_cmd.buf, *(unsigned int *)&priv_cmd.buf[4]);
    else if (strcasecmp(argV[2], "GET_MAC_ADDR") == 0) {
        printf("done: get macaddr = %x : %x : %x : %x : %x : %x\n",
            *(unsigned char *)&priv_cmd.buf[5], *(unsigned char *)&priv_cmd.buf[4], *(unsigned char *)&priv_cmd.buf[3],
            *(unsigned char *)&priv_cmd.buf[2], *(unsigned char *)&priv_cmd.buf[1], *(unsigned char *)&priv_cmd.buf[0]);
    } else if (strcasecmp(argV[2], "GET_BT_MAC_ADDR") == 0) {
        printf("done: get bt macaddr = %x : %x : %x : %x : %x : %x\n",
            *(unsigned char *)&priv_cmd.buf[5], *(unsigned char *)&priv_cmd.buf[4], *(unsigned char *)&priv_cmd.buf[3],
            *(unsigned char *)&priv_cmd.buf[2], *(unsigned char *)&priv_cmd.buf[1], *(unsigned char *)&priv_cmd.buf[0]);
    } else if (strcasecmp(argV[2], "GET_FREQ_CAL") == 0) {
        unsigned int val = *(unsigned int *)&priv_cmd.buf[0];
        printf("done: get_freq_cal: xtal_cap=0x%x, xtal_cap_fine=%x\n", val & 0x0000ffff, (val >> 16) & 0x0000ffff);
    } else if (strcasecmp(argV[2], "GET_VENDOR_INFO") == 0) {
        printf("done: get_vendor_info = 0x%x\n", *(unsigned char *)&priv_cmd.buf[0]);
    } else if (strcasecmp(argV[2], "RDWR_PWRMM") == 0) {
        printf("done: txpwr manual mode = %x\n", *(unsigned int *)&priv_cmd.buf[0]);
    } else if (strcasecmp(argV[2], "RDWR_PWRIDX") == 0) {
        char *buff = &priv_cmd.buf[0];
        printf("done:\n"
               "txpwr index 2.4g:\n"
               "  [0]=%d(ofdmlowrate)\n"
               "  [1]=%d(ofdmhighrate)\n"
               "  [2]=%d(ofdm1024qam)\n"
               "  [3]=%d(dsss)\n", buff[0], buff[1], buff[2], buff[3]);
        printf("txpwr index 5g:\n"
               "  [0]=%d(ofdmlowrate)\n"
               "  [1]=%d(ofdmhighrate)\n"
               "  [2]=%d(ofdm1024qam)\n", buff[4], buff[5], buff[6]);
    } else if (strcasecmp(argV[2], "RDWR_PWROFST") == 0) {
        char *buff = &priv_cmd.buf[0];
        printf("done:\n"
               "txpwr offset 2.4g:\n"
               "  [0]=%d(ch1~4)\n"
               "  [1]=%d(ch5~9)\n"
               "  [2]=%d(ch10~13)\n", buff[0], buff[1], buff[2]);
        printf("txpwr offset 5g:\n"
               "  [0]=%d(ch36~64)\n"
               "  [1]=%d(ch100~120)\n"
               "  [2]=%d(ch122~140)\n"
               "  [3]=%d(ch142~165)\n", buff[3], buff[4], buff[5], buff[6]);
    } else if (strcasecmp(argV[2], "RDWR_DRVIBIT") == 0) {
        char *buff = &priv_cmd.buf[0];
	int idx = 0;
        printf("done: 2.4g txgain tbl pa drv_ibit:\n");
        for (idx = 0; idx < 16; idx++) {
            printf(" %x", buff[idx]);
            if (!((idx + 1) & 0x03)) {
                printf(" [%x~%x]\n", idx - 3, idx);
            }
        }
    } else if (strcasecmp(argV[2], "RDWR_EFUSE_PWROFST") == 0) {
        char *buff = &priv_cmd.buf[0];
        printf("done:\n"
               "efuse txpwr offset 2.4g:\n"
               "  [0]=%d(ch1~4)\n"
               "  [1]=%d(ch5~9)\n"
               "  [2]=%d(ch10~13)\n", buff[0], buff[1], buff[2]);
        printf("efuse txpwr offset 5g:\n"
               "  [0]=%d(ch36~64)\n"
               "  [1]=%d(ch100~120)\n"
               "  [2]=%d(ch122~140)\n"
               "  [3]=%d(ch142~165)\n", buff[3], buff[4], buff[5], buff[6]);
    } else if (strcasecmp(argV[2], "RDWR_EFUSE_DRVIBIT") == 0) {
        printf("done: efsue 2.4g txgain tbl pa drv_ibit: %x\n", priv_cmd.buf[0]);
    } else {
        printf("done\n");
    }

    return ret;
}

int main(int argC, char *argV[])
{

	char* ins = "insmod";
	char* rm = "rmmod";
	char* ko = "rwnx_fdrv.ko";

//	printf("enter!!!AIC    argC=%d    argV[0]=%s    argV[1]=%s    argV[2]=%s\n", argC, argV[0], argV[1],argV[2]);
	if(argC >= 3)
		wifi_send_cmd_to_net_interface(argV[1], argC, argV);
	else
		printf("Bad parameter! %d\n",argC);

	return 0;
}
