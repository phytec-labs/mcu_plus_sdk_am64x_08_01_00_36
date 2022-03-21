
let common = system.getScript("/common");
let soc = system.getScript(`/networking/soc/networking_${common.getSocName()}`);

exports = {
    displayName: "TI Networking",
    topModules: soc.getTopModules(),
};
