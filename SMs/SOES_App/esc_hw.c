/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

 /** \file
 * \brief
 * ESC hardware layer functions for LAN9252.
 *
 * Function to read and write commands to the ESC. Used to read/write ESC
 * registers and memory.
 */

#include "esc.h"
#include "esc_hw.h"
#include "LAN9252_spi.h"

#include <string.h>


#define ESC_CMD_SERIAL_WRITE     0x02
#define ESC_CMD_SERIAL_READ      0x03
#define ESC_CMD_FAST_READ        0x0B
#define ESC_CMD_RESET_SQI        0xFF

#define ESC_CMD_FAST_READ_DUMMY  1
#define ESC_CMD_ADDR_INC         BIT(6)

#define ESC_PRAM_RD_FIFO_REG     0x000
#define ESC_PRAM_WR_FIFO_REG     0x020
#define ESC_PRAM_RD_ADDR_LEN_REG 0x308
#define ESC_PRAM_RD_CMD_REG      0x30C
#define ESC_PRAM_WR_ADDR_LEN_REG 0x310
#define ESC_PRAM_WR_CMD_REG      0x314

#define ESC_PRAM_CMD_BUSY        BIT(31)
#define ESC_PRAM_CMD_ABORT       BIT(30)

#define ESC_PRAM_CMD_CNT(x)      ((x >> 8) & 0x1F)
#define ESC_PRAM_CMD_AVAIL       BIT(0)

#define ESC_PRAM_SIZE(x)         ((x) << 16)
#define ESC_PRAM_ADDR(x)         ((x) << 0)




#define ESC_CSR_DATA_REG         0x300
#define ESC_CSR_CMD_REG          0x304

#define ESC_CSR_CMD_BUSY         BIT(31)
#define ESC_CSR_CMD_READ         (BIT(31) | BIT(30))
#define ESC_CSR_CMD_WRITE        BIT(31)
#define ESC_CSR_CMD_SIZE(x)      (x << 16)

#define ESC_RESET_CTRL_REG       0x1F8
#define ESC_RESET_CTRL_RST       BIT(6)


/******************EXTERN********************************/
extern int lan9252; //From lan9252_spi.c

uint32_t invert32data(uint32_t data) {
	return data<<31&(1<<31)|
			data<<30&(1<<30)|
			data<<29&(1<<29)|
			data<<28&(1<<28)|
			data<<27&(1<<27)|
			data<<26&(1<<26)|
			data<<25&(1<<25)|
			data<<24&(1<<24)|
			data<<23&(1<<23)|
			data<<22&(1<<22)|
			data<<21&(1<<21)|
			data<<20&(1<<20)|
			data<<19&(1<<19)|
			data<<18&(1<<18)|
			data<<17&(1<<17)|
			data<<16&(1<<16)|
			data<<15&(1<<15)|
			data<<14&(1<<14)|
			data<<13&(1<<13)|
			data<<12&(1<<12)|
			data<<11&(1<<11)|
			data<<10&(1<<10)|
			data<<9&(1<<9)|
			data<<8&(1<<8)|
			data<<7&(1<<7)|
			data<<6&(1<<6)|
			data<<5&(1<<5)|
			data<<4&(1<<4)|
			data<<3&(1<<3)|
			data<<2&(1<<2)|
			data<<1&(1<<1)|
			data&(1);
}

uint16_t invert16data(uint16_t data) {
	return 	data<<15&(1<<15)|
			data<<14&(1<<14)|
			data<<13&(1<<13)|
			data<<12&(1<<12)|
			data<<11&(1<<11)|
			data<<10&(1<<10)|
			data<<9&(1<<9)|
			data<<8&(1<<8)|
			data<<7&(1<<7)|
			data<<6&(1<<6)|
			data<<5&(1<<5)|
			data<<4&(1<<4)|
			data<<3&(1<<3)|
			data<<2&(1<<2)|
			data<<1&(1<<1)|
			data&(1);
}

/* ESC read CSR function */
void ESC_read_csr (uint16_t address, void *buf, uint16_t len)
{
	uint16_t counter = 0;
   uint32_t value;
   uint32_t value2;

   value = (ESC_CSR_CMD_READ | ESC_CSR_CMD_SIZE(len) | (address<<2));//
   //spi_select (lan9252);
   lan9252_write_32(ESC_CSR_CMD_REG, value);
   do
   {
	  value = lan9252_read_32(ESC_CSR_CMD_REG);

   } while(value & ESC_CSR_CMD_BUSY );


   value = lan9252_read_32(ESC_CSR_DATA_REG);
   ////spi_unselect (lan9252);
   memcpy(buf, (uint8_t *)&value, len);
}

/* ESC write CSR function */
static void ESC_write_csr (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;
   uint16_t counter = 0;

   memcpy((uint8_t*)&value, buf,len);



   lan9252_write_32(ESC_CSR_DATA_REG, value);
   value = (ESC_CSR_CMD_WRITE | ESC_CSR_CMD_SIZE(len) | address);
   lan9252_write_32(ESC_CSR_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_CSR_CMD_REG);
      counter++;
   } while((value & ESC_CSR_CMD_BUSY) && counter<100 );
   counter = 0;


}

/* ESC read process data ram function */
void ESC_read_pram (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;
   uint8_t * temp_buf = buf;
   uint16_t byte_offset = 0;
   uint8_t fifo_cnt, first_byte_position, temp_len, data[4];

   value = ESC_PRAM_CMD_ABORT;
   lan9252_write_32(ESC_PRAM_RD_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_RD_CMD_REG);
   }while(value & ESC_PRAM_CMD_BUSY);

   value = ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address);
   lan9252_write_32(ESC_PRAM_RD_ADDR_LEN_REG, value);

   value = ESC_PRAM_CMD_BUSY;
   lan9252_write_32(ESC_PRAM_RD_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_RD_CMD_REG);
   }while((value & ESC_PRAM_CMD_AVAIL) == 0);

   /* Fifo count */
   fifo_cnt = ESC_PRAM_CMD_CNT(value);

   /* Read first value from FIFO */
   value = lan9252_read_32(ESC_PRAM_RD_FIFO_REG);
   fifo_cnt--;

   /* Find out first byte position and adjust the copy from that
    * according to LAN9252 datasheet and MicroChip SDK code
    */
   first_byte_position = (address & 0x03);
   temp_len = ((4 - first_byte_position) > len) ? len : (4 - first_byte_position);

   memcpy(temp_buf ,((uint8_t *)&value + first_byte_position), temp_len);
   len -= temp_len;
   byte_offset += temp_len;

   /* Select device. */
   //spi_select (lan9252);
   /* Send command and address for fifo read */
   data[0] = ESC_CMD_FAST_READ;
   data[1] = ((ESC_PRAM_RD_FIFO_REG >> 8) & 0xFF);
   data[2] = (ESC_PRAM_RD_FIFO_REG & 0xFF);
   data[3] = ESC_CMD_FAST_READ_DUMMY;
   ecat_write_raw (lan9252, data, sizeof(data));

   /* Continue reading until we have read len */
   while(len > 0)	//Pending this part could be DMA
   {
      temp_len = (len > 4) ? 4: len;
      /* Always read 4 byte */
      ecat_read_raw (lan9252, (temp_buf + byte_offset), sizeof(uint32_t));

      fifo_cnt--;
      len -= temp_len;
      byte_offset += temp_len;
   }
   /* Un-select device. */
   //spi_unselect (lan9252);
}

/* ESC write process data ram function */
static void ESC_write_pram (uint16_t address, void *buf, uint16_t len)
{
   uint32_t value;
   uint8_t * temp_buf = buf;
   uint16_t byte_offset = 0;
   uint8_t fifo_cnt, first_byte_position, temp_len, data[3];

   value = ESC_PRAM_CMD_ABORT;
   lan9252_write_32(ESC_PRAM_WR_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_WR_CMD_REG);
   }while(value & ESC_PRAM_CMD_BUSY);

   value = ESC_PRAM_SIZE(len) | ESC_PRAM_ADDR(address);
   lan9252_write_32(ESC_PRAM_WR_ADDR_LEN_REG, value);

   value = ESC_PRAM_CMD_BUSY;
   lan9252_write_32(ESC_PRAM_WR_CMD_REG, value);

   do
   {
      value = lan9252_read_32(ESC_PRAM_WR_CMD_REG);
   }while((value & ESC_PRAM_CMD_AVAIL) == 0);

   /* Fifo count */
   fifo_cnt = ESC_PRAM_CMD_CNT(value);

   /* Find out first byte position and adjust the copy from that
    * according to LAN9252 datasheet
    */
   first_byte_position = (address & 0x03);
   temp_len = ((4 - first_byte_position) > len) ? len : (4 - first_byte_position);

   memcpy(((uint8_t *)&value + first_byte_position), temp_buf, temp_len);

   /* Write first value from FIFO */
   lan9252_write_32(ESC_PRAM_WR_FIFO_REG, value);

   len -= temp_len;
   byte_offset += temp_len;
   fifo_cnt--;

   /* Select device. */
   spi_select (lan9252);
   /* Send command and address for incrementing write */
   data[0] = ESC_CMD_SERIAL_WRITE;
   data[1] = ((ESC_PRAM_WR_FIFO_REG >> 8) & 0xFF);
   data[2] = (ESC_PRAM_WR_FIFO_REG & 0xFF);
   ecat_write_raw (lan9252, data, sizeof(data));

   /* Continue reading until we have read len */
   while(len > 0)
   {
      temp_len = (len > 4) ? 4 : len;
      value = 0;
      memcpy((uint8_t *)&value, (temp_buf + byte_offset), temp_len);
      /* Always write 4 byte */
      ecat_write_raw (lan9252, (void *)&value, sizeof(value));

      fifo_cnt--;
      len -= temp_len;
      byte_offset += temp_len;
   }
   /* Un-select device. */
   spi_unselect (lan9252);
}


/** ESC read functi+on used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to read
 * @param[out]  buf         = pointer to buffer to read in
 * @param[in]   len         = number of bytes to read
 */
void ESC_read (uint16_t address, void *buf, uint16_t len)
{
   /* Select Read function depending on address, process data ram or not */
   if (address >= 0x1000)
   {
      ESC_read_pram(address, buf, len);
   }
   else
   {
      uint16_t size;
      uint8_t *temp_buf = (uint8_t *)buf;

      while(len > 0)
      {
         /* We write maximum 4 bytes at the time */
         size = (len > 4) ? 4 : len;
         /* Make size aligned to address according to LAN9252 datasheet
          * Table 12-14 EtherCAT CSR Address VS size and MicroChip SDK code
          */
         /* If we got an odd address size is 1 , 01b 11b is captured */
         if(address & BIT(0))
         {
            size = 1;
         }
         /* If address 1xb and size != 1 and 3 , allow size 2 else size 1 */
         else if (address & BIT(1))
         {
            size = (size & BIT(0)) ? 1 : 2;
         }
         /* size 3 not valid */
         else if (size == 3)
         {
            size = 1;
         }
         /* else size is kept AS IS */
         ESC_read_csr(address, temp_buf, size);

         /* next address */
         len -= size;
         temp_buf += size;
         address += size;
      }
   }
   /* To mimic the ET1100 always providing AlEvent on every read or write */
   ESC_read_csr(ESCREG_ALEVENT,(void *)&ESCvar.ALevent,sizeof(ESCvar.ALevent));
   ESCvar.ALevent = etohs (ESCvar.ALevent);

}

/** ESC write function used by the Slave stack.
 *
 * @param[in]   address     = address of ESC register to write
 * @param[out]  buf         = pointer to buffer to write from
 * @param[in]   len         = number of bytes to write
 */
void ESC_write (uint16_t address, void *buf, uint16_t len)
{
   /* Select Write function depending on address, process data ram or not */
   if (address >= 0x1000)
   {
      ESC_write_pram(address, buf, len);
   }
   else
   {
      uint16_t size;
      uint8_t *temp_buf = (uint8_t *)buf;

      while(len > 0)
      {
         /* We write maximum 4 bytes at the time */
         size = (len > 4) ? 4 : len;
         /* Make size aligned to address according to LAN9252 datasheet
          * Table 12-14 EtherCAT CSR Address VS size  and MicroChip SDK code
          */
         /* If we got an odd address size is 1 , 01b 11b is captured */
         if(address & BIT(0))
         {
            size = 1;
         }
         /* If address 1xb and size != 1 and 3 , allow size 2 else size 1 */
         else if (address & BIT(1))
         {
            size = (size & BIT(0)) ? 1 : 2;
         }
         /* size 3 not valid */
         else if (size == 3)
         {
            size = 1;
         }
         /* else size is kept AS IS */
         ESC_write_csr(address, temp_buf, size);

         /* next address */
         len -= size;
         temp_buf += size;
         address += size;
      }
   }

   /* To mimic the ET1x00 always providing AlEvent on every read or write */
   ESC_read_csr(ESCREG_ALEVENT,(void *)&ESCvar.ALevent,sizeof(ESCvar.ALevent));
   ESCvar.ALevent = etohs (ESCvar.ALevent);
}

/* Un-used due to evb-lan9252-digio not havning any possability to
 * reset except over SPI.
 */
void ESC_reset (void)
{

}

void ESC_init (const esc_cfg_t * config)
{
   uint32_t value;
   const char * spi_name = (char *)config->user_arg;
   lan9252 = open (spi_name, O_RDWR, 0);

   /* Reset the ecat core here due to evb-lan9252-digio not having any GPIO
    * for that purpose.
    */
   lan9252_write_32(ESC_RESET_CTRL_REG,ESC_RESET_CTRL_RST);
   do
   {
      value = lan9252_read_32(ESC_CSR_CMD_REG);
   } while(value & ESC_RESET_CTRL_RST);



}

int open(const char *pathname, int flags, uint8_t mode) {
	char* tempPort = "LOCAL_SPI";

	if((strcmp(pathname,tempPort) == 0) && (flags == O_RDWR) && (mode == 0))
		return STM32_SPI;
	else
		return -1;
}


