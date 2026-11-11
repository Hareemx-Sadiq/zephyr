/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (C) 2022-2023, Intel Corporation
 *
 */

#include <zephyr/drivers/clock_control.h>
#include <zephyr/dt-bindings/clock/intel_socfpga_clock.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "clock_control_agilex5_ll.h"

#define DT_DRV_COMPAT intel_agilex5_clock

LOG_MODULE_REGISTER(clock_control_agilex5, CONFIG_CLOCK_CONTROL_LOG_LEVEL);

/* Virtual base used by the CLKCTRL_* macros in the low-level header. */
uintptr_t clkctrl_base;

struct clock_control_config {
	DEVICE_MMIO_ROM;
	const struct device *sysmgr;
};

struct clock_control_data {
	DEVICE_MMIO_RAM;
};

static int clock_init(const struct device *dev)
{
	const struct clock_control_config *cfg = dev->config;

	if (!device_is_ready(cfg->sysmgr)) {
		LOG_ERR_DEVICE_NOT_READY(cfg->sysmgr);
		return -ENODEV;
	}

	DEVICE_MMIO_MAP(dev, K_MEM_CACHE_NONE);
	clkctrl_base = DEVICE_MMIO_GET(dev);

	LOG_DBG("Intel Agilex5 clock driver initialized!");
	return 0;
}

static int clock_get_rate(const struct device *dev, clock_control_subsys_t sub_system,
			  uint32_t *rate)
{
	const struct clock_control_config *config = dev->config;

	switch ((intptr_t)sub_system) {
	case INTEL_SOCFPGA_CLOCK_MPU:
		*rate = get_mpu_clk(config->sysmgr);
		break;

	case INTEL_SOCFPGA_CLOCK_WDT:
		*rate = get_wdt_clk(config->sysmgr);
		break;

	case INTEL_SOCFPGA_CLOCK_UART:
		*rate = get_uart_clk(config->sysmgr);
		break;

	case INTEL_SOCFPGA_CLOCK_MMC:
		*rate = get_sdmmc_clk(config->sysmgr);
		break;

	case INTEL_SOCFPGA_CLOCK_TIMER:
		*rate = get_timer_clk(config->sysmgr);
		break;

	case INTEL_SOCFPGA_CLOCK_QSPI:
		*rate = get_qspi_clk(config->sysmgr);
		break;

	case INTEL_SOCFPGA_CLOCK_I2C:
		*rate = get_i2c_clk(config->sysmgr);
		break;

	case INTEL_SOCFPGA_CLOCK_I3C:
		*rate = get_i3c_clk(config->sysmgr);
		break;

	default:
		LOG_ERR("Clock ID %ld is not supported\n", (intptr_t)sub_system);
		return -ENOTSUP;
	}

	return 0;
}

static DEVICE_API(clock_control, clock_api) = {
	.get_rate = clock_get_rate,
};

#define CLOCK_CONTROL_DEVICE(_inst)                                                                \
                                                                                                   \
	static struct clock_control_data clock_control_data_##_inst;                               \
                                                                                                   \
	static const struct clock_control_config clock_control_config_##_inst = {                  \
		DEVICE_MMIO_ROM_INIT(DT_DRV_INST(_inst)),                                          \
		.sysmgr = DEVICE_DT_GET(DT_INST_PHANDLE(_inst, sysmgr)), \
	};                                                                                         \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(_inst, clock_init, NULL, &clock_control_data_##_inst,                \
			      &clock_control_config_##_inst, PRE_KERNEL_1,                         \
			      CONFIG_CLOCK_CONTROL_INIT_PRIORITY, &clock_api);

DT_INST_FOREACH_STATUS_OKAY(CLOCK_CONTROL_DEVICE)
