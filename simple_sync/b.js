const ad=require('./build/Release/qrencode-js');
const {makeMeasurable}=require('performance-meter');

const str="mama";
//ctitic 82
const str2="https://example.com/a3iocn/13iunxri3cn";
const buf=Buffer.from(str2);
//const buf2=Buffer.from(str2);
const b=ad.qrencodeBuffer(buf);
console.log('Some buffer: ',b.toString('base64').substring(0,14));
console.log('********************');
//console.log('Some buffer: ',b.toString());

//const abc=ad.qrencodeBuffer(buf2);
//console.log('Some buffer: ',abc.toString());
//console.log('Some buffer: ',b.toString('utf8'));
// str2=iVBORw0KGgoAAAANSUhEUgAAAG8AAABvAQMAAADYCwwjAAAABlBMVEUAAAD///+l2Z/dAAAAAnRSTlP//8i138cAAAAJcEhZcwAA
//str=
var boo=makeMeasurable(boo);
boo(50);
//boo(5);
function boo(n){
for(var i=0;i<n;i++){
let a=ad.qrencodeBuffer(buf);//.toString();
	//console.log(a.toString());
	console.log(a.toString('base64').substring(0,30));
}
}
setInterval(function(){console.log(ad.qrencodeBuffer(buf).toString('base64').substring(0,16));},0);
//1.814ms=n1 2.787ms=n5 5.387ms=n5=base64  44,5ms=n50 without param hardcoded to "mama";
// another module qr-node.js boo=n1: 21.223ms boo=n5: 24.156ms much slower 20x?? through process.child.on_data 
// qrcode-js.js:
//boo=n1: 345.011ms boo=n5: 1117.979ms