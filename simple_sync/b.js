const ad=require('./build/Release/qrencode-js');
const {makeMeasurable}=require('performance-meter');

const str="fishki misch";
const buf=Buffer.from(str);
const b=ad.qrencodeBuffer(buf);
console.log('Some buffer: ',b);
console.log('Some buffer: ',b.toString());
//console.log('Some buffer: ',b.toString('base64'));
//console.log('Some buffer: ',b.toString('utf8'));

var boo=makeMeasurable(boo);
boo(1);
boo(5);
function boo(n){
for(var i=0;i<n;i++){
ad.qrencodeBuffer(buf).toString('base64');
}
}
//1.340ms=n1 2.787ms=n5 2.947ms=n5=base64  27,5ms=n50 without param hardcoded to "mama";
// another module qr-node.js boo=n1: 21.223ms boo=n5: 24.156ms much slower 20x?? through process.child.on_data 
// qrcode-js.js:
//boo=n1: 345.011ms boo=n5: 1117.979ms