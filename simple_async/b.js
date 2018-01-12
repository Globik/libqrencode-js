const ad=require('./build/Release/qrencode-js');
const {makeMeasurable}=require('performance-meter');

const str="mama";
const buf=Buffer.from(str);
ad.Test(buf,{type:'start'},function(err,val){console.log('error: ',err);console.log('valA: ',val.toString());});
var k=0;
//dura();
function dura(){
var t=setInterval(function(){
ad.Test(buf,{type:'start'},function(err,val){
	k++;
	console.log('error: ',err);console.log('valA: ',val.toString());
console.log("K: ",k);
if(k==300){su();}
});
},0);
}
function su(){clearInterval(t);}
var boo=makeMeasurable(boo);
boo(100);
var dam=["mama","papa","sister"];

function boo(n){
for(var i=0;i<n;i++){
ad.Test(buf,{type:'start'},function(err,val){console.log('valA: ',val.toString('base64').substring(0,16));});	
}
}
var e=0;


// SYNC:  1.540ms=n1 4.318ms=n5  35.373ms=n50            //using synchronous napi and napi create_buffer_copy folder=simple_sync
// ASYNC: 0.788=n1   1.030ms=n5  2.209ms=n=50            //napi_create_async_work and napi create_buffer_copy folder=simple_async
// ASYNC: 0.704      1.070       9.208        2.443=n100 // using C++ uv_queue_work and new Buffer  folder=addon1
// another module qr-node.js boo=n1: 21.223ms boo=n5: 24.156ms much slower 20x?? through process.child.on_data 
// qrcode-js.js:
//boo=n1: 345.011ms boo=n5: 1117.979ms

//ASYNC:               0.98
//boo: 1.054ms=n1 boo: 0.116ms=n1 if from cache