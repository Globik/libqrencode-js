const ad=require('./build/Release/qrencode-js');
const {makeMeasurable}=require('performance-meter');

const str="mama";
const buf=Buffer.from(str+'\0');
//ad.Test(buf,{type:'start'},function(err,val){console.log('error: ',err);console.log('val: ',val.toString());});

var boo=makeMeasurable(boo);
boo(1);
//setTimeout(function(){
console.log('fifa');
boo(1);
//},80);
boo(50);
function boo(n){
for(var i=0;i<n;i++){
//setTimeout(()=>{
	//ad.Test(buf,{type:'start'},function(err,val){console.log('error: ',err);console.log('val: ',val.toString('base64'));})
	ad.Test(5,{type:'start'},function(err,val){console.log('err: ',err);console.log('val: ',val);});
//},1000)
}
}
//SYNC:
//1.340ms=n1 2.787ms=n5 2.947ms=n5=base64  27,5ms=n50 without param hardcoded to "mama";
// another module qr-node.js boo=n1: 21.223ms boo=n5: 24.156ms much slower 20x?? through process.child.on_data 
// qrcode-js.js:
//boo=n1: 345.011ms boo=n5: 1117.979ms

//ASYNC:
//boo: 1.054ms=n1 boo: 0.116ms=n1 if from cache