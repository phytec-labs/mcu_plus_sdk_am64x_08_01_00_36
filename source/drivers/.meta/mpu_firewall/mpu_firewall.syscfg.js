
let common = system.getScript("/common");
let soc = system.getScript(`/drivers/soc/drivers_${common.getSocName()}`);

function getModule() {

    let driverVer = soc.getDriverVer("mpu_firewall");

    return system.getScript(`/drivers/mpu_firewall/${driverVer}/mpu_firewall_${driverVer}`);
}

exports = getModule();
