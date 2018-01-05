const addon=require('./build/Release/test_async');

//STEP 1 is OK

addon.Test(5,{type:'start'},function(err,val){
	console.log('err: ',err);
	console.log('val: ',val);
});

// STEP 2  and further CRASHES
// RACE CONDITION OR WHAT? 
//Console error report:
//undefined:0
//Error: Invalid argument


addon.Test(5,{type:'start'},function(err, val){
console.log(err);
console.log(val);
})


addon.Test(5,{type:'start'},function(err, val){
console.log(err);
console.log(val);
})

addon.Test(5,{type:'start'},function(err, val){
console.log(err);
console.log(val);
})