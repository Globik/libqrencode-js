//simple_async.cc from simple_async
const ad=require('./build/Release/qrencode-js');
const {makeMeasurable}=require('performance-meter');

const str="mama";
const buf=Buffer.from(str);
ad.Test(buf,[40,70],function(err,val){console.log('error: ',err);console.log('valA: ',val.toString());});
var k=0;
//dura();
function dura(){
var t=setInterval(function(){
ad.Test(buf,[36,3],function(err,val){
	k++;
	console.log('error: ',err);console.log('valA: ',val.toString('base64').substring(0,30));
//console.log("K: ",k);
//if(k==300){su();}
});
},0);
}
function su(){clearInterval(t);}
var boo=makeMeasurable(boo);
boo(1);

//margin,micro,8,caseinsetiv,color1,color2
function boo(n){
for(var i=0;i<n;i++){
ad.Test(buf,[36,200],function(err,val){console.log('valA: ',val.toString('base64').substring(0,16));});	
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

/*
valgrind --leak-check=yes node b.js

 definitely lost: 0 bytes in 0 blocks
==10308==    indirectly lost: 0 bytes in 0 blocks
==10308==      possibly lost: 16,478 bytes in 277 blocks
==10308==    still reachable: 370,915 bytes in 1,559 blocks
==10308==         suppressed: 0 bytes in 0 blocks
==10308== Reachable blocks (those to which a pointer was found) are not shown.
==10308== To see them, rerun with: --leak-check=full --show-leak-kinds=all
==10308== 
==10308== For counts of detected and suppressed errors, rerun with: -v
==10308== ERROR SUMMARY: 222 errors from 222 contexts (suppressed: 0 from 0)
*/