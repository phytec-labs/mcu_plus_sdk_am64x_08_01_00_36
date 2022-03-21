let common = system.getScript("/common");


let mcspi_input_clk_freq = 50000000;

const mcspi_config_r5fss = [
    {
        name            : "SPI0",
        baseAddr        : "CSL_MCSPI0_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 204,
        clockIds        : [ "TISCI_DEV_MCSPI0" ],
    },
    {
        name            : "SPI1",
        baseAddr        : "CSL_MCSPI1_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 205,
        clockIds        : [ "TISCI_DEV_MCSPI1" ],
    },
    {
        name            : "SPI2",
        baseAddr        : "CSL_MCSPI2_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 206,
        clockIds        : [ "TISCI_DEV_MCSPI2" ],
    },
    {
        name            : "SPI3",
        baseAddr        : "CSL_MCSPI3_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 63,
        clockIds        : [ "TISCI_DEV_MCSPI3" ],
    },
    {
        name            : "SPI4",
        baseAddr        : "CSL_MCSPI4_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 207,
        clockIds        : [ "TISCI_DEV_MCSPI4" ],
    },
    {
        name            : "MCU_SPI0",
        baseAddr        : "CSL_MCU_MCSPI0_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 208,
        clockIds        : [ "TISCI_DEV_MCU_MCSPI0" ],
    },
    {
        name            : "MCU_SPI1",
        baseAddr        : "CSL_MCU_MCSPI1_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 209,
        clockIds        : [ "TISCI_DEV_MCU_MCSPI1" ],
    },
];

/* On M4F, interrupt number as specified in TRM is input to the NVIC but from M4 point of view there are 16 internal interrupts
 * and then the NVIC input interrupts start, hence we need to add +16 to the value specified by TRM */
const mcspi_config_m4f = [
    {
        name            : "MCU_SPI0",
        baseAddr        : "CSL_MCU_MCSPI0_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 22 + 16,
        clockIds        : [ "TISCI_DEV_MCU_MCSPI0" ],
    },
    {
        name            : "MCU_SPI1",
        baseAddr        : "CSL_MCU_MCSPI1_CFG_BASE",
        inputClkFreq    : mcspi_input_clk_freq,
        intrNum         : 23 + 16,
        clockIds        : [ "TISCI_DEV_MCU_MCSPI1" ],
    },
];

function getMaxChannels(inst) {
    return 4;   /* max number of channels per MCSPI */
}

function getConfigArr() {
    let mcspi_config;

    if(common.getSelfSysCfgCoreName().includes("m4f")) {
        mcspi_config = mcspi_config_m4f;
    }
    else {
        mcspi_config = mcspi_config_r5fss;
    }

    return mcspi_config;
}

function isFrequencyDefined()
{
    return false;
}

exports = {
    getConfigArr,
    getMaxChannels,
    isFrequencyDefined,
};
