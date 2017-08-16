#ifndef _SW_CTRL_H
#define _SW_CTRL_H


#define u32	unsigned int
#define u16   unsigned short
#define u8	unsigned char

#define BCM5645_NAME	"/dev/switch"
#define SW_IOCTL_MAGIC 'L'

#define IOCTL_READ_PHY_REG _IOWR( SW_IOCTL_MAGIC, 0x4, drv_phy_reg )
#define IOCTL_WRITE_PHY_REG _IOW( SW_IOCTL_MAGIC, 0x5, drv_phy_reg )
#define IOCTL_READ_RAM _IOWR( SW_IOCTL_MAGIC, 0x8, drv_ram )
#define IOCTL_WRITE_RAM _IOW( SW_IOCTL_MAGIC, 0x9, drv_ram )
#define IOCTL_SET_PVLAN  _IOW( SW_IOCTL_MAGIC, 0x15, drv_pvlan_struct )	
#define IOCTL_GET_PVLAN  _IOR( SW_IOCTL_MAGIC, 0x16, drv_pvlan_struct )
#define IOCTL_GET_PORT_CFG _IOWR( SW_IOCTL_MAGIC, 0x24, drv_port_cfg_struct )
#define IOCTL_GET_MCGRP	_IOWR( SW_IOCTL_MAGIC, 0x32, drv_mcgrp_struct )
#define IOCTL_JOIN_MCGRP	_IOWR( SW_IOCTL_MAGIC, 0x35, drv_mcgrp_struct )
#define IOCTL_LEAVE_MCGRP	_IOWR( SW_IOCTL_MAGIC, 0x36, drv_mcgrp_struct )
#define IOCTL_GET_AMCGRP	_IOWR( SW_IOCTL_MAGIC, 0x60, mc_group_struct )
#define IOCTL_GET_IGMPQRYTIME _IOWR(SW_IOCTL_MAGIC, 0x61, drv_igmp_qrytime_struct )
#define IOCTL_SET_IGMPQRYTIME _IOWR(SW_IOCTL_MAGIC, 0x62, drv_igmp_qrytime_struct )
#define IOCTL_GET_IGMP_PARAM _IOR(SW_IOCTL_MAGIC, 0x63, drv_igmp_parameter_struct )
#define IOCTL_SET_IGMP_PARAM _IOW(SW_IOCTL_MAGIC, 0x64, drv_igmp_parameter_struct )
#define IOCTL_GET_DMACOUNTERS _IOWR(SW_IOCTL_MAGIC, 0x71, drv_dma_counters_struct)
#define IOCTL_CLEAR_DMACOUNTERS _IOWR(SW_IOCTL_MAGIC, 0x72, drv_dma_counters_struct)

#define IOCTL_GET_L3_CAP  _IOW( SW_IOCTL_MAGIC, 0x81, u32)	
#define IOCTL_ADD_L3_INTF  _IOW( SW_IOCTL_MAGIC, 0x82, u32)	
#define IOCTL_GET_L3_INTF  _IOW( SW_IOCTL_MAGIC, 0x83, u32)	
#define IOCTL_DEL_L3_INTF  _IOW( SW_IOCTL_MAGIC, 0x84, u32)	

#define MARL_LEN_MAX	256
#define DEFAULT_TIMEOUT_FOR_PORT		180000000u	/* 180 seconds. */
#define LONGLIVE_TIMEOUT_FOR_PORT		0xFFFFFFFFu


/* *******************************************
  *   Virtual Eth driver mapping with L3 interface
  *        eth0 and eth1 are used by CPU, 
  *        eth2 is used by in-band management IP port
  *        eth3 ~ MAX is used for L3 interface      
  ********************************************/
#define MAX_L3INTERFACE	32
#define VNET_BASE 			2


/*
m Identifies a block number:
0 = 100-Mbit Ports 1-8
1 = 100-Mbit Ports 9-16
2 = 100-Mbit Ports 17-24
3 = Gigabit Port 1
4 = Gigabit Port 2
5 = the CMIC (CPU)

n Identifies a specific port within a block of eight. 0 = Port 1 and 7 = Port 8
*/
#define RBYT( m, n )		(0x27 + ((m) << 20) + ((n) << 12))//32 bits
#define RPKT( m, n )		(0x28 + ((m) << 20) + ((n) << 12))//20 bits
#define RMCA( m, n )		(0x2a + ((m) << 20) + ((n) << 12))//20 bits
#define RBCA( m, n )		(0x2b + ((m) << 20) + ((n) << 12))//20 bits

#define TBYT( m, n )		(0x38 + ((m) << 20) + ((n) << 12))//32 bits
#define TPKT( m, n )		(0x39 + ((m) << 20) + ((n) << 12))//20 bits
#define TMCA( m, n )		(0x3A + ((m) << 20) + ((n) << 12))//20 bits
#define TBCA( m, n )		(0x3B + ((m) << 20) + ((n) << 12))//20 bits

#define GRBYT(m)		(0x27 + ((m)<<20))//32 bits
#define GRPKT(m)			(0x28 + ((m)<<20))//24 bits	
#define GRMCA(m)		(0x2A + ((m)<<20))//24 bits
#define GRBCA(m)		(0x2B + ((m)<<20))//24 bits

#define GTBYT(m)			(0x38 + ((m)<<20))//32 bits
#define GTPKT(m)			(0x39 + ((m)<<20))//24 bits	
#define GTMCA(m)		(0x3A + ((m)<<20))//24 bits
#define GTBCA(m)		(0x3B + ((m)<<20))//24 bits

#define CAR1( m, n )		(0x4C + ((m) << 20) +((n) << 12))
#define CAR2( m, n )		(0x4D + ((m) << 20) +((n) << 12))
#define GCAR1(m)		(0x4C + ((m) << 20))
#define GCAR2(m)		(0x4D + ((m) << 20))

typedef struct drv_ram_struct{
	u32 addr;
	u32 data[21];
	u32 len;
}drv_ram;

typedef struct drv_phy_reg_struct{
	u32 addr;
	u16 data;
}drv_phy_reg;

typedef struct drv_pvlan_struct {
	u32 ut_port_bitmap:29;
	u32 port_bitmap:29;
	u32 valid;
	u32 vlan_id:12;
}drv_pvlan_struct;

typedef struct drv_port_cfg_struct {
	u16	pid:12;		/* 1-based port id	*/
	u16 tag:1,		/* tag=1 indicates packets from the port are tagged. */
		type:2,		/* 0=10/100BASE-T,1=10/100/1000BASE-T,2=1000BASE-X */
		port_en:1,	/* 1=enable */
		autoneg_en:1,	/* 1=enable auto negotiate */
		duplex:1,		/* 1=full duplex */
		speed:2,			/* 0=10M, 1=100M, 2=1000M */
		flowctrl_en:1,	/* 1=enable 802.3 flow control */
		link:1;
} drv_port_cfg_struct;


typedef struct drv_mcgrp_struct{
	u8 pid;		/* inner port id, 0 based. */
	u32 gip;		/* multicast group IP address. */
	u16 vid;		/* vlan id, 1 based. */
	u8 ports_stat[26];/* DRV_MCGRP_PORT_STAT */
}drv_mcgrp_struct;

typedef struct{
        u8 stat;                /* Port status, STAT_ALIVE or STAT_DEAD in a group. */
        u32 ttl;                /* Time for port to live in a group, unit is usec. */
}member_port_struct;

typedef struct{
        u8 port_num;    /* Number of member ports of this group. IDENTIFY if the group is in use. */
        u16 pvid;               /* Port vlan id. */
        u32 grp_ip;             /* IP address of the multicast group. */
        u32 bitmap;             /* Port bitmap of the group, 0-based. */
        member_port_struct ports[28];      /* Port id is 0 based. */
}mc_group_struct;

typedef struct drv_igmp_qrytime_struct{
	u32 query_time;/*unit:second*/
}drv_igmp_qrytime_struct;

typedef struct drv_igmp_parameter_struct{
	u32 query_time;/*unit:second*/
	u32 timeout; 
	u32 query_ports;
	u32 router_ports;
}drv_igmp_parameter_struct;

typedef struct drv_dma_counters_struct{
	unsigned long counterdata[1664];/*26ports * 64 counters per port */
}drv_dma_counters_struct;


typedef struct drv_sw_l3_interface_struct{
	u32	reserved:15,
		venet_idx:5,
		vlan_tag:12;
}drv_sw_l3_interface_struct;

int GetSWCounters( unsigned char cardno,  SW_PORT_COUNT *swcount );
int ReseSWCounters(unsigned char  card_no );
int GetAllVlan(unsigned char card_no );
int SaveAndDelVlan(unsigned char  card_no,SW_VLAN *vlan, unsigned char  msg_code );
int GetSWportstate(unsigned char  card_no, SW_PORT_STATE *swport );
int GetALLMCgroup( unsigned char card_no, SW_MC_STATE *amcg, int *tnum );
int AddNewgrp( unsigned char card_no,  SW_MC_STATE *ngrp );
int SaveAndDelMC(unsigned char  card_no,SW_MC_STATE *mcg, unsigned char  msg_code );
int GetConfigParam(unsigned char card_no, SW_CONFIG *swconfig );
int SetConfigParam(unsigned char card_no, SW_CONFIG *swconfig );
int saveIGMPParam(const SW_CONFIG *param);
int UpdateL3Interface(SW_L3INTF *l3intf, unsigned char  msg_code );
void  InitL3Interface();
int SetFEPort100MFull();



#endif
