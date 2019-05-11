
#define 	KSZ8841_BASE               0xbe000300// 0xba000300//0xbd000300


#define		KSZ8841_REG_00        		(KSZ8841_BASE + 0x00)
#define 	KSZ8841_REG_01        		(KSZ8841_BASE + 0x01)
#define 	KSZ8841_REG_02        		(KSZ8841_BASE + 0x02)
#define 	KSZ8841_REG_03        		(KSZ8841_BASE + 0x03)
#define 	KSZ8841_REG_04        		(KSZ8841_BASE + 0x04)
#define 	KSZ8841_REG_05        		(KSZ8841_BASE + 0x05)
#define 	KSZ8841_REG_06        		(KSZ8841_BASE + 0x06)
#define 	KSZ8841_REG_07        		(KSZ8841_BASE + 0x07)
#define 	KSZ8841_REG_08        		(KSZ8841_BASE + 0x08)
#define 	KSZ8841_REG_09        		(KSZ8841_BASE + 0x09)
#define 	KSZ8841_REG_0a        		(KSZ8841_BASE + 0x0a)
#define 	KSZ8841_REG_0b        		(KSZ8841_BASE + 0x0b)
#define 	KSZ8841_REG_0c        		(KSZ8841_BASE + 0x0c)
#define 	KSZ8841_REG_0d        		(KSZ8841_BASE + 0x0d)
#define 	KSZ8841_REG_0e       	 	(KSZ8841_BASE + 0x0e)
#define 	KSZ8841_REG_0f        		(KSZ8841_BASE + 0x0f)

#define 	REG16_BankSelect							KSZ8841_REG_0e
#define 	REG16_Bank0_BaseAddr						KSZ8841_REG_00
#define 	REG16_Bank0_BusErrorStatus					KSZ8841_REG_06
#define		REG16_Bank2_HostMacAddLow					KSZ8841_REG_00
#define 	REG16_Bank2_HostMacAddMid					KSZ8841_REG_02
#define 	REG16_Bank2_HostMacAddHig					KSZ8841_REG_04
#define		REG16_Bank3_OnChipBus						KSZ8841_REG_00
#define		REG16_Bank3_GlobalReset						KSZ8841_REG_06
#define		REG16_Bank3_PowerManagement 				KSZ8841_REG_08
#define		REG16_Bank16_TransmitControl				KSZ8841_REG_00
#define		REG16_Bank16_TransmitStatus					KSZ8841_REG_02
#define		REG16_Bank16_ReceiveControl					KSZ8841_REG_04
#define		REG16_Bank16_TXQMemoryInfo					KSZ8841_REG_08
#define 	REG16_Bank16_RXQMemoryInfo					KSZ8841_REG_0a
#define		REG16_Bank17_TXQCommand						KSZ8841_REG_00
#define		REG16_Bank17_RXQCommand						KSZ8841_REG_02
#define		REG16_Bank17_TXFrameDataPointer				KSZ8841_REG_04
#define		REG16_Bank17_RXFrameDataPointer				KSZ8841_REG_06
#define		REG16_Bank17_QMUDataLow						KSZ8841_REG_08
#define		REG16_Bank17_QMUDataHig						KSZ8841_REG_0a
#define		REG16_Bank18_InterruptEnable				KSZ8841_REG_00
#define		REG16_Bank18_InterruptStatus				KSZ8841_REG_02
#define		REG16_Bank18_ReceiveStatus					KSZ8841_REG_04
#define		REG16_Bank18_ReceiveByteCounter				KSZ8841_REG_06
#define		REG16_Bank19_PowerManagementControl			KSZ8841_REG_08
#define		REG16_Bank19_PowerManagementStatus			KSZ8841_REG_08
#define		REG16_Bank32_ChipId							KSZ8841_REG_00
#define		REG16_Bank32_ChipGlobalControl				KSZ8841_REG_0a
#define		REG16_Bank42_IndirectAccessControl			KSZ8841_REG_00
#define		REG16_Bank42_IndirectAccessData1			KSZ8841_REG_02
#define		REG16_Bank42_IndirectAccessData2			KSZ8841_REG_04
#define		REG16_Bank42_IndirectAccessData3			KSZ8841_REG_06
#define		REG16_Bank42_IndirectAccessData4			KSZ8841_REG_08
#define		REG16_Bank42_IndirectAccessData5			KSZ8841_REG_0a
#define		REG16_Bank45_PHY1MIIRegisterBasicControl	KSZ8841_REG_00
#define		REG16_Bank45_PHY1MIIRegisterBasicStatus		KSZ8841_REG_02
#define		REG16_Bank45_PHY1PHYIDLow					KSZ8841_REG_04
#define		REG16_Bank45_PHY1PHYIDHig					KSZ8841_REG_06
#define		REG16_Bank45_PHY1ANAdvertisement			KSZ8841_REG_08
#define		REG16_Bank45_PHY1ANLinkPatnerAbility		KSZ8841_REG_10
#define		REG16_Bank47_PHY1LinkMDControl				KSZ8841_REG_00
#define		REG16_Bank47_PHY1LinkMDStatus				KSZ8841_REG_00
#define		REG16_Bank47_PHY1SpecialControl				KSZ8841_REG_02
#define		REG16_Bank47_PHY1SpecialStatus				KSZ8841_REG_02
#define		REG16_Bank49_Port1PHYSpecialContol			KSZ8841_REG_00
#define		REG16_Bank49_Port1PHYSpecialStatus			KSZ8841_REG_00
#define		REG16_Bank49_Port1Control4					KSZ8841_REG_02
#define		REG16_Bank49_Port1Status					KSZ8814_REG_04

#define ARP_HDR_SIZE	42		/* Size assuming ethernet	*/
#define OUR_IP_ADDR     0xC0A80109
#define DST_IP_ADDR     0xC0A80105

