const ad=require('./build/Release/qrencode-js');
const {makeMeasurable}=require('performance-meter');

const str="mama";
const buf=Buffer.from(str+'\0');
ad.Test(buf,{type:'start'},function(err,val){console.log('error: ',err);console.log('valA: ',val.toString('utf8'));});

var boo=makeMeasurable(boo);

//setTimeout(function(){
console.log('fifa');
//boo();
//},80);
//boo();
var dam=["mama","papa","sister"];
function m(){
dam.forEach((el,i)=>{console.log(el);})
console.log('array:',dam);
}
//m();
function boo(dam){
//for(var i=0;i<n;i++){
	console.log(dam);
	dam.forEach(function(el,i){
	ad.Test(Buffer.from(el+'\0'),{type:'start'},function(err,val){console.log('error: ',err);console.log('val: ',val.toString('base64'));})

})
}
var e=0;
/*
setInterval(function(){
ad.Test(Buffer.from(buf),{type:'start'},function(err,val){console.log('error: ',err);console.log('val: ',val.toString('base64'));})
e++;
	console.log('e: ',e)
},0)
*/
//boo(dam);
//SYNC:
//1.340ms=n1 2.787ms=n5 2.947ms=n5=base64  27,5ms=n50 without param hardcoded to "mama";
// another module qr-node.js boo=n1: 21.223ms boo=n5: 24.156ms much slower 20x?? through process.child.on_data 
// qrcode-js.js:
//boo=n1: 345.011ms boo=n5: 1117.979ms

//ASYNC:               0.98
//boo: 1.054ms=n1 boo: 0.116ms=n1 if from cache