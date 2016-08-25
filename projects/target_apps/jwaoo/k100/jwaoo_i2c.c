#include <stdint.h>
#include "jwaoo_i2c.h"

#define JWAOO_I2C_AUTO_POWER_DOWN		1
#define JWAOO_I2C_SPEED					JWAOO_I2C_SPEED_400K
#define JWAOO_I2C_ADDRESS_MODE			JWAOO_I2C_ADDRESS_MODE_7BIT

static int jwaoo_i2c_wait_while_tx_fifo_full()
{
	int i;

	for (i = 0; i < 50000; i++) {
		if (GetWord16(I2C_STATUS_REG) & TFNF) {
			return 0;
		}
	}

	return -1;
}

static int jwaoo_i2c_wait_until_tx_fifo_empty()
{
	int i;

	for (i = 0; i < 50000; i++) {
		if (GetWord16(I2C_STATUS_REG) & TFE) {
			return 0;
		}
	}

	return -1;
}

static int jwaoo_i2c_send_command(uint16_t command)
{
	int ret;

	ret = jwaoo_i2c_wait_while_tx_fifo_full();
	if (ret < 0) {
		return ret;
	}

	SetWord16(I2C_DATA_CMD_REG, command);

	return 0;
}

static int jwaoo_i2c_get_rx_fifo_size()
{
	int i;

	for (i = 0; i < 50000; i++) {
		uint16_t size = GetWord16(I2C_RXFLR_REG);
		if (size > 0) {
			return size;
		}
	}

	return -1;
}

static int jwaoo_i2c_wait_until_idle()
{
	int i;

	for (i = 0; i < 500000; i++) {
		if ((GetWord16(I2C_STATUS_REG) & MST_ACTIVITY) == 0) {
			return 0;
		}
	}

	return -1;
}

int jwaoo_i2c_transfer(uint8_t slave, struct jwaoo_i2c_message *msgs, int count)
{
	int ret;
	struct jwaoo_i2c_message *msg = msgs;
	struct jwaoo_i2c_message *msg_end = msg + count;

#if JWAOO_I2C_AUTO_POWER_DOWN
	SetWord16(I2C_TAR_REG, slave);
	SetWord16(I2C_ENABLE_REG, 1);
#else
	static uint8_t last_slave;

	if (slave != last_slave) {
		last_slave = slave;
		SetWord16(I2C_ENABLE_REG, 0);
		SetWord16(I2C_TAR_REG, slave);
		SetWord16(I2C_ENABLE_REG, 1);
	}
#endif

	ret = jwaoo_i2c_wait_until_idle();
	if (ret < 0) {
		goto out_i2c_disable;
	}

	GLOBAL_INT_DISABLE();

	GetWord16(I2C_CLR_TX_ABRT_REG);

	while (msg < msg_end) {
		uint8_t *data = msg->data;
		uint8_t *data_end = data + msg->count;

		if (msg->read) {
			int i;

			for (i = msg->count; i > 0; i--) {
				ret = jwaoo_i2c_send_command(0x0100);
				if (ret < 0) {
					goto out_enable_irq;
				}
			}

			while (data < data_end) {
				int fifo_size = jwaoo_i2c_get_rx_fifo_size();
				if (fifo_size < 0) {
					ret = fifo_size;
					goto out_enable_irq;
				}

				while (fifo_size > 0 && data < data_end) {
					*data++ = GetWord16(I2C_DATA_CMD_REG) & 0xFF;
					fifo_size--;
				}
			}

			msg++;
		} else {
			while (data < data_end) {
				ret = jwaoo_i2c_send_command(*data++);
				if (ret < 0) {
					goto out_enable_irq;
				}
			}

			msg++;

			if (msg >= msg_end || msg->read) {
				uint16_t status;

				ret = jwaoo_i2c_wait_until_tx_fifo_empty();
				if (ret < 0) {
					goto out_enable_irq;
				}

				status = GetWord16(I2C_TX_ABRT_SOURCE_REG);
				if (status) {
					ret = -1;
					goto out_enable_irq;
				}
			}
		}
	}

	ret = count;

out_enable_irq:
	GLOBAL_INT_RESTORE();
out_i2c_disable:
#if JWAOO_I2C_JWAOO_I2C_AUTO_POWER_DOWN
	SetWord16(I2C_ENABLE_REG, 0);
#endif

	return ret;
}

int jwaoo_i2c_read_data(uint8_t slave, uint8_t addr, uint8_t *data, int size)
{
	struct jwaoo_i2c_message msgs[] = {
		{
			.data = &addr,
			.count = 1,
			.read = 0,
		}, {
			.data = data,
			.count = size,
			.read = 1,
		}
	};

	if (jwaoo_i2c_transfer(slave, msgs, NELEM(msgs)) == NELEM(msgs)) {
		return size;
	}

	return -1;
}

int jwaoo_i2c_write_data(uint8_t slave, uint8_t addr, const uint8_t *data, int size)
{
	struct jwaoo_i2c_message msgs[] = {
		{
			.data = &addr,
			.count = 1,
			.read = 0,
		}, {
			.data = (uint8_t *) data,
			.count = size,
			.read = 0,
		}
	};

	if (jwaoo_i2c_transfer(slave, msgs, NELEM(msgs)) == NELEM(msgs)) {
		return size;
	}

	return -1;
}

int jwaoo_i2c_update_u8(uint8_t slave, uint8_t addr, uint8_t value, uint8_t mask)
{
	int ret;
	uint8_t value_old;

	ret = jwaoo_i2c_read_u8(slave, addr, &value_old);
	if (ret < 0) {
		return ret;
	}

	if ((value_old & mask) == value) {
		return 0;
	}

	return jwaoo_i2c_write_u8(slave, addr, value | (value_old & (~mask)));
}

int jwaoo_i2c_update_u16(uint8_t slave, uint8_t addr, uint16_t value, uint16_t mask)
{
	int ret;
	uint16_t value_old;

	ret = jwaoo_i2c_read_u16(slave, addr, &value_old);
	if (ret < 0) {
		return ret;
	}

	if ((value_old & mask) == value) {
		return 0;
	}

	return jwaoo_i2c_write_u16(slave, addr, value | (value_old & (~mask)));
}

int jwaoo_i2c_update_u32(uint8_t slave, uint8_t addr, uint32_t value, uint32_t mask)
{
	int ret;
	uint32_t value_old;

	ret = jwaoo_i2c_read_u32(slave, addr, &value_old);
	if (ret < 0) {
		return ret;
	}

	if ((value_old & mask) == value) {
		return 0;
	}

	return jwaoo_i2c_write_u32(slave, addr, value | (value_old & (~mask)));
}

void jwaoo_i2c_set_enable(bool enable)
{
	if (enable) {
		GPIO_ConfigurePin(I2C1_GPIO_PORT, I2C1_SCL_GPIO_PIN, OUTPUT, PID_I2C_SCL, false);
		GPIO_ConfigurePin(I2C1_GPIO_PORT, I2C1_SDA_GPIO_PIN, OUTPUT, PID_I2C_SDA, false);

		SetBits16(CLK_PER_REG, I2C_ENABLE, 1);										  // enable  clock for I2C
		SetWord16(I2C_ENABLE_REG, 0x0); 											  // Disable the I2C controller
		SetWord16(I2C_CON_REG, I2C_MASTER_MODE | I2C_SLAVE_DISABLE | I2C_RESTART_EN); // Slave is disabled
		SetBits16(I2C_CON_REG, I2C_SPEED, JWAOO_I2C_SPEED);									  // Set speed
		SetBits16(I2C_CON_REG, I2C_10BITADDR_MASTER, JWAOO_I2C_ADDRESS_MODE); 				  // Set addressing mode

#if JWAOO_I2C_AUTO_POWER_DOWN == 0
		SetWord16(I2C_ENABLE_REG, 1);
		jwaoo_i2c_wait_until_idle();
#endif
	} else {
		SetWord16(I2C_ENABLE_REG, 0x0); 							// Disable the I2C controller
		SetBits16(CLK_PER_REG, I2C_ENABLE, 0);						// Disable clock for I2C

		GPIO_ConfigurePin(I2C1_GPIO_PORT, I2C1_SCL_GPIO_PIN, INPUT, PID_GPIO, false);
		GPIO_ConfigurePin(I2C1_GPIO_PORT, I2C1_SDA_GPIO_PIN, INPUT, PID_GPIO, false);
	}
}
