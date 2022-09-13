
const { SerialPort } = require('serialport');
const { exec } = require("child_process");
const { prototype } = require('events');
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
            player.play('positive.wav', () => { });
        } else if (parts[1] == '0') {
            // player.play('negative.wav', () => {});
        }
    }
    if (line.includes("Count down")) {
        player.play('countdown.wav', () => {
            player.play('ticking.wav', () => { }); });
    }
    if (line.includes("Tool ")) {
        player.play('switch.wav', () => { });
    }
    if (line.includes("Last 17 seconds")) {
        player.play('ticking.wav', () => { });
    }
    if (line.includes("End game")) {
        player.play('end.wav', () => { });
    }
});

var stdin = process.openStdin();

stdin.addListener("data", function (d) {
    // note:  d is an object, and when converted to a string it will
    // end with a linefeed.  so we (rather crudely) account for that  
    // with toString() and then trim() 
    console.log("you entered: [" +
        d.toString().trim() + "]");
    var val = d.toString().trim();

    if(val == "s") {
        port.write("5");
    }else {

        port.write(val);
    }

});