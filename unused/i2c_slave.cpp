#include "i2c_slave.h"

#include <avr/interrupt.h>

static_assert(I2C_SCL_PIN < 6, "I2C SCL must be on a PORTB pin");
static_assert(I2C_SDA_PIN < 6, "I2C SDA must be on a PORTB pin");
static_assert(I2C_SCL_PIN != I2C_SDA_PIN, "I2C SCL and SDA must use different pins");

TinyI2CSlave i2cSlave;

void TinyI2CSlave::begin(uint8_t address) {
	const uint8_t savedStatus = SREG;
	cli();

	address_ = address & 0x7F;
	state_ = Idle;
	afterAcknowledge_ = Idle;
	shiftRegister_ = 0;
	bitCount_ = 0;
	transmitBit_ = 0;
	acknowledge_ = false;
	acknowledgeClockSeen_ = false;
	masterAcknowledged_ = false;
	receivedByteAvailable_ = false;
	transmitByte_ = 0xFF;

	PORTB &= static_cast<uint8_t>(~(_BV(I2C_SCL_PIN) | _BV(I2C_SDA_PIN)));
	DDRB &= static_cast<uint8_t>(~(_BV(I2C_SCL_PIN) | _BV(I2C_SDA_PIN)));
	previousPins_ = PINB;

	PCMSK |= _BV(I2C_SCL_PIN) | _BV(I2C_SDA_PIN);
	GIFR = _BV(PCIF);
	GIMSK |= _BV(PCIE);

	SREG = savedStatus;
}

bool TinyI2CSlave::available() const {
	return receivedByteAvailable_;
}

int16_t TinyI2CSlave::read() {
	const uint8_t savedStatus = SREG;
	cli();

	if (!receivedByteAvailable_) {
		SREG = savedStatus;
		return -1;
	}

	const uint8_t value = receivedByte_;
	receivedByteAvailable_ = false;
	SREG = savedStatus;
	return value;
}

void TinyI2CSlave::write(uint8_t value) {
	transmitByte_ = value;
}

void TinyI2CSlave::releaseSda() {
	DDRB &= static_cast<uint8_t>(~_BV(I2C_SDA_PIN));
}

void TinyI2CSlave::driveSdaLow() {
	PORTB &= static_cast<uint8_t>(~_BV(I2C_SDA_PIN));
	DDRB |= _BV(I2C_SDA_PIN);
}

void TinyI2CSlave::startTransmitByte() {
	shiftRegister_ = transmitByte_;
	transmitBit_ = 7;
	state_ = TransmitData;

	if (shiftRegister_ & _BV(transmitBit_)) {
		releaseSda();
	} else {
		driveSdaLow();
	}
}

void TinyI2CSlave::handlePinChange() {
	const uint8_t pins = PINB;
	const uint8_t changed = pins ^ previousPins_;
	const bool sclHigh = (pins & _BV(I2C_SCL_PIN)) != 0;
	const bool sdaHigh = (pins & _BV(I2C_SDA_PIN)) != 0;
	const bool sdaChanged = (changed & _BV(I2C_SDA_PIN)) != 0;
	const bool sclChanged = (changed & _BV(I2C_SCL_PIN)) != 0;
	previousPins_ = pins;

	if (sdaChanged && sclHigh) {
		if (!sdaHigh) {
			state_ = ReceiveAddress;
			shiftRegister_ = 0;
			bitCount_ = 0;
			acknowledgeClockSeen_ = false;
			releaseSda();
		} else {
			state_ = Idle;
			releaseSda();
		}
		return;
	}

	if (!sclChanged) {
		return;
	}

	if (sclHigh) {
		switch (state_) {
		case ReceiveAddress:
		case ReceiveData:
			shiftRegister_ = static_cast<uint8_t>((shiftRegister_ << 1) | (sdaHigh ? 1 : 0));
			if (++bitCount_ == 8) {
				if (state_ == ReceiveAddress) {
					const uint8_t receivedAddress = shiftRegister_ >> 1;
					acknowledge_ = receivedAddress == address_;
					if (acknowledge_) {
						afterAcknowledge_ = (shiftRegister_ & 1) ? TransmitData : ReceiveData;
					} else {
						afterAcknowledge_ = Idle;
					}
				} else {
					acknowledge_ = !receivedByteAvailable_;
					if (acknowledge_) {
						receivedByte_ = shiftRegister_;
					}
					afterAcknowledge_ = ReceiveData;
				}
				state_ = Acknowledge;
				acknowledgeClockSeen_ = false;
			}
			break;
		case Acknowledge:
			acknowledgeClockSeen_ = true;
			break;
		case TransmitAcknowledge:
			masterAcknowledged_ = !sdaHigh;
			break;
		case Idle:
		case TransmitData:
			break;
		}
		return;
	}

	switch (state_) {
	case Acknowledge:
		if (!acknowledgeClockSeen_) {
			if (acknowledge_) {
				driveSdaLow();
			} else {
				releaseSda();
			}
		} else {
			releaseSda();
			state_ = afterAcknowledge_;
			bitCount_ = 0;
			shiftRegister_ = 0;
			acknowledgeClockSeen_ = false;
			if (state_ == TransmitData) {
				startTransmitByte();
			}
		}
		break;
	case TransmitData:
		if (transmitBit_ == 0) {
			releaseSda();
			state_ = TransmitAcknowledge;
			masterAcknowledged_ = false;
		} else {
			--transmitBit_;
			if (shiftRegister_ & _BV(transmitBit_)) {
				releaseSda();
			} else {
				driveSdaLow();
			}
		}
		break;
	case TransmitAcknowledge:
		if (masterAcknowledged_) {
			startTransmitByte();
		} else {
			state_ = Idle;
			releaseSda();
		}
		break;
	case Idle:
	case ReceiveAddress:
	case ReceiveData:
		break;
	}
}

ISR(PCINT0_vect) {
	i2cSlave.handlePinChange();
}
