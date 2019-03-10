
//#define USE_BMP085
#define ALTITUDE_M	210.0f

void scheduler_realtime();
void scheduler_standard();
void calculate_values(unsigned char *buf);

// Tony

#define RFM01_CE        BCM2835_SPI_CS0         // SPI chip select
#define HIGH 0x1
#define LOW  0x0
#define RFM01_IRQ       RPI_V2_GPIO_P1_15          // SPI IRQ GPIO pin. tony
#define RFM01_DATA 		RPI_V2_GPIO_P1_13    		// Data tony


#define CMD_STATUS              0x0000
#define CMD_FREQ                0xa000

#define CMD_CONFIG              0x8000
#define CMD_RCON                0xc000
#define CMD_WAKEUP              0xe000
#define CMD_LOWDUTY             0xcc00
#define CMD_LOWBATT             0xc200
#define CMD_AFC                 0xc600
#define CMD_DFILTER             0xc420
#define CMD_DRATE               0xc800
#define CMD_FIFO                0xce00
#define CMD_RSTMODE             0xda00

#define CMD_RESET               0xff00

// Configuration settings

// Frequency Band (in MHz), bits 12-11

#define BAND_315                (0 << 11)   //  ...0 0... .... ....
#define BAND_433                (1 << 11)   //  ...0 1... .... ....
#define BAND_868                (2 << 11)   //  ...1 0... .... ....
#define BAND_915                (3 << 11)   //  ...1 1... .... ....

#define LOWBATT_EN              (1 << 10)   //  .... .1.. .... .... --> controls the operation of the low battery detector
#define WAKEUP_EN               (1 << 9)    //  .... ..1. .... .... --> controls the operation of the wake-up timer
#define CRYSTAL_EN              (1 << 8)    //  .... ...1 .... .... --> contros if the crystal is active during sleep mode
#define NO_CLOCK                1           //  .... .... .... ...1 --> disables the clock output

// Crystal Load Capacitance (in pF), bits 7-4

#define LOAD_CAP_8C5            (0 << 4)
#define LOAD_CAP_9C0            (1 << 4)
#define LOAD_CAP_9C5            (2 << 4)
#define LOAD_CAP_10C0           (3 << 4)
#define LOAD_CAP_10C5           (4 << 4)
#define LOAD_CAP_11C0           (5 << 4)
#define LOAD_CAP_11C5           (6 << 4)
#define LOAD_CAP_12C0           (7 << 4)
#define LOAD_CAP_12C5           (8 << 4)
#define LOAD_CAP_13C0           (9 << 4)
#define LOAD_CAP_13C5           (10 << 4)
#define LOAD_CAP_14C0           (11 << 4)
#define LOAD_CAP_14C5           (12 << 4)
#define LOAD_CAP_15C0           (13 << 4)
#define LOAD_CAP_15C5           (14 << 4)
#define LOAD_CAP_16C0           (15 << 4)

// Baseband Bandwidth (in kHz)

#define BW_X1                   (0 << 1)    // reserved
#define BW_400                  (1 << 1)
#define BW_340                  (2 << 1)
#define BW_270                  (3 << 1)
#define BW_200                  (4 << 1)
#define BW_134                  (5 << 1)
#define BW_67                   (6 << 1)
#define BW_X2                   (7 << 1)    // reserved


// Receiver Settings

// VDI (valid data indicator) signal, bits 7-6

#define VDI_DRSSI               (0 << 6)    // .... .... 00.. .... --> Digital RSSI Out(DRSSI)
#define VDI_DQD                 (1 << 6)    // .... .... 01.. .... --> Data Quality Detector Output(DQD)
#define VDI_CR_LOCK             (2 << 6)    // .... .... 10.. .... --> Clock recovery lock
#define VDI_DRSSIDQD            (3 << 6)    // .... .... 11.. .... --> DRSSI && DQD

// LNA gain set (in dB), bits 5-4

#define LNA_0                   (0 << 4)    // .... .... ..00 ....
#define LNA_6                   (2 << 4)    // .... .... ..01 ....
#define LNA_14                  (1 << 4)    // .... .... ..10 ....
#define LNA_20                  (3 << 4)    // .... .... ..11 ....

#define LNA_XX                  (3 << 4)

// Threshold of the RSSI detector (in dBm), bits 1-3

#define RSSI_103                (0 << 1)    // .... .... .... 000.
#define RSSI_97                 (1 << 1)    // .... .... .... 001.
#define RSSI_91                 (2 << 1)    // .... .... .... 010.
#define RSSI_85                 (3 << 1)    // .... .... .... 011.
#define RSSI_79                 (4 << 1)    // .... .... .... 100.
#define RSSI_73                 (5 << 1)    // .... .... .... 101.
#define RSSI_X1                 (6 << 1)    // .... .... .... 110. --> reserved
#define RSSI_X2                 (7 << 1)    // .... .... .... 111. --> reserved

#define RX_EN                   1           // .... .... .... ...1 --> enables the whole receiver chain


// DFILTER values

// Clock recovery, bit 7-6

#define CR_AUTO                 (1 << 7)    // .... .... 1... .... --> auto lock control
#define CR_LOCK_FAST            (1 << 6)    // .... .... .1.. .... --> fast mode
#define CR_LOCK_SLOW            (0 << 6)    // .... .... .0.. .... --> slow mode

// Data filter type, bits 4-3

#define FILTER_OOK              (0 << 3)    // .... .... ...0 0... --> OOK to filter
#define FILTER_DIGITAL          (1 << 3)    // .... .... ...0 1... --> Digital filter
#define FILTER_X                (2 << 3)    // .... .... ...1 0... --> reserved
#define FILTER_ANALOG           (3 << 3)    // .... .... ...1 1... --> Analog RC filter

// DQD threshold parameter, bits 2-0
// Note: To let the DQD report "good signal quality" the threshold parameter should be less than 4 in the case when the bitrate is
// close to the deviation. At higher deviation/bitrate settings higher threshold parameter can report "good signal quality" as well.

#define DQD_0                   0
#define DQD_1                   1
#define DQD_2                   2
#define DQD_3                   3
#define DQD_4                   4
#define DQD_5                   5
#define DQD_6                   6
#define DQD_7                   7

// AFC values

#define AFC_ON                  (1 << 0)    // .... .... .... ...1 --> enables the calculation of the offset frequency
#define AFC_OFF                 (0 << 0)    // .... .... .... ...0 --> disables the calculation of the offset frequency
#define AFC_OUT_ON              (1 << 1)    // .... .... .... ..1. --> enables the output (frequency offset) register
#define AFC_OUT_OFF             (0 << 1)    // .... .... .... ..0. --> disables the output (frequency offset) register
#define AFC_FINE                (1 << 2)    // .... .... .... .1.. --> switches the circuit to high accuracy (fine) mode
#define AFC_STROBE              (1 << 3)    // .... .... .... 1... --> strobe edge

// Range limit, bit 4-5

#define AFC_RL_NONE             (0 << 4)    // .... .... ..00 .... --> No restriction
#define AFC_RL_15               (1 << 4)    // .... .... ..01 .... --> +15/-16
#define AFC_RL_7                (2 << 4)    // .... .... ..10 .... --> +7/-8
#define AFC_RL_3                (3 << 4)    // .... .... ..11 .... --> +3/-4

// Automatic operation mode selector, bits 6-7

#define AFC_MANUAL              (0 << 6)    // .... .... 00.. .... --> Auto mode off (Strobe is controlled by microcontroller)
#define AFC_POWERUP             (1 << 6)    // .... .... 01.. .... --> Runs only once after each power-up
#define AFC_VDI                 (2 << 6)    // .... .... 10.. .... --> Drop the foffset value when the VDI signal is low
#define AFC_ALWAYS              (3 << 6)    // .... .... 11.. .... --> Keep the foffset value independently from the state of the VDI signal


// Status Register Read Sequence

#define STATUS_FFIT             (1 << 15)   // 1... .... .... .... --> Number of the data bits in the FIFO is reached the preprogrammed limit
#define STATUS_FFOV             (1 << 14)   // .1.. .... .... .... --> FIFO overflow
#define STATUS_WKUP             (1 << 13)   // ..1. .... .... .... --> Wake-up timer overflow
#define STATUS_LBD              (1 << 12)   // ...1 .... .... .... --> Low battery detect, the power supply voltage is below the preprogrammed limit
#define STATUS_FFEM             (1 << 11)   // .... 1... .... .... --> FIFO is empty
#define STATUS_RSSI             (1 << 10)   // .... .1.. .... .... -->
#define STATUS_DRSSI            (1 << 10)   // .... .1.. .... .... --> The strength of the incoming signal is above the preprogrammed limit
#define STATUS_DQD              (1 << 9)    // .... ..1. .... .... --> Data Quality Detector detected a good quality signal
#define STATUS_CRL              (1 << 8)    // .... ...1 .... .... --> Clock recovery lock
#define STATUS_ATGL             (1 << 7)    // .... .... 1... .... --> Toggling in each AFC cycle
#define STATUS_ASAME            (1 << 6)    // .... .... .1.. .... --> AFC stabilized (measured twice the same offset value)
#define STATUS_OFFS6            (1 << 5)    // .... .... ..1. .... --> Offset value to be added to the value of the Frequency control word
#define STATUS_OFFS4            (1 << 4)    // .... .... ...1 .... -->
#define STATUS_OFFS3            (1 << 3)    // .... .... .... 1... -->
#define STATUS_OFFS2            (1 << 2)    // .... .... .... .1.. -->
#define STATUS_OFFS1            (1 << 1)    // .... .... .... ..1. -->
#define STATUS_OFFS0            (1 << 0)    // .... .... .... ...1 -->
#define STATUS_OFFS             0x3f
#define STATUS_OFFSIGN          0x20

// RSSIth = RSSIsetth + Glna, where RSSIsetth is dBm and Glna is dB and is expressed as dB referenced to max G.
// This means that zero is the highest amplification, and the negative dB scales down the RSSIsetth threshold.

#define L0R73                   {"LNA_0,RSSI_73",LNA_0,RSSI_73}
#define L0R79                   {"LNA_0,RSSI_79",LNA_0,RSSI_79}
#define L0R85                   {"LNA_0,RSSI_85",LNA_0,RSSI_85}
#define L0R91                   {"LNA_0,RSSI_91",LNA_0,RSSI_91}
#define L0R97                   {"LNA_0,RSSI_97",LNA_0,RSSI_97}
#define L0R103                  {"LNA_0,RSSI_103",LNA_0,RSSI_103}
#define L6R73                   {"LNA_6,RSSI_73",LNA_6,RSSI_73}
#define L6R79                   {"LNA_6,RSSI_79",LNA_6,RSSI_79}
#define L6R85                   {"LNA_6,RSSI_85",LNA_6,RSSI_85}
#define L6R91                   {"LNA_6,RSSI_91",LNA_6,RSSI_91}
#define L6R97                   {"LNA_6,RSSI_97",LNA_6,RSSI_97}
#define L6R103                  {"LNA_6,RSSI_103",LNA_6,RSSI_103}
#define L14R73                  {"LNA_14,RSSI_73",LNA_14,RSSI_73}
#define L14R79                  {"LNA_14,RSSI_79",LNA_14,RSSI_79}
#define L14R85                  {"LNA_14,RSSI_85",LNA_14,RSSI_85}
#define L14R91                  {"LNA_14,RSSI_91",LNA_14,RSSI_91}
#define L14R97                  {"LNA_14,RSSI_97",LNA_14,RSSI_97}
#define L14R103                 {"LNA_14,RSSI_103",LNA_14,RSSI_103}
#define L20R73                  {"LNA_20,RSSI_73",LNA_20,RSSI_73}
#define L20R79                  {"LNA_20,RSSI_79",LNA_20,RSSI_79}
#define L20R85                  {"LNA_20,RSSI_85",LNA_20,RSSI_85}
#define L20R91                  {"LNA_20,RSSI_91",LNA_20,RSSI_91}
#define L20R97                  {"LNA_20,RSSI_97",LNA_20,RSSI_97}
#define L20R103                 {"LNA_20,RSSI_103",LNA_20,RSSI_103}
