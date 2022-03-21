
let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");
let soc = system.getScript(`/drivers/pruicss/soc/pruicss_${common.getSocName()}`);

function getConfigArr() {
    return soc.getConfigArr();
}

function getInstanceConfig(moduleInstance) {
    let configArr = getConfigArr();
    let config = configArr.find(o => o.name === moduleInstance.instance);

    /* make IEP clock == core clock */
    config.clockFrequencies[0].clkRate = moduleInstance.coreClk;
    config.clockFrequencies[2].clkRate = moduleInstance.coreClk;

    return {
        ...config,
        ...moduleInstance,
    };
};

function getMdioBaseAddr(pruicssInstance)
{
    let configArr = getConfigArr();
    let config = configArr.find(o => o.name === pruicssInstance);

    return config.mdioBaseAddr;
}

function getClockEnableIds(inst) {

    let instConfig = getInstanceConfig(inst);

    return instConfig.clockIds;
}

function getClockFrequencies(inst) {

    let instConfig = getInstanceConfig(inst);

    return instConfig.clockFrequencies;
}

let pruicss_top_module_name = "/drivers/pruicss/pruicss";

let pruicss_top_module = {
    displayName: "PRU (ICSS)",

    templates: {
        "/drivers/system/system_config.c.xdt": {
            driver_config: "/drivers/pruicss/templates/pruicss_config.c.xdt",
            driver_init: "/drivers/pruicss/templates/pruicss_init.c.xdt",
            driver_deinit: "/drivers/pruicss/templates/pruicss_deinit.c.xdt",
        },
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/drivers/pruicss/templates/pruicss.h.xdt",
        },
        "/drivers/system/power_clock_config.c.xdt": {
            moduleName: pruicss_top_module_name,
        },
    },

    defaultInstanceName: "CONFIG_PRU_ICSS",
    config: [
        {
            name: "instance",
            displayName: "Instance",
            default: "ICSSG0",
            options: [
                {
                    name: "ICSSG0",
                },
                {
                    name: "ICSSG1",
                }
            ],
        },
        {
            name: "coreClk",
            displayName: "Core Clk (Hz)",
            default: 200*1000000,
            options: [
                {
                    name: 200*1000000,
                },
                {
                    name: 225*1000000,
                },
                {
                    name: 250*1000000,
                },
                {
                    name: 300*1000000,
                },
                {
                    name: 333*1000000,
                },
            ],
        }
    ],
    validate: validate,
    moduleInstances: moduleInstances,
    getInstanceConfig,
    getClockFrequencies,
    getClockEnableIds,
    getMdioBaseAddr,
};

function validate(inst, report) {
    common.validate.checkSameInstanceName(inst, report);
}

function moduleInstances(instance) {
    let device = common.getDeviceName();
    let modInstances = new Array();
    if((device === "am64x-evm") || (device === "am243x-evm"))
    {
         modInstances.push({
             name: "AdditionalICSSSettings",
             displayName: "Additional ICSS Settings",
             moduleName: '/drivers/pruicss/pruicss_gpio',
             useArray: true,
             minInstanceCount: 0,
             defaultInstanceCount: 0,
         });
    }
    return (modInstances);
}
exports = pruicss_top_module;
