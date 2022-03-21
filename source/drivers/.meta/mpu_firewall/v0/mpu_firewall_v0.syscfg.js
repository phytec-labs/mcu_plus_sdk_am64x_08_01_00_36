
let common = system.getScript("/common");

function getConfigArr() {
    return system.getScript(`/drivers/mpu_firewall/soc/mpu_firewall_${common.getSocName()}`).getConfigArr();
}

function getInstanceConfig(moduleInstance) {
    let configArr = getConfigArr();
    let config = configArr.find( o => o.name === moduleInstance.instance);

    return {
        ...config,
        ...moduleInstance,
    };
};

let mpu_firewall_module = {
    displayName: "MPU FIREWALL",
    templates: {
        "/drivers/system/system_config.c.xdt": {
            driver_config: "/drivers/mpu_firewall/templates/mpu_firewall_config.c.xdt",
            driver_init: "/drivers/mpu_firewall/templates/mpu_firewall_init.c.xdt",
        },
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/drivers/mpu_firewall/templates/mpu_firewall.h.xdt",
        },
    },
    maxInstances: getConfigArr().length,
    defaultInstanceName: "CONFIG_MPU_FIREWALL",
    config: [
        common.ui.makeInstanceConfig(getConfigArr()),
    ],
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
};

/*
 *  ======== moduleInstances ========
 */
function moduleInstances(instance) {
    let modInstances = new Array();

    modInstances.push({
        name: "mpu_region",
        displayName: "MPU Region Configuration",
        moduleName: '/drivers/mpu_firewall/v0/mpu_firewall_v0_region',
        collapsed: false,
        useArray: true,
        minInstanceCount: 0,
        maxInstanceCount: getInstanceConfig(instance).regionCount,
        defaultInstanceCount: 0,
        args: {
            startAddr: getInstanceConfig(instance).memSpace[0].startAddr,
            size: getInstanceConfig(instance).memSpace[0].size,
        }
    });

    return (modInstances);
}

function validate(instance, report) {
    common.validate.checkSameInstanceName(instance, report);
}

exports = mpu_firewall_module;
