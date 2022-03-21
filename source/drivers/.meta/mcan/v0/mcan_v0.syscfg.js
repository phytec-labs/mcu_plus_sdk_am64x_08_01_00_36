
let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");
let soc = system.getScript(`/drivers/mcan/soc/mcan_${common.getSocName()}`);

function getConfigArr() {
    return system.getScript(`/drivers/mcan/soc/mcan_${common.getSocName()}`).getConfigArr();
}

function getInstanceConfig(moduleInstance) {
    let solution = moduleInstance[getInterfaceName(moduleInstance)].$solution;
    let configArr = getConfigArr();
    let config = configArr.find( o => o.name === solution.peripheralName);

    return {
        ...config,
        ...moduleInstance
    }
}

function getPeripheralPinNames(inst) {
    return [ "RX", "TX" ];
}

function pinmuxRequirements(inst) {
   let interfaceName = getInterfaceName(inst);

    let resources = [];
    let pinResource = {};

    pinResource = pinmux.getPinRequirements(interfaceName, "RX", "MCAN RX Pin");
    pinmux.setConfigurableDefault( pinResource, "rx", true );
    resources.push( pinResource);
    pinResource = pinmux.getPinRequirements(interfaceName, "TX", "MCAN TX Pin");
    pinmux.setConfigurableDefault( pinResource, "rx", false );
    resources.push( pinResource);

    let peripheral = {
        name: interfaceName,
        displayName: "MCAN Instance",
        interfaceName: interfaceName,
        resources: resources,
    };

    return [peripheral];
}

function getInterfaceName(instance) {
    return soc.getInterfaceName(instance);
}

function getClockEnableIds(instance) {
    let instConfig = getInstanceConfig(instance);
    return instConfig.clockIds;
}

function getClockFrequencies(inst) {

    let instConfig = getInstanceConfig(inst);

    return instConfig.clockFrequencies;
}

function validate(instance, report) {
    /* None. Verified by SYSCFG based on selected pin */
}

let mcan_module_name = "/drivers/mcan/mcan";

let mcan_module = {
    displayName: "MCAN",
    templates: {
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/drivers/mcan/templates/mcan.h.xdt",
            moduleName: mcan_module_name,
        },
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: mcan_module_name,
        },
        "/drivers/system/power_clock_config.c.xdt": {
            moduleName: mcan_module_name,
        },
    },
    defaultInstanceName: "CONFIG_MCAN",
    validate: validate,
    modules: function(instance) {
        return [{
            name: "system_common",
            moduleName: "/system_common",
        }]
    },
    getInstanceConfig,
    pinmuxRequirements,
    getInterfaceName,
    getPeripheralPinNames,
    getClockEnableIds,
    getClockFrequencies,
};

exports = mcan_module;
