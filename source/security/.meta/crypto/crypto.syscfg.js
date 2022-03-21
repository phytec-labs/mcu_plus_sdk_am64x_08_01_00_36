
let common = system.getScript("/common");
let soc = system.getScript(`/security/crypto/crypto_${common.getSocName()}`);

function getConfigArr() {
    return soc.getConfigArr();
}

function getInstanceConfig(moduleInstance) {
    let configArr = getConfigArr();
    let config = configArr.find(o => o.name === moduleInstance.instance);

    return {
        ...config,
        ...moduleInstance,
    };
};

let crypto_module_name = "/security/crypto/crypto";

let crypto_module = {
    displayName: "Crypto",
    templates: {
        "/drivers/system/system_config.c.xdt": {
            driver_config: "/security/crypto/templates/crypto_config.c.xdt",
            driver_init: "/security/crypto/templates/crypto_init.c.xdt",
            driver_deinit: "/security/crypto/templates/crypto_deinit.c.xdt",
        },
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/security/crypto/templates/crypto.h.xdt",
        },
    },
    maxInstances: getConfigArr().length,
    defaultInstanceName: "CONFIG_CRYPTO",
    config: [
        common.ui.makeInstanceConfig(getConfigArr()),
        {
                name: "enableSw",
                displayName: "Enable SW Crypto",
                default: true,
                description: "Enable or Disable SW Crypto for the selected Crypto Module",
        },
        {
                name: "enableHw",
                displayName: "Enable HW Crypto",
                default: false,
                description: "Enable or Disable HW Crypto for the selected Crypto Module. The actual module selected depends on the device",
        },
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

    if(instance.enableHw == true) {
        modInstances.push({
            name: "sa2ul",
            displayName: "SA2UL Instance Configuration",
            description: "SA2UL Instance to use for HW processing",
            moduleName: '/security/sa2ul/sa2ul',
            collapsed: false,
            useArray: false,
        });
    }

    return (modInstances);
}

/*
 *  ======== validate ========
 */
function validate(instance, report) {
    common.validate.checkSameInstanceName(instance, report);
    if((instance.enableSw == false) && (instance.enableHw == false)) {
        report.logError("Either one of the SW or HW modules should be selected", instance, "enableSw");
    }
}

exports = crypto_module;
