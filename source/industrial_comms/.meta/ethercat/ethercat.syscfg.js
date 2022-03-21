
let common = system.getScript("/common");
let soc = system.getScript(`/industrial_comms/ethercat/ethercat_${common.getSocName()}`);

function pinmuxRequirements(inst) {
    return soc.getPinmuxRequirements(inst);
}

function getInterfaceNameList(inst) {

    return soc.getInterfaceNameList(inst);
}

function getPeripheralPinNames(inst)
{
    return soc.getPeripheralPinNames(inst);
}

function getConfigurables()
{
    /* get 'CPU enable' configurables */
    let config = [];
    let icssConfig = soc.getIcssInstancesArr();

    config.push(common.ui.makeConfig(icssConfig, "instance", "ICSS Instance"));

    return config;
}

let ethercat_module_name = "/industrial_comms/ethercat/ethercat";

let ethercat_module = {

    displayName: "EtherCAT",
    templates: {
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: ethercat_module_name,
        },
    },
    defaultInstanceName: "CONFIG_ETHERCAT",
    config: getConfigurables(),
    moduleStatic: {
        modules: function(inst) {
            return [{
                name: "system_common",
                moduleName: "/system_common",
            }]
        },
    },
    pinmuxRequirements,
    getInterfaceNameList,
    getPeripheralPinNames,
    sharedModuleInstances: sharedModuleInstances,
    moduleInstances: moduleInstances,
};

function sharedModuleInstances(instance) {
    let modInstances = new Array();
    let icssRequiredArgs = soc.getRequiredArgsIcssInstance(instance);

    modInstances.push({
        name: "icss",
        displayName: "PRU Configuration",
        moduleName: '/drivers/pruicss/pruicss',
        requiredArgs: icssRequiredArgs,
    });

    return (modInstances);
}

function moduleInstances(instance) {
    let modInstances = new Array();

    modInstances.push({
        name: "ethphy",
        displayName: "ETHPHY Configuration",
        moduleName: '/board/ethphy/ethphy',
        useArray: true,
        maxInstanceCount: 2,
        minInstanceCount: 2,
        defaultInstanceCount: 2,
        requiredArgs: {
            mdioInstance: instance.instance,
        },
    });

    return (modInstances);
}

exports = ethercat_module;
