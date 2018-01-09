/*
*   An example of an asynchronous C++ node addon.
*   Provided by paulhauner https://github.com/paulhauner
*   License: MIT
*   Tested in node.js v4.4.2 LTS in Ubuntu Linux
*/
const addon = require('./build/Release/asyncAddon');

addon.doTask(function(data) {console.log('data1: ',data.toString());});
addon.doTask(function(data) {console.log('data2: ',data.toString());});
//addon.doTask(function(data) {console.log('data3: ',data);});
//addon.doTask(function(data) {console.log('data4: ',data);});
console.log('Async task started.')
var i=0;
setInterval(function(){
setImmediate(function(){
i++;addon.doTask(function(data) {console.log('data2: ',data.toString());console.log('i',i)});console.log('i: ',i);
});
},1000)