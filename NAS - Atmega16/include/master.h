/*
 * master.h
 *
 * Created: 1/1/2025 2:15:39 AM
 *  Author: momen
 */

#ifndef FREQ_
#define F_CPU 8000000UL
#define F_SCL 500000UL
#endif

#ifndef MASTER_H_
#define MASTER_H_

#include <stdint.h>
#include <avr/io.h>

#define SUCCESSFULLY_STARTED 0x08
#define START_REPEATED 0x10
#define SLAW_ACK 0x18
#define SLAW_NACK 0x20
#define TX_BYTE_ACKED 0x28
#define TX_BYTE_NACKED 0x30
#define PACKET_ERROR 0x38
#define SLAR_ACK 0x40
#define SLAR_NACK 0x48
#define RX_BYTE_ACKED 0x50
#define RX_BYTE_NACKED 0x58

class Master
{
    public:
        Master();
        int8_t tstart(void);
        void tstop(void);
        int8_t tslaw(uint8_t addr);
        int8_t tslar(uint8_t addr);
        int8_t ttx(char data);
        int8_t trx(bool ea);
};

#endif /* MASTER_H_ */
