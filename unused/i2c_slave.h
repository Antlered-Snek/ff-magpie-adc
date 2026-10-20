#pragma once

#include <Arduino.h>
#include <stdint.h>

// The ATtiny13A implementation uses PORTB pin-change interrupts.
// SDA is open-drain; fit external pull-up resistors on both bus lines.
#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN PB1
#endif

#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN PB2
#endif

class TinyI2CSlave {
public:
	// address is a 7-bit I2C address.
	void begin(uint8_t address);
	bool available() const;
	// Returns 0..255 when a byte is waiting, or -1 when the mailbox is empty.
	int16_t read();
	// Preloads the byte returned during master-read transactions. If the
	// master ACKs, that byte is repeated until the master NACKs or stops.
	void write(uint8_t value);

	void handlePinChange();

private:
	enum State : uint8_t {
		Idle,
		ReceiveAddress,
		ReceiveData,
		Acknowledge,
		TransmitData,
		TransmitAcknowledge
	};

	void releaseSda();
	void driveSdaLow();
	void startTransmitByte();

	State state_;
	State afterAcknowledge_;
	uint8_t address_;
	uint8_t previousPins_;
	uint8_t shiftRegister_;
	uint8_t bitCount_;
	uint8_t transmitBit_;
	bool acknowledge_;
	bool acknowledgeClockSeen_;
	bool masterAcknowledged_;
	volatile uint8_t receivedByte_;
	volatile uint8_t transmitByte_;
	volatile bool receivedByteAvailable_;
};

extern TinyI2CSlave i2cSlave;
