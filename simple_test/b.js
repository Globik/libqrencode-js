const addon=require('./build/Release/test_async');
addon.Test(15,{type:'start'},function(err,val){
	console.log('err: ',err);
	console.log('val: ',val);
});

addon.Test(2,{type:'start'},function(err, val){
console.log('err: ',err);
console.log('val: ',val);
})

var i=0;
var t=setInterval(function(){
	i++;
addon.Test(4,{type:'start'},function(err, val){
console.log('err: ',err);
console.log('val: ',val);
console.log('I: ',i);
if(i==300){clear();}
})
},0)
function clear(){clearInterval(t);}
// I got it! Thank you to all.