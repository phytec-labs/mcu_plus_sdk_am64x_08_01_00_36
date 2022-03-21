
let common = system.getScript("/common");
let device_peripheral = system.getScript(`/drivers/mpu_firewall/soc/mpu_firewall_${common.getSocName()}`);

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

let id_list = device_peripheral.getAidList();
let default_id_list = device_peripheral.getdefaultAidList();

let permission_list = [
	{ name: "PER0", displayName:"Supervisor Read" },
	{ name: "PER1", displayName:"Supervisor Write" },
    { name: "PER2", displayName:"Supervisor Execute" },
	{ name: "PER3", displayName:"User Read" },
    { name: "PER4", displayName:"User Write" },
	{ name: "PER5", displayName:"User Execute" },
    { name: "PER6", displayName:"Non Secure Access" },
    { name: "PER7", displayName:"Emulation" },
]

let mpu_firewall_region_module = {
    displayName: "MPU Region Configuration",
    longDescription: `This adds and configures a MPU Region.`,
    defaultInstanceName: "CONFIG_MPU_FIREWALL_REGION",
    config: [
        {
            name: "startAddr",
            displayName: "Region Start Address (HEX)",
            description: "MUST be <= 32 bits and within firewall space",
            default: 0x0,
            displayFormat: "hex",
        },
        {
            name: "size",
            displayName: "Region Size (HEX)",
            description: "The End address(Start Address + Size - 1) must not exceed firewall space.",
            default: 0x0,
            displayFormat: "hex",
        },
        {
            name: "allaidConfig",
            displayName: "Enable All ID's",
            default: false,
            onChange: function (inst, ui) {
                if(inst.allaidConfig == true) {
                    var idArray = [];
                    for (let i = 0; i < id_list.length; i++) {
                        idArray[i] = id_list[i].name;
                    }
                    inst.aidConfig = idArray;
                    ui.aidConfig.readOnly = true;
                }
                else
                {
                    ui.aidConfig.readOnly = false;
                }
            },
        },
        {
            name: "aidConfig",
            displayName: "Priv ID's Allowed",
            description: "Select ID's which can access the region",
            hidden      : false,
            readOnly    : false,
            default     : default_id_list,
            minSelections: 0,
            options     : id_list
        },
        {
            name: "allPermissions",
            displayName: "Enable All Permissions",
            default: false,
            onChange: function (inst, ui) {
                if(inst.allPermissions == true) {
                    var permArray = [];
                    for (let i = 0; i < permission_list.length; i++) {
                        permArray[i] = permission_list[i].name;
                    }
                    inst.permissionConfig = permArray;
                    ui.permissionConfig.readOnly = true;
                }
                else{
                    ui.permissionConfig.readOnly = false;
                }
            },
        },
        {
            name: "permissionConfig",
            displayName: "Permission Config",
            description: "Select access permissions for the region",
            hidden      : false,
            readOnly    : false,
            default     : ["PER6", "PER7"],
            minSelections: 0,
            options     : permission_list
        },
    ],
    validate : function (instance, report) {
        let parent = instance.$ownedBy;
        let instConfig = getInstanceConfig(parent);
        let startAddrErrFlag = true;
        let sizeErrFlag = true;
        for( let i = 0; i < instConfig.memSpace.length; i++)
        {
            let regionStartAddr = instConfig.memSpace[i].startAddr;
            let regionEndAddr = instConfig.memSpace[i].startAddr + instConfig.memSpace[i].size - 1;
            let currentEndAddr = instance.startAddr + instance.size - 1;
            if ( (instance.startAddr >= regionStartAddr) && (instance.startAddr <= regionEndAddr))
            {
                startAddrErrFlag = false;
                if (currentEndAddr <= regionEndAddr)
                {
                    sizeErrFlag = false;
                }
            }
        }
        if(startAddrErrFlag == true)
        {
            report.logError( `Region Start address must be within firewall space`, instance, "startAddr");
        }
        if(sizeErrFlag == true)
        {
            report.logError( `Region End address must be within firewall space`, instance, "size");
        }
        if( instance.size == 0)
        {
            report.logError( `Region size must be > 0`, instance, "size");
        }
    },
    getInstanceConfig,
    permission_list,
};

exports = mpu_firewall_region_module;
