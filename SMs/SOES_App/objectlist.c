/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * CoE Object Dictionary.
 *
 * Part of application, describe the slave and its process data.
 */

#include "esc_coe.h"
#include "utypes.h"
#include <stddef.h>

extern _Rbuffer Rb;
extern _Wbuffer Wb;
extern _Cbuffer Cb;
extern uint32_t encoder_scale;
extern uint32_t encoder_scale_mirror;

static const char acName1000[] = "Device Type";
static const char acName1008[] = "Manufacturer Device Name";
static const char acName1009[] = "Manufacturer Hardware Version";
static const char acName100A[] = "Manufacturer Software Version";
static const char acName1018[] = "Identity Object";
static const char acName1018_01[] = "Vendor ID";
static const char acName1018_02[] = "Product Code";
static const char acName1018_03[] = "Revision Number";
static const char acName1018_04[] = "Serial Number";
static const char acNameMO[] = "Mapped object";
static const char acName1600[] = "Receive PDO mapping";
static const char acName1A00[] = "Transmit PDO mapping";
static const char acName1C00[] = "Sync Manager Communication type";
static const char acName1C00_01[] = "Communications type SM0";
static const char acName1C00_02[] = "Communications type SM1";
static const char acName1C00_03[] = "Communications type SM2";
static const char acName1C00_04[] = "Communications type SM3";
static const char acName1C10[] = "Sync Manager 0 PDO Assignment";
static const char acName1C11[] = "Sync Manager 1 PDO Assignment";
static const char acName1C12[] = "Sync Manager 2 PDO Assignment";
static const char acName1C13[] = "Sync Manager 3 PDO Assignment";
static const char acNameNOE[] = "Number of entries";
/*-------------------AXIS COMM BOARD MOD-----------------------------*/
static const char acName6000[] = "Temperature Inputs";
static const char acName6000_01[] = "TEMPERATURE0";
static const char acName6000_02[] = "TEMPERATURE1";
static const char acName6000_03[] = "TEMPERATURE2";
static const char acName6000_04[] = "TEMPERATURE3";
static const char acName6000_05[] = "TEMPERATURE4";
static const char acName6000_06[] = "TEMPERATURE5";
static const char acName6000_07[] = "TEMPERATURE6";
static const char acName6000_08[] = "TEMPERATURE7";
static const char acName6000_09[] = "TEMPERATURE8";
static const char acName6000_10[] = "TEMPERATURE9";
static const char acName6000_11[] = "TEMPERATURE10";
static const char acName6000_12[] = "TEMPERATURE11";
static const char acName6000_13[] = "TEMPERATURE12";
static const char acName6000_14[] = "TEMPERATURE13";
static const char acName6000_15[] = "TEMPERATURE14";


static const char acName6001[] = "System Inputs";
static const char acName6001_01[] = "STATUS";
static const char acName6001_02[] = "EVENT";
static const char acName6001_03[] = "ERROR";

static const char acName7000[] = "Command outputs";
static const char acName7000_01[] = "COMMAND";
static const char acName7000_02[] = "TEST VALUE0";
static const char acName7000_03[] = "TEST VALUE1";
static const char acName7000_04[] = "TEST VALUE2";

static const char acNameF000[] = "Command outputs";
static const char acNameF000_01[] = "Index distance";
static const char acNameF000_02[] = "Maximum number of modules";


/*
static const char acName7100[] = "Parameters";
static const char acName7100_01[] = "Encoder scale";
static const char acName7100_02[] = "Encoder scale mirror";
static const char acName8001[] = "Slave commands";
static const char acName8001_01[] = "Reset counter";
*/

//Modified for axis comm board
static char ac1008_00[] = "axcommbsoes";
static char ac1009_00[] = "1.0s";
static char ac100A_00[] = "1.0s";

static char ac1018_01[] = "nos";	//VendorID
static char ac100A_02[] = "nos";	//ProductCode


const _objd SDO1000[] =
{ {0x00, DTYPE_UNSIGNED32, 32, ATYPE_RO, &acName1000[0], 0x00000000, NULL} };
const _objd SDO1008[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac1008_00) << 3, ATYPE_RO, &acName1008[0], 0, &ac1008_00[0]}
};
const _objd SDO1009[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac1009_00) << 3, ATYPE_RO, &acName1009[0], 0, &ac1009_00[0]}
};
const _objd SDO100A[] =
{ {0x00, DTYPE_VISIBLE_STRING, sizeof (ac100A_00) << 3, ATYPE_RO, &acName100A[0], 0, &ac100A_00[0]}
};
const _objd SDO1018[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x04, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RO, &acName1018_01[0], 0x00001337, NULL},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, &acName1018_02[0], 0x12783456, NULL},
  {0x03, DTYPE_UNSIGNED32, 32, ATYPE_RO, &acName1018_03[0], 0x00000001, NULL},
  {0x04, DTYPE_UNSIGNED32, 32, ATYPE_RO, &acName1018_04[0], 0x00000000, NULL}
};
const _objd SDO1600[] =	//Outputs from Master, to RXPDO
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x04, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x70000110, NULL},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x70000210, NULL},
  {0x03, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x70000310, NULL},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x70000410, NULL}

};
const _objd SDO1A00[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x12, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000110, NULL},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000210, NULL},
  {0x03, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000310, NULL},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000410, NULL},
  {0x05, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000510, NULL},
  {0x06, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000610, NULL},
  {0x07, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000710, NULL},
  {0x08, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000810, NULL},
  {0x09, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000910, NULL},
  {0x0A, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000A10, NULL},
  {0x0B, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000B10, NULL},
  {0x0C, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000C10, NULL},
  {0x0D, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000D10, NULL},
  {0x0E, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000E10, NULL},
  {0x0F, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60000F10, NULL},
  {0x10, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60010110, NULL},
  {0x11, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60010210, NULL},
  {0x12, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x60010310, NULL}

};
const _objd SDO1C00[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x04, NULL},
  {0x01, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acName1C00_01[0], 0x01, NULL},
  {0x02, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acName1C00_02[0], 0x02, NULL},
  {0x03, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acName1C00_03[0], 0x03, NULL},
  {0x04, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acName1C00_04[0], 0x04, NULL}
};
const _objd SDO1C10[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acName1C10[0], 0x00, NULL}
};
const _objd SDO1C11[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acName1C11[0], 0x00, NULL}
};
const _objd SDO1C12[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x1600, NULL}
};
const _objd SDO1C13[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acNameMO[0], 0x1A00, NULL}
};
//Modified for Axis Comm Board

const _objd SDO6000[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x0F, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_01[0], 0, &(Rb.temp[0])},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_02[0], 0, &(Rb.temp[1])},
  {0x03, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_03[0], 0, &(Rb.temp[2])},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_04[0], 0, &(Rb.temp[3])},
  {0x05, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_05[0], 0, &(Rb.temp[4])},
  {0x06, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_06[0], 0, &(Rb.temp[5])},
  {0x07, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_07[0], 0, &(Rb.temp[6])},
  {0x08, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_08[0], 0, &(Rb.temp[7])},
  {0x09, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_09[0], 0, &(Rb.temp[8])},
  {0x0A, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_10[0], 0, &(Rb.temp[9])},
  {0x0B, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_11[0], 0, &(Rb.temp[10])},
  {0x0C, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_12[0], 0, &(Rb.temp[11])},
  {0x0D, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_13[0], 0, &(Rb.temp[12])},
  {0x0E, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_14[0], 0, &(Rb.temp[13])},
  {0x0F, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6000_15[0], 0, &(Rb.temp[14])},

};

const _objd SDO6001[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x03, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6001_01[0], 0, &(Rb.status)},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6001_02[0], 0, &(Rb.event)},
  {0x03, DTYPE_UNSIGNED16, 16, ATYPE_RO, &acName6001_03[0], 0, &(Rb.error)},
};

const _objd SDO7000[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x04, NULL},
  {0x01, DTYPE_UNSIGNED16, 16, ATYPE_RW, &acName7000_01[0], 0, &(Wb.command)},
  {0x02, DTYPE_UNSIGNED16, 16, ATYPE_RW, &acName7000_02[0], 0, &(Wb.testVal0)},
  {0x03, DTYPE_UNSIGNED16, 16, ATYPE_RW, &acName7000_03[0], 0, &(Wb.testVal1)},
  {0x04, DTYPE_UNSIGNED16, 16, ATYPE_RW, &acName7000_04[0], 0, &(Wb.testVal2)}
};

/*--------------------NOT defined in XML--------------------------*/
/*
const _objd SDO7100[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x02, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, &acName7100_01[0], 0, &(encoder_scale)},
  {0x02, DTYPE_UNSIGNED32, 32, ATYPE_RO, &acName7100_02[0], 0, &(encoder_scale_mirror)}
  
};

const _objd SDO8001[] =
{ {0x00, DTYPE_UNSIGNED8, 8, ATYPE_RO, &acNameNOE[0], 0x01, NULL},
  {0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, &acName8001_01[0], 0, &(Cb.reset_counter)},
};
*/
const _objectlist SDOobjects[] =
{ {0x1000, OTYPE_VAR, 0, 0, &acName1000[0], &SDO1000[0]},
  {0x1008, OTYPE_VAR, 0, 0, &acName1008[0], &SDO1008[0]},
  {0x1009, OTYPE_VAR, 0, 0, &acName1009[0], &SDO1009[0]},
  {0x100A, OTYPE_VAR, 0, 0, &acName100A[0], &SDO100A[0]},
  {0x1018, OTYPE_RECORD, 4, 0, &acName1018[0], &SDO1018[0]},
  {0x1600, OTYPE_RECORD, 0x01, 0, &acName1600[0], &SDO1600[0]},
  {0x1A00, OTYPE_RECORD, 0x02, 0, &acName1A00[0], &SDO1A00[0]},
  {0x1C00, OTYPE_ARRAY, 4, 0, &acName1C00[0], &SDO1C00[0]},
  {0x1C10, OTYPE_ARRAY, 0, 0, &acName1C10[0], &SDO1C10[0]},
  {0x1C11, OTYPE_ARRAY, 0, 0, &acName1C11[0], &SDO1C11[0]},
  {0x1C12, OTYPE_ARRAY, 1, 0, &acName1C12[0], &SDO1C12[0]},
  {0x1C13, OTYPE_ARRAY, 1, 0, &acName1C13[0], &SDO1C13[0]},
  {0x6000, OTYPE_RECORD, 0x15, 0, &acName6000[0], &SDO6000[0]},
  {0x6001, OTYPE_RECORD, 0x03, 0, &acName6001[0], &SDO6001[0]},
  {0x7000, OTYPE_RECORD, 0x04, 0, &acName7000[0], &SDO7000[0]},
  //{0x7100, OTYPE_ARRAY, 0x02, 0, &acName7100[0], &SDO7100[0]},
  //{0x8001, OTYPE_ARRAY, 0x01, 0, &acName8001[0], &SDO8001[0]},
  {0xffff, 0xff, 0xff, 0xff, NULL, NULL}
};
