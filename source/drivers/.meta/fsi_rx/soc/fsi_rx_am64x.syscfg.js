
let common = system.getScript("/common");

let fsi_rx_func_clk = 500 * 1000 * 1000;

const staticConfig = [
    {
        name: "FSI_RX0",
        baseAddr: "CSL_FSIRX0_CFG_BASE",
        intrNum1: 16,
        intrNum2: 17,
        funcClk: fsi_rx_func_clk,
        clockIds: [ "TISCI_DEV_FSIRX0_FSI_RX_CK" ],
    },
    {
        name: "FSI_RX1",
        baseAddr: "CSL_FSIRX1_CFG_BASE",
        intrNum1: 18,
        intrNum2: 19,
        funcClk: fsi_rx_func_clk,
        clockIds: [ "TISCI_DEV_FSIRX1_FSI_RX_CK" ],
    },
    {
        name: "FSI_RX2",
        baseAddr: "CSL_FSIRX2_CFG_BASE",
        intrNum1: 20,
        intrNum2: 21,
        funcClk: fsi_rx_func_clk,
        clockIds: [ "TISCI_DEV_FSIRX2_FSI_RX_CK" ],
    },
    {
        name: "FSI_RX3",
        baseAddr: "CSL_FSIRX3_CFG_BASE",
        intrNum1: 22,
        intrNum2: 23,
        funcClk: fsi_rx_func_clk,
        clockIds: [ "TISCI_DEV_FSIRX3_FSI_RX_CK" ],
    },
    {
        name: "FSI_RX4",
        baseAddr: "CSL_FSIRX4_CFG_BASE",
        intrNum1: 24,
        intrNum2: 25,
        funcClk: fsi_rx_func_clk,
        clockIds: [ "TISCI_DEV_FSIRX4_FSI_RX_CK" ],
    },
    {
        name: "FSI_RX5",
        baseAddr: "CSL_FSIRX5_CFG_BASE",
        intrNum1: 26,
        intrNum2: 27,
        funcClk: fsi_rx_func_clk,
        clockIds: [ "TISCI_DEV_FSIRX5_FSI_RX_CK" ],
    },
];

function getStaticConfigArr() {
    return staticConfig;
}

function getInterfaceName(inst) {
    return "FSI_RX";
}

let soc = {
    getStaticConfigArr,
    getInterfaceName,
};

exports = soc;
