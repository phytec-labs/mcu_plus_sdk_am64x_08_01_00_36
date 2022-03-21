let common = system.getScript("/common");

let uart_input_clk_freq = 96000000;

const uart_config_r5fss = [
    {
        name            : "MCU_USART0",
        baseAddr        : "CSL_MCU_UART0_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 217,
        clockIds        : [ "TISCI_DEV_MCU_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCU_UART0",
                clkId   : "TISCI_DEV_MCU_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "MCU_USART1",
        baseAddr        : "CSL_MCU_UART1_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 218,
        clockIds        : [ "TISCI_DEV_MCU_UART1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCU_UART1",
                clkId   : "TISCI_DEV_MCU_UART1_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART0",
        baseAddr        : "CSL_UART0_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 210,
        clockIds        : [ "TISCI_DEV_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART0",
                clkId   : "TISCI_DEV_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART1",
        baseAddr        : "CSL_UART1_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 211,
        clockIds        : [ "TISCI_DEV_UART1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART1",
                clkId   : "TISCI_DEV_UART1_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART2",
        baseAddr        : "CSL_UART2_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 212,
        clockIds        : [ "TISCI_DEV_UART2" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART2",
                clkId   : "TISCI_DEV_UART2_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART3",
        baseAddr        : "CSL_UART3_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 213,
        clockIds        : [ "TISCI_DEV_UART3" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART3",
                clkId   : "TISCI_DEV_UART3_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART4",
        baseAddr        : "CSL_UART4_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 214,
        clockIds        : [ "TISCI_DEV_UART4" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART4",
                clkId   : "TISCI_DEV_UART4_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART5",
        baseAddr        : "CSL_UART5_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 215,
        clockIds        : [ "TISCI_DEV_UART5" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART5",
                clkId   : "TISCI_DEV_UART5_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART6",
        baseAddr        : "CSL_UART6_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 216,
        clockIds        : [ "TISCI_DEV_UART6" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART6",
                clkId   : "TISCI_DEV_UART6_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
];

/* On M4F, interrupt number as specified in TRM is input to the NVIC but from M4 point of view there are 16 internal interrupts
 * and then the NVIC input interrupts start, hence we need to add +16 to the value specified by TRM */
const uart_config_m4f = [
    {
        name            : "MCU_USART0",
        baseAddr        : "CSL_MCU_UART0_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 24 + 16,
        clockIds        : [ "TISCI_DEV_MCU_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCU_UART0",
                clkId   : "TISCI_DEV_MCU_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "MCU_USART1",
        baseAddr        : "CSL_MCU_UART1_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 25 + 16,
        clockIds        : [ "TISCI_DEV_MCU_UART1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCU_UART1",
                clkId   : "TISCI_DEV_MCU_UART1_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
];

const uart_config_a53ss = [
    {
        name            : "MCU_USART0",
        baseAddr        : "CSL_MCU_UART0_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 217,
        clockIds        : [ "TISCI_DEV_MCU_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCU_UART0",
                clkId   : "TISCI_DEV_MCU_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "MCU_USART1",
        baseAddr        : "CSL_MCU_UART1_BASE",
        inputClkFreq    : uart_input_clk_freq,
        intrNum         : 218,
        clockIds        : [ "TISCI_DEV_MCU_UART1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_MCU_UART1",
                clkId   : "TISCI_DEV_MCU_UART1_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART0",
        baseAddr        : "CSL_UART0_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 210,
        clockIds        : [ "TISCI_DEV_UART0" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART0",
                clkId   : "TISCI_DEV_UART0_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART1",
        baseAddr        : "CSL_UART1_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 211,
        clockIds        : [ "TISCI_DEV_UART1" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART1",
                clkId   : "TISCI_DEV_UART1_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART2",
        baseAddr        : "CSL_UART2_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 212,
        clockIds        : [ "TISCI_DEV_UART2" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART2",
                clkId   : "TISCI_DEV_UART2_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART3",
        baseAddr        : "CSL_UART3_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 213,
        clockIds        : [ "TISCI_DEV_UART3" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART3",
                clkId   : "TISCI_DEV_UART3_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART4",
        baseAddr        : "CSL_UART4_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 214,
        clockIds        : [ "TISCI_DEV_UART4" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART4",
                clkId   : "TISCI_DEV_UART4_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART5",
        baseAddr        : "CSL_UART5_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 215,
        clockIds        : [ "TISCI_DEV_UART5" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART5",
                clkId   : "TISCI_DEV_UART5_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
    {
        name            : "USART6",
        baseAddr        : "CSL_UART6_BASE",
        inputClkFreq    : 48000000,
        intrNum         : 216,
        clockIds        : [ "TISCI_DEV_UART6" ],
        clockFrequencies: [
            {
                moduleId: "TISCI_DEV_UART6",
                clkId   : "TISCI_DEV_UART6_FCLK_CLK",
                clkRate : uart_input_clk_freq,
            },
        ],
    },
];

function getConfigArr() {
    let uart_config;

    if(common.getSelfSysCfgCoreName().includes("m4f")) {
        uart_config = uart_config_m4f;
    }
    else if(common.getSelfSysCfgCoreName().includes("a53")){
        uart_config = uart_config_a53ss;
    }
    else {
        uart_config = uart_config_r5fss;
    }

    return uart_config;
}

function getDefaultClkRate() {
    return uart_input_clk_freq;
}

exports = {
    getConfigArr,
    getDefaultClkRate
};
