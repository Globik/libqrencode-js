/*
*   An example of an asynchronous C++ node addon.
*   Provided by paulhauner https://github.com/paulhauner
*   License: MIT
*   Tested in node.js v4.4.2 LTS in Ubuntu Linux
*/
const addon = require('./build/Release/asyncAddon');

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
addon.runCall(5,function(er,d){console.log(d.toString());})
addon.runCall(5,function(er,d){console.log(d);})
var i=0;
var t=setInterval(()=>{i++;addon.runMakeCallback(5,function(er,d){console.log(er,d);});console.log('i:',i);if(i==202){suka();}},10)
function suka(){clearInterval(t);}