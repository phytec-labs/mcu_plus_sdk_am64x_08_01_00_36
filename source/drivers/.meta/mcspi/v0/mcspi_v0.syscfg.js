
let common = system.getScript("/common");
let hwi = system.getScript("/kernel/dpl/hwi.js");
let pinmux = system.getScript("/drivers/pinmux/pinmux");
let soc = system.getScript(`/drivers/mcspi/soc/mcspi_${common.getSocName()}`);

function getConfigArr() {
    return system.getScript(`/drivers/mcspi/soc/mcspi_${common.getSocName()}`).getConfigArr();
}

function getInstanceConfig(moduleInstance) {
    let solution = moduleInstance[getInterfaceName(moduleInstance)].$solution;
    let configArr = getConfigArr();
    let config = configArr.find(o => o.name === solution.peripheralName);

    return {
        ...config,
        ...moduleInstance,
    };
};

function getMaxChannels(inst) {
    if((inst.mode == "SINGLE_MASTER") || (inst.mode == "SLAVE")) {
        return 1;
    }
    else {
        return soc.getMaxChannels(inst);
    }
}

function pinmuxRequirements(inst) {

    let interfaceName = getInterfaceName(inst);

    let misoRequired = false;
    let mosiRequired = false;
    let txRequired = true;
    let rxRequired = true;

    switch (inst.trMode) {
        case "TX_RX":
            misoRequired = true;
            mosiRequired = true;
            break;
        case "TX_ONLY":
            if(inst.mode == "SLAVE") {
                misoRequired = true;
                mosiRequired = false;
            }
            else {
                misoRequired = false;
                mosiRequired = true;
            }
            rxRequired = false;
            break;
        case "RX_ONLY":
            if(inst.mode == "SLAVE") {
                misoRequired = false;
                mosiRequired = true;
            }
            else {
                misoRequired = true;
                mosiRequired = false;
            }
            txRequired = false;
            break;
    }

    let resources = [];
    resources.push( pinmux.getPinRequirements(interfaceName, "CLK", "SPI Clock Pin") );
    resources.push( pinmux.getPinRequirements(interfaceName, "D0", "SPI D0 Pin") );
    resources.push( pinmux.getPinRequirements(interfaceName, "D1", "SPI D1 Pin") );

    let spi = {
        name: interfaceName,
        displayName: "SPI Instance",
        interfaceName: interfaceName,
        resources: resources,
    };

    return [spi];
}

function getInterfaceName(inst) {

    if(inst.useMcuDomainPeripherals)
        return "MCU_SPI"

    return "SPI";
}

function getPeripheralPinNames(inst) {

    return [ "CLK", "D0", "D1", "CS0", "CS1", "CS2", "CS3", "CSn" ];
}

function getClockEnableIds(inst) {

    let instConfig = getInstanceConfig(inst);

    return instConfig.clockIds;
}

let mcspi_module_name = "/drivers/mcspi/mcspi";

let mcspi_module = {
    displayName: "MCSPI",

    templates: {
        "/drivers/system/system_config.c.xdt": {
            driver_config: "/drivers/mcspi/templates/mcspi_config.c.xdt",
            driver_init: "/drivers/mcspi/templates/mcspi_init.c.xdt",
            driver_deinit: "/drivers/mcspi/templates/mcspi_deinit.c.xdt",
        },
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/drivers/mcspi/templates/mcspi.h.xdt",
        },
        "/drivers/system/drivers_open_close.c.xdt": {
            driver_open_close_config: "/drivers/mcspi/templates/mcspi_open_close_config.c.xdt",
            driver_open: "/drivers/mcspi/templates/mcspi_open.c.xdt",
            driver_close: "/drivers/mcspi/templates/mcspi_close.c.xdt",
        },
        "/drivers/system/drivers_open_close.h.xdt": {
            driver_open_close_config: "/drivers/mcspi/templates/mcspi_open_close.h.xdt",
        },
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: mcspi_module_name,
        },
        "/drivers/system/power_clock_config.c.xdt": {
            moduleName: mcspi_module_name,
        },
    },
    defaultInstanceName: "CONFIG_MCSPI",
    config: getConfigurables(),
    validate: validate,
    moduleInstances: moduleInstances,
    moduleStatic: {
        modules: function(inst) {
            return [{
                name: "system_common",
                moduleName: "/system_common",
            }]
        },
    },
    getInstanceConfig,
    pinmuxRequirements,
    getMaxChannels,
    getInterfaceName,
    getPeripheralPinNames,
    getClockEnableIds,
};

function getConfigurables()
{
    let config = [];

    config.push(
        {
            name: "mode",
            displayName: "Mode of Operation",
            default: "SINGLE_MASTER",
            options: [
                {
                    name: "SINGLE_MASTER",
                    displayName: "Single Master"
                },
                {
                    name: "MULTI_MASTER",
                    displayName: "Multi Master"
                },
                {
                    name: "SLAVE",
                    displayName: "Single Slave"
                },
            ],
            description: "Master/Slave or Single/Multi channel mode of operation",
            onChange: function (inst, ui) {
                /* Init delay applicable only for single master mode */
                if((inst.advanced == true) &&
                   (inst.mode == "SINGLE_MASTER")) {
                    ui.initDelay.hidden = false;
                }
                else {
                    ui.initDelay.hidden = true;
                }
                /* 3/4 pin mode applicable only in single channel mode */
                if(inst.mode == "SINGLE_MASTER") {
                    ui.pinMode.hidden = false;
                }
                else {
                    ui.pinMode.hidden = true;
                }
            },
        },
        {
            name: "pinMode",
            displayName: "Pin Mode",
            default: 4,
            options: [
                {
                    name: 3,
                    displayName: "3 Pin Mode"
                },
                {
                    name: 4,
                    displayName: "4 Pin Mode"
                },
            ],
            description: "3 pin mode: Chip-select (CS) is not used and all related options to CS have no meaning. 4 pin mode: CS is used. This is selectable only in single channel mode",
        },
        {
            name: "trMode",
            displayName: "TR Mode",
            default: "TX_RX",
            options: [
                {
                    name: "TX_RX",
                    displayName: "TX and RX"
                },
                {
                    name: "RX_ONLY",
                    displayName: "RX Only"
                },
                {
                    name: "TX_ONLY",
                    displayName: "TX Only"
                },
            ],
            description: "Channel transmit/receive mode",
        },
        {
            name: "inputSelect",
            displayName: "Input Select",
            default: "1",
            options: [
                {
                    name: "0",
                    displayName: "D0"
                },
                {
                    name: "1",
                    displayName: "D1"
                },
            ],
            description: "Input selected on D0 or D1 line",
        },
        {
            name: "dpe0",
            displayName: "D0 TX Enable",
            default: "ENABLE",
            options: [
                {
                    name: "ENABLE",
                    displayName: "TX ENABLED"
                },
                {
                    name: "DISABLE",
                    displayName: "TX DISABLED"
                },
            ],
            description: "Transmission enable/disable for D0",
        },
        {
            name: "dpe1",
            displayName: "D1 TX Enable",
            default: "DISABLE",
            options: [
                {
                    name: "ENABLE",
                    displayName: "TX ENABLED"
                },
                {
                    name: "DISABLE",
                    displayName: "TX DISABLED"
                },
            ],
            description: "Transmission enable/disable for D1",
        },
        {
            name: "intrEnable",
            displayName: "Interrupt Mode Enable",
            default: true,
            description: "Interrupt Mode",
            onChange: function (inst, ui) {
                if(inst.intrEnable == true) {
                    ui.intrPriority.hidden = false;
                }
                else {
                    ui.intrPriority.hidden = true;
                }
                if(inst.intrEnable == true) {
                    ui.transferMode.hidden = false;
                    ui.transferTimeout.hidden = false;
                }
                else {
                    ui.transferMode.hidden = true;
                    ui.transferTimeout.hidden = true;
                }
            },
        },
        {
            name: "intrPriority",
            displayName: "Interrupt Priority",
            default: 4,
            hidden: false,
            description: `Interrupt Priority: 0 (highest) to ${hwi.getHwiMaxPriority()} (lowest)`,
        },
        {
            name: "transferMode",
            displayName: "Transfer Mode",
            default: "BLOCKING",
            hidden: false,
            options: [
                {
                    name: "BLOCKING",
                    displayName: "Blocking"
                },
                {
                    name: "CALLBACK",
                    displayName: "Callback"
                },
            ],
            onChange: function (inst, ui) {
                if(inst.transferMode == "CALLBACK") {
                    ui.transferCallbackFxn.hidden = false;
                    if(inst.transferCallbackFxn == "NULL") {
                        /* Clear NULL entry as user need to provide a fxn */
                        inst.transferCallbackFxn = "";
                    }
                }
                else {
                    ui.transferCallbackFxn.hidden = true;
                    inst.transferCallbackFxn = "NULL";
                }
            },
            description: "This determines whether the driver operates synchronously or asynchronously",
        },
        {
            name: "transferCallbackFxn",
            displayName: "Transfer Callback",
            default: "NULL",
            hidden: true,
            description: "Transfer callback function when callback mode is selected",
        },
        {
            name: "transferTimeout",
            displayName: "Transfer Timeout",
            default: 0xFFFFFFFF,
            hidden: false,
            description: "Transfer timeout in system ticks. Provide 0xFFFFFFFF to wait forever",
            displayFormat: "hex",
        },
        /* Advanced parameters */
        {
            name: "advanced",
            displayName: "Show Advanced Config",
            default: false,
            onChange: function (inst, ui) {
                /* Init delay applicable only for single master mode */
                if((inst.advanced == true) &&
                   (inst.mode == "SINGLE_MASTER")) {
                    ui.initDelay.hidden = false;
                }
                else {
                    ui.initDelay.hidden = true;
                }
            },
        },
        /* Advance Instance attributes */
        {
            name: "initDelay",
            displayName: "Init Delay",
            default: "0",
            hidden: true,
            options: [
                {
                    name: "0",
                },
                {
                    name: "4",
                },
                {
                    name: "8",
                },
                {
                    name: "16",
                },
                {
                    name: "32",
                },
            ],
            description: "Initial delay for first transfer in bus clock cycles. Applicable only for single master mode",
        },
    )

    if(common.isMcuDomainSupported())
    {
        config.push(common.getUseMcuDomainPeripheralsConfig());
    }

    return config;
}
/*
 *  ======== validate ========
 */
function validate(inst, report) {
    common.validate.checkNumberRange(inst, report, "transferTimeout", 0x0, 0xFFFFFFFF, "hex");
    common.validate.checkValidCName(inst, report, "transferCallbackFxn");
    if((inst.transferMode == "CALLBACK") &&
        ((inst.transferCallbackFxn == "NULL") ||
            (inst.transferCallbackFxn == ""))) {
        report.logError("Callback function MUST be provided for callback transfer mode", inst, "transferCallbackFxn");
    }
    common.validate.checkNumberRange(inst, report, "intrPriority", 0, hwi.getHwiMaxPriority(), "dec");
}

/*
 *  ======== moduleInstances ========
 */
function moduleInstances(inst) {
    let modInstances = new Array();

    let maxCh = getMaxChannels(inst);
    modInstances.push({
        name: "mcspiChannel",
        displayName: "MCSPI Channel Configuration",
        moduleName: '/drivers/mcspi/v0/mcspi_v0_channel',
        useArray: true,
        maxInstanceCount: maxCh,
        minInstanceCount: 1,
        defaultInstanceCount: 1,
        args: {
            interfaceName: getInterfaceName(inst),
        },
    });

    return (modInstances);
}

function getClockFrequencies(inst) {

    let instConfig = getInstanceConfig(inst);

    return instConfig.clockFrequencies;
}

function getModule()
{
    let module = mcspi_module;
    if(soc.isFrequencyDefined())
    {
        module.getClockFrequencies = getClockFrequencies;
    }
    return module;
}

exports = getModule();
