const addon=require('./build/Release/test_async');

//STEP 1 is OK
var fl=0;
addon.Test(5,{type:'stakkkrt'},function(err,val){
	console.log('err: ',err);
	console.log('val: ',val);
	
	//addon.Test(5,{type:'start'},function(err, val){
//console.log(err);
//console.log(val);
//})
});

// STEP 2  and further CRASHES
// RACE CONDITION OR WHAT? 
//Console error report:
//undefined:0
//Error: Invalid argument

/*
addon.Test(5,{type:'start'},function(err, val){
console.log(err);
console.log(val);
})
*/var i=0;
/*
setTimeout(function(){
	i++;
	//if(fl==1)
addon.Test(5+i,{type:'start'},function(err, val){
console.log(err);
console.log(val);
})
},19);*/
setInterval(function(){
setImmediate(()=>{
	i++;
addon.Test(1+i,{type:'start'},function(err, val){
console.log(err);
console.log(val);
})
})
},4)
