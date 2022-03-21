let common = system.getScript("/common");
let soc = system.getScript(`/drivers/gpadc/soc/gpadc_${common.getSocName()}`);

const MAXCHANNELS = 9; /* max number of GPADC Channels */

function getConfigArr() {
    return soc.getConfigArr();
}

function getInstanceConfig(moduleInstance) {
    let configArr = getConfigArr();
    let config = configArr.find( o => o.name === moduleInstance.instance);

     return {
        ...config,
        ...moduleInstance,
     };
};

function getMaxChannels(inst) {

     if(inst.readMode == "GROUP")
     {
         return MAXCHANNELS;
     }
     else if (inst.readMode == "TEMPERATURE")
     {
         return 3;
     }
     else
     {
         return 1;
     }
}
 
let gpadc_module_name = "/drivers/gpadc/gpadc";
let gpadc_module = {
    displayName: "GPADC",
    templates: {
        "/drivers/system/system_config.c.xdt": {
            driver_config: "/drivers/gpadc/templates/gpadc_config.c.xdt",
            driver_deinit: "/drivers/gpadc/templates/gpadc_deinit.c.xdt",
        },
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/drivers/gpadc/templates/gpadc.h.xdt",
        },
        "/drivers/system/drivers_open_close.c.xdt": {
            driver_open_close_config: "/drivers/gpadc/templates/gpadc_open_close_config.c.xdt",
            driver_open: "/drivers/gpadc/templates/gpadc_open.c.xdt",
            driver_close: "/drivers/gpadc/templates/gpadc_close.c.xdt",
        },
        "/drivers/system/drivers_open_close.h.xdt": {
            driver_open_close_config: "/drivers/gpadc/templates/gpadc_open_close.h.xdt",
        },
    },
    maxInstances: getConfigArr().length,
    defaultInstanceName: "CONFIG_GPADC",
    config: [
        {
            name: "readMode",
            displayName: "Channel Read Mode",
            default: "GROUP",
            options: [
                {
                    name: "SINGLE",
                    displayName: "Single Channel"
                },
                {
                    name: "GROUP",
                    displayName: "Group Channel"
                },
                {
                    name: "TEMPERATURE",
                    displayName: "Temperature Sensor"
                },
            ],
            onChange: function (inst, ui) {
                if(inst.readMode == "GROUP") {
                    ui.ChannelBitmap.hidden = false;
                }
                else {
                    ui.ChannelBitmap.hidden = true;
                }
            }
        },
        {
            name: "ChannelIndex",
            displayName: "Channel Index(1-9)",
            description: "Set Channel number between 1 to 9",
            default: "1",
        },
        {
            name: "ChannelBitmap",
            displayName: "Channel Bitmap",
            description: "Set Channel Bitmap value between 1 to 511",
            default: "1",
        },
        {
            name: "triggerSource",
            displayName: "Trigger Source",
            default: "GPADC_TRIGG_SRC_SW",
        },
        {
            name: "conversionMode",
            displayName: "Conversion Mode",
            default: "GPADC_ONESHOT_CONV_MODE",
        },
        common.ui.makeInstanceConfig(getConfigArr()),
    ],
    moduleInstances: moduleInstances,
    validate : validate,
    getInstanceConfig,
    getMaxChannels,
};

function validate(inst, report) {
        common.validate.checkNumberRange(inst, report, "ChannelIndex", 1, 9, "dec");
        common.validate.checkNumberRange(inst, report, "ChannelBitmap", 1, 511, "dec");
}

/*
 *  ======== moduleInstances ========
 */
function moduleInstances(inst) {
    let modInstances = new Array();
    
    let maxCh = getMaxChannels(inst);
    modInstances.push({
        name: "gpadcChannel",
        displayName: "GPADC Channel Configuration",
        moduleName: '/drivers/gpadc/v0/gpadc_v0_channel',
        useArray: true,
        maxInstanceCount: maxCh,
        minInstanceCount: 1,
        defaultInstanceCount: 1,
     });

    return (modInstances);
}

exports = gpadc_module;
