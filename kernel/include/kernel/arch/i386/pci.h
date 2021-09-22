#ifndef __ARCH_I386_PCI_H__

    #define __ARCH_I386_PCI_H__

    #define PCI_CONFIG_ADDRESS 0xCF8
    #define PCI_CONFIG_DATA    0xCFC

    #include <stdint.h>

    enum PCI_Header_Types {
        PCI_ENDPOINT                = 0x00,
        PCI_TO_PCI                  = 0x01,
        PCI_TO_CARDBUS              = 0x02,
    };

    enum PCI_Classes {
        PCI_UNCLASSIFIED            = 0x00,
        PCI_MASS_STORAGE            = 0x01,
        PCI_NETWORK                 = 0x02,
        PCI_DISPLAY                 = 0x03,
        PCI_MULTIMEDIA              = 0x04,
        PCI_MEMORY                  = 0x05,
        PCI_BRIDGE                  = 0x06,
        PCI_SIMPLE_COMMUNICATION    = 0x07,
        PCI_BASE_SYSTEM_PERIPHERAL  = 0x08,
        PCI_INPUT_DEVICE            = 0x09,
        PCI_DOCKING_STATION         = 0x0A,
        PCI_PROCESSOR               = 0x0B,
        PCI_SERIAL_BUS              = 0x0C,
        PCI_WIRELESS                = 0x0D,
        PCI_INTELLIGENT             = 0x0E,
        PCI_SATELLITE_COMMUNICATION = 0x0F,
        PCI_ENCRYPTION              = 0x10,
        PCI_SIGNAL_PROCESSING       = 0x11,
        PCI_PROCESSING_ACCELERATOR  = 0x12,
        PCI_NON_ESSENTIAL           = 0x13,
        PCI_COPROCESSOR             = 0x40,
        PCI_UNASSIGNED              = 0xFF,
    };

    enum PCI_Subclasses {
        PCI_OTHER                   = 0x80,

        PCI_NON_VGA_UNCLASSIFIED    = 0x00,
        PCI_VGA_UNCLASSIFIED        = 0x01,

        PCI_SCSI_CONTROLLER         = 0x00,
        PCI_IDE_CONTROLLER          = 0x01,
        PCI_FLOPPY_CONTROLLER       = 0x02,
        PCI_IPI_CONTROLLER          = 0x03,
        PCI_RAID_CONTROLLER         = 0x04,
        PCI_ATA_CONTROLLER          = 0x05,
        PCI_SATA_CONTROLLER         = 0x06,
        PCI_SERIAL_SCSI_CONTROLLER  = 0x07,
        PCI_NON_VOLATILE_MEMORY     = 0x08,

        PCI_ETHERNET_CONTROLLER     = 0x00,
        PCI_TOKEN_RING_CONTROLLER   = 0x01,
        PCI_FDDI_CONTROLLER         = 0x02,
        PCI_ATM_CONTROLLER          = 0x03,
        PCI_ISDN_CONTROLLER         = 0x04,
        PCI_WORLDFIP_CONTROLLER     = 0x05,
        PCI_PICMG_CONTROLLER        = 0x06,
        PCI_INFINIBAND_CONTROLLER   = 0x07,
        PCI_FABRIC_CONTROLLER       = 0x08,

        PCI_VGA_CONTROLLER          = 0x00,
        PCI_XGA_CONTROLLER          = 0x01,
        PCI_3D_CONTROLLER           = 0x02,

        PCI_MULTIMEDIA_VIDEO        = 0x00,
        PCI_MULTIMEDIA_AUDIO        = 0x01,
        PCI_COMPUTER_TELEPHONY      = 0x02,
        PCI_AUDIO_DEVICE            = 0x03,

        PCI_RAM_CONTROLLER          = 0x00,
        PCI_FLASH_CONTROLLER        = 0x01,

        PCI_HOST_BRIDGE             = 0x00,
        PCI_ISA_BRIDGE              = 0x01,
        PCI_EISA_BRIDGE             = 0x02,
        PCI_MCA_BRIDGE              = 0x03,
        PCI_PCI_BRIDGE              = 0x04,
        PCI_PCMCIA_BRIDGE           = 0x05,
        PCI_NUBUS_BRIDGE            = 0x06,
        PCI_CARDBUS_BRIDGE          = 0x07,
        PCI_RACEWAY_BRIDGE          = 0x08,
        PCI_PCI_TO_PCI_BRIDGE       = 0x09,
        PCI_INFINIBAND_BRIDGE       = 0x0A,

        PCI_SERIAL_CONTROLLER       = 0x00,
        PCI_PARALLEL_CONTROLLER     = 0x01,
        PCI_MULTIPORT_SERIAL        = 0x02,
        PCI_MODEM                   = 0x03,
        PCI_GPIB_CONTROLLER         = 0x04,
        PCI_SMART_CARD_CONTROLLER   = 0x05,

        PCI_PIC                     = 0x00,
        PCI_DMA_CONTROLLER          = 0x01,
        PCI_TIMER                   = 0x02,
        PCI_RTC_CONTROLLER          = 0x03,
        PCI_HOT_PLUG_CONTROLLER     = 0x04,
        PCI_SD_HOST_CONTROLLER      = 0x05,
        PCI_IOMMU                   = 0x06,

        PCI_KEYBOARD_CONTROLLER     = 0x00,
        PCI_DIGITIZER_PEN           = 0x01,
        PCI_MOUSE_CONTROLLER        = 0x02,
        PCI_SCANNER_CONTROLLER      = 0x03,
        PCI_GAMEPORT_CONTROLLER     = 0x04,

        PCI_DOCKING_GENERIC         = 0x00,

        PCI_PROCESSOR_386           = 0x00,
        PCI_PROCESSOR_486           = 0x01,
        PCI_PROCESSOR_PENTIUM       = 0x02,
        PCI_PROCESSOR_PENIUM_PRO    = 0x03,
        PCI_PROCESSOR_ALPHA         = 0x10,
        PCI_PROCESSOR_POWERPC       = 0x20,
        PCI_PROCESSOR_MIPS          = 0x30,
        PCI_PROCESSOR_COPROCESSOR   = 0x40,

        PCI_FIREWIRE_CONTROLLER     = 0x00,
        PCI_ACCESS_BUS_CONTROLLER   = 0x01,
        PCI_SSA                     = 0x02,
        PCI_USB_CONTROLLER          = 0x03,
        PCI_FIBRE_CHANNEL           = 0x04,
        PCI_SMBUS_CONTROLLER        = 0x05,
        PCI_SERIAL_INFINIBAND       = 0x06,
        PCI_IPMI_INTERFACE          = 0x07,
        PCI_SERCOS_INTERFACE        = 0x08,
        PCI_CANBUS_CONTROLLER       = 0x09,

        PCI_IRDA_COMPATIBLE         = 0x00,
        PCI_IR_CONTROLLER           = 0x01,
        PCI_RF_CONTROLLER           = 0x10,
        PCI_BLUETOOTH_CONTROLLER    = 0x11,
        PCI_BROADBAND_CONTROLLER    = 0x12,
        PCI_ETHERNET_802_1_A        = 0x20,
        PCI_ETHERNET_802_1_B        = 0x21,

        PCI_SAT_TV_CONTROLLER       = 0x01,
        PCI_SAT_AUDIO_CONTROLLER    = 0x02,
        PCI_SAT_VOICE_CONTROLLER    = 0x03,
        PCI_SAT_DATA_CONTROLLER     = 0x04,

        PCI_NETWORK_ENCRYPT         = 0x00,
        PCI_ENTERTAINMENT_ENCRYPT   = 0x10,

        PCI_DPIO_MODULE             = 0x00,
        PCI_PERFORMANCE_COUNTER     = 0x01,
        PCI_COMMUNICATION_SYNC      = 0x10,
        PCI_PROCESSING_MANAGEMENT   = 0x20,
    };

    uint16_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg);

#endif
