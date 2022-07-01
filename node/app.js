
const { SerialPort } = require('serialport');
const { exec } = require("child_process");
var createInterface = require('readline').createInterface;

var player = require('play-sound')(opts = {})

var port = new SerialPort({
    path: "COM13",
    baudRate: 115200
});

var lineReader = createInterface({
    input: port
});



lineReader.on('line', function (line) {
    console.log(line);
    if (line.includes("Color ")) {
        var parts = line.split(": ");
        console.log(parts);
        if (parts[1] == '1') {
            player.play('positive.wav', () => {});
        }else {
            
            player.play('negative.wav', () => {});
        }
    }
});
