
let common = system.getScript("/common");

let crypto_instances = [
    {
        name: "SHA",
    },
    {
        name: "AES",
    },
];

function getConfigArr() {
    return crypto_instances;
}

exports = {
    getConfigArr,
};
