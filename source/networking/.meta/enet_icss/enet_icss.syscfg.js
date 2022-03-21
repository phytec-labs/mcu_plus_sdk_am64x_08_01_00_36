
let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");

function getInterfaceName(inst, peripheralName)
{
    return `PRU_${inst.instance}_${peripheralName}`;
}

function getInterfacePinList(inst, peripheralName)
{
    let interfaceName = getInterfaceName(inst, peripheralName);
    let pinList = [];

    if(peripheralName=="IEP")
    {
        pinList.push("EDC_LATCH_IN0");
        pinList.push("EDC_SYNC_OUT0");
    }
    else
    {
        pinList = pinmux.getInterfacePinList(interfaceName);
    }

    return pinList;
}


function getPeripheralRequirements(inst, peripheralName, name)
{
    let interfaceName = getInterfaceName(inst, peripheralName);
    let pinList = getInterfacePinList(inst, peripheralName);
    let resources = [];

    if(name == undefined)
    {
        name = interfaceName;
    }
    else
    {
        name = getInterfaceName(inst, name);
    }

    for(let pin of pinList)
    {
        let pinResource = pinmux.getPinRequirements(interfaceName, pin);

        /* make all pins as "rx" and then override to make "rx" as false as needed  */
        pinmux.setConfigurableDefault( pinResource, "rx", true );

        resources.push( pinResource );
    }

    let peripheralRequirements = {
        name: name,
        displayName: name,
        interfaceName: interfaceName,
        resources: resources,
    };

    return peripheralRequirements;
}

function pinmuxRequirements(inst) {

    let mdio = getPeripheralRequirements(inst, "MDIO");
    let iep = getPeripheralRequirements(inst, "IEP");

    /* set default values for "rx" for different pins, based on use case */
    pinmux.setPeripheralPinConfigurableDefault( mdio, "MDC", "rx", false);
    pinmux.setPeripheralPinConfigurableDefault( iep, "EDC_SYNC_OUT0", "rx", false);

    if( inst.phyToMacInterfaceMode === "MII")
    {
        let mii_g_rt = getPeripheralRequirements(inst, "MII_G_RT");

        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD0", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD1", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD2", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD3", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXEN", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD0", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD1", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD2", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD3", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXEN", "rx", false);

        return [mdio, iep, mii_g_rt];
    }
    else
    {
        let rgmii1 = getPeripheralRequirements(inst, "RGMII", "RGMII1");
        let rgmii2 = getPeripheralRequirements(inst, "RGMII", "RGMII2");

        return [mdio, iep, rgmii1, rgmii2];
    }
}



function getInterfaceNameList(inst) {

    if(inst.phyToMacInterfaceMode === "MII")
    {
        return [
            getInterfaceName(inst, "MDIO"),
            getInterfaceName(inst, "IEP"),
            getInterfaceName(inst, "MII_G_RT" ),
        ];
    }
    else
    {
        return [
            getInterfaceName(inst, "MDIO"),
            getInterfaceName(inst, "IEP"),
            getInterfaceName(inst, "RGMII1" ),
            getInterfaceName(inst, "RGMII2" ),
        ];

    }
}

function getPeripheralPinNames(inst)
{
    let pinList = [];

    if(inst.phyToMacInterfaceMode === "MII")
    {
        pinList = pinList.concat( getInterfacePinList(inst, "MDIO"),
                        getInterfacePinList(inst, "IEP"),
                        getInterfacePinList(inst, "MII_G_RT" )
        );
    }
    else
    {
        pinList = pinList.concat( getInterfacePinList(inst, "MDIO"),
                        getInterfacePinList(inst, "IEP"),
                        getInterfacePinList(inst, "RGMII" )
        );

    }
    return pinList;
}

let enet_icss_module_name = "/networking/enet_icss/enet_icss";

let enet_icss_module = {

    displayName: "Enet (ICSS)",
    templates: {
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: enet_icss_module_name,
        },
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/networking/enet_icss/templates/enet_icss.h.xdt",
            moduleName: enet_icss_module_name,
        },
    },
    defaultInstanceName: "CONFIG_ENET_ICSS",
    config: [
        {
            name: "instance",
            displayName: "Instance",
            default: "ICSSG1",
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
            name: "mode",
            displayName: "EMAC Mode",
            default: "SWITCH",
            options: [
                {
                    name: "SWITCH",
                },
                {
                    name: "DUAL MAC",
                },

            ],
        },
        {
            name: "phyToMacInterfaceMode",
            displayName: "MII/RGMII",
            default: "MII",
            options: [
                {
                    name: "MII",
                },
                {
                    name: "RGMII",
                },
            ],
        },
        {
            name: "phyAddr1",
            description: "Phy Address of the port in single/dual EMAC mode or Port 1 in Switch mode. Value MUST be between 0 .. 31",
            displayName: "Phy Address 1",
            default: 15,
        },
        {
            name: "phyAddr2",
            description: "Phy Address of the port in single/dual EMAC mode or Port 2 in Switch mode. Value MUST be between 0 .. 31",
            displayName: "Phy Address 2",
            default: 3,
        },

    ],
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
};

function sharedModuleInstances(instance) {
    let modInstances = new Array();

    modInstances.push({
        name: "icss",
        displayName: "PRU Configuration",
        moduleName: '/drivers/pruicss/pruicss',
        requiredArgs: {
            instance: instance.instance,
            coreClk: 250*1000000,
        },
    });

    return (modInstances);
}

exports = enet_icss_module;
