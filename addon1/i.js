/*
*   An example of an asynchronous C++ node addon.
*   Provided by paulhauner https://github.com/paulhauner
*   License: MIT
*   Tested in node.js v4.4.2 LTS in Ubuntu Linux
*/
const addon = require('./build/Release/asyncAddon');
const {makeMeasurable}=require('performance-meter');

const str="mama";
const buf=Buffer.from(str);
//addon.runCall('5',function(er,data) {console.log('er data: ',er,data);});
//addon.runCall(6,function(er,data) {console.log('er data2: ',er,data);});
//setInterval(function(){
//addon.runCall(5,function(er,data) {console.log('er data: ',er,data);});
//},0);
//addon.doTask(function(data) {console.log('data3: ',data);});
//addon.doTask(function(data) {console.log('data4: ',data);});
//console.log('Async task started.')
//addon.runMakeCallback(10,function(er,v){console.log(er,v);})
//setInterval(()=>{addon.runCall(5,function(er,data) {console.log('er data: ',er,data);});},20)
addon.runCall(buf,function(er,d){console.log(d.toString());})
//addon.runCall(5,function(er,d){console.log(d);})
/*var i=0;
var t=setInterval(()=>{i++;addon.runMakeCallback(buf,function(er,d){console.log(er,d.toString('base64').substring(0,16));});
					   console.log('i:',i);if(i==366){suka();}},0)
function suka(){clearInterval(t);}*/
var boo=makeMeasurable(boo);
boo(1);
function boo(n){
for(var i=0;i<n;i++){
addon.runMakeCallback(buf,function(er,d){console.log(er,d.toString('base64').substring(0,16))})
}
}

// 0.712ms=n1 0.989ms=n5 2.092ms=n50 runCall method
//-d --directory --out
//node_modules/cmake-js/bin/cmake-js -d addon1/deps/libqrencode -D